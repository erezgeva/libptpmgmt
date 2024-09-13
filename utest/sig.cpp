/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Signals structures unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

#include "sig.h"
#include "msg.h"
#include "comp.h"

__PTPMGMT_NAMESPACE_USE;

class SigTest : public ::testing::Test, public Message
{
  protected:
    const ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    const PortIdentity_t portId = { clockId, 1 };
    uint8_t buf[400];
    size_t curLen;
    MsgParams a;
    void SetUp() override {
        // signaling = 36 header + 10 targetPortIdentity = 44
        curLen = 44;
        build(buf, sizeof buf, 1);
        buf[0] = (buf[0] & 0xf0) | Signaling; // messageType
        buf[32] = 5; // controlField
        a = getParams();
        a.rcvSignaling = true;
    }
    void addTlv(tlvType_e type, uint8_t *msg, size_t len) {
        uint16_t *cur = (uint16_t *)(buf + curLen);
        *cur++ = cpu_to_net16(type);
        *cur++ = cpu_to_net16(len);
        memcpy(cur, msg, len);
        curLen += len + 4;
    }
    MNG_PARSE_ERROR_e doParse() {
        // header.messageLength
        buf[2] = curLen >> 8;
        buf[3] = curLen & 0xff;
        return parse(buf, curLen);
    }
};

// Tests one management TLV
TEST_F(SigTest, OneManagementTlvs)
{
    uint8_t m[4] = {0x20, 5, 137}; // PRIORITY1 priority1 = 137
    addTlv(MANAGEMENT, m, sizeof m);
    a.filterSignaling = false;
    EXPECT_TRUE(updateParams(a));
    ASSERT_EQ(doParse(), MNG_PARSE_ERROR_SIG);
    EXPECT_TRUE(isLastMsgSig());
    EXPECT_EQ(getSigTlvsCount(), 1);
    ASSERT_EQ(getSigTlvType(0), MANAGEMENT);
    EXPECT_EQ(getSigMngTlvType(0), PRIORITY1);
    const PRIORITY1_t *p = dynamic_cast<const PRIORITY1_t *>(getSigMngTlv(0));
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->priority1, 137);
}

// Tests two management TLVs
TEST_F(SigTest, TwoManagementTlvs)
{
    uint8_t m1[4] = {0x20, 6, 119}; // PRIORITY2 priority2 = 119
    uint8_t m2[4] = {0x20, 7, 7};  // DOMAIN domainNumber = 7
    addTlv(MANAGEMENT, m1, sizeof m1);
    addTlv(MANAGEMENT, m2, sizeof m2);
    a.filterSignaling = false;
    EXPECT_TRUE(updateParams(a));
    ASSERT_EQ(doParse(), MNG_PARSE_ERROR_SIG);
    EXPECT_TRUE(isLastMsgSig());
    EXPECT_EQ(getSigTlvsCount(), 2);
    ASSERT_EQ(getSigTlvType(0), MANAGEMENT);
    EXPECT_EQ(getSigMngTlvType(0), PRIORITY2);
    const PRIORITY2_t *p1 = dynamic_cast<const PRIORITY2_t *>(getSigMngTlv(0));
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(p1->priority2, 119);
    ASSERT_EQ(getSigTlvType(1), MANAGEMENT);
    EXPECT_EQ(getSigMngTlvType(1), DOMAIN);
    const DOMAIN_t *p2 = dynamic_cast<const DOMAIN_t *>(getSigMngTlv(1));
    ASSERT_NE(p2, nullptr);
    EXPECT_EQ(p2->domainNumber, 7);
}

// Tests organization and two management TLVs
TEST_F(SigTest, OrgTwoManagementTlvs)
{
    uint8_t m1[4] = {0x20, 6, 119}; // PRIORITY2 priority2 = 119
    // ORGANIZATION_EXTENSION_PROPAGATE
    uint8_t m2[10] = {0x12, 0x34, 0x56, 7, 0x19, 0xa3, 0x17, 0x23, 0x75, 0x31};
    uint8_t m3[4] = {0x20, 7, 7};  // DOMAIN domainNumber = 7
    addTlv(MANAGEMENT, m1, sizeof m1);
    addTlv(ORGANIZATION_EXTENSION_PROPAGATE, m2, sizeof m2);
    addTlv(MANAGEMENT, m3, sizeof m3);
    a.filterSignaling = false;
    EXPECT_TRUE(updateParams(a));
    ASSERT_EQ(doParse(), MNG_PARSE_ERROR_SIG);
    EXPECT_TRUE(isLastMsgSig());
    EXPECT_EQ(getSigTlvsCount(), 3);
    ASSERT_EQ(getSigTlvType(0), MANAGEMENT);
    EXPECT_EQ(getSigMngTlvType(0), PRIORITY2);
    const PRIORITY2_t *p1 = dynamic_cast<const PRIORITY2_t *>(getSigMngTlv(0));
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(p1->priority2, 119);
    ASSERT_EQ(getSigTlvType(1), ORGANIZATION_EXTENSION_PROPAGATE);
    const ORGANIZATION_EXTENSION_t *p2 =
        dynamic_cast<const ORGANIZATION_EXTENSION_t *>(getSigTlv(1));
    ASSERT_NE(p2, nullptr);
    EXPECT_EQ(memcmp(p2->organizationId, "\x12\x34\x56", 3), 0);
    EXPECT_EQ(memcmp(p2->organizationSubType, "\x7\x19\xa3", 3), 0);
    EXPECT_EQ(p2->dataField, Binary("\x17\x23\x75\x31", 4));
    ASSERT_EQ(getSigTlvType(2), MANAGEMENT);
    EXPECT_EQ(getSigMngTlvType(2), DOMAIN);
    const DOMAIN_t *p3 = dynamic_cast<const DOMAIN_t *>(getSigMngTlv(2));
    ASSERT_NE(p3, nullptr);
    EXPECT_EQ(p3->domainNumber, 7);
}

// Tests loop two management TLV
static bool loopCheck(const Message &, tlvType_e tlvType, const BaseSigTlv *tlv)
{
    /**
     * Function return true to stop!
     * So we return false if all pass!
     */
    if(tlvType != MANAGEMENT || tlv == nullptr)
        return true;
    const MANAGEMENT_t *mng = dynamic_cast<const MANAGEMENT_t *>(tlv);
    if(mng == nullptr)
        return true;
    const BaseMngTlv *p = mng->tlvData.get();
    if(p == nullptr)
        return true;
    // printf("loopCheck %s-%s\n", Message::tlv2str_c(tlvType),
    //     Message::mng2str_c(mng->managementId));
    if(mng->managementId == PRIORITY1) {
        const PRIORITY1_t *p1 = dynamic_cast<const PRIORITY1_t *>(p);
        return p1 == nullptr || p1->priority1 != 137;
    } else if(mng->managementId == PRIORITY2) {
        const PRIORITY2_t *p2 = dynamic_cast<const PRIORITY2_t *>(p);
        return p2 == nullptr || p2->priority2 != 119;
    }
    return true;
}
TEST_F(SigTest, LoopTwoManagementTlvs)
{
    uint8_t m1[4] = {0x20, 5, 137}; // PRIORITY1 priority1 = 137
    uint8_t m2[4] = {0x20, 6, 119}; // PRIORITY2 priority2 = 119
    addTlv(MANAGEMENT, m1, sizeof m1);
    addTlv(MANAGEMENT, m2, sizeof m2);
    a.filterSignaling = false;
    EXPECT_TRUE(updateParams(a));
    ASSERT_EQ(doParse(), MNG_PARSE_ERROR_SIG);
    EXPECT_TRUE(isLastMsgSig());
    EXPECT_EQ(getSigTlvsCount(), 2);
    ASSERT_EQ(getSigTlvType(0), MANAGEMENT);
    EXPECT_EQ(getSigMngTlvType(0), PRIORITY1);
    const PRIORITY1_t *p1 = dynamic_cast<const PRIORITY1_t *>(getSigMngTlv(0));
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(p1->priority1, 137);
    EXPECT_EQ(getSigMngTlvType(1), PRIORITY2);
    const PRIORITY2_t *p2 = dynamic_cast<const PRIORITY2_t *>(getSigMngTlv(1));
    ASSERT_NE(p2, nullptr);
    EXPECT_EQ(p2->priority2, 119);
    size_t cnt = 0;
    EXPECT_FALSE(traversSigTlvs([&cnt](const Message & msg, tlvType_e tlvType,
    const BaseSigTlv * tlv) {
        bool ret = loopCheck(msg, tlvType, tlv);
        if(!ret)
            cnt++;
        return ret;
    }));
    EXPECT_EQ(cnt, 2);
}

// Tests all organization TLVs
TEST_F(SigTest, AllOrgTlvs)
{
    uint8_t m1[10] = {0x12, 0x34, 0x15, 7, 0xf3, 0xa3, 0x17, 3, 5, 1};
    uint8_t m2[10] = {0x23, 0x13, 0x27, 8, 7, 0xb2, 9, 7, 0xb5, 3};
    uint8_t m3[10] = {0x45, 2, 0x29, 9, 0x19, 0xc5, 3, 9, 0xd7, 5};
    addTlv(ORGANIZATION_EXTENSION, m1, sizeof m1);
    addTlv(ORGANIZATION_EXTENSION_PROPAGATE, m2, sizeof m2);
    addTlv(ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE, m3, sizeof m3);
    a.filterSignaling = false;
    EXPECT_TRUE(updateParams(a));
    ASSERT_EQ(doParse(), MNG_PARSE_ERROR_SIG);
    EXPECT_TRUE(isLastMsgSig());
    EXPECT_EQ(getSigTlvsCount(), 3);
    ASSERT_EQ(getSigTlvType(0), ORGANIZATION_EXTENSION);
    const ORGANIZATION_EXTENSION_t *p0 =
        dynamic_cast<const ORGANIZATION_EXTENSION_t *>(getSigTlv(0));
    ASSERT_NE(p0, nullptr);
    EXPECT_EQ(memcmp(p0->organizationId, "\x12\x34\x15", 3), 0);
    EXPECT_EQ(memcmp(p0->organizationSubType, "\x7\xf3\xa3", 3), 0);
    EXPECT_EQ(p0->dataField, Binary("\x17\x3\x05\x1", 4));
    ASSERT_EQ(getSigTlvType(1), ORGANIZATION_EXTENSION_PROPAGATE);
    const ORGANIZATION_EXTENSION_t *p1 =
        dynamic_cast<const ORGANIZATION_EXTENSION_t *>(getSigTlv(1));
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(memcmp(p1->organizationId, "\x23\x13\x27", 3), 0);
    EXPECT_EQ(memcmp(p1->organizationSubType, "\x8\x07\xb2", 3), 0);
    EXPECT_EQ(p1->dataField, Binary("\x09\x7\xb5\x3", 4));
    ASSERT_EQ(getSigTlvType(2), ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE);
    const ORGANIZATION_EXTENSION_t *p2 =
        dynamic_cast<const ORGANIZATION_EXTENSION_t *>(getSigTlv(2));
    ASSERT_NE(p2, nullptr);
    EXPECT_EQ(memcmp(p2->organizationId, "\x45\x02\x29", 3), 0);
    EXPECT_EQ(memcmp(p2->organizationSubType, "\x9\x19\xc5", 3), 0);
    EXPECT_EQ(p2->dataField, Binary("\x03\x9\xd7\x5", 4));
}

// Tests filter with organization TLVs
TEST_F(SigTest, FilterWithOrgTlvs)
{
    uint8_t m1[10] = {0x12, 0x34, 0x15, 7, 0xf3, 0xa3, 0x17, 3, 5, 1};
    uint8_t m2[10] = {0x23, 0x13, 0x27, 8, 7, 0xb2, 9, 7, 0xb5, 3};
    uint8_t m3[10] = {0x45, 2, 0x29, 9, 0x19, 0xc5, 3, 9, 0xd7, 5};
    addTlv(ORGANIZATION_EXTENSION, m1, sizeof m1);
    addTlv(ORGANIZATION_EXTENSION_PROPAGATE, m2, sizeof m2);
    addTlv(ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE, m3, sizeof m3);
    a.filterSignaling = true;
    a.allowSigTlv(ORGANIZATION_EXTENSION);
    a.allowSigTlv(ORGANIZATION_EXTENSION_PROPAGATE);
    a.allowSigTlv(ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE);
    EXPECT_TRUE(a.isSigTlv(ORGANIZATION_EXTENSION));
    EXPECT_TRUE(a.isSigTlv(ORGANIZATION_EXTENSION_PROPAGATE));
    EXPECT_TRUE(a.isSigTlv(ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE));
    EXPECT_EQ(a.countSigTlvs(), 3);
    // Filter the middle TLV :-)
    a.removeSigTlv(ORGANIZATION_EXTENSION_PROPAGATE);
    EXPECT_TRUE(a.isSigTlv(ORGANIZATION_EXTENSION));
    EXPECT_FALSE(a.isSigTlv(ORGANIZATION_EXTENSION_PROPAGATE));
    EXPECT_TRUE(a.isSigTlv(ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE));
    EXPECT_EQ(a.countSigTlvs(), 2);
    EXPECT_TRUE(updateParams(a));
    ASSERT_EQ(doParse(), MNG_PARSE_ERROR_SIG);
    EXPECT_TRUE(isLastMsgSig());
    EXPECT_EQ(getSigTlvsCount(), 2);
    ASSERT_EQ(getSigTlvType(0), ORGANIZATION_EXTENSION);
    const ORGANIZATION_EXTENSION_t *p0 =
        dynamic_cast<const ORGANIZATION_EXTENSION_t *>(getSigTlv(0));
    ASSERT_NE(p0, nullptr);
    EXPECT_EQ(memcmp(p0->organizationId, "\x12\x34\x15", 3), 0);
    EXPECT_EQ(memcmp(p0->organizationSubType, "\x7\xf3\xa3", 3), 0);
    EXPECT_EQ(p0->dataField, Binary("\x17\x3\x05\x1", 4));
    ASSERT_EQ(getSigTlvType(1), ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE);
    const ORGANIZATION_EXTENSION_t *p2 =
        dynamic_cast<const ORGANIZATION_EXTENSION_t *>(getSigTlv(1));
    ASSERT_NE(p2, nullptr);
    EXPECT_EQ(memcmp(p2->organizationId, "\x45\x02\x29", 3), 0);
    EXPECT_EQ(memcmp(p2->organizationSubType, "\x9\x19\xc5", 3), 0);
    EXPECT_EQ(p2->dataField, Binary("\x03\x9\xd7\x5", 4));
}

// Tests management error, alternate time offset,
//  layer 1 synchronization, port communication availability,
//  protocol address and cumulative rate ratio TLVs
TEST_F(SigTest, MngErrMoreTlvs)
{
    // PRIORITY1, WRONG_LENGTH, "test"
    uint8_t m0[18] = {0, 3, 0x20, 5, 0, 0, 0, 0, 4, 116, 101, 115, 116};
    addTlv(MANAGEMENT_ERROR_STATUS, m0, sizeof m0);
    uint8_t m1[22] = {4, 0x80, 0, 0x76, 0x5c, 0xbb, 0xcb, 0xe3, 0xd4, 0x12,
            0x57, 0x89, 0x19, 0x33, 0x24, 5, 97, 108, 116, 101, 114
        };
    addTlv(ALTERNATE_TIME_OFFSET_INDICATOR, m1, sizeof m1);
    uint8_t m2[2] = {15, 7};
    addTlv(L1_SYNC, m2, sizeof m2);
    uint8_t m3[2] = {15, 15};
    addTlv(PORT_COMMUNICATION_AVAILABILITY, m3, sizeof m3);
    uint8_t m4[8] = {0, 1, 0, 4, 0x12, 0x34, 0x56, 0x78};
    addTlv(PROTOCOL_ADDRESS, m4, sizeof m4);
    uint8_t m5[4] = {0x99, 0x1a, 0x11, 0xbd};
    addTlv(CUMULATIVE_RATE_RATIO, m5, sizeof m5);
    a.filterSignaling = false;
    EXPECT_TRUE(updateParams(a));
    ASSERT_EQ(doParse(), MNG_PARSE_ERROR_SIG);
    EXPECT_TRUE(isLastMsgSig());
    EXPECT_EQ(getSigTlvsCount(), 6);
    ASSERT_EQ(getSigTlvType(0), MANAGEMENT_ERROR_STATUS);
    const MANAGEMENT_ERROR_STATUS_t *p0 =
        dynamic_cast<const MANAGEMENT_ERROR_STATUS_t *>(getSigTlv(0));
    ASSERT_NE(p0, nullptr);
    EXPECT_EQ(p0->managementId, PRIORITY1);
    EXPECT_EQ(p0->managementErrorId, WRONG_LENGTH);
    EXPECT_EQ(p0->displayData.lengthField, 4);
    EXPECT_STREQ(p0->displayData.string(), "test");
    ASSERT_EQ(getSigTlvType(1), ALTERNATE_TIME_OFFSET_INDICATOR);
    const ALTERNATE_TIME_OFFSET_INDICATOR_t *p1 =
        dynamic_cast<const ALTERNATE_TIME_OFFSET_INDICATOR_t *>(getSigTlv(1));
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(p1->keyField, 4);
    EXPECT_EQ(p1->currentOffset, -2147453348);
    EXPECT_EQ(p1->jumpSeconds, -1144265772);
    EXPECT_EQ(p1->timeOfNextJump, 0x125789193324);
    EXPECT_EQ(p1->displayName.lengthField, 5);
    EXPECT_STREQ(p1->displayName.string(), "alter");
    ASSERT_EQ(getSigTlvType(2), L1_SYNC);
    const L1_SYNC_t *p2 =
        dynamic_cast<const L1_SYNC_t *>(getSigTlv(2));
    ASSERT_NE(p2, nullptr);
    EXPECT_EQ(p2->flags1, 15);
    EXPECT_EQ(p2->flags2, 7);
    ASSERT_EQ(getSigTlvType(3), PORT_COMMUNICATION_AVAILABILITY);
    const PORT_COMMUNICATION_AVAILABILITY_t *p3 =
        dynamic_cast<const PORT_COMMUNICATION_AVAILABILITY_t *>(getSigTlv(3));
    ASSERT_NE(p3, nullptr);
    EXPECT_EQ(p3->syncMessageAvailability, 15);
    EXPECT_EQ(p3->delayRespMessageAvailability, 15);
    ASSERT_EQ(getSigTlvType(4), PROTOCOL_ADDRESS);
    const PROTOCOL_ADDRESS_t *p4 =
        dynamic_cast<const PROTOCOL_ADDRESS_t *>(getSigTlv(4));
    ASSERT_NE(p4, nullptr);
    PortAddress_t portAddress = { UDP_IPv4, 4, {"\x12\x34\x56\x78", 4} };
    EXPECT_EQ(p4->portProtocolAddress, portAddress);
    ASSERT_EQ(getSigTlvType(5), CUMULATIVE_RATE_RATIO);
    const CUMULATIVE_RATE_RATIO_t *p5 =
        dynamic_cast<const CUMULATIVE_RATE_RATIO_t *>(getSigTlv(5));
    ASSERT_NE(p5, nullptr);
    EXPECT_EQ(p5->scaledCumulativeRateRatio, -1726344771);
}

// Tests vector TLVs
TEST_F(SigTest, VectorTlvs)
{
    uint8_t m0[16] = {196, 125, 70, 255, 254, 32, 172, 174, 5, 7, 9, 1,
            172, 201, 3, 45
        };
    addTlv(PATH_TRACE, m0, sizeof m0);
    uint8_t m1[78] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1,
            4, 0, 0, 0x90, 8, 0x20, 0x11, 0, 0x36, 0xf9, 0xdf, 0xb8,
            0x45, 0x38, 0xaf, 0xb7, 0x17, 0x94, 0xd2, 0xa1, 0x99, 0x1a, 0x11,
            0xbd, 0, 0x98, 0x41, 0, 2, 0x4e, 0x38, 0xd0, 0, 0,
            11, 0xc7, 0, 0x81, 4, 8, 0x22, 8, 0, 0, 0, 0,
            0x12, 0x43, 0x5b, 0x4a, 0xf4, 0xd4, 0x1e, 0x48, 0xbd, 0xde,
            0xfa, 0x5c, 0, 0x81, 0x90, 0x58, 0x24, 0x20, 0x38, 0x1a, 0, 0
        };
    addTlv(SLAVE_RX_SYNC_TIMING_DATA, m1, sizeof m1);
    uint8_t m2[56] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1, 7, 0,
            11, 0xe6, 0x81, 1, 0x14, 0, 2, 0x24, 4, 0, 0x81, 0x12, 0x14, 0, 2,
            0x20, 4, 0, 0xbe, 0xbd, 0xe0, 0,
            7, 3, 0x81, 0x12, 0x14, 0x50, 0xb0, 0x20, 4, 0, 0x98, 0x42, 0x14,
            0x50, 0xb0, 0x20, 4, 0, 0xbe, 0x95, 0x4e, 0xf0
        };
    addTlv(SLAVE_RX_SYNC_COMPUTED_DATA, m2, sizeof m2);
    uint8_t m3[36] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1, 9, 0,
            2, 0xf1, 0, 2, 9, 8, 2, 0x20,
            0x36, 0x61, 0x20, 0x10,
            9, 0xf3, 0, 0x20, 0, 0x90, 8, 0x40,
            0x36, 0x61, 0x6d, 0x7c
        };
    addTlv(SLAVE_TX_EVENT_TIMESTAMPS, m3, sizeof m3);
    a.filterSignaling = false;
    EXPECT_TRUE(updateParams(a));
    ASSERT_EQ(doParse(), MNG_PARSE_ERROR_SIG);
    EXPECT_TRUE(isLastMsgSig());
    EXPECT_EQ(getSigTlvsCount(), 4);
    ASSERT_EQ(getSigTlvType(0), PATH_TRACE);
    const PATH_TRACE_t *p0 =
        dynamic_cast<const PATH_TRACE_t *>(getSigTlv(0));
    ASSERT_NE(p0, nullptr);
    EXPECT_EQ(p0->pathSequence.size(), 2);
    EXPECT_EQ(p0->pathSequence[0], clockId);
    EXPECT_EQ(p0->pathSequence[1], ClockIdentity_t({5, 7, 9, 1, 172, 201, 3, 45}));
    ASSERT_EQ(getSigTlvType(1), SLAVE_RX_SYNC_TIMING_DATA);
    const SLAVE_RX_SYNC_TIMING_DATA_t *p1 =
        dynamic_cast<const SLAVE_RX_SYNC_TIMING_DATA_t *>(getSigTlv(1));
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(p1->syncSourcePortIdentity, portId);
    EXPECT_EQ(p1->list.size(), 2);
    EXPECT_EQ(p1->list[0].sequenceId, 1024);
    EXPECT_EQ(p1->list[0].syncOriginTimestamp,
        Timestamp_t(618611609856, 922345400));
    EXPECT_EQ(p1->list[0].totalCorrectionField.scaledNanoseconds,
        0x4538afb71794d2a1);
    EXPECT_EQ(p1->list[0].scaledCumulativeRateOffset, -1726344771);
    EXPECT_EQ(p1->list[0].syncEventIngressTimestamp,
        Timestamp_t(653925548622, 953155584));
    EXPECT_EQ(p1->list[1].sequenceId, 3015);
    EXPECT_EQ(p1->list[1].syncOriginTimestamp, (float_seconds)554118423048);
    EXPECT_EQ(p1->list[1].totalCorrectionField.scaledNanoseconds,
        0x12435b4af4d41e48);
    EXPECT_EQ(p1->list[1].scaledCumulativeRateOffset, -1109460388);
    EXPECT_EQ(p1->list[1].syncEventIngressTimestamp,
        Timestamp_t(556472476704, 941228032));
    ASSERT_EQ(getSigTlvType(2), SLAVE_RX_SYNC_COMPUTED_DATA);
    const SLAVE_RX_SYNC_COMPUTED_DATA_t *p2 =
        dynamic_cast<const SLAVE_RX_SYNC_COMPUTED_DATA_t *>(getSigTlv(2));
    ASSERT_NE(p2, nullptr);
    EXPECT_EQ(p2->sourcePortIdentity, portId);
    EXPECT_EQ(p2->flagsMask, 7);
    EXPECT_EQ(p2->list.size(), 2);
    EXPECT_EQ(p2->list[0].sequenceId, 3046);
    EXPECT_EQ(p2->list[0].offsetFromMaster.scaledNanoseconds, 0x8101140002240400);
    EXPECT_EQ(p2->list[0].meanPathDelay.scaledNanoseconds, 0x8112140002200400);
    EXPECT_EQ(p2->list[0].scaledNeighborRateRatio, -1094852608);
    EXPECT_EQ(p2->list[1].sequenceId, 1795);
    EXPECT_EQ(p2->list[1].offsetFromMaster.scaledNanoseconds, 0x81121450b0200400);
    EXPECT_EQ(p2->list[1].meanPathDelay.scaledNanoseconds, 0x98421450b0200400);
    EXPECT_EQ(p2->list[1].scaledNeighborRateRatio, -1097511184);
    ASSERT_EQ(getSigTlvType(3), SLAVE_TX_EVENT_TIMESTAMPS);
    const SLAVE_TX_EVENT_TIMESTAMPS_t *p3 =
        dynamic_cast<const SLAVE_TX_EVENT_TIMESTAMPS_t *>(getSigTlv(3));
    ASSERT_NE(p3, nullptr);
    EXPECT_EQ(p3->sourcePortIdentity, portId);
    EXPECT_EQ(p3->eventMessageType, Delay_Resp);
    EXPECT_EQ(p3->list.size(), 2);
    EXPECT_EQ(p3->list[0].sequenceId, 753);
    EXPECT_EQ(p3->list[0].eventEgressTimestamp, Timestamp_t(8741454368, 912334864));
    EXPECT_EQ(p3->list[1].sequenceId, 2547);
    EXPECT_EQ(p3->list[1].eventEgressTimestamp,
        Timestamp_t(137448392768, 912354684));
}

// Tests enhanced accuracy_metrics TLV
TEST_F(SigTest, AccuracyTlv)
{
    uint8_t m[88] = {171, 231, 0, 0, 0x80, 0x2c, 0x40, 0, 0x48, 0x10, 5, 0,
            64, 234, 32, 6, 102, 102, 102, 102, 0x91, 0x3c, 0x40, 0, 0x4e,
            0x80, 5, 0, 65, 0, 143, 2, 184, 81, 235, 133, 0x91, 0x60, 0, 0,
            0x4e, 0xB5, 5, 0, 65, 210, 102, 120, 12, 99, 231, 109, 0x91,
            0x60, 0, 0, 0x4e, 0x90, 0x90, 0x65, 65, 149, 197, 130, 9, 172,
            244, 31, 0x91, 0x6a, 0x54, 4, 0x4e, 0x94, 0x90, 0x65, 65, 199,
            136, 144, 30, 119, 223, 59
        };
    addTlv(ENHANCED_ACCURACY_METRICS, m, sizeof m);
    a.filterSignaling = false;
    EXPECT_TRUE(updateParams(a));
    ASSERT_EQ(doParse(), MNG_PARSE_ERROR_SIG);
    EXPECT_TRUE(isLastMsgSig());
    EXPECT_EQ(getSigTlvsCount(), 1);
    ASSERT_EQ(getSigTlvType(0), ENHANCED_ACCURACY_METRICS);
    const ENHANCED_ACCURACY_METRICS_t *p =
        dynamic_cast<const ENHANCED_ACCURACY_METRICS_t *>(getSigTlv(0));
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->bcHopCount, 171);
    EXPECT_EQ(p->tcHopCount, 231);
    EXPECT_EQ(p->maxGmInaccuracy.scaledNanoseconds, 9235827305783231744ul);
    EXPECT_EQ(p->varGmInaccuracy, 53504.2);
    EXPECT_EQ(p->maxTransientInaccuracy.scaledNanoseconds, 10465310004163380480ul);
    EXPECT_EQ(p->varTransientInaccuracy, 135648.34);
    EXPECT_EQ(p->maxDynamicInaccuracy.scaledNanoseconds, 10475372734584259840ul);
    EXPECT_EQ(p->varDynamicInaccuracy, 1234821169.561);
    EXPECT_EQ(p->maxStaticInstanceInaccuracy.scaledNanoseconds,
        10475372734581870693ul);
    EXPECT_EQ(p->varStaticInstanceInaccuracy, 91316354.4189);
    EXPECT_EQ(p->maxStaticMediumInaccuracy.scaledNanoseconds,
        10478279860505841765ul);
    EXPECT_EQ(p->varStaticMediumInaccuracy, 789651516.9365);
}

// Tests Linuxptp TLVs
TEST_F(SigTest, LinuxptpTlvs)
{
    uint8_t m[70] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1,
            8, 0x6e, 0x84, 0x10, 9, 0x42, 8, 0xa4, 0x39, 0x50, 0x44,
            0xd3, 0x48, 1, 0x20, 0x40, 0x10, 0x10, 0, 0, 0, 0x40, 8,
            0x42, 8, 0x80, 0x36, 0xa0, 0xf5, 2,
            0xc, 0x49, 0x80, 0x41, 0x21, 0x12, 8, 0xa4, 0x38, 0xde,
            0xa5, 0x3a, 0x8c, 0x42, 0xa2, 0x40, 0x10, 2, 0, 0, 0xc5,
            0, 8, 0x42, 8, 0xa4, 0x37, 6, 0x2c, 0xe2
        };
    addTlv(SLAVE_DELAY_TIMING_DATA_NP, m, sizeof m);
    a.filterSignaling = false;
    a.implementSpecific = noImplementSpecific;
    EXPECT_TRUE(updateParams(a));
    ASSERT_EQ(doParse(), MNG_PARSE_ERROR_SIG);
    EXPECT_TRUE(isLastMsgSig());
    EXPECT_EQ(getSigTlvsCount(), 0);
    a.implementSpecific = linuxptp;
    EXPECT_TRUE(updateParams(a));
    ASSERT_EQ(doParse(), MNG_PARSE_ERROR_SIG);
    EXPECT_TRUE(isLastMsgSig());
    EXPECT_EQ(getSigTlvsCount(), 1);
    ASSERT_EQ(getSigTlvType(0), SLAVE_DELAY_TIMING_DATA_NP);
    const SLAVE_DELAY_TIMING_DATA_NP_t *p =
        dynamic_cast<const SLAVE_DELAY_TIMING_DATA_NP_t *>(getSigTlv(0));
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(p->sourcePortIdentity, portId);
    EXPECT_EQ(p->list.size(), 2);
    EXPECT_EQ(p->list[0].sequenceId, 2158);
    EXPECT_EQ(p->list[0].delayOriginTimestamp,
        Timestamp_t(145204409665700, 961561811));
    EXPECT_EQ(p->list[0].totalCorrectionField.scaledNanoseconds,
        5188463705227001856l);
    EXPECT_EQ(p->list[0].delayResponseTimestamp,
        Timestamp_t(275016452224, 916518146));
    EXPECT_EQ(p->list[1].sequenceId, 3145);
    EXPECT_EQ(p->list[1].delayOriginTimestamp,
        Timestamp_t(141017216059556, 954115386));
    EXPECT_EQ(p->list[1].totalCorrectionField.scaledNanoseconds,
        10106818909802987520ul);
    EXPECT_EQ(p->list[1].delayResponseTimestamp,
        Timestamp_t(216603929217188, 923151586));
}

// Tests enhanced accuracy_metrics TLV
TEST(SMPTETest, SMPTE_Org)
{
    Message m;
    uint8_t b[100] = {0xd, 2, 0, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0x74, 0xda, 0x38, 0xff, 0xfe, 0xf6, 0x98, 0x5e, 0, 1, 0, 0, 4,
            0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 3,
            3, 3, 0, 0, 3, 0, 0x30, 0x68, 0x97, 0xe8, 0, 0, 1, 0, 0, 0, 0x1e, 0,
            0, 0, 1, 1
        };
    MsgParams a = m.getParams();
    a.rcvSMPTEOrg = 1;
    EXPECT_TRUE(m.updateParams(a));
    ASSERT_EQ(m.parse(b, sizeof b), MNG_PARSE_ERROR_SMPTE);
    ASSERT_TRUE(m.isLastMsgSMPTE());
    ASSERT_EQ(m.getReplyAction(), COMMAND);
    const BaseMngTlv *t = m.getData();
    ASSERT_NE(t, nullptr);
    SMPTE_ORGANIZATION_EXTENSION_t *o = (SMPTE_ORGANIZATION_EXTENSION_t *)t;
    ASSERT_NE(o, nullptr);
    ASSERT_EQ(o->size(), 48); // Fixed size of tlv
    ASSERT_EQ(memcmp(o->organizationId, "\x68\x97\xe8", 3), 0); //always SMPTE OUI
    ASSERT_EQ(memcmp(o->organizationSubType, "\x0\x0\x1", 3), 0); // SM TLV version
    ASSERT_EQ(o->defaultSystemFrameRate_numerator, 30);
    ASSERT_EQ(o->defaultSystemFrameRate_denominator, 1);
    ASSERT_EQ(o->masterLockingStatus, SMPTE_FREE_RUN);
    ASSERT_EQ(o->timeAddressFlags, 0);
    ASSERT_EQ(o->currentLocalOffset, 0);
    ASSERT_EQ(o->jumpSeconds, 0);
    ASSERT_EQ(o->timeOfNextJump, 0);
    ASSERT_EQ(o->timeOfNextJam, 0);
    ASSERT_EQ(o->timeOfPreviousJam, 0);
    ASSERT_EQ(o->previousJamLocalOffset, 0);
    ASSERT_EQ(o->daylightSaving, 0);
    ASSERT_EQ(o->leapSecondJump, 0);
}
