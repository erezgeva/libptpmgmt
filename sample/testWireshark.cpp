/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Generate PTPv2 packets with Management TLVs
 *
 * The packets can be used to verify proper handling by wireshark
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2025 Erez Geva
 *
 * @note compile : g++ -Wall testWireshark.cpp -o testWireshark -lptpmgmt
 *
 */

#include <stdio.h>
#include <ptpmgmt/msg.h>
#include <ptpmgmt/proc.h>
#include <ptpmgmt/sock.h>
#include <ptpmgmt/err.h>

using namespace ptpmgmt;

SockIp4 sk;
Message msg;
uint8_t buf[400];
const Binary ip("\x12\x34\x56\x78", 4);
const Binary physicalAddress = Binary("\xc4\x7d\x46\x20\xac\xae", 6);
const ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
const PortAddress_t portAddress0 = { IEEE_802_3, 6, physicalAddress };
const PortAddress_t portAddress1 = { UDP_IPv4, 4, ip };
const PortIdentity_t a0 = { clockId, 1 };
const PortIdentity_t a1 = { { 9, 8, 7, 6, 5, 4, 1, 7}, 2 };
const PortIdentity_t a2 = { { 19, 28, 77, 6, 15, 24, 21, 7}, 7 };

/* Create PTPv2 packet using a TLV object
 * For TLVs that support set */
void buidSend(mng_vals_e id, const BaseMngTlv *dataSend)
{
    if(!msg.setAction(SET, id, dataSend) ||
        msg.build(buf, sizeof buf, 1) != MNG_PARSE_ERROR_OK ||
        !sk.send(buf, msg.getMsgLen()))
        printf("build %s\n", Error::getError().c_str());
}
void USER_DESCRIPTION_f()
{
    USER_DESCRIPTION_t t;
    t.userDescription.textField = "test123";
    buidSend(USER_DESCRIPTION, &t);
}
void MASTER_ONLY_f()
{
    MASTER_ONLY_t t;
    t.flags = 1;
    buidSend(MASTER_ONLY, &t);
}
void GRANDMASTER_CLUSTER_TABLE_f()
{
    GRANDMASTER_CLUSTER_TABLE_t t;
    t.logQueryInterval = -19;
    t.PortAddress.push_back(portAddress0);
    t.PortAddress.push_back(portAddress1);
    buidSend(GRANDMASTER_CLUSTER_TABLE, &t);
}
void UNICAST_MASTER_TABLE_f()
{
    UNICAST_MASTER_TABLE_t t;
    t.logQueryInterval = -19;
    t.PortAddress.push_back(portAddress0);
    t.PortAddress.push_back(portAddress1);
    buidSend(UNICAST_MASTER_TABLE, &t);
}
void ACCEPTABLE_MASTER_TABLE_f()
{
    ACCEPTABLE_MASTER_TABLE_t t;
    t.list.push_back({a0, 127});
    t.list.push_back({a1, 111});
    buidSend(ACCEPTABLE_MASTER_TABLE, &t);
    /* The second one have a pad */
    t.list.push_back({a2, 65});
    buidSend(ACCEPTABLE_MASTER_TABLE, &t);
}
void ACCEPTABLE_MASTER_TABLE_ENABLED_f()
{
    ACCEPTABLE_MASTER_TABLE_ENABLED_t t;
    t.flags = 1;
    buidSend(ACCEPTABLE_MASTER_TABLE_ENABLED, &t);
}
void EXTERNAL_PORT_CONFIGURATION_ENABLED_f()
{
    EXTERNAL_PORT_CONFIGURATION_ENABLED_t t;
    t.flags = 1;
    buidSend(EXTERNAL_PORT_CONFIGURATION_ENABLED, &t);
}
void HOLDOVER_UPGRADE_ENABLE_f()
{
    HOLDOVER_UPGRADE_ENABLE_t t;
    t.flags = 1;
    buidSend(HOLDOVER_UPGRADE_ENABLE, &t);
}
void EXT_PORT_CONFIG_PORT_DATA_SET_f()
{
    EXT_PORT_CONFIG_PORT_DATA_SET_t t;
    t.flags = 1;
    t.desiredState = PASSIVE;
    buidSend(EXT_PORT_CONFIG_PORT_DATA_SET, &t);
}

#include <endian.h>
const size_t tlvLoc = 54;
inline uint16_t cpu_to_net16(uint16_t value) {return htobe16(value);}
/* Create PTPv2 packet from a TLV data
 * We use it when a TLV do not support a set operation */
void rsp(uint16_t mngId, uint8_t *tlv, size_t tlvLength)
{
    size_t len = tlvLoc + tlvLength;
    // header.messageLength
    buf[2] = len >> 8;
    buf[3] = len & 0xff;
    buf[46] = RESPONSE;
    uint16_t *ptr16 = (uint16_t *)(buf + 48);
    *ptr16++ = cpu_to_net16(MANAGEMENT);     // 48
    *ptr16++ = cpu_to_net16(tlvLength + 2);  // 50
    *ptr16++ = cpu_to_net16(mngId);          // 52
    memcpy(ptr16, tlv, tlvLength);           // 54
    if(!sk.send(buf, len))
        printf("rsp %s\n", Error::getError().c_str());
}
/* A base PTPv2 packet we add a TLV on top */
bool base()
{
    if(msg.setAction(GET, NULL_PTP_MANAGEMENT) &&
        msg.build(buf, sizeof buf, 1) == MNG_PARSE_ERROR_OK)
        return true;
    printf("base %s\n", Error::getError().c_str());
    return false;
}
void UNICAST_MASTER_MAX_TABLE_SIZE_f()
{
    if(!base())
        return;
    uint8_t m[] = {33, 67};
    rsp(0x2019, m, sizeof m);
}
void ACCEPTABLE_MASTER_MAX_TABLE_SIZE_f()
{
    if(!base())
        return;
    uint8_t m[] = {103, 186};
    rsp(0x201c, m, sizeof m);
}
void PATH_TRACE_LIST_f()
{
    if(!base())
        return;
    uint8_t m[18] = {0xc4, 0x7d, 0x46, 0xff, 0xfe, 0x20, 0xac, 0xae,
            12, 4, 19, 97, 11, 74, 12, 74
        };
    rsp(0x2015, m, sizeof m);
}

void CLOCK_DESCRIPTION_f()
{
    if(!base())
        return;
    uint8_t m[58] = {128, 0, 10, 73, 69, 69, 69, 32, 56, 48, 50, 46, 51, 0, 6,
            196, 125, 70, 32, 172, 174, 0, 3, 0, 6, 196, 125, 70, 32, 172, 174,
            0, 0, 0, 0, 2, 59, 59, 2, 59, 59, 7, 116, 101, 115, 116, 49, 50,
            51, 0, 27, 25, 0, 2
        };
    rsp(0x0001, m, sizeof m);
}
int main()
{
    if(!sk.setUdpTtl(1) || !sk.setIfUsingName("lo") || !sk.init()) {
        printf("init %s\n", Error::getError().c_str());
        return 0;
    }
    // Created with set operation
    USER_DESCRIPTION_f();
    MASTER_ONLY_f();
    GRANDMASTER_CLUSTER_TABLE_f();
    UNICAST_MASTER_TABLE_f();
    ACCEPTABLE_MASTER_TABLE_f();
    ACCEPTABLE_MASTER_TABLE_ENABLED_f();
    EXTERNAL_PORT_CONFIGURATION_ENABLED_f();
    HOLDOVER_UPGRADE_ENABLE_f();
    EXT_PORT_CONFIG_PORT_DATA_SET_f();
    // Created from static TLV data
    // TLV data is verifyed in the library unit tests
    UNICAST_MASTER_MAX_TABLE_SIZE_f();
    ACCEPTABLE_MASTER_MAX_TABLE_SIZE_f();
    PATH_TRACE_LIST_f();
    CLOCK_DESCRIPTION_f();
    return 0;
}
