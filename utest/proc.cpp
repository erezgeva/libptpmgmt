/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief PTP management TLV structures unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

#include "msg.h"
#include "proc.h"
#include "comp.h"

__PTPMGMT_NAMESPACE_USE;

class ProcTest : public ::testing::Test, public Message
{
  protected:
    uint8_t buf[400];
    const size_t tlvLoc = 54;
    const Binary physicalAddress = Binary("\xc4\x7d\x46\x20\xac\xae", 6);
    const ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    size_t sizeMsg(size_t tlvLength, actionField_e act = RESPONSE) {
        size_t len = tlvLoc + tlvLength;
        // header.messageLength
        buf[2] = len >> 8;
        buf[3] = len & 0xff;
        buf[46] = act;
        return len;
    }
    size_t rsp(uint16_t mngId, uint8_t *tlv, size_t tlvLength,
        actionField_e act = RESPONSE) {
        size_t len = tlvLoc + tlvLength;
        // header.messageLength
        buf[2] = len >> 8;
        buf[3] = len & 0xff;
        buf[46] = act;
        uint16_t *ptr16 = (uint16_t *)(buf + 48);
        *ptr16++ = cpu_to_net16(MANAGEMENT);     // 48
        *ptr16++ = cpu_to_net16(tlvLength + 2);  // 50
        *ptr16++ = cpu_to_net16(mngId);          // 52
        memcpy(ptr16, tlv, tlvLength);           // 54
        return len;
    }
};

// Tests CLOCK_DESCRIPTION structure
TEST_F(ProcTest, CLOCK_DESCRIPTION)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[58] = {128, 0, 10, 73, 69, 69, 69, 32, 56, 48, 50, 46, 51, 0, 6,
            196, 125, 70, 32, 172, 174, 0, 3, 0, 6, 196, 125, 70, 32, 172, 174,
            0, 0, 0, 0, 2, 59, 59, 2, 59, 59, 7, 116, 101, 115, 116, 49, 50,
            51, 0, 27, 25, 0, 2
        };
    ASSERT_EQ(parse(buf, rsp(0x0001, m, sizeof m)), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), CLOCK_DESCRIPTION);
    const CLOCK_DESCRIPTION_t *r = (const CLOCK_DESCRIPTION_t *)getData();
    EXPECT_EQ(r->clockType, ordinaryClock);
    EXPECT_EQ(r->physicalLayerProtocol.lengthField, 10);
    EXPECT_STREQ(r->physicalLayerProtocol.string(), "IEEE 802.3");
    EXPECT_EQ(r->physicalAddressLength, 6);
    EXPECT_EQ(r->physicalAddress, physicalAddress);
    PortAddress_t portAddress = { IEEE_802_3, 6, physicalAddress };
    EXPECT_EQ(r->protocolAddress, portAddress);
    EXPECT_EQ(memcmp(r->manufacturerIdentity, "\x0\x0\x0", 3), 0);
    EXPECT_EQ(r->productDescription.lengthField, 2);
    EXPECT_STREQ(r->productDescription.string(), ";;");
    EXPECT_EQ(r->revisionData.lengthField, 2);
    EXPECT_STREQ(r->revisionData.string(), ";;");
    EXPECT_EQ(r->userDescription.lengthField, 7);
    EXPECT_STREQ(r->userDescription.string(), "test123");
    EXPECT_EQ(memcmp(r->profileIdentity, "\x0\x1b\x19\x0\x2\x0", 6), 0);
}

// Tests USER_DESCRIPTION structure
TEST_F(ProcTest, USER_DESCRIPTION)
{
    USER_DESCRIPTION_t t;
    t.userDescription.textField = "test123";
    EXPECT_TRUE(setAction(SET, USER_DESCRIPTION, &t));
    EXPECT_EQ(getBuildTlvId(), USER_DESCRIPTION);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc + 8);
    uint8_t m[8] = {7, 116, 101, 115, 116, 49, 50, 51};
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const USER_DESCRIPTION_t *r = (const USER_DESCRIPTION_t *)getData();
    EXPECT_EQ(r->userDescription.lengthField, 7);
    EXPECT_STREQ(r->userDescription.string(), "test123");
}

// Tests INITIALIZE structure
TEST_F(ProcTest, INITIALIZE)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[2] = {18, 52};
    ASSERT_EQ(parse(buf, rsp(0x0005, m, sizeof m, ACKNOWLEDGE)),
        MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), INITIALIZE);
    const INITIALIZE_t *r = (const INITIALIZE_t *)getData();
    EXPECT_EQ(r->initializationKey, 0x1234);
}

// Tests FAULT_LOG structure
TEST_F(ProcTest, FAULT_LOG)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[108] = {0, 2, 0, 50, 0, 0, 0, 0, 0, 9, 0, 10, 209, 136, 2, 7, 101,
            114, 114, 111, 114, 32, 49, 7, 116, 101, 115, 116, 49, 50, 51, 20,
            84, 104, 105, 115, 32, 105, 115, 32, 102, 105, 114, 115, 116, 32,
            114, 101, 99, 111, 114, 100, 0, 55, 0, 0, 0, 0, 6, 255, 0, 46, 16,
            248, 4, 7, 101, 114, 114, 111, 114, 32, 50, 7, 116, 101, 115, 116,
            51, 50, 49, 25, 84, 104, 105, 115, 32, 105, 115, 32, 116, 104, 101,
            32, 115, 101, 99, 111, 110, 100, 32, 114, 101, 99, 111, 114, 100
        };
    ASSERT_EQ(parse(buf, rsp(0x0006, m, sizeof m)), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), FAULT_LOG);
    const FAULT_LOG_t *r = (const FAULT_LOG_t *)getData();
    EXPECT_EQ(r->numberOfFaultRecords, 2);
    EXPECT_EQ(r->faultRecords[0].faultRecordLength, 50);
    EXPECT_EQ(r->faultRecords[0].faultTime, (float_seconds)9.000709);
    EXPECT_EQ(r->faultRecords[0].severityCode, F_Critical);
    EXPECT_EQ(r->faultRecords[0].faultName.lengthField, 7);
    EXPECT_STREQ(r->faultRecords[0].faultName.string(), "error 1");
    EXPECT_EQ(r->faultRecords[0].faultValue.lengthField, 7);
    EXPECT_STREQ(r->faultRecords[0].faultValue.string(), "test123");
    EXPECT_EQ(r->faultRecords[0].faultDescription.lengthField, 20);
    EXPECT_STREQ(r->faultRecords[0].faultDescription.string(),
        "This is first record");
    EXPECT_EQ(r->faultRecords[1].faultRecordLength, 55);
    EXPECT_EQ(r->faultRecords[1].faultTime, (float_seconds)1791.003019);
    EXPECT_EQ(r->faultRecords[1].severityCode, F_Warning);
    EXPECT_EQ(r->faultRecords[1].faultName.lengthField, 7);
    EXPECT_STREQ(r->faultRecords[1].faultName.string(), "error 2");
    EXPECT_EQ(r->faultRecords[1].faultValue.lengthField, 7);
    EXPECT_STREQ(r->faultRecords[1].faultValue.string(), "test321");
    EXPECT_EQ(r->faultRecords[1].faultDescription.lengthField, 25);
    EXPECT_STREQ(r->faultRecords[1].faultDescription.string(),
        "This is the second record");
}

// Tests DEFAULT_DATA_SET structure
TEST_F(ProcTest, DEFAULT_DATA_SET)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[22] = {3, 0, 0, 1, 153, 255, 254, 255, 255, 137,
            196, 125, 70, 255, 254, 32, 172, 174
        };
    ASSERT_EQ(parse(buf, rsp(0x2000, m, sizeof m)), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), DEFAULT_DATA_SET);
    const DEFAULT_DATA_SET_t *r = (const DEFAULT_DATA_SET_t *)getData();
    EXPECT_EQ(r->flags, 0x3);
    EXPECT_EQ(r->numberPorts, 1);
    EXPECT_EQ(r->priority1, 153);
    EXPECT_EQ(r->clockQuality.clockClass, 255);
    EXPECT_EQ(r->clockQuality.clockAccuracy, Accurate_Unknown);
    EXPECT_EQ(r->clockQuality.offsetScaledLogVariance, 0xffff);
    EXPECT_EQ(r->priority2, 137);
    EXPECT_EQ(r->clockIdentity, clockId);
    EXPECT_EQ(r->domainNumber, 0);
}

// Tests CURRENT_DATA_SET structure
TEST_F(ProcTest, CURRENT_DATA_SET)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[20] = {0x12, 0x34, 0x32, 0x10, 0x47, 0xab, 0xcd, 0x54, 0x12,
            0x85, 0x9, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1
        };
    ASSERT_EQ(parse(buf, rsp(0x2001, m, sizeof m)), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), CURRENT_DATA_SET);
    const CURRENT_DATA_SET_t *r = (const CURRENT_DATA_SET_t *)getData();
    EXPECT_EQ(r->stepsRemoved, 0x1234);
    EXPECT_EQ(r->offsetFromMaster.scaledNanoseconds, 0x321047abcd541285LL);
    EXPECT_EQ(r->meanPathDelay.scaledNanoseconds, 0x0906050403020100LL);
}

// Tests PARENT_DATA_SET structure
TEST_F(ProcTest, PARENT_DATA_SET)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[34] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 0, 0, 0, 255,
            255, 127, 255, 255, 255, 255, 255, 254, 255, 255, 255, 196, 125,
            70, 255, 254, 32, 172, 174
        };
    ASSERT_EQ(parse(buf, rsp(0x2002, m, sizeof m)), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), PARENT_DATA_SET);
    const PARENT_DATA_SET_t *r = (const PARENT_DATA_SET_t *)getData();
    PortIdentity_t portIdentity = { clockId, 0 };
    EXPECT_EQ(r->parentPortIdentity, portIdentity);
    EXPECT_EQ(r->flags, 0);
    EXPECT_EQ(r->observedParentOffsetScaledLogVariance, 0xffff);
    EXPECT_EQ(r->observedParentClockPhaseChangeRate, 0x7fffffff);
    EXPECT_EQ(r->grandmasterPriority1, 255);
    EXPECT_EQ(r->grandmasterClockQuality.clockClass, 255);
    EXPECT_EQ(r->grandmasterClockQuality.clockAccuracy, Accurate_Unknown);
    EXPECT_EQ(r->grandmasterClockQuality.offsetScaledLogVariance, 0xffff);
    EXPECT_EQ(r->grandmasterPriority2, 255);
    EXPECT_EQ(r->grandmasterIdentity, clockId);
}

// Tests TIME_PROPERTIES_DATA_SET structure
TEST_F(ProcTest, TIME_PROPERTIES_DATA_SET)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[6] = {0, 37, 8, 160};
    ASSERT_EQ(parse(buf, rsp(0x2003, m, sizeof m)), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), TIME_PROPERTIES_DATA_SET);
    const TIME_PROPERTIES_DATA_SET_t *r =
        (const TIME_PROPERTIES_DATA_SET_t *)getData();
    EXPECT_EQ(r->currentUtcOffset, 37);
    EXPECT_EQ(r->flags, F_PTP); // ptpTimescale bit
    EXPECT_EQ(r->timeSource, INTERNAL_OSCILLATOR);
}

// Tests PORT_DATA_SET structure
TEST_F(ProcTest, PORT_DATA_SET)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[28] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1, 4, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 1, 3, 0, 2, 0, 2
        };
    ASSERT_EQ(parse(buf, rsp(0x2004, m, sizeof m)), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), PORT_DATA_SET);
    const PORT_DATA_SET_t *r = (const PORT_DATA_SET_t *)getData();
    PortIdentity_t portIdentity = { clockId, 1 };
    EXPECT_EQ(r->portIdentity, portIdentity);
    EXPECT_EQ(r->portState, LISTENING);
    EXPECT_EQ(r->logMinDelayReqInterval, 0);
    EXPECT_EQ(r->peerMeanPathDelay.scaledNanoseconds, 0);
    EXPECT_EQ(r->logAnnounceInterval, 1);
    EXPECT_EQ(r->announceReceiptTimeout, 3);
    EXPECT_EQ(r->logSyncInterval, 0);
    EXPECT_EQ(r->delayMechanism, P2P);
    EXPECT_EQ(r->logMinPdelayReqInterval, 0);
    EXPECT_EQ(r->versionNumber, 2);
};

// Tests PRIORITY1 structure
TEST_F(ProcTest, PRIORITY1)
{
    PRIORITY1_t t;
    t.priority1 = 153;
    EXPECT_TRUE(setAction(SET, PRIORITY1, &t));
    EXPECT_EQ(getBuildTlvId(), PRIORITY1);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {153};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const PRIORITY1_t *r = (const PRIORITY1_t *)getData();
    EXPECT_EQ(r->priority1, 153);
}

// Tests PRIORITY2 structure
TEST_F(ProcTest, PRIORITY2)
{
    PRIORITY2_t t;
    t.priority2 = 137;
    EXPECT_TRUE(setAction(SET, PRIORITY2, &t));
    EXPECT_EQ(getBuildTlvId(), PRIORITY2);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {137};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const PRIORITY2_t *r = (const PRIORITY2_t *)getData();
    EXPECT_EQ(r->priority2, 137);
}

// Tests DOMAIN structure
TEST_F(ProcTest, DOMAIN)
{
    DOMAIN_t t;
    t.domainNumber = 7;
    EXPECT_TRUE(setAction(SET, DOMAIN, &t));
    EXPECT_EQ(getBuildTlvId(), DOMAIN);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {7};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const DOMAIN_t *r = (const DOMAIN_t *)getData();
    EXPECT_EQ(r->domainNumber, 7);
}

// Tests SLAVE_ONLY structure
TEST_F(ProcTest, SLAVE_ONLY)
{
    SLAVE_ONLY_t t;
    t.flags = 1;
    EXPECT_TRUE(setAction(SET, SLAVE_ONLY, &t));
    EXPECT_EQ(getBuildTlvId(), SLAVE_ONLY);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {1};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const SLAVE_ONLY_t *r = (const SLAVE_ONLY_t *)getData();
    EXPECT_EQ(r->flags, 1);
}

// Tests LOG_ANNOUNCE_INTERVAL structure
TEST_F(ProcTest, LOG_ANNOUNCE_INTERVAL)
{
    LOG_ANNOUNCE_INTERVAL_t t;
    t.logAnnounceInterval = 1;
    EXPECT_TRUE(setAction(SET, LOG_ANNOUNCE_INTERVAL, &t));
    EXPECT_EQ(getBuildTlvId(), LOG_ANNOUNCE_INTERVAL);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {1};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const LOG_ANNOUNCE_INTERVAL_t *r = (const LOG_ANNOUNCE_INTERVAL_t *)getData();
    EXPECT_EQ(r->logAnnounceInterval, 1);
}

// Tests ANNOUNCE_RECEIPT_TIMEOUT structure
TEST_F(ProcTest, ANNOUNCE_RECEIPT_TIMEOUT)
{
    ANNOUNCE_RECEIPT_TIMEOUT_t t;
    t.announceReceiptTimeout = 3;
    EXPECT_TRUE(setAction(SET, ANNOUNCE_RECEIPT_TIMEOUT, &t));
    EXPECT_EQ(getBuildTlvId(), ANNOUNCE_RECEIPT_TIMEOUT);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {3};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const ANNOUNCE_RECEIPT_TIMEOUT_t *r =
        (const ANNOUNCE_RECEIPT_TIMEOUT_t *)getData();
    EXPECT_EQ(r->announceReceiptTimeout, 3);
}

// Tests LOG_SYNC_INTERVAL structure
TEST_F(ProcTest, LOG_SYNC_INTERVAL)
{
    LOG_SYNC_INTERVAL_t t;
    t.logSyncInterval = 7;
    EXPECT_TRUE(setAction(SET, LOG_SYNC_INTERVAL, &t));
    EXPECT_EQ(getBuildTlvId(), LOG_SYNC_INTERVAL);
    EXPECT_EQ(build(buf, sizeof buf, 7), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {7};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const LOG_SYNC_INTERVAL_t *r = (const LOG_SYNC_INTERVAL_t *)getData();
    EXPECT_EQ(r->logSyncInterval, 7);
}

// Tests VERSION_NUMBER structure
TEST_F(ProcTest, VERSION_NUMBER)
{
    VERSION_NUMBER_t t;
    t.versionNumber = 2;
    EXPECT_TRUE(setAction(SET, VERSION_NUMBER, &t));
    EXPECT_EQ(getBuildTlvId(), VERSION_NUMBER);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {2};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const VERSION_NUMBER_t *r = (const VERSION_NUMBER_t *)getData();
    EXPECT_EQ(r->versionNumber, 2);
}

// Tests TIME structure
TEST_F(ProcTest, TIME)
{
    TIME_t t;
    t.currentTime = 13.15;
    EXPECT_TRUE(setAction(SET, TIME, &t));
    EXPECT_EQ(getBuildTlvId(), TIME);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[10] = {0, 0, 0, 0, 0, 13, 8, 240, 209, 128};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const TIME_t *r = (const TIME_t *)getData();
    ASSERT_NE(r, nullptr);
    EXPECT_EQ(r->currentTime, (float_seconds)13.15);
}

// Tests CLOCK_ACCURACY structure
TEST_F(ProcTest, CLOCK_ACCURACY)
{
    CLOCK_ACCURACY_t t;
    t.clockAccuracy = Accurate_Unknown;
    EXPECT_TRUE(setAction(SET, CLOCK_ACCURACY, &t));
    EXPECT_EQ(getBuildTlvId(), CLOCK_ACCURACY);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {254};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const CLOCK_ACCURACY_t *r = (const CLOCK_ACCURACY_t *)getData();
    EXPECT_EQ(r->clockAccuracy, Accurate_Unknown);
}

// Tests UTC_PROPERTIES structure
TEST_F(ProcTest, UTC_PROPERTIES)
{
    UTC_PROPERTIES_t t;
    t.currentUtcOffset = -0x5433;
    t.flags = 7;
    EXPECT_TRUE(setAction(SET, UTC_PROPERTIES, &t));
    EXPECT_EQ(getBuildTlvId(), UTC_PROPERTIES);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[4] = {171, 205, 7};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const UTC_PROPERTIES_t *r = (const UTC_PROPERTIES_t *)getData();
    EXPECT_EQ(r->currentUtcOffset, -0x5433);
    EXPECT_EQ(r->flags, 7);
}

// Tests TRACEABILITY_PROPERTIES structure
TEST_F(ProcTest, TRACEABILITY_PROPERTIES)
{
    TRACEABILITY_PROPERTIES_t t;
    t.flags = F_TTRA | F_FTRA;
    EXPECT_TRUE(setAction(SET, TRACEABILITY_PROPERTIES, &t));
    EXPECT_EQ(getBuildTlvId(), TRACEABILITY_PROPERTIES);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {48};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const TRACEABILITY_PROPERTIES_t *r =
        (const TRACEABILITY_PROPERTIES_t *)getData();
    EXPECT_EQ(r->flags, F_TTRA | F_FTRA);
}

// Tests TIMESCALE_PROPERTIES structure
TEST_F(ProcTest, TIMESCALE_PROPERTIES)
{
    TIMESCALE_PROPERTIES_t t;
    t.flags = F_PTP;
    t.timeSource = HAND_SET;
    EXPECT_TRUE(setAction(SET, TIMESCALE_PROPERTIES, &t));
    EXPECT_EQ(getBuildTlvId(), TIMESCALE_PROPERTIES);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {8, 96};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const TIMESCALE_PROPERTIES_t *r = (const TIMESCALE_PROPERTIES_t *)getData();
    EXPECT_EQ(r->flags, F_PTP);
    EXPECT_EQ(r->timeSource, HAND_SET);
}

// Tests UNICAST_NEGOTIATION_ENABLE structure
TEST_F(ProcTest, UNICAST_NEGOTIATION_ENABLE)
{
    UNICAST_NEGOTIATION_ENABLE_t t;
    t.flags = 1;
    EXPECT_TRUE(setAction(SET, UNICAST_NEGOTIATION_ENABLE, &t));
    EXPECT_EQ(getBuildTlvId(), UNICAST_NEGOTIATION_ENABLE);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {1};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const UNICAST_NEGOTIATION_ENABLE_t *r =
        (const UNICAST_NEGOTIATION_ENABLE_t *)getData();
    EXPECT_EQ(r->flags, 1);
}

// Tests PATH_TRACE_LIST structure
TEST_F(ProcTest, PATH_TRACE_LIST)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[18] = {0xc4, 0x7d, 0x46, 0xff, 0xfe, 0x20, 0xac, 0xae,
            12, 4, 19, 97, 11, 74, 12, 74
        };
    ASSERT_EQ(parse(buf, rsp(0x2015, m, sizeof m)), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), PATH_TRACE_LIST);
    const PATH_TRACE_LIST_t *r = (const PATH_TRACE_LIST_t *)getData();
    EXPECT_EQ(r->pathSequence[0], clockId);
    ClockIdentity_t c2 = {12, 4, 19, 97, 11, 74, 12, 74};
    EXPECT_EQ(r->pathSequence[1], c2);
}

// Tests PATH_TRACE_ENABLE structure
TEST_F(ProcTest, PATH_TRACE_ENABLE)
{
    PATH_TRACE_ENABLE_t t;
    t.flags = 1;
    EXPECT_TRUE(setAction(SET, PATH_TRACE_ENABLE, &t));
    EXPECT_EQ(getBuildTlvId(), PATH_TRACE_ENABLE);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {1};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const PATH_TRACE_ENABLE_t *r = (const PATH_TRACE_ENABLE_t *)getData();
    EXPECT_EQ(r->flags, 1);
}

// Tests GRANDMASTER_CLUSTER_TABLE structure
TEST_F(ProcTest, GRANDMASTER_CLUSTER_TABLE)
{
    GRANDMASTER_CLUSTER_TABLE_t t;
    t.logQueryInterval = -19;
    PortAddress_t portAddress0 = { IEEE_802_3, 6, physicalAddress };
    t.PortAddress.push_back(portAddress0);
    Binary ip("\x12\x34\x56\x78", 4);
    PortAddress_t portAddress1 = { UDP_IPv4, 4, ip };
    t.PortAddress.push_back(portAddress1);
    EXPECT_TRUE(setAction(SET, GRANDMASTER_CLUSTER_TABLE, &t));
    EXPECT_EQ(getBuildTlvId(), GRANDMASTER_CLUSTER_TABLE);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[20] = {237, 2,
            0, 3, 0, 6, 196, 125, 70, 32, 172, 174,
            0, 1, 0, 4, 0x12, 0x34, 0x56, 0x78
        };
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const GRANDMASTER_CLUSTER_TABLE_t *r =
        (const GRANDMASTER_CLUSTER_TABLE_t *)getData();
    EXPECT_EQ(r->logQueryInterval, -19);
    EXPECT_EQ(r->actualTableSize, 2);
    EXPECT_EQ(r->PortAddress[0], portAddress0);
    EXPECT_EQ(r->PortAddress[1], portAddress1);
}

// Tests UNICAST_MASTER_TABLE structure
TEST_F(ProcTest, UNICAST_MASTER_TABLE)
{
    UNICAST_MASTER_TABLE_t t;
    t.logQueryInterval = -19;
    PortAddress_t portAddress0 = { IEEE_802_3, 6, physicalAddress };
    t.PortAddress.push_back(portAddress0);
    Binary ip("\x12\x34\x56\x78", 4);
    PortAddress_t portAddress1 = { UDP_IPv4, 4, ip };
    t.PortAddress.push_back(portAddress1);
    EXPECT_TRUE(setAction(SET, UNICAST_MASTER_TABLE, &t));
    EXPECT_EQ(getBuildTlvId(), UNICAST_MASTER_TABLE);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[22] = {237, 0, 2,
            0, 3, 0, 6, 196, 125, 70, 32, 172, 174,
            0, 1, 0, 4, 0x12, 0x34, 0x56, 0x78
        };
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const UNICAST_MASTER_TABLE_t *r = (const UNICAST_MASTER_TABLE_t *)getData();
    EXPECT_EQ(r->logQueryInterval, -19);
    EXPECT_EQ(r->actualTableSize, 2);
    EXPECT_EQ(r->PortAddress[0], portAddress0);
    EXPECT_EQ(r->PortAddress[1], portAddress1);
}

// Tests UNICAST_MASTER_MAX_TABLE_SIZE structure
TEST_F(ProcTest, UNICAST_MASTER_MAX_TABLE_SIZE)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[2] = {33, 67};
    ASSERT_EQ(parse(buf, rsp(0x2019, m, sizeof m)), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), UNICAST_MASTER_MAX_TABLE_SIZE);
    const UNICAST_MASTER_MAX_TABLE_SIZE_t *r =
        (const UNICAST_MASTER_MAX_TABLE_SIZE_t *)getData();
    EXPECT_EQ(r->maxTableSize, 0x2143);
}

// Tests ACCEPTABLE_MASTER_TABLE structure
TEST_F(ProcTest, ACCEPTABLE_MASTER_TABLE)
{
    ACCEPTABLE_MASTER_TABLE_t t;
    PortIdentity_t a0 = { clockId, 1 };
    t.list.push_back({a0, 127});
    PortIdentity_t a1 = { { 9, 8, 7, 6, 5, 4, 1, 7}, 2 };
    t.list.push_back({a1, 111});
    EXPECT_TRUE(setAction(SET, ACCEPTABLE_MASTER_TABLE, &t));
    EXPECT_EQ(getBuildTlvId(), ACCEPTABLE_MASTER_TABLE);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[24] = {0, 2, 196, 125, 70, 255, 254, 32, 172, 174, 0, 1, 127,
            9, 8, 7, 6, 5, 4, 1, 7, 0, 2, 111
        };
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const ACCEPTABLE_MASTER_TABLE_t *r =
        (const ACCEPTABLE_MASTER_TABLE_t *)getData();
    EXPECT_EQ(r->actualTableSize, 2);
    EXPECT_EQ(r->list[0].acceptablePortIdentity, a0);
    EXPECT_EQ(r->list[0].alternatePriority1, 127);
    EXPECT_EQ(r->list[1].acceptablePortIdentity, a1);
    EXPECT_EQ(r->list[1].alternatePriority1, 111);
}

// Tests ACCEPTABLE_MASTER_TABLE_ENABLED structure
TEST_F(ProcTest, ACCEPTABLE_MASTER_TABLE_ENABLED)
{
    ACCEPTABLE_MASTER_TABLE_ENABLED_t t;
    t.flags = 1;
    EXPECT_TRUE(setAction(SET, ACCEPTABLE_MASTER_TABLE_ENABLED, &t));
    EXPECT_EQ(getBuildTlvId(), ACCEPTABLE_MASTER_TABLE_ENABLED);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {1};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const ACCEPTABLE_MASTER_TABLE_ENABLED_t *r =
        (const ACCEPTABLE_MASTER_TABLE_ENABLED_t *)getData();
    EXPECT_EQ(r->flags, 1);
}

// Tests ACCEPTABLE_MASTER_MAX_TABLE_SIZE structure
TEST_F(ProcTest, ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[] = {103, 186};
    ASSERT_EQ(parse(buf, rsp(0x201c, m, sizeof m)), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), ACCEPTABLE_MASTER_MAX_TABLE_SIZE);
    const ACCEPTABLE_MASTER_MAX_TABLE_SIZE_t *r =
        (const ACCEPTABLE_MASTER_MAX_TABLE_SIZE_t *)getData();
    EXPECT_EQ(r->maxTableSize, 0x67ba);
}

// Tests ALTERNATE_MASTER structure
TEST_F(ProcTest, ALTERNATE_MASTER)
{
    ALTERNATE_MASTER_t t;
    t.flags = 1;
    t.logAlternateMulticastSyncInterval = -17;
    t.numberOfAlternateMasters = 210;
    EXPECT_TRUE(setAction(SET, ALTERNATE_MASTER, &t));
    EXPECT_EQ(getBuildTlvId(), ALTERNATE_MASTER);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[4] = {1, 239, 210};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const ALTERNATE_MASTER_t *r = (const ALTERNATE_MASTER_t *)getData();
    EXPECT_EQ(r->flags, 1);
    EXPECT_EQ(r->logAlternateMulticastSyncInterval, -17);
    EXPECT_EQ(r->numberOfAlternateMasters, 210);
}

// Tests ALTERNATE_TIME_OFFSET_ENABLE structure
TEST_F(ProcTest, ALTERNATE_TIME_OFFSET_ENABLE)
{
    ALTERNATE_TIME_OFFSET_ENABLE_t t;
    t.keyField = 7;
    t.flags = 1;
    EXPECT_TRUE(setAction(SET, ALTERNATE_TIME_OFFSET_ENABLE, &t));
    EXPECT_EQ(getBuildTlvId(), ALTERNATE_TIME_OFFSET_ENABLE);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {7, 1};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const ALTERNATE_TIME_OFFSET_ENABLE_t *r =
        (const ALTERNATE_TIME_OFFSET_ENABLE_t *)getData();
    EXPECT_EQ(r->keyField, 7);
    EXPECT_EQ(r->flags, 1);
}

// Tests ALTERNATE_TIME_OFFSET_NAME structure
TEST_F(ProcTest, ALTERNATE_TIME_OFFSET_NAME)
{
    ALTERNATE_TIME_OFFSET_NAME_t t;
    t.keyField = 11;
    t.displayName.textField = "123";
    EXPECT_TRUE(setAction(SET, ALTERNATE_TIME_OFFSET_NAME, &t));
    EXPECT_EQ(getBuildTlvId(), ALTERNATE_TIME_OFFSET_NAME);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[6] = {11, 3, 49, 50, 51};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const ALTERNATE_TIME_OFFSET_NAME_t *r =
        (const ALTERNATE_TIME_OFFSET_NAME_t *)getData();
    EXPECT_EQ(r->keyField, 11);
    EXPECT_EQ(r->displayName.lengthField, 3);
    EXPECT_STREQ(r->displayName.string(), "123");
}

// Tests ALTERNATE_TIME_OFFSET_MAX_KEY structure
TEST_F(ProcTest, ALTERNATE_TIME_OFFSET_MAX_KEY)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[2] = {9};
    ASSERT_EQ(parse(buf, rsp(0x2020, m, sizeof m)), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), ALTERNATE_TIME_OFFSET_MAX_KEY);
    const ALTERNATE_TIME_OFFSET_MAX_KEY_t *r =
        (const ALTERNATE_TIME_OFFSET_MAX_KEY_t *)getData();
    EXPECT_EQ(r->maxKey, 9);
}

// Tests ALTERNATE_TIME_OFFSET_PROPERTIES structure
TEST_F(ProcTest, ALTERNATE_TIME_OFFSET_PROPERTIES)
{
    ALTERNATE_TIME_OFFSET_PROPERTIES_t t;
    t.keyField = 13;
    t.currentOffset = -2145493247;
    t.jumpSeconds = -2147413249;
    t.timeOfNextJump = 0x912478321891LL;
    EXPECT_TRUE(setAction(SET, ALTERNATE_TIME_OFFSET_PROPERTIES, &t));
    EXPECT_EQ(getBuildTlvId(), ALTERNATE_TIME_OFFSET_PROPERTIES);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[16] = {13, 128, 30, 95, 1, 128, 1, 18, 255, 145, 36,
            120, 50, 24, 145
        };
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const ALTERNATE_TIME_OFFSET_PROPERTIES_t *r =
        (const ALTERNATE_TIME_OFFSET_PROPERTIES_t *)getData();
    EXPECT_EQ(r->keyField, 13);
    EXPECT_EQ(r->currentOffset, -2145493247);
    EXPECT_EQ(r->jumpSeconds, -2147413249);
    EXPECT_EQ(r->timeOfNextJump, 0x912478321891);
}

// Tests TRANSPARENT_CLOCK_PORT_DATA_SET structure
TEST_F(ProcTest, TRANSPARENT_CLOCK_PORT_DATA_SET)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[20] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1,
            1, 235, 0xdc, 0xf8, 0x72, 0x40, 0xdc, 0xd1, 0x23, 1
        };
    ASSERT_EQ(parse(buf, rsp(0x4001, m, sizeof m)), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), TRANSPARENT_CLOCK_PORT_DATA_SET);
    const TRANSPARENT_CLOCK_PORT_DATA_SET_t *r =
        (const TRANSPARENT_CLOCK_PORT_DATA_SET_t *)getData();
    PortIdentity_t portIdentity = { clockId, 1 };
    EXPECT_EQ(r->portIdentity, portIdentity);
    EXPECT_EQ(r->flags, 1);
    EXPECT_EQ(r->logMinPdelayReqInterval, -21);
    EXPECT_EQ(r->peerMeanPathDelay.scaledNanoseconds, 0xdcf87240dcd12301LL);
}

// Tests LOG_MIN_PDELAY_REQ_INTERVAL structure
TEST_F(ProcTest, LOG_MIN_PDELAY_REQ_INTERVAL)
{
    LOG_MIN_PDELAY_REQ_INTERVAL_t t;
    t.logMinPdelayReqInterval = 9;
    EXPECT_TRUE(setAction(SET, LOG_MIN_PDELAY_REQ_INTERVAL, &t));
    EXPECT_EQ(getBuildTlvId(), LOG_MIN_PDELAY_REQ_INTERVAL);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {9};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const LOG_MIN_PDELAY_REQ_INTERVAL_t *r =
        (const LOG_MIN_PDELAY_REQ_INTERVAL_t *)getData();
    EXPECT_EQ(r->logMinPdelayReqInterval, 9);
}

// Tests TRANSPARENT_CLOCK_DEFAULT_DATA_SET structure
TEST_F(ProcTest, TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[14] = {196, 125, 70, 255, 254, 32, 172, 174, 23, 122, 254, 18};
    ASSERT_EQ(parse(buf, rsp(0x4000, m, sizeof m)), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), TRANSPARENT_CLOCK_DEFAULT_DATA_SET);
    const TRANSPARENT_CLOCK_DEFAULT_DATA_SET_t *r =
        (const TRANSPARENT_CLOCK_DEFAULT_DATA_SET_t *)getData();
    EXPECT_EQ(r->clockIdentity, clockId);
    EXPECT_EQ(r->numberPorts, 0x177a);
    EXPECT_EQ(r->delayMechanism, NO_MECHANISM);
    EXPECT_EQ(r->primaryDomain, 18);
}

// Tests PRIMARY_DOMAIN structure
TEST_F(ProcTest, PRIMARY_DOMAIN)
{
    PRIMARY_DOMAIN_t t;
    t.primaryDomain = 17;
    EXPECT_TRUE(setAction(SET, PRIMARY_DOMAIN, &t));
    EXPECT_EQ(getBuildTlvId(), PRIMARY_DOMAIN);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {17};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const PRIMARY_DOMAIN_t *r = (const PRIMARY_DOMAIN_t *)getData();
    EXPECT_EQ(r->primaryDomain, 17);
}

// Tests DELAY_MECHANISM structure
TEST_F(ProcTest, DELAY_MECHANISM)
{
    DELAY_MECHANISM_t t;
    t.delayMechanism = P2P;
    EXPECT_TRUE(setAction(SET, DELAY_MECHANISM, &t));
    EXPECT_EQ(getBuildTlvId(), DELAY_MECHANISM);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {2};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const DELAY_MECHANISM_t *r = (const DELAY_MECHANISM_t *)getData();
    EXPECT_EQ(r->delayMechanism, P2P);
}

// Tests EXTERNAL_PORT_CONFIGURATION_ENABLED structure
TEST_F(ProcTest, EXTERNAL_PORT_CONFIGURATION_ENABLED)
{
    EXTERNAL_PORT_CONFIGURATION_ENABLED_t t;
    t.flags = 1;
    EXPECT_TRUE(setAction(SET, EXTERNAL_PORT_CONFIGURATION_ENABLED, &t));
    EXPECT_EQ(getBuildTlvId(), EXTERNAL_PORT_CONFIGURATION_ENABLED);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {1};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const EXTERNAL_PORT_CONFIGURATION_ENABLED_t *r =
        (const EXTERNAL_PORT_CONFIGURATION_ENABLED_t *)getData();
    EXPECT_EQ(r->flags, 1);
}

// Tests MASTER_ONLY structure
TEST_F(ProcTest, MASTER_ONLY)
{
    MASTER_ONLY_t t;
    t.flags = 1;
    EXPECT_TRUE(setAction(SET, MASTER_ONLY, &t));
    EXPECT_EQ(getBuildTlvId(), MASTER_ONLY);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {1};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const MASTER_ONLY_t *r = (const MASTER_ONLY_t *)getData();
    EXPECT_EQ(r->flags, 1);
}

// Tests HOLDOVER_UPGRADE_ENABLE structure
TEST_F(ProcTest, HOLDOVER_UPGRADE_ENABLE)
{
    HOLDOVER_UPGRADE_ENABLE_t t;
    t.flags = 1;
    EXPECT_TRUE(setAction(SET, HOLDOVER_UPGRADE_ENABLE, &t));
    EXPECT_EQ(getBuildTlvId(), HOLDOVER_UPGRADE_ENABLE);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {1};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const HOLDOVER_UPGRADE_ENABLE_t *r =
        (const HOLDOVER_UPGRADE_ENABLE_t *)getData();
    EXPECT_EQ(r->flags, 1);
}

// Tests EXT_PORT_CONFIG_PORT_DATA_SET structure
TEST_F(ProcTest, EXT_PORT_CONFIG_PORT_DATA_SET)
{
    EXT_PORT_CONFIG_PORT_DATA_SET_t t;
    t.flags = 1;
    t.desiredState = PASSIVE;
    EXPECT_TRUE(setAction(SET, EXT_PORT_CONFIG_PORT_DATA_SET, &t));
    EXPECT_EQ(getBuildTlvId(), EXT_PORT_CONFIG_PORT_DATA_SET);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {1, 7};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const EXT_PORT_CONFIG_PORT_DATA_SET_t *r =
        (const EXT_PORT_CONFIG_PORT_DATA_SET_t *)getData();
    EXPECT_EQ(r->flags, 1);
    EXPECT_EQ(r->desiredState, PASSIVE);
}

// Tests TIME_STATUS_NP structure
TEST_F(ProcTest, TIME_STATUS_NP)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[52] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 196, 125, 70, 255, 254, 32, 172, 174
        };
    ASSERT_EQ(parse(buf, rsp(0xc000, m, sizeof m)), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), TIME_STATUS_NP);
    const TIME_STATUS_NP_t *r = (const TIME_STATUS_NP_t *)getData();
    EXPECT_EQ(r->master_offset, 0);
    EXPECT_EQ(r->ingress_time, 0);
    EXPECT_EQ(r->cumulativeScaledRateOffset, 0);
    EXPECT_EQ(r->scaledLastGmPhaseChange, 0);
    EXPECT_EQ(r->gmTimeBaseIndicator, 0);
    EXPECT_EQ(r->nanoseconds_msb, 0);
    EXPECT_EQ(r->nanoseconds_lsb, 0);
    EXPECT_EQ(r->fractional_nanoseconds, 0);
    EXPECT_EQ(r->gmPresent, 0);
    EXPECT_EQ(r->gmIdentity, clockId);
}

// Tests GRANDMASTER_SETTINGS_NP structure
TEST_F(ProcTest, GRANDMASTER_SETTINGS_NP)
{
    GRANDMASTER_SETTINGS_NP_t t;
    t.clockQuality.clockClass = 255;
    t.clockQuality.clockAccuracy = Accurate_Unknown;
    t.clockQuality.offsetScaledLogVariance = 0xffff;
    t.currentUtcOffset = 37;
    t.flags = F_PTP;
    t.timeSource = INTERNAL_OSCILLATOR;
    EXPECT_TRUE(setAction(SET, GRANDMASTER_SETTINGS_NP, &t));
    EXPECT_EQ(getBuildTlvId(), GRANDMASTER_SETTINGS_NP);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[8] = {255, 254, 255, 255, 0, 37, 8, 160};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const GRANDMASTER_SETTINGS_NP_t *r =
        (const GRANDMASTER_SETTINGS_NP_t *)getData();
    EXPECT_EQ(r->clockQuality.clockClass, 255);
    EXPECT_EQ(r->clockQuality.clockAccuracy, Accurate_Unknown);
    EXPECT_EQ(r->clockQuality.offsetScaledLogVariance, 0xffff);
    EXPECT_EQ(r->currentUtcOffset, 37);
    EXPECT_EQ(r->flags, F_PTP);
    EXPECT_EQ(r->timeSource, INTERNAL_OSCILLATOR);
}

// Tests PORT_DATA_SET_NP structure
TEST_F(ProcTest, PORT_DATA_SET_NP)
{
    PORT_DATA_SET_NP_t t;
    t.neighborPropDelayThresh = 20000000;
    t.asCapable = 1;
    EXPECT_TRUE(setAction(SET, PORT_DATA_SET_NP, &t));
    EXPECT_EQ(getBuildTlvId(), PORT_DATA_SET_NP);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[8] = {1, 49, 45, 0, 0, 0, 0, 1};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const PORT_DATA_SET_NP_t *r = (const PORT_DATA_SET_NP_t *)getData();
    EXPECT_EQ(r->neighborPropDelayThresh, 20000000);
    EXPECT_EQ(r->asCapable, 1);
}

// Tests SUBSCRIBE_EVENTS_NP structure
TEST_F(ProcTest, SUBSCRIBE_EVENTS_NP)
{
    SUBSCRIBE_EVENTS_NP_t t;
    t.duration = 0x1234;
    t.setEvent(NOTIFY_PORT_STATE);
    t.setEvent(NOTIFY_TIME_SYNC);
    t.setEvent(NOTIFY_PARENT_DATA_SET);
    t.setEvent(NOTIFY_CMLDS);
    EXPECT_TRUE(setAction(SET, SUBSCRIBE_EVENTS_NP, &t));
    EXPECT_EQ(getBuildTlvId(), SUBSCRIBE_EVENTS_NP);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[66] = {18, 52, 15};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const SUBSCRIBE_EVENTS_NP_t *r = (const SUBSCRIBE_EVENTS_NP_t *)getData();
    EXPECT_EQ(r->duration, 0x1234);
    EXPECT_TRUE(r->getEvent(NOTIFY_PORT_STATE));
    EXPECT_TRUE(r->getEvent(NOTIFY_TIME_SYNC));
    EXPECT_TRUE(r->getEvent(NOTIFY_PARENT_DATA_SET));
    EXPECT_TRUE(r->getEvent(NOTIFY_CMLDS));
}

// Tests PORT_PROPERTIES_NP structure
TEST_F(ProcTest, PORT_PROPERTIES_NP)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[22] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1, 4, 1, 7,
            101, 110, 112, 48, 115, 50, 53
        };
    ASSERT_EQ(parse(buf, rsp(0xc004, m, sizeof m)), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), PORT_PROPERTIES_NP);
    const PORT_PROPERTIES_NP_t *r = (const PORT_PROPERTIES_NP_t *)getData();
    PortIdentity_t portIdentity = { clockId, 1 };
    EXPECT_EQ(r->portIdentity, portIdentity);
    EXPECT_EQ(r->portState, LISTENING);
    EXPECT_EQ(r->timestamping, TS_HARDWARE);
    EXPECT_EQ(r->interface.lengthField, 7);
    EXPECT_STREQ(r->interface.string(), "enp0s25");
}

// Tests PORT_STATS_NP structure
TEST_F(ProcTest, PORT_STATS_NP)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[266] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1};
    m[154] = 114;
    m[155] = 247;
    ASSERT_EQ(parse(buf, rsp(0xc005, m, sizeof m)), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), PORT_STATS_NP);
    const PORT_STATS_NP_t *r = (const PORT_STATS_NP_t *)getData();
    PortIdentity_t portIdentity = { clockId, 1 };
    EXPECT_EQ(r->portIdentity, portIdentity);
    EXPECT_EQ(r->rxMsgType[STAT_SYNC], 0);
    EXPECT_EQ(r->rxMsgType[STAT_DELAY_REQ], 0);
    EXPECT_EQ(r->rxMsgType[STAT_PDELAY_REQ], 0);
    EXPECT_EQ(r->rxMsgType[STAT_PDELAY_RESP], 0);
    EXPECT_EQ(r->rxMsgType[STAT_FOLLOW_UP], 0);
    EXPECT_EQ(r->rxMsgType[STAT_DELAY_RESP], 0);
    EXPECT_EQ(r->rxMsgType[STAT_PDELAY_RESP_FOLLOW_UP], 0);
    EXPECT_EQ(r->rxMsgType[STAT_ANNOUNCE], 0);
    EXPECT_EQ(r->rxMsgType[STAT_SIGNALING], 0);
    EXPECT_EQ(r->rxMsgType[STAT_MANAGEMENT], 0);
    EXPECT_EQ(r->txMsgType[STAT_SYNC], 0);
    EXPECT_EQ(r->txMsgType[STAT_DELAY_REQ], 0);
    EXPECT_EQ(r->txMsgType[STAT_PDELAY_REQ], 63346);
    EXPECT_EQ(r->txMsgType[STAT_PDELAY_RESP], 0);
    EXPECT_EQ(r->txMsgType[STAT_FOLLOW_UP], 0);
    EXPECT_EQ(r->txMsgType[STAT_DELAY_RESP], 0);
    EXPECT_EQ(r->txMsgType[STAT_PDELAY_RESP_FOLLOW_UP], 0);
    EXPECT_EQ(r->txMsgType[STAT_ANNOUNCE], 0);
    EXPECT_EQ(r->txMsgType[STAT_SIGNALING], 0);
    EXPECT_EQ(r->txMsgType[STAT_MANAGEMENT], 0);
}

// Tests SYNCHRONIZATION_UNCERTAIN_NP structure
TEST_F(ProcTest, SYNCHRONIZATION_UNCERTAIN_NP)
{
    SYNCHRONIZATION_UNCERTAIN_NP_t t;
    t.val = SYNC_UNCERTAIN_DONTCARE;
    EXPECT_TRUE(setAction(SET, SYNCHRONIZATION_UNCERTAIN_NP, &t));
    EXPECT_EQ(getBuildTlvId(), SYNCHRONIZATION_UNCERTAIN_NP);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[2] = {255};
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const SYNCHRONIZATION_UNCERTAIN_NP_t *r =
        (const SYNCHRONIZATION_UNCERTAIN_NP_t *)getData();
    EXPECT_EQ(r->val, SYNC_UNCERTAIN_DONTCARE);
}

// Tests PORT_SERVICE_STATS_NP structure
TEST_F(ProcTest, PORT_SERVICE_STATS_NP)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[92] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1, 81, 35, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 114, 247
        };
    ASSERT_EQ(parse(buf, rsp(0xc007, m, sizeof m)), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), PORT_SERVICE_STATS_NP);
    const PORT_SERVICE_STATS_NP_t *r = (const PORT_SERVICE_STATS_NP_t *)getData();
    PortIdentity_t portIdentity = { clockId, 1 };
    EXPECT_EQ(r->portIdentity, portIdentity);
    EXPECT_EQ(r->announce_timeout, 9041);
    EXPECT_EQ(r->sync_timeout, 0);
    EXPECT_EQ(r->delay_timeout, 63346);
    EXPECT_EQ(r->unicast_service_timeout, 0);
    EXPECT_EQ(r->unicast_request_timeout, 0);
    EXPECT_EQ(r->master_announce_timeout, 0);
    EXPECT_EQ(r->master_sync_timeout, 0);
    EXPECT_EQ(r->qualification_timeout, 0);
    EXPECT_EQ(r->sync_mismatch, 0);
    EXPECT_EQ(r->followup_mismatch, 0);
}

// Tests UNICAST_MASTER_TABLE_NP structure
TEST_F(ProcTest, UNICAST_MASTER_TABLE_NP)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[32] = {0, 1, 196, 125, 70, 255, 254, 32, 172, 174, 0, 1,
            255, 254, 255, 255, 1, 3, 126, 134,
            0, 3, 0, 6, 196, 125, 70, 32, 172, 174
        };
    ASSERT_EQ(parse(buf, rsp(0xc008, m, sizeof m)), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), UNICAST_MASTER_TABLE_NP);
    const UNICAST_MASTER_TABLE_NP_t *r =
        (const UNICAST_MASTER_TABLE_NP_t *)getData();
    EXPECT_EQ(r->actualTableSize, 1);
    PortIdentity_t portIdentity = { clockId, 1 };
    EXPECT_EQ(r->unicastMasters[0].portIdentity, portIdentity);
    EXPECT_EQ(r->unicastMasters[0].clockQuality.clockClass, 255);
    EXPECT_EQ(r->unicastMasters[0].clockQuality.clockAccuracy, Accurate_Unknown);
    EXPECT_EQ(r->unicastMasters[0].clockQuality.offsetScaledLogVariance, 0xffff);
    EXPECT_EQ(r->unicastMasters[0].selected, 1);
    EXPECT_EQ(r->unicastMasters[0].portState, UC_HAVE_SYDY);
    EXPECT_EQ(r->unicastMasters[0].priority1, 126);
    EXPECT_EQ(r->unicastMasters[0].priority2, 134);
    PortAddress_t portAddress = { IEEE_802_3, 6, physicalAddress };
    EXPECT_EQ(r->unicastMasters[0].portAddress, portAddress);
}

// Tests PORT_HWCLOCK_NP structure
TEST_F(ProcTest, PORT_HWCLOCK_NP)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[18] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1, 0, 0, 0, 1};
    ASSERT_EQ(parse(buf, rsp(0xc009, m, sizeof m)), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), PORT_HWCLOCK_NP);
    const PORT_HWCLOCK_NP_t *r = (const PORT_HWCLOCK_NP_t *)getData();
    PortIdentity_t portIdentity = { clockId, 1 };
    EXPECT_EQ(r->portIdentity, portIdentity);
    EXPECT_EQ(r->phc_index, 1);
    EXPECT_EQ(r->flags, 0);
}

// Tests POWER_PROFILE_SETTINGS_NP structure
TEST_F(ProcTest, POWER_PROFILE_SETTINGS_NP)
{
    POWER_PROFILE_SETTINGS_NP_t t;
    t.version = IEEE_C37_238_VERSION_2011;
    t.grandmasterID = 56230;
    t.grandmasterTimeInaccuracy = 4124796349;
    t.networkTimeInaccuracy = 3655058877;
    t.totalTimeInaccuracy = 4223530875;
    EXPECT_TRUE(setAction(SET, POWER_PROFILE_SETTINGS_NP, &t));
    EXPECT_EQ(getBuildTlvId(), POWER_PROFILE_SETTINGS_NP);
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    uint8_t m[16] = {0, 1, 219, 166, 245, 219, 101, 189, 217, 219,
            197, 189, 251, 189, 247, 123
        };
    EXPECT_EQ(getMsgLen(), tlvLoc + sizeof m);
    EXPECT_EQ(memcmp(buf + tlvLoc, m, sizeof m), 0);
    ASSERT_EQ(parse(buf, sizeMsg(sizeof m)), MNG_PARSE_ERROR_OK);
    const POWER_PROFILE_SETTINGS_NP_t *r =
        (const POWER_PROFILE_SETTINGS_NP_t *)getData();
    EXPECT_EQ(r->version, IEEE_C37_238_VERSION_2011);
    EXPECT_EQ(r->grandmasterID, 56230);
    EXPECT_EQ(r->grandmasterTimeInaccuracy, 4124796349);
    EXPECT_EQ(r->networkTimeInaccuracy, 3655058877);
    EXPECT_EQ(r->totalTimeInaccuracy, 4223530875);
}

// Tests CMLDS_INFO_NP structure
TEST_F(ProcTest, CMLDS_INFO_NP)
{
    EXPECT_EQ(build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getMsgLen(), tlvLoc);
    uint8_t m[18] = {0xdc, 0xf8, 0x72, 0x40, 0xdc, 0xd1, 0x23, 1, 0x41, 0x17,
            0x34, 0x45, 0, 0, 0, 1
        };
    ASSERT_EQ(parse(buf, rsp(0xc00b, m, sizeof m)), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(getTlvId(), CMLDS_INFO_NP);
    const CMLDS_INFO_NP_t *r = (const CMLDS_INFO_NP_t *)getData();
    EXPECT_EQ(r->meanLinkDelay.scaledNanoseconds, 0xdcf87240dcd12301LL);
    EXPECT_EQ(r->scaledNeighborRateRatio, 0x41173445);
    EXPECT_EQ(r->as_capable, 1);
}
