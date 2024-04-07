/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief PTP management TLV structures unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include "config.h"
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include "proc.h"
#include "msg.h" // We use message to test the managment TLV

#define tlvLoc (54)
#define physicalAddress_v ("\xc4\x7d\x46\x20\xac\xae")
#define clockId           ("\xc4\x7d\x46\xff\xfe\x20\xac\xae")
#define ip                ("\x12\x34\x56\x78")

static inline size_t sizeMsg(uint8_t *buf, size_t tlvLength)
{
    size_t len = tlvLoc + tlvLength;
    // header.messageLength
    buf[2] = len >> 8;
    buf[3] = len & 0xff;
    buf[46] = PTPMGMT_RESPONSE;
    return len;
}
static inline size_t _rsp(uint8_t *buf, uint16_t mngId, uint8_t *tlv,
    size_t tlvLength, enum ptpmgmt_actionField_e act)
{
    size_t len = tlvLoc + tlvLength;
    // header.messageLength
    buf[2] = len >> 8;
    buf[3] = len & 0xff;
    buf[46] = act;
    uint16_t *ptr16 = (uint16_t *)(buf + 48);
    *ptr16++ = htons(PTPMGMT_MANAGEMENT);     // 48
    *ptr16++ = htons(tlvLength + 2);  // 50
    *ptr16++ = htons(mngId);          // 52
    memcpy(ptr16, tlv, tlvLength);           // 54
    return len;
}
static inline size_t rsp(uint8_t *buf, uint16_t mngId, uint8_t *tlv,
    size_t tlvLength)
{
    return _rsp(buf, mngId, tlv, tlvLength, PTPMGMT_RESPONSE);
}

// Tests CLOCK_DESCRIPTION structure
Test(ProcTest, CLOCK_DESCRIPTION)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[58] = {128, 0, 10, 73, 69, 69, 69, 32, 56, 48, 50, 46, 51, 0, 6,
            196, 125, 70, 32, 172, 174, 0, 3, 0, 6, 196, 125, 70, 32, 172, 174,
            0, 0, 0, 0, 2, 59, 59, 2, 59, 59, 7, 116, 101, 115, 116, 49, 50,
            51, 0, 27, 25, 0, 2
        };
    cr_assert(eq(int, m->parse(m, buf, rsp(buf, 0x0001, mb, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_CLOCK_DESCRIPTION));
    const struct ptpmgmt_CLOCK_DESCRIPTION_t *r =
        (const struct ptpmgmt_CLOCK_DESCRIPTION_t *)m->getData(m);
    cr_expect(eq(int, r->clockType, ptpmgmt_ordinaryClock));
    cr_expect(eq(u8, r->physicalLayerProtocol.lengthField, 10));
    cr_expect(eq(str, (char *)r->physicalLayerProtocol.textField, "IEEE 802.3"));
    cr_expect(eq(u16, r->physicalAddressLength, 6));
    cr_expect(zero(memcmp(r->physicalAddress, physicalAddress_v, 6)));
    cr_expect(eq(int, r->protocolAddress.networkProtocol, ptpmgmt_IEEE_802_3));
    cr_expect(eq(u16, r->protocolAddress.addressLength, 6));
    cr_expect(zero(memcmp(r->protocolAddress.addressField, physicalAddress_v, 6)));
    cr_expect(zero(memcmp(r->manufacturerIdentity, "\x0\x0\x0", 3)));
    cr_expect(eq(u8, r->productDescription.lengthField, 2));
    cr_expect(eq(str, (char *)r->productDescription.textField, ";;"));
    cr_expect(eq(u8, r->revisionData.lengthField, 2));
    cr_expect(eq(str, (char *)r->revisionData.textField, ";;"));
    cr_expect(eq(u8, r->userDescription.lengthField, 7));
    cr_expect(eq(str, (char *)r->userDescription.textField, "test123"));
    cr_expect(zero(memcmp(r->profileIdentity, "\x0\x1b\x19\x0\x2\x0", 6)));
    m->free(m);
}

// Tests USER_DESCRIPTION structure
Test(ProcTest, USER_DESCRIPTION)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_USER_DESCRIPTION_t t;
    char tst[] = "test123";
    t.userDescription.lengthField = strlen(tst);
    t.userDescription.textField = tst;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_USER_DESCRIPTION, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_USER_DESCRIPTION));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + 8));
    uint8_t mb[8] = {7, 116, 101, 115, 116, 49, 50, 51};
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_USER_DESCRIPTION_t *r =
        (const struct ptpmgmt_USER_DESCRIPTION_t *)m->getData(m);
    cr_expect(eq(u8, r->userDescription.lengthField, 7));
    cr_expect(eq(str, (char *)r->userDescription.textField, "test123"));
    m->free(m);
}

// Tests INITIALIZE structure
Test(ProcTest, INITIALIZE)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[2] = {18, 52};
    cr_assert(eq(int, m->parse(m, buf, _rsp(buf, 0x0005, mb, sizeof mb,
                    PTPMGMT_ACKNOWLEDGE)), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_INITIALIZE));
    const struct ptpmgmt_INITIALIZE_t *r =
        (const struct ptpmgmt_INITIALIZE_t *)m->getData(m);
    cr_expect(eq(u16, r->initializationKey, 0x1234));
    m->free(m);
}

// Tests FAULT_LOG structure
Test(ProcTest, FAULT_LOG)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[108] = {0, 2, 0, 50, 0, 0, 0, 0, 0, 9, 0, 10, 209, 136, 2, 7, 101,
            114, 114, 111, 114, 32, 49, 7, 116, 101, 115, 116, 49, 50, 51, 20,
            84, 104, 105, 115, 32, 105, 115, 32, 102, 105, 114, 115, 116, 32,
            114, 101, 99, 111, 114, 100, 0, 55, 0, 0, 0, 0, 6, 255, 0, 46, 16,
            248, 4, 7, 101, 114, 114, 111, 114, 32, 50, 7, 116, 101, 115, 116,
            51, 50, 49, 25, 84, 104, 105, 115, 32, 105, 115, 32, 116, 104, 101,
            32, 115, 101, 99, 111, 110, 100, 32, 114, 101, 99, 111, 114, 100
        };
    cr_assert(eq(int, m->parse(m, buf, rsp(buf, 0x0006, mb, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_FAULT_LOG));
    const struct ptpmgmt_FAULT_LOG_t *r =
        (const struct ptpmgmt_FAULT_LOG_t *)m->getData(m);
    cr_expect(eq(u16, r->numberOfFaultRecords, 2));
    cr_expect(eq(u16, r->faultRecords[0].faultRecordLength, 50));
    cr_expect(eq(u64, r->faultRecords[0].faultTime.secondsField, 9));
    cr_expect(eq(u32, r->faultRecords[0].faultTime.nanosecondsField, 709000));
    cr_expect(eq(int, r->faultRecords[0].severityCode, ptpmgmt_F_Critical));
    cr_expect(eq(u8, r->faultRecords[0].faultName.lengthField, 7));
    cr_expect(eq(str, (char *)r->faultRecords[0].faultName.textField, "error 1"));
    cr_expect(eq(u8, r->faultRecords[0].faultValue.lengthField, 7));
    cr_expect(eq(str, (char *)r->faultRecords[0].faultValue.textField, "test123"));
    cr_expect(eq(u8, r->faultRecords[0].faultDescription.lengthField, 20));
    cr_expect(eq(str, (char *)r->faultRecords[0].faultDescription.textField,
            "This is first record"));
    cr_expect(eq(u16, r->faultRecords[1].faultRecordLength, 55));
    cr_expect(eq(u64, r->faultRecords[1].faultTime.secondsField, 1791));
    cr_expect(eq(u32, r->faultRecords[1].faultTime.nanosecondsField, 3019000));
    cr_expect(eq(int, r->faultRecords[1].severityCode, ptpmgmt_F_Warning));
    cr_expect(eq(u8, r->faultRecords[1].faultName.lengthField, 7));
    cr_expect(eq(str, (char *)r->faultRecords[1].faultName.textField, "error 2"));
    cr_expect(eq(u8, r->faultRecords[1].faultValue.lengthField, 7));
    cr_expect(eq(str, (char *)r->faultRecords[1].faultValue.textField, "test321"));
    cr_expect(eq(u8, r->faultRecords[1].faultDescription.lengthField, 25));
    cr_expect(eq(str, (char *)r->faultRecords[1].faultDescription.textField,
            "This is the second record"));
    m->free(m);
}

// Tests DEFAULT_DATA_SET structure
Test(ProcTest, DEFAULT_DATA_SET)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[22] = {3, 0, 0, 1, 153, 255, 254, 255, 255, 137,
            196, 125, 70, 255, 254, 32, 172, 174
        };
    cr_assert(eq(int, m->parse(m, buf, rsp(buf, 0x2000, mb, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_DEFAULT_DATA_SET));
    const struct ptpmgmt_DEFAULT_DATA_SET_t *r =
        (const struct ptpmgmt_DEFAULT_DATA_SET_t *)m->getData(m);
    cr_expect(eq(int, r->flags, 0x3));
    cr_expect(eq(int, r->numberPorts, 1));
    cr_expect(eq(int, r->priority1, 153));
    cr_expect(eq(int, r->clockQuality.clockClass, 255));
    cr_expect(eq(int, r->clockQuality.clockAccuracy, ptpmgmt_Accurate_Unknown));
    cr_expect(eq(int, r->clockQuality.offsetScaledLogVariance, 0xffff));
    cr_expect(eq(int, r->priority2, 137));
    cr_expect(zero(memcmp(r->clockIdentity.v, clockId, 8)));
    cr_expect(eq(int, r->domainNumber, 0));
    m->free(m);
}

// Tests CURRENT_DATA_SET structure
Test(ProcTest, CURRENT_DATA_SET)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[20] = {0x12, 0x34, 0x32, 0x10, 0x47, 0xab, 0xcd, 0x54, 0x12,
            0x85, 0x9, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1
        };
    cr_assert(eq(int, m->parse(m, buf, rsp(buf, 0x2001, mb, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_CURRENT_DATA_SET));
    const struct ptpmgmt_CURRENT_DATA_SET_t *r =
        (const struct ptpmgmt_CURRENT_DATA_SET_t *)m->getData(m);
    cr_expect(eq(int, r->stepsRemoved, 0x1234));
    cr_expect(eq(i64, r->offsetFromMaster.scaledNanoseconds, 0x321047abcd541285LL));
    cr_expect(eq(i64, r->meanPathDelay.scaledNanoseconds, 0x0906050403020100LL));
    m->free(m);
}

// Tests PARENT_DATA_SET structure
Test(ProcTest, PARENT_DATA_SET)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[34] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 0, 0, 0, 255,
            255, 127, 255, 255, 255, 255, 255, 254, 255, 255, 255, 196, 125,
            70, 255, 254, 32, 172, 174
        };
    cr_assert(eq(int, m->parse(m, buf, rsp(buf, 0x2002, mb, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_PARENT_DATA_SET));
    const struct ptpmgmt_PARENT_DATA_SET_t *r =
        (const struct ptpmgmt_PARENT_DATA_SET_t *)m->getData(m);
    cr_expect(eq(u16, r->parentPortIdentity.portNumber, 0));
    cr_expect(zero(memcmp(r->parentPortIdentity.clockIdentity.v, clockId, 8)));
    cr_expect(eq(int, r->flags, 0));
    cr_expect(eq(int, r->observedParentOffsetScaledLogVariance, 0xffff));
    cr_expect(eq(int, r->observedParentClockPhaseChangeRate, 0x7fffffff));
    cr_expect(eq(int, r->grandmasterPriority1, 255));
    cr_expect(eq(int, r->grandmasterClockQuality.clockClass, 255));
    cr_expect(eq(int, r->grandmasterClockQuality.clockAccuracy,
            ptpmgmt_Accurate_Unknown));
    cr_expect(eq(int, r->grandmasterClockQuality.offsetScaledLogVariance, 0xffff));
    cr_expect(eq(int, r->grandmasterPriority2, 255));
    cr_expect(zero(memcmp(r->grandmasterIdentity.v, clockId, 8)));
    m->free(m);
}

// Tests TIME_PROPERTIES_DATA_SET structure
Test(ProcTest, TIME_PROPERTIES_DATA_SET)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[6] = {0, 37, 8, 160};
    cr_assert(eq(int, m->parse(m, buf, rsp(buf, 0x2003, mb, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_TIME_PROPERTIES_DATA_SET));
    const struct ptpmgmt_TIME_PROPERTIES_DATA_SET_t *r =
        (const struct ptpmgmt_TIME_PROPERTIES_DATA_SET_t *)m->getData(m);
    cr_expect(eq(int, r->currentUtcOffset, 37));
    // check ptpTimescale bit
    cr_expect(eq(int, r->flags, PTPMGMT_F_PTP));
    cr_expect(eq(int, r->timeSource, PTPMGMT_INTERNAL_OSCILLATOR));
    m->free(m);
}

// Tests PORT_DATA_SET structure
Test(ProcTest, PORT_DATA_SET)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[28] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1, 4, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 1, 3, 0, 2, 0, 2
        };
    cr_assert(eq(int, m->parse(m, buf, rsp(buf, 0x2004, mb, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_PORT_DATA_SET));
    const struct ptpmgmt_PORT_DATA_SET_t *r =
        (const struct ptpmgmt_PORT_DATA_SET_t *)m->getData(m);
    cr_expect(eq(u16, r->portIdentity.portNumber, 1));
    cr_expect(zero(memcmp(r->portIdentity.clockIdentity.v, clockId, 8)));
    cr_expect(eq(int, r->portState, PTPMGMT_LISTENING));
    cr_expect(eq(int, r->logMinDelayReqInterval, 0));
    cr_expect(eq(i64, r->peerMeanPathDelay.scaledNanoseconds, 0));
    cr_expect(eq(int, r->logAnnounceInterval, 1));
    cr_expect(eq(int, r->announceReceiptTimeout, 3));
    cr_expect(eq(int, r->logSyncInterval, 0));
    cr_expect(eq(int, r->delayMechanism, PTPMGMT_P2P));
    cr_expect(eq(int, r->logMinPdelayReqInterval, 0));
    cr_expect(eq(int, r->versionNumber, 2));
    m->free(m);
};

// Tests PRIORITY1 structure
Test(ProcTest, PRIORITY1)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_PRIORITY1_t t;
    t.priority1 = 153;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY1, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {153};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_PRIORITY1_t *r =
        (const struct ptpmgmt_PRIORITY1_t *)m->getData(m);
    cr_expect(eq(int, r->priority1, 153));
    m->free(m);
}

// Tests PRIORITY2 structure
Test(ProcTest, PRIORITY2)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_PRIORITY2_t t;
    t.priority2 = 137;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY2, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY2));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {137};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_PRIORITY2_t *r =
        (const struct ptpmgmt_PRIORITY2_t *)m->getData(m);
    cr_expect(eq(int, r->priority2, 137));
    m->free(m);
}

// Tests DOMAIN structure
Test(ProcTest, DOMAIN)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_DOMAIN_t t;
    t.domainNumber = 7;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_DOMAIN, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_DOMAIN));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {7};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_DOMAIN_t *r =
        (const struct ptpmgmt_DOMAIN_t *)m->getData(m);
    cr_expect(eq(int, r->domainNumber, 7));
    m->free(m);
}

// Tests SLAVE_ONLY structure
Test(ProcTest, SLAVE_ONLY)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_SLAVE_ONLY_t t;
    t.flags = 1;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_SLAVE_ONLY, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_SLAVE_ONLY));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {1};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_SLAVE_ONLY_t *r =
        (const struct ptpmgmt_SLAVE_ONLY_t *)m->getData(m);
    cr_expect(eq(int, r->flags, 1));
    m->free(m);
}

// Tests LOG_ANNOUNCE_INTERVAL structure
Test(ProcTest, LOG_ANNOUNCE_INTERVAL)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_LOG_ANNOUNCE_INTERVAL_t t;
    t.logAnnounceInterval = 1;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_LOG_ANNOUNCE_INTERVAL, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_LOG_ANNOUNCE_INTERVAL));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {1};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_LOG_ANNOUNCE_INTERVAL_t *r =
        (const struct ptpmgmt_LOG_ANNOUNCE_INTERVAL_t *)m->getData(m);
    cr_expect(eq(int, r->logAnnounceInterval, 1));
    m->free(m);
}

// Tests ANNOUNCE_RECEIPT_TIMEOUT structure
Test(ProcTest, ANNOUNCE_RECEIPT_TIMEOUT)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_ANNOUNCE_RECEIPT_TIMEOUT_t t;
    t.announceReceiptTimeout = 3;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_ANNOUNCE_RECEIPT_TIMEOUT, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_ANNOUNCE_RECEIPT_TIMEOUT));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {3};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_ANNOUNCE_RECEIPT_TIMEOUT_t *r =
        (const struct ptpmgmt_ANNOUNCE_RECEIPT_TIMEOUT_t *)m->getData(m);
    cr_expect(eq(int, r->announceReceiptTimeout, 3));
    m->free(m);
}

// Tests LOG_SYNC_INTERVAL structure
Test(ProcTest, LOG_SYNC_INTERVAL)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_LOG_SYNC_INTERVAL_t t;
    t.logSyncInterval = 7;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_LOG_SYNC_INTERVAL, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_LOG_SYNC_INTERVAL));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 7), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {7};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_LOG_SYNC_INTERVAL_t *r =
        (const struct ptpmgmt_LOG_SYNC_INTERVAL_t *)m->getData(m);
    cr_expect(eq(int, r->logSyncInterval, 7));
    m->free(m);
}

// Tests VERSION_NUMBER structure
Test(ProcTest, VERSION_NUMBER)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_VERSION_NUMBER_t t;
    t.versionNumber = 2;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_VERSION_NUMBER, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_VERSION_NUMBER));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {2};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_VERSION_NUMBER_t *r =
        (const struct ptpmgmt_VERSION_NUMBER_t *)m->getData(m);
    cr_expect(eq(int, r->versionNumber, 2));
    m->free(m);
}

// Tests TIME structure
Test(ProcTest, TIME)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_TIME_t t;
    t.currentTime.secondsField = 13;
    t.currentTime.nanosecondsField = 150000000;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_TIME, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_TIME));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[10] = {0, 0, 0, 0, 0, 13, 8, 240, 209, 128};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_TIME_t *r = (const struct ptpmgmt_TIME_t *)m->getData(m);
    cr_expect(eq(u64, r->currentTime.secondsField, 13));
    cr_expect(eq(u32, r->currentTime.nanosecondsField, 150000000));
    m->free(m);
}

// Tests CLOCK_ACCURACY structure
Test(ProcTest, CLOCK_ACCURACY)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_CLOCK_ACCURACY_t t;
    t.clockAccuracy = ptpmgmt_Accurate_Unknown;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_CLOCK_ACCURACY, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_CLOCK_ACCURACY));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {254};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_CLOCK_ACCURACY_t *r =
        (const struct ptpmgmt_CLOCK_ACCURACY_t *)m->getData(m);
    cr_expect(eq(int, r->clockAccuracy, ptpmgmt_Accurate_Unknown));
    m->free(m);
}

// Tests UTC_PROPERTIES structure
Test(ProcTest, UTC_PROPERTIES)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_UTC_PROPERTIES_t t;
    t.currentUtcOffset = -0x5433;
    t.flags = 7;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_UTC_PROPERTIES, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_UTC_PROPERTIES));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[4] = {171, 205, 7};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_UTC_PROPERTIES_t *r =
        (const struct ptpmgmt_UTC_PROPERTIES_t *)m->getData(m);
    cr_expect(eq(int, r->currentUtcOffset, -0x5433));
    cr_expect(eq(int, r->flags, 7));
    m->free(m);
}

// Tests TRACEABILITY_PROPERTIES structure
Test(ProcTest, TRACEABILITY_PROPERTIES)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_TRACEABILITY_PROPERTIES_t t;
    t.flags = PTPMGMT_F_TTRA | PTPMGMT_F_FTRA;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_TRACEABILITY_PROPERTIES, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_TRACEABILITY_PROPERTIES));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {48};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_TRACEABILITY_PROPERTIES_t *r =
        (const struct ptpmgmt_TRACEABILITY_PROPERTIES_t *)m->getData(m);
    cr_expect(eq(int, r->flags, PTPMGMT_F_TTRA | PTPMGMT_F_FTRA));
    m->free(m);
}

// Tests TIMESCALE_PROPERTIES structure
Test(ProcTest, TIMESCALE_PROPERTIES)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_TIMESCALE_PROPERTIES_t t;
    t.flags = PTPMGMT_F_PTP;
    t.timeSource = PTPMGMT_HAND_SET;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_TIMESCALE_PROPERTIES, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_TIMESCALE_PROPERTIES));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {8, 96};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_TIMESCALE_PROPERTIES_t *r =
        (const struct ptpmgmt_TIMESCALE_PROPERTIES_t *)m->getData(m);
    cr_expect(eq(int, r->flags, PTPMGMT_F_PTP));
    cr_expect(eq(int, r->timeSource, PTPMGMT_HAND_SET));
    m->free(m);
}

// Tests UNICAST_NEGOTIATION_ENABLE structure
Test(ProcTest, UNICAST_NEGOTIATION_ENABLE)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_UNICAST_NEGOTIATION_ENABLE_t t;
    t.flags = 1;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_UNICAST_NEGOTIATION_ENABLE, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_UNICAST_NEGOTIATION_ENABLE));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {1};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_UNICAST_NEGOTIATION_ENABLE_t *r =
        (const struct ptpmgmt_UNICAST_NEGOTIATION_ENABLE_t *)m->getData(m);
    cr_expect(eq(int, r->flags, 1));
    m->free(m);
}

// Tests PATH_TRACE_LIST structure
Test(ProcTest, PATH_TRACE_LIST)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[18] = {0xc4, 0x7d, 0x46, 0xff, 0xfe, 0x20, 0xac, 0xae,
            12, 4, 19, 97, 11, 74, 12, 74
        };
    cr_assert(eq(int, m->parse(m, buf, rsp(buf, 0x2015, mb, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_PATH_TRACE_LIST));
    const struct ptpmgmt_PATH_TRACE_LIST_t *r =
        (const struct ptpmgmt_PATH_TRACE_LIST_t *)m->getData(m);
    cr_expect(zero(memcmp(r->pathSequence[0].v, "\xc4\x7d\x46\xff\xfe\x20\xac\xae",
                8)));
    cr_expect(zero(memcmp(r->pathSequence[1].v, "\xc\x4\x13\x61\xb\x4a\xc\x4a",
                8)));
    m->free(m);
}

// Tests PATH_TRACE_ENABLE structure
Test(ProcTest, PATH_TRACE_ENABLE)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_PATH_TRACE_ENABLE_t t;
    t.flags = 1;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PATH_TRACE_ENABLE, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PATH_TRACE_ENABLE));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {1};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_PATH_TRACE_ENABLE_t *r =
        (const struct ptpmgmt_PATH_TRACE_ENABLE_t *)m->getData(m);
    cr_expect(eq(int, r->flags, 1));
    m->free(m);
}

// Tests GRANDMASTER_CLUSTER_TABLE structure
Test(ProcTest, GRANDMASTER_CLUSTER_TABLE)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_GRANDMASTER_CLUSTER_TABLE_t t;
    t.logQueryInterval = -19;
    t.actualTableSize = 2;
    void *x = malloc(sizeof(struct ptpmgmt_PortAddress_t) * t.actualTableSize);
    cr_assert(not(zero(ptr, x)));
    t.PortAddress = x;
    t.PortAddress[0].networkProtocol = ptpmgmt_IEEE_802_3;
    t.PortAddress[0].addressLength = 6;
    t.PortAddress[0].addressField = (uint8_t *)physicalAddress_v;
    t.PortAddress[1].networkProtocol = ptpmgmt_UDP_IPv4;
    t.PortAddress[1].addressLength = 4;
    t.PortAddress[1].addressField = (uint8_t *)ip;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_GRANDMASTER_CLUSTER_TABLE, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_GRANDMASTER_CLUSTER_TABLE));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[20] = {237, 2,
            0, 3, 0, 6, 196, 125, 70, 32, 172, 174,
            0, 1, 0, 4, 0x12, 0x34, 0x56, 0x78
        };
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_GRANDMASTER_CLUSTER_TABLE_t *r =
        (const struct ptpmgmt_GRANDMASTER_CLUSTER_TABLE_t *)m->getData(m);
    cr_expect(eq(int, r->logQueryInterval, -19));
    cr_expect(eq(int, r->actualTableSize, 2));
    cr_expect(eq(int, r->PortAddress[0].networkProtocol, ptpmgmt_IEEE_802_3));
    cr_expect(eq(u16, r->PortAddress[0].addressLength, 6));
    cr_expect(zero(memcmp(r->PortAddress[0].addressField, physicalAddress_v, 6)));
    cr_expect(eq(int, r->PortAddress[1].networkProtocol, ptpmgmt_UDP_IPv4));
    cr_expect(eq(u16, r->PortAddress[1].addressLength, 4));
    cr_expect(zero(memcmp(r->PortAddress[1].addressField, ip, 4)));
    free(x);
    m->free(m);
}

// Tests UNICAST_MASTER_TABLE structure
Test(ProcTest, UNICAST_MASTER_TABLE)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_UNICAST_MASTER_TABLE_t t;
    t.logQueryInterval = -19;
    t.actualTableSize = 2;
    void *x = malloc(sizeof(struct ptpmgmt_PortAddress_t) * t.actualTableSize);
    cr_assert(not(zero(ptr, x)));
    t.PortAddress = x;
    t.PortAddress[0].networkProtocol = ptpmgmt_IEEE_802_3;
    t.PortAddress[0].addressLength = 6;
    t.PortAddress[0].addressField = (uint8_t *)physicalAddress_v;
    t.PortAddress[1].networkProtocol = ptpmgmt_UDP_IPv4;
    t.PortAddress[1].addressLength = 4;
    t.PortAddress[1].addressField = (uint8_t *)ip;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_UNICAST_MASTER_TABLE, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_UNICAST_MASTER_TABLE));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[22] = {237, 0, 2,
            0, 3, 0, 6, 196, 125, 70, 32, 172, 174,
            0, 1, 0, 4, 0x12, 0x34, 0x56, 0x78
        };
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_UNICAST_MASTER_TABLE_t *r =
        (const struct ptpmgmt_UNICAST_MASTER_TABLE_t *)m->getData(m);
    cr_expect(eq(int, r->logQueryInterval, -19));
    cr_expect(eq(int, r->actualTableSize, 2));
    cr_expect(eq(int, r->PortAddress[0].networkProtocol, ptpmgmt_IEEE_802_3));
    cr_expect(eq(u16, r->PortAddress[0].addressLength, 6));
    cr_expect(zero(memcmp(r->PortAddress[0].addressField, physicalAddress_v, 6)));
    cr_expect(eq(int, r->PortAddress[1].networkProtocol, ptpmgmt_UDP_IPv4));
    cr_expect(eq(u16, r->PortAddress[1].addressLength, 4));
    cr_expect(zero(memcmp(r->PortAddress[1].addressField, ip, 4)));
    free(x);
    m->free(m);
}

// Tests UNICAST_MASTER_MAX_TABLE_SIZE structure
Test(ProcTest, UNICAST_MASTER_MAX_TABLE_SIZE)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[2] = {33, 67};
    cr_assert(eq(int, m->parse(m, buf, rsp(buf, 0x2019, mb, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_UNICAST_MASTER_MAX_TABLE_SIZE));
    const struct ptpmgmt_UNICAST_MASTER_MAX_TABLE_SIZE_t *r =
        (const struct ptpmgmt_UNICAST_MASTER_MAX_TABLE_SIZE_t *)m->getData(m);
    cr_expect(eq(int, r->maxTableSize, 0x2143));
    m->free(m);
}

// Tests ACCEPTABLE_MASTER_TABLE structure
Test(ProcTest, ACCEPTABLE_MASTER_TABLE)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_ACCEPTABLE_MASTER_TABLE_t t;
    t.actualTableSize = 2;
    void *x = malloc(sizeof(struct ptpmgmt_AcceptableMaster_t) * t.actualTableSize);
    cr_assert(not(zero(ptr, x)));
    t.list = x;
    uint8_t *clockId2 = (uint8_t *)"\x9\x8\x7\x6\x5\x4\x1\x7";
    t.list[0].acceptablePortIdentity.portNumber = 1;
    memcpy(t.list[0].acceptablePortIdentity.clockIdentity.v, clockId, 8);
    t.list[0].alternatePriority1 = 127;
    t.list[1].acceptablePortIdentity.portNumber = 2;
    memcpy(t.list[1].acceptablePortIdentity.clockIdentity.v, clockId2, 8);
    t.list[1].alternatePriority1 = 111;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_ACCEPTABLE_MASTER_TABLE, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_ACCEPTABLE_MASTER_TABLE));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[24] = {0, 2, 196, 125, 70, 255, 254, 32, 172, 174, 0, 1, 127,
            9, 8, 7, 6, 5, 4, 1, 7, 0, 2, 111
        };
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_ACCEPTABLE_MASTER_TABLE_t *r =
        (const struct ptpmgmt_ACCEPTABLE_MASTER_TABLE_t *)m->getData(m);
    cr_expect(eq(int, r->actualTableSize, 2));
    cr_expect(eq(u16, r->list[0].acceptablePortIdentity.portNumber, 1));
    cr_expect(zero(memcmp(r->list[0].acceptablePortIdentity.clockIdentity.v,
                clockId, 8)));
    cr_expect(eq(int, r->list[0].alternatePriority1, 127));
    cr_expect(eq(u16, r->list[1].acceptablePortIdentity.portNumber, 2));
    cr_expect(zero(memcmp(r->list[1].acceptablePortIdentity.clockIdentity.v,
                clockId2, 8)));
    cr_expect(eq(int, r->list[1].alternatePriority1, 111));
    free(x);
    m->free(m);
}

// Tests ACCEPTABLE_MASTER_TABLE_ENABLED structure
Test(ProcTest, ACCEPTABLE_MASTER_TABLE_ENABLED)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_ACCEPTABLE_MASTER_TABLE_ENABLED_t t;
    t.flags = 1;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_ACCEPTABLE_MASTER_TABLE_ENABLED,
            &t));
    cr_expect(eq(int, m->getBuildTlvId(m),
            PTPMGMT_ACCEPTABLE_MASTER_TABLE_ENABLED));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {1};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_ACCEPTABLE_MASTER_TABLE_ENABLED_t *r =
        (const struct ptpmgmt_ACCEPTABLE_MASTER_TABLE_ENABLED_t *)m->getData(m);
    cr_expect(eq(int, r->flags, 1));
    m->free(m);
}

// Tests ACCEPTABLE_MASTER_MAX_TABLE_SIZE structure
Test(ProcTest, ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[] = {103, 186};
    cr_assert(eq(int, m->parse(m, buf, rsp(buf, 0x201c, mb, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_ACCEPTABLE_MASTER_MAX_TABLE_SIZE));
    const struct ptpmgmt_ACCEPTABLE_MASTER_MAX_TABLE_SIZE_t *r =
        (const struct ptpmgmt_ACCEPTABLE_MASTER_MAX_TABLE_SIZE_t *)m->getData(m);
    cr_expect(eq(int, r->maxTableSize, 0x67ba));
    m->free(m);
}

// Tests ALTERNATE_MASTER structure
Test(ProcTest, ALTERNATE_MASTER)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_ALTERNATE_MASTER_t t;
    t.flags = 1;
    t.logAlternateMulticastSyncInterval = -17;
    t.numberOfAlternateMasters = 210;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_ALTERNATE_MASTER, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_ALTERNATE_MASTER));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[4] = {1, 239, 210};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_ALTERNATE_MASTER_t *r =
        (const struct ptpmgmt_ALTERNATE_MASTER_t *)m->getData(m);
    cr_expect(eq(int, r->flags, 1));
    cr_expect(eq(int, r->logAlternateMulticastSyncInterval, -17));
    cr_expect(eq(int, r->numberOfAlternateMasters, 210));
    m->free(m);
}

// Tests ALTERNATE_TIME_OFFSET_ENABLE structure
Test(ProcTest, ALTERNATE_TIME_OFFSET_ENABLE)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_ALTERNATE_TIME_OFFSET_ENABLE_t t;
    t.keyField = 7;
    t.flags = 1;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_ALTERNATE_TIME_OFFSET_ENABLE,
            &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_ALTERNATE_TIME_OFFSET_ENABLE));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {7, 1};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_ALTERNATE_TIME_OFFSET_ENABLE_t *r =
        (const struct ptpmgmt_ALTERNATE_TIME_OFFSET_ENABLE_t *)m->getData(m);
    cr_expect(eq(int, r->keyField, 7));
    cr_expect(eq(int, r->flags, 1));
    m->free(m);
}

// Tests ALTERNATE_TIME_OFFSET_NAME structure
Test(ProcTest, ALTERNATE_TIME_OFFSET_NAME)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_ALTERNATE_TIME_OFFSET_NAME_t t;
    t.keyField = 11;
    char tst[] = "123";
    t.displayName.lengthField = strlen(tst);
    t.displayName.textField = tst;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_ALTERNATE_TIME_OFFSET_NAME, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_ALTERNATE_TIME_OFFSET_NAME));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[6] = {11, 3, 49, 50, 51};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_ALTERNATE_TIME_OFFSET_NAME_t *r =
        (const struct ptpmgmt_ALTERNATE_TIME_OFFSET_NAME_t *)m->getData(m);
    cr_expect(eq(int, r->keyField, 11));
    cr_expect(eq(u8, r->displayName.lengthField, 3));
    cr_expect(eq(str, (char *)r->displayName.textField, "123"));
    m->free(m);
}

// Tests ALTERNATE_TIME_OFFSET_MAX_KEY structure
Test(ProcTest, ALTERNATE_TIME_OFFSET_MAX_KEY)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[2] = {9};
    cr_assert(eq(int, m->parse(m, buf, rsp(buf, 0x2020, mb, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_ALTERNATE_TIME_OFFSET_MAX_KEY));
    const struct ptpmgmt_ALTERNATE_TIME_OFFSET_MAX_KEY_t *r =
        (const struct ptpmgmt_ALTERNATE_TIME_OFFSET_MAX_KEY_t *)m->getData(m);
    cr_expect(eq(int, r->maxKey, 9));
    m->free(m);
}

// Tests ALTERNATE_TIME_OFFSET_PROPERTIES structure
Test(ProcTest, ALTERNATE_TIME_OFFSET_PROPERTIES)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_ALTERNATE_TIME_OFFSET_PROPERTIES_t t;
    t.keyField = 13;
    t.currentOffset = -2145493247;
    t.jumpSeconds = -2147413249;
    t.timeOfNextJump = 0x912478321891LL;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_ALTERNATE_TIME_OFFSET_PROPERTIES,
            &t));
    cr_expect(eq(int, m->getBuildTlvId(m),
            PTPMGMT_ALTERNATE_TIME_OFFSET_PROPERTIES));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[16] = {13, 128, 30, 95, 1, 128, 1, 18, 255, 145, 36,
            120, 50, 24, 145
        };
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_ALTERNATE_TIME_OFFSET_PROPERTIES_t *r =
        (const struct ptpmgmt_ALTERNATE_TIME_OFFSET_PROPERTIES_t *)m->getData(m);
    cr_expect(eq(int, r->keyField, 13));
    cr_expect(eq(int, r->currentOffset, -2145493247));
    cr_expect(eq(int, r->jumpSeconds, -2147413249));
    cr_expect(eq(u64, r->timeOfNextJump, 0x912478321891));
    m->free(m);
}

// Tests TRANSPARENT_CLOCK_PORT_DATA_SET structure
Test(ProcTest, TRANSPARENT_CLOCK_PORT_DATA_SET)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[20] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1,
            1, 235, 0xdc, 0xf8, 0x72, 0x40, 0xdc, 0xd1, 0x23, 1
        };
    cr_assert(eq(int, m->parse(m, buf, rsp(buf, 0x4001, mb, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_TRANSPARENT_CLOCK_PORT_DATA_SET));
    const struct ptpmgmt_TRANSPARENT_CLOCK_PORT_DATA_SET_t *r =
        (const struct ptpmgmt_TRANSPARENT_CLOCK_PORT_DATA_SET_t *)m->getData(m);
    cr_expect(eq(u16, r->portIdentity.portNumber, 1));
    cr_expect(zero(memcmp(r->portIdentity.clockIdentity.v, clockId, 8)));
    cr_expect(eq(int, r->flags, 1));
    cr_expect(eq(int, r->logMinPdelayReqInterval, -21));
    cr_expect(eq(i64, r->peerMeanPathDelay.scaledNanoseconds,
            0xdcf87240dcd12301LL));
    m->free(m);
}

// Tests LOG_MIN_PDELAY_REQ_INTERVAL structure
Test(ProcTest, LOG_MIN_PDELAY_REQ_INTERVAL)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_LOG_MIN_PDELAY_REQ_INTERVAL_t t;
    t.logMinPdelayReqInterval = 9;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_LOG_MIN_PDELAY_REQ_INTERVAL,
            &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_LOG_MIN_PDELAY_REQ_INTERVAL));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {9};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_LOG_MIN_PDELAY_REQ_INTERVAL_t *r =
        (const struct ptpmgmt_LOG_MIN_PDELAY_REQ_INTERVAL_t *)m->getData(m);
    cr_expect(eq(int, r->logMinPdelayReqInterval, 9));
    m->free(m);
}

// Tests TRANSPARENT_CLOCK_DEFAULT_DATA_SET structure
Test(ProcTest, TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[14] = {196, 125, 70, 255, 254, 32, 172, 174, 23, 122, 254, 18};
    cr_assert(eq(int, m->parse(m, buf, rsp(buf, 0x4000, mb, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_TRANSPARENT_CLOCK_DEFAULT_DATA_SET));
    const struct ptpmgmt_TRANSPARENT_CLOCK_DEFAULT_DATA_SET_t *r =
        (const struct ptpmgmt_TRANSPARENT_CLOCK_DEFAULT_DATA_SET_t *)m->getData(m);
    cr_expect(zero(memcmp(r->clockIdentity.v, clockId, 8)));
    cr_expect(eq(int, r->numberPorts, 0x177a));
    cr_expect(eq(int, r->delayMechanism, PTPMGMT_NO_MECHANISM));
    cr_expect(eq(int, r->primaryDomain, 18));
    m->free(m);
}

// Tests PRIMARY_DOMAIN structure
Test(ProcTest, PRIMARY_DOMAIN)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_PRIMARY_DOMAIN_t t;
    t.primaryDomain = 17;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIMARY_DOMAIN, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIMARY_DOMAIN));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {17};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_PRIMARY_DOMAIN_t *r =
        (const struct ptpmgmt_PRIMARY_DOMAIN_t *)m->getData(m);
    cr_expect(eq(int, r->primaryDomain, 17));
    m->free(m);
}

// Tests DELAY_MECHANISM structure
Test(ProcTest, DELAY_MECHANISM)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_DELAY_MECHANISM_t t;
    t.delayMechanism = PTPMGMT_P2P;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_DELAY_MECHANISM, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_DELAY_MECHANISM));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {2};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_DELAY_MECHANISM_t *r
        = (const struct ptpmgmt_DELAY_MECHANISM_t *)m->getData(m);
    cr_expect(eq(int, r->delayMechanism, PTPMGMT_P2P));
    m->free(m);
}

// Tests EXTERNAL_PORT_CONFIGURATION_ENABLED structure
Test(ProcTest, EXTERNAL_PORT_CONFIGURATION_ENABLED)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_EXTERNAL_PORT_CONFIGURATION_ENABLED_t t;
    t.flags = 1;
    cr_expect(m->setAction(m, PTPMGMT_SET,
            PTPMGMT_EXTERNAL_PORT_CONFIGURATION_ENABLED, &t));
    cr_expect(eq(int, m->getBuildTlvId(m),
            PTPMGMT_EXTERNAL_PORT_CONFIGURATION_ENABLED));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {1};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_EXTERNAL_PORT_CONFIGURATION_ENABLED_t *r =
        (const struct ptpmgmt_EXTERNAL_PORT_CONFIGURATION_ENABLED_t *)m->getData(m);
    cr_expect(eq(int, r->flags, 1));
    m->free(m);
}

// Tests MASTER_ONLY structure
Test(ProcTest, MASTER_ONLY)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_MASTER_ONLY_t t;
    t.flags = 1;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_MASTER_ONLY, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_MASTER_ONLY));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {1};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_MASTER_ONLY_t *r =
        (const struct ptpmgmt_MASTER_ONLY_t *)m->getData(m);
    cr_expect(eq(int, r->flags, 1));
    m->free(m);
}

// Tests HOLDOVER_UPGRADE_ENABLE structure
Test(ProcTest, HOLDOVER_UPGRADE_ENABLE)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_HOLDOVER_UPGRADE_ENABLE_t t;
    t.flags = 1;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_HOLDOVER_UPGRADE_ENABLE, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_HOLDOVER_UPGRADE_ENABLE));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {1};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_HOLDOVER_UPGRADE_ENABLE_t *r =
        (const struct ptpmgmt_HOLDOVER_UPGRADE_ENABLE_t *)m->getData(m);
    cr_expect(eq(int, r->flags, 1));
    m->free(m);
}

// Tests EXT_PORT_CONFIG_PORT_DATA_SET structure
Test(ProcTest, EXT_PORT_CONFIG_PORT_DATA_SET)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_EXT_PORT_CONFIG_PORT_DATA_SET_t t;
    t.flags = 1;
    t.desiredState = PTPMGMT_PASSIVE;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_EXT_PORT_CONFIG_PORT_DATA_SET,
            &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_EXT_PORT_CONFIG_PORT_DATA_SET));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {1, 7};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_EXT_PORT_CONFIG_PORT_DATA_SET_t *r =
        (const struct ptpmgmt_EXT_PORT_CONFIG_PORT_DATA_SET_t *)m->getData(m);
    cr_expect(eq(int, r->flags, 1));
    cr_expect(eq(int, r->desiredState, PTPMGMT_PASSIVE));
    m->free(m);
}

// Tests TIME_STATUS_NP structure
Test(ProcTest, TIME_STATUS_NP)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[52] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 196, 125, 70, 255, 254, 32, 172, 174
        };
    cr_assert(eq(int, m->parse(m, buf, rsp(buf, 0xc000, mb, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_TIME_STATUS_NP));
    const struct ptpmgmt_TIME_STATUS_NP_t *r =
        (const struct ptpmgmt_TIME_STATUS_NP_t *)m->getData(m);
    cr_expect(eq(int, r->master_offset, 0));
    cr_expect(eq(int, r->ingress_time, 0));
    cr_expect(eq(int, r->cumulativeScaledRateOffset, 0));
    cr_expect(eq(int, r->scaledLastGmPhaseChange, 0));
    cr_expect(eq(int, r->gmTimeBaseIndicator, 0));
    cr_expect(eq(int, r->nanoseconds_msb, 0));
    cr_expect(eq(int, r->nanoseconds_lsb, 0));
    cr_expect(eq(int, r->fractional_nanoseconds, 0));
    cr_expect(eq(int, r->gmPresent, 0));
    cr_expect(zero(memcmp(r->gmIdentity.v, clockId, 8)));
    m->free(m);
}

// Tests GRANDMASTER_SETTINGS_NP structure
Test(ProcTest, GRANDMASTER_SETTINGS_NP)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_GRANDMASTER_SETTINGS_NP_t t;
    t.clockQuality.clockClass = 255;
    t.clockQuality.clockAccuracy = ptpmgmt_Accurate_Unknown;
    t.clockQuality.offsetScaledLogVariance = 0xffff;
    t.currentUtcOffset = 37;
    t.flags = PTPMGMT_F_PTP;
    t.timeSource = PTPMGMT_INTERNAL_OSCILLATOR;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_GRANDMASTER_SETTINGS_NP, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_GRANDMASTER_SETTINGS_NP));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[8] = {255, 254, 255, 255, 0, 37, 8, 160};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_GRANDMASTER_SETTINGS_NP_t *r =
        (const struct ptpmgmt_GRANDMASTER_SETTINGS_NP_t *)m->getData(m);
    cr_expect(eq(int, r->clockQuality.clockClass, 255));
    cr_expect(eq(int, r->clockQuality.clockAccuracy, ptpmgmt_Accurate_Unknown));
    cr_expect(eq(int, r->clockQuality.offsetScaledLogVariance, 0xffff));
    cr_expect(eq(int, r->currentUtcOffset, 37));
    cr_expect(eq(int, r->flags, PTPMGMT_F_PTP));
    cr_expect(eq(int, r->timeSource, PTPMGMT_INTERNAL_OSCILLATOR));
    m->free(m);
}

// Tests PORT_DATA_SET_NP structure
Test(ProcTest, PORT_DATA_SET_NP)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_PORT_DATA_SET_NP_t t;
    t.neighborPropDelayThresh = 20000000;
    t.asCapable = 1;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PORT_DATA_SET_NP, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PORT_DATA_SET_NP));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[8] = {1, 49, 45, 0, 0, 0, 0, 1};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_PORT_DATA_SET_NP_t *r =
        (const struct ptpmgmt_PORT_DATA_SET_NP_t *)m->getData(m);
    cr_expect(eq(int, r->neighborPropDelayThresh, 20000000));
    cr_expect(eq(int, r->asCapable, 1));
    m->free(m);
}

// Tests SUBSCRIBE_EVENTS_NP structure
Test(ProcTest, SUBSCRIBE_EVENTS_NP)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_SUBSCRIBE_EVENTS_NP_t t;
    t.duration = 0x1234;
    ptpmgmt_clearAll_lnp(&t);
    ptpmgmt_setEvent_lnp(&t, PTPMGMT_NOTIFY_PORT_STATE);
    ptpmgmt_setEvent_lnp(&t, PTPMGMT_NOTIFY_TIME_SYNC);
    ptpmgmt_setEvent_lnp(&t, PTPMGMT_NOTIFY_PARENT_DATA_SET);
    ptpmgmt_setEvent_lnp(&t, PTPMGMT_NOTIFY_CMLDS);
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_SUBSCRIBE_EVENTS_NP, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_SUBSCRIBE_EVENTS_NP));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[66] = {18, 52, 15};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_SUBSCRIBE_EVENTS_NP_t *r =
        (const struct ptpmgmt_SUBSCRIBE_EVENTS_NP_t *)m->getData(m);
    cr_expect(eq(int, r->duration, 0x1234));
    cr_expect(ptpmgmt_getEvent_lnp(r, PTPMGMT_NOTIFY_PORT_STATE));
    cr_expect(ptpmgmt_getEvent_lnp(r, PTPMGMT_NOTIFY_TIME_SYNC));
    cr_expect(ptpmgmt_getEvent_lnp(r, PTPMGMT_NOTIFY_PARENT_DATA_SET));
    cr_expect(ptpmgmt_getEvent_lnp(r, PTPMGMT_NOTIFY_CMLDS));
    m->free(m);
}

// Tests PORT_PROPERTIES_NP structure
Test(ProcTest, PORT_PROPERTIES_NP)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[22] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1, 4, 1, 7,
            101, 110, 112, 48, 115, 50, 53
        };
    cr_assert(eq(int, m->parse(m, buf, rsp(buf, 0xc004, mb, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_PORT_PROPERTIES_NP));
    const struct ptpmgmt_PORT_PROPERTIES_NP_t *r =
        (const struct ptpmgmt_PORT_PROPERTIES_NP_t *)m->getData(m);
    cr_expect(eq(u16, r->portIdentity.portNumber, 1));
    cr_expect(zero(memcmp(r->portIdentity.clockIdentity.v, clockId, 8)));
    cr_expect(eq(int, r->portState, PTPMGMT_LISTENING));
    cr_expect(eq(int, r->timestamping, PTPMGMT_TS_HARDWARE));
    cr_expect(eq(u8, r->interface.lengthField, 7));
    cr_expect(eq(str, (char *)r->interface.textField, "enp0s25"));
    m->free(m);
}

// Tests PORT_STATS_NP structure
Test(ProcTest, PORT_STATS_NP)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[266] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1};
    mb[154] = 114;
    mb[155] = 247;
    cr_assert(eq(int, m->parse(m, buf, rsp(buf, 0xc005, mb, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_PORT_STATS_NP));
    const struct ptpmgmt_PORT_STATS_NP_t *r =
        (const struct ptpmgmt_PORT_STATS_NP_t *)m->getData(m);
    cr_expect(eq(u16, r->portIdentity.portNumber, 1));
    cr_expect(zero(memcmp(r->portIdentity.clockIdentity.v, clockId, 8)));
    cr_expect(eq(int, r->rxMsgType[PTPMGMT_STAT_SYNC], 0));
    cr_expect(eq(int, r->rxMsgType[PTPMGMT_STAT_DELAY_REQ], 0));
    cr_expect(eq(int, r->rxMsgType[PTPMGMT_STAT_PDELAY_REQ], 0));
    cr_expect(eq(int, r->rxMsgType[PTPMGMT_STAT_PDELAY_RESP], 0));
    cr_expect(eq(int, r->rxMsgType[PTPMGMT_STAT_FOLLOW_UP], 0));
    cr_expect(eq(int, r->rxMsgType[PTPMGMT_STAT_DELAY_RESP], 0));
    cr_expect(eq(int, r->rxMsgType[PTPMGMT_STAT_PDELAY_RESP_FOLLOW_UP], 0));
    cr_expect(eq(int, r->rxMsgType[PTPMGMT_STAT_ANNOUNCE], 0));
    cr_expect(eq(int, r->rxMsgType[PTPMGMT_STAT_SIGNALING], 0));
    cr_expect(eq(int, r->rxMsgType[PTPMGMT_STAT_MANAGEMENT], 0));
    cr_expect(eq(int, r->txMsgType[PTPMGMT_STAT_SYNC], 0));
    cr_expect(eq(int, r->txMsgType[PTPMGMT_STAT_DELAY_REQ], 0));
    cr_expect(eq(int, r->txMsgType[PTPMGMT_STAT_PDELAY_REQ], 63346));
    cr_expect(eq(int, r->txMsgType[PTPMGMT_STAT_PDELAY_RESP], 0));
    cr_expect(eq(int, r->txMsgType[PTPMGMT_STAT_FOLLOW_UP], 0));
    cr_expect(eq(int, r->txMsgType[PTPMGMT_STAT_DELAY_RESP], 0));
    cr_expect(eq(int, r->txMsgType[PTPMGMT_STAT_PDELAY_RESP_FOLLOW_UP], 0));
    cr_expect(eq(int, r->txMsgType[PTPMGMT_STAT_ANNOUNCE], 0));
    cr_expect(eq(int, r->txMsgType[PTPMGMT_STAT_SIGNALING], 0));
    cr_expect(eq(int, r->txMsgType[PTPMGMT_STAT_MANAGEMENT], 0));
    m->free(m);
}

// Tests SYNCHRONIZATION_UNCERTAIN_NP structure
Test(ProcTest, SYNCHRONIZATION_UNCERTAIN_NP)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_SYNCHRONIZATION_UNCERTAIN_NP_t t;
    t.val = PTPMGMT_SYNC_UNCERTAIN_DONTCARE;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_SYNCHRONIZATION_UNCERTAIN_NP,
            &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_SYNCHRONIZATION_UNCERTAIN_NP));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[2] = {255};
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_SYNCHRONIZATION_UNCERTAIN_NP_t *r =
        (const struct ptpmgmt_SYNCHRONIZATION_UNCERTAIN_NP_t *)m->getData(m);
    cr_expect(eq(int, r->val, PTPMGMT_SYNC_UNCERTAIN_DONTCARE));
    m->free(m);
}

// Tests PORT_SERVICE_STATS_NP structure
Test(ProcTest, PORT_SERVICE_STATS_NP)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[92] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1, 81, 35, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 114, 247
        };
    cr_assert(eq(int, m->parse(m, buf, rsp(buf, 0xc007, mb, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_PORT_SERVICE_STATS_NP));
    const struct ptpmgmt_PORT_SERVICE_STATS_NP_t *r =
        (const struct ptpmgmt_PORT_SERVICE_STATS_NP_t *)m->getData(m);
    cr_expect(eq(u16, r->portIdentity.portNumber, 1));
    cr_expect(zero(memcmp(r->portIdentity.clockIdentity.v, clockId, 8)));
    cr_expect(eq(int, r->announce_timeout, 9041));
    cr_expect(eq(int, r->sync_timeout, 0));
    cr_expect(eq(int, r->delay_timeout, 63346));
    cr_expect(eq(int, r->unicast_service_timeout, 0));
    cr_expect(eq(int, r->unicast_request_timeout, 0));
    cr_expect(eq(int, r->master_announce_timeout, 0));
    cr_expect(eq(int, r->master_sync_timeout, 0));
    cr_expect(eq(int, r->qualification_timeout, 0));
    cr_expect(eq(int, r->sync_mismatch, 0));
    cr_expect(eq(int, r->followup_mismatch, 0));
    m->free(m);
}

// Tests UNICAST_MASTER_TABLE_NP structure
Test(ProcTest, UNICAST_MASTER_TABLE_NP)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[32] = {0, 1, 196, 125, 70, 255, 254, 32, 172, 174, 0, 1,
            255, 254, 255, 255, 1, 3, 126, 134,
            0, 3, 0, 6, 196, 125, 70, 32, 172, 174
        };
    cr_assert(eq(int, m->parse(m, buf, rsp(buf, 0xc008, mb, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_UNICAST_MASTER_TABLE_NP));
    const struct ptpmgmt_UNICAST_MASTER_TABLE_NP_t *r =
        (const struct ptpmgmt_UNICAST_MASTER_TABLE_NP_t *)m->getData(m);
    cr_expect(eq(int, r->actualTableSize, 1));
    cr_expect(eq(u16, r->unicastMasters[0].portIdentity.portNumber, 1));
    cr_expect(zero(memcmp(r->unicastMasters[0].portIdentity.clockIdentity.v,
                clockId, 8)));
    cr_expect(eq(int, r->unicastMasters[0].clockQuality.clockClass, 255));
    cr_expect(eq(int, r->unicastMasters[0].clockQuality.clockAccuracy,
            ptpmgmt_Accurate_Unknown));
    cr_expect(eq(int, r->unicastMasters[0].clockQuality.offsetScaledLogVariance,
            0xffff));
    cr_expect(eq(int, r->unicastMasters[0].selected, 1));
    cr_expect(eq(int, r->unicastMasters[0].portState, PTPMGMT_UC_HAVE_SYDY));
    cr_expect(eq(int, r->unicastMasters[0].priority1, 126));
    cr_expect(eq(int, r->unicastMasters[0].priority2, 134));
    cr_expect(eq(int, r->unicastMasters[0].portAddress.networkProtocol,
            ptpmgmt_IEEE_802_3));
    cr_expect(eq(u16, r->unicastMasters[0].portAddress.addressLength, 6));
    cr_expect(zero(memcmp(r->unicastMasters[0].portAddress.addressField,
                physicalAddress_v, 6)));
    m->free(m);
}

// Tests PORT_HWCLOCK_NP structure
Test(ProcTest, PORT_HWCLOCK_NP)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[18] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1, 0, 0, 0, 1};
    cr_assert(eq(int, m->parse(m, buf, rsp(buf, 0xc009, mb, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_PORT_HWCLOCK_NP));
    const struct ptpmgmt_PORT_HWCLOCK_NP_t *r =
        (const struct ptpmgmt_PORT_HWCLOCK_NP_t *)m->getData(m);
    cr_expect(eq(u16, r->portIdentity.portNumber, 1));
    cr_expect(zero(memcmp(r->portIdentity.clockIdentity.v, clockId, 8)));
    cr_expect(eq(int, r->phc_index, 1));
    cr_expect(eq(int, r->flags, 0));
    m->free(m);
}

// Tests POWER_PROFILE_SETTINGS_NP structure
Test(ProcTest, POWER_PROFILE_SETTINGS_NP)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    struct ptpmgmt_POWER_PROFILE_SETTINGS_NP_t t;
    t.version = PTPMGMT_IEEE_C37_238_VERSION_2011;
    t.grandmasterID = 56230;
    t.grandmasterTimeInaccuracy = 4124796349;
    t.networkTimeInaccuracy = 3655058877;
    t.totalTimeInaccuracy = 4223530875;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_POWER_PROFILE_SETTINGS_NP, &t));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_POWER_PROFILE_SETTINGS_NP));
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[16] = {0, 1, 219, 166, 245, 219, 101, 189, 217, 219,
            197, 189, 251, 189, 247, 123
        };
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc + sizeof mb));
    cr_expect(zero(memcmp(buf + tlvLoc, mb, sizeof mb)));
    cr_assert(eq(int, m->parse(m, buf, sizeMsg(buf, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    const struct ptpmgmt_POWER_PROFILE_SETTINGS_NP_t *r = (const struct
            ptpmgmt_POWER_PROFILE_SETTINGS_NP_t *)m->getData(m);
    cr_expect(eq(int, r->version, PTPMGMT_IEEE_C37_238_VERSION_2011));
    cr_expect(eq(int, r->grandmasterID, 56230));
    cr_expect(eq(int, r->grandmasterTimeInaccuracy, 4124796349));
    cr_expect(eq(int, r->networkTimeInaccuracy, 3655058877));
    cr_expect(eq(int, r->totalTimeInaccuracy, 4223530875));
    m->free(m);
}

// Tests CMLDS_INFO_NP structure
Test(ProcTest, CMLDS_INFO_NP)
{
    uint8_t buf[400];
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 1), PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(sz, m->getMsgLen(m), tlvLoc));
    uint8_t mb[18] = {0xdc, 0xf8, 0x72, 0x40, 0xdc, 0xd1, 0x23, 1, 0x41, 0x17,
            0x34, 0x45, 0, 0, 0, 1
        };
    cr_assert(eq(int, m->parse(m, buf, rsp(buf, 0xc00b, mb, sizeof mb)),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getTlvId(m), PTPMGMT_CMLDS_INFO_NP));
    const struct ptpmgmt_CMLDS_INFO_NP_t *r =
        (const struct ptpmgmt_CMLDS_INFO_NP_t *)m->getData(m);
    cr_expect(eq(i64, r->meanLinkDelay.scaledNanoseconds, 0xdcf87240dcd12301LL));
    cr_expect(eq(i32, r->scaledNeighborRateRatio, 0x41173445));
    cr_expect(eq(i32, r->as_capable, 1));
    m->free(m);
}
