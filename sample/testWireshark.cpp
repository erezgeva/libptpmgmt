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
uint8_t bufBase[100];
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
        printf("buidSend %s\n", Error::getError().c_str());
}
/* ********************************************************************* */
void USER_DESCRIPTION_f()
{
    USER_DESCRIPTION_t t;
    t.userDescription.textField = "test123";
    buidSend(USER_DESCRIPTION, &t);
}
void PRIORITY1_f()
{
    PRIORITY1_t t;
    t.priority1 = 153;
    buidSend(PRIORITY1, &t);
}
void PRIORITY2_f()
{
    PRIORITY2_t t;
    t.priority2 = 137;
    buidSend(PRIORITY2, &t);
}
void DOMAIN_f()
{
    DOMAIN_t t;
    t.domainNumber = 7;
    buidSend(DOMAIN, &t);
}
void SLAVE_ONLY_f()
{
    SLAVE_ONLY_t t;
    t.flags = 1;
    buidSend(SLAVE_ONLY, &t);
}
void LOG_ANNOUNCE_INTERVAL_f()
{
    LOG_ANNOUNCE_INTERVAL_t t;
    t.logAnnounceInterval = 1;
    buidSend(LOG_ANNOUNCE_INTERVAL, &t);
}
void ANNOUNCE_RECEIPT_TIMEOUT_f()
{
    ANNOUNCE_RECEIPT_TIMEOUT_t t;
    t.announceReceiptTimeout = 3;
    buidSend(ANNOUNCE_RECEIPT_TIMEOUT, &t);
}
void LOG_SYNC_INTERVAL_f()
{
    LOG_SYNC_INTERVAL_t t;
    t.logSyncInterval = 7;
    buidSend(LOG_SYNC_INTERVAL, &t);
}
void VERSION_NUMBER_f()
{
    VERSION_NUMBER_t t;
    t.versionNumber = 2;
    buidSend(VERSION_NUMBER, &t);
}
void TIME_f()
{
    TIME_t t;
    t.currentTime = 13.15;
    buidSend(TIME, &t);
}
void CLOCK_ACCURACY_f()
{
    CLOCK_ACCURACY_t t;
    t.clockAccuracy = Accurate_Unknown;
    buidSend(CLOCK_ACCURACY, &t);
}
void UTC_PROPERTIES_f()
{
    UTC_PROPERTIES_t t;
    t.currentUtcOffset = -0x5433;
    t.flags = 7;
    buidSend(UTC_PROPERTIES, &t);
}
void TRACEABILITY_PROPERTIES_f()
{
    TRACEABILITY_PROPERTIES_t t;
    t.flags = F_TTRA | F_FTRA;
    buidSend(TRACEABILITY_PROPERTIES, &t);
}
void TIMESCALE_PROPERTIES_f()
{
    TIMESCALE_PROPERTIES_t t;
    t.flags = F_PTP;
    t.timeSource = HAND_SET;
    buidSend(TIMESCALE_PROPERTIES, &t);
}
void UNICAST_NEGOTIATION_ENABLE_f()
{
    UNICAST_NEGOTIATION_ENABLE_t t;
    t.flags = 1;
    buidSend(UNICAST_NEGOTIATION_ENABLE, &t);
}
void PATH_TRACE_ENABLE_f()
{
    PATH_TRACE_ENABLE_t t;
    t.flags = 1;
    buidSend(PATH_TRACE_ENABLE, &t);
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
void ALTERNATE_MASTER_f()
{
    ALTERNATE_MASTER_t t;
    t.flags = 1;
    t.logAlternateMulticastSyncInterval = -17;
    t.numberOfAlternateMasters = 210;
    buidSend(ALTERNATE_MASTER, &t);
}
void ALTERNATE_TIME_OFFSET_ENABLE_f()
{
    ALTERNATE_TIME_OFFSET_ENABLE_t t;
    t.keyField = 7;
    t.flags = 1;
    buidSend(ALTERNATE_TIME_OFFSET_ENABLE, &t);
}
void ALTERNATE_TIME_OFFSET_NAME_f()
{
    ALTERNATE_TIME_OFFSET_NAME_t t;
    t.keyField = 11;
    t.displayName.textField = "123";
    buidSend(ALTERNATE_TIME_OFFSET_NAME, &t);
}
void ALTERNATE_TIME_OFFSET_PROPERTIES_f()
{
    ALTERNATE_TIME_OFFSET_PROPERTIES_t t;
    t.keyField = 13;
    t.currentOffset = -2145493247;
    t.jumpSeconds = -2147413249;
    t.timeOfNextJump = 0x912478321891LL;
    buidSend(ALTERNATE_TIME_OFFSET_PROPERTIES, &t);
}
void LOG_MIN_PDELAY_REQ_INTERVAL_f()
{
    LOG_MIN_PDELAY_REQ_INTERVAL_t t;
    t.logMinPdelayReqInterval = 9;
    buidSend(LOG_MIN_PDELAY_REQ_INTERVAL, &t);
}
void PRIMARY_DOMAIN_f()
{
    PRIMARY_DOMAIN_t t;
    t.primaryDomain = 17;
    buidSend(PRIMARY_DOMAIN, &t);
}
void DELAY_MECHANISM_f()
{
    DELAY_MECHANISM_t t;
    t.delayMechanism = P2P;
    buidSend(DELAY_MECHANISM, &t);
}
void EXTERNAL_PORT_CONFIGURATION_ENABLED_f()
{
    EXTERNAL_PORT_CONFIGURATION_ENABLED_t t;
    t.flags = 1;
    buidSend(EXTERNAL_PORT_CONFIGURATION_ENABLED, &t);
}
void MASTER_ONLY_f()
{
    MASTER_ONLY_t t;
    t.flags = 1;
    buidSend(MASTER_ONLY, &t);
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
/* ********************************************************************* */
#include <endian.h>
inline uint16_t cpu_to_net16(uint16_t value) {return htobe16(value);}
/* Create PTPv2 packet from a TLV data
 * We use it when a TLV do not support a set operation */
bool rspReady()
{
    if(!msg.setAction(GET, NULL_PTP_MANAGEMENT) ||
        msg.build(bufBase, sizeof bufBase, 1) != MNG_PARSE_ERROR_OK) {
        printf("bufBase %s\n", Error::getError().c_str());
        return false;
    }
    bufBase[46] = RESPONSE;
    uint16_t *ptr16 = (uint16_t *)(bufBase + 48);
    *ptr16 = cpu_to_net16(MANAGEMENT);
    return true;
}
void rsp(uint16_t mngId, uint8_t *tlv, size_t tlvLength)
{
    size_t len = 54 + tlvLength;
    // header.messageLength
    uint16_t *ptr16 = (uint16_t *)(buf + 2);
    *ptr16 = cpu_to_net16(len);              //  2
    ptr16 += 24;
    *ptr16++ = cpu_to_net16(tlvLength + 2);  // 50
    *ptr16++ = cpu_to_net16(mngId);          // 52
    memcpy(ptr16, tlv, tlvLength);           // 54
    if(!sk.send(buf, len))
        printf("rsp %s\n", Error::getError().c_str());
}
/* A base PTPv2 packet we add a TLV on top */
void base()
{
    memcpy(buf, bufBase, 50);
}
/* ********************************************************************* */
void CLOCK_DESCRIPTION_f()
{
    base();
    uint8_t m[58] = {128, 0, 10, 73, 69, 69, 69, 32, 56, 48, 50, 46, 51, 0, 6,
            196, 125, 70, 32, 172, 174, 0, 3, 0, 6, 196, 125, 70, 32, 172, 174,
            0, 0, 0, 0, 2, 59, 59, 2, 59, 59, 7, 116, 101, 115, 116, 49, 50,
            51, 0, 27, 25, 0, 2
        };
    rsp(0x0001, m, sizeof m);
}
void INITIALIZE_f()
{
    base();
    uint8_t m[] = {18, 52};
    buf[46] = ACKNOWLEDGE;
    rsp(0x0005, m, sizeof m);
}
void FAULT_LOG_f()
{
    base();
    uint8_t m[108] = {0, 2, 0, 50, 0, 0, 0, 0, 0, 9, 0, 10, 209, 136, 2, 7, 101,
            114, 114, 111, 114, 32, 49, 7, 116, 101, 115, 116, 49, 50, 51, 20,
            84, 104, 105, 115, 32, 105, 115, 32, 102, 105, 114, 115, 116, 32,
            114, 101, 99, 111, 114, 100, 0, 55, 0, 0, 0, 0, 6, 255, 0, 46, 16,
            248, 4, 7, 101, 114, 114, 111, 114, 32, 50, 7, 116, 101, 115, 116,
            51, 50, 49, 25, 84, 104, 105, 115, 32, 105, 115, 32, 116, 104, 101,
            32, 115, 101, 99, 111, 110, 100, 32, 114, 101, 99, 111, 114, 100
        };
    rsp(0x0006, m, sizeof m);
}
void DEFAULT_DATA_SET_f()
{
    base();
    uint8_t m[22] = {3, 0, 0, 1, 153, 255, 254, 255, 255, 137,
            196, 125, 70, 255, 254, 32, 172, 174
        };
    rsp(0x2000, m, sizeof m);
}
void CURRENT_DATA_SET_f()
{
    base();
    uint8_t m[20] = {0x12, 0x34, 0x32, 0x10, 0x47, 0xab, 0xcd, 0x54, 0x12,
            0x85, 0x9, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1
        };
    rsp(0x2001, m, sizeof m);
}
void PARENT_DATA_SET_f()
{
    base();
    uint8_t m[34] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 0, 0, 0, 255,
            255, 127, 255, 255, 255, 255, 255, 254, 255, 255, 255, 196, 125,
            70, 255, 254, 32, 172, 174
        };
    rsp(0x2002, m, sizeof m);
}
void TIME_PROPERTIES_DATA_SET_f()
{
    base();
    uint8_t m[6] = {0, 37, 8, 160};
    rsp(0x2003, m, sizeof m);
}
void PORT_DATA_SET_f()
{
    base();
    uint8_t m[28] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1, 4, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 1, 3, 0, 2, 0, 2
        };
    rsp(0x2004, m, sizeof m);
};
void PATH_TRACE_LIST_f()
{
    base();
    uint8_t m[18] = {0xc4, 0x7d, 0x46, 0xff, 0xfe, 0x20, 0xac, 0xae,
            12, 4, 19, 97, 11, 74, 12, 74
        };
    rsp(0x2015, m, sizeof m);
}
void UNICAST_MASTER_MAX_TABLE_SIZE_f()
{
    base();
    uint8_t m[] = {33, 67};
    rsp(0x2019, m, sizeof m);
}
void ACCEPTABLE_MASTER_MAX_TABLE_SIZE_f()
{
    base();
    uint8_t m[] = {103, 186};
    rsp(0x201c, m, sizeof m);
}
void ALTERNATE_TIME_OFFSET_MAX_KEY_f()
{
    base();
    uint8_t m[2] = {9};
    rsp(0x2020, m, sizeof m);
}
void TRANSPARENT_CLOCK_PORT_DATA_SET_f()
{
    base();
    uint8_t m[20] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1,
            1, 235, 0xdc, 0xf8, 0x72, 0x40, 0xdc, 0xd1, 0x23, 1
        };
    rsp(0x4001, m, sizeof m);
}
void TRANSPARENT_CLOCK_DEFAULT_DATA_SET_f()
{
    base();
    uint8_t m[14] = {196, 125, 70, 255, 254, 32, 172, 174, 23, 122, 254, 18};
    rsp(0x4000, m, sizeof m);
}
/* ********************************************************************* */
int main()
{
    if(!sk.setUdpTtl(1) || !sk.setIfUsingName("lo") || !sk.init()) {
        printf("init %s\n", Error::getError().c_str());
        return 0;
    }
    // Created with set operation
    USER_DESCRIPTION_f();
    PRIORITY1_f();
    PRIORITY2_f();
    DOMAIN_f();
    SLAVE_ONLY_f();
    LOG_ANNOUNCE_INTERVAL_f();
    ANNOUNCE_RECEIPT_TIMEOUT_f();
    LOG_SYNC_INTERVAL_f();
    VERSION_NUMBER_f();
    TIME_f();
    CLOCK_ACCURACY_f();
    UTC_PROPERTIES_f();
    TRACEABILITY_PROPERTIES_f();
    TIMESCALE_PROPERTIES_f();
    UNICAST_NEGOTIATION_ENABLE_f();
    PATH_TRACE_ENABLE_f();
    GRANDMASTER_CLUSTER_TABLE_f();
    UNICAST_MASTER_TABLE_f();
    ACCEPTABLE_MASTER_TABLE_f();
    ACCEPTABLE_MASTER_TABLE_ENABLED_f();
    ALTERNATE_MASTER_f();
    ALTERNATE_TIME_OFFSET_ENABLE_f();
    ALTERNATE_TIME_OFFSET_NAME_f();
    ALTERNATE_TIME_OFFSET_PROPERTIES_f();
    LOG_MIN_PDELAY_REQ_INTERVAL_f();
    PRIMARY_DOMAIN_f();
    DELAY_MECHANISM_f();
    EXTERNAL_PORT_CONFIGURATION_ENABLED_f();
    MASTER_ONLY_f();
    HOLDOVER_UPGRADE_ENABLE_f();
    EXT_PORT_CONFIG_PORT_DATA_SET_f();
    // Created from static TLV data
    // TLV data is verifyed in the library unit tests
    if(!rspReady())
        return 0;
    CLOCK_DESCRIPTION_f();
    INITIALIZE_f();
    FAULT_LOG_f();
    DEFAULT_DATA_SET_f();
    CURRENT_DATA_SET_f();
    PARENT_DATA_SET_f();
    TIME_PROPERTIES_DATA_SET_f();
    PORT_DATA_SET_f();
    PATH_TRACE_LIST_f();
    UNICAST_MASTER_MAX_TABLE_SIZE_f();
    ACCEPTABLE_MASTER_MAX_TABLE_SIZE_f();
    ALTERNATE_TIME_OFFSET_MAX_KEY_f();
    TRANSPARENT_CLOCK_PORT_DATA_SET_f();
    TRANSPARENT_CLOCK_DEFAULT_DATA_SET_f();
    return 0;
}
