/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief PTP message and TLV to JSON functions unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

#include "json.h"
#include "comp.h"

__PTPMGMT_NAMESPACE_USE;

// Test PTP message without TLV
TEST(Msg2JsonTest, Empty)
{
    uint8_t buf[60];
    Message m;
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(m.getMsgLen(), 54);
    buf[46] = RESPONSE;
    ASSERT_EQ(m.parse(buf, 54), MNG_PARSE_ERROR_OK);
    EXPECT_STREQ(msg2json(m).c_str(),
        "{\n"
        "  \"sequenceId\" : 1,\n"
        "  \"sdoId\" : 0,\n"
        "  \"domainNumber\" : 0,\n"
        "  \"versionPTP\" : 2,\n"
        "  \"minorVersionPTP\" : 0,\n"
        "  \"unicastFlag\" : true,\n"
        "  \"PTPProfileSpecific\" : 0,\n"
        "  \"messageType\" : \"Management\",\n"
        "  \"sourcePortIdentity\" :\n"
        "  {\n"
        "    \"clockIdentity\" : \"000000.0000.000000\",\n"
        "    \"portNumber\" : 0\n"
        "  },\n"
        "  \"targetPortIdentity\" :\n"
        "  {\n"
        "    \"clockIdentity\" : \"ffffff.ffff.ffffff\",\n"
        "    \"portNumber\" : 65535\n"
        "  },\n"
        "  \"actionField\" : \"RESPONSE\",\n"
        "  \"tlvType\" : \"MANAGEMENT\",\n"
        "  \"managementId\" : \"NULL_PTP_MANAGEMENT\"\n"
        "}");
    EXPECT_STREQ(msg2json(m, 3).c_str(),
        "   {\n"
        "     \"sequenceId\" : 1,\n"
        "     \"sdoId\" : 0,\n"
        "     \"domainNumber\" : 0,\n"
        "     \"versionPTP\" : 2,\n"
        "     \"minorVersionPTP\" : 0,\n"
        "     \"unicastFlag\" : true,\n"
        "     \"PTPProfileSpecific\" : 0,\n"
        "     \"messageType\" : \"Management\",\n"
        "     \"sourcePortIdentity\" :\n"
        "     {\n"
        "       \"clockIdentity\" : \"000000.0000.000000\",\n"
        "       \"portNumber\" : 0\n"
        "     },\n"
        "     \"targetPortIdentity\" :\n"
        "     {\n"
        "       \"clockIdentity\" : \"ffffff.ffff.ffffff\",\n"
        "       \"portNumber\" : 65535\n"
        "     },\n"
        "     \"actionField\" : \"RESPONSE\",\n"
        "     \"tlvType\" : \"MANAGEMENT\",\n"
        "     \"managementId\" : \"NULL_PTP_MANAGEMENT\"\n"
        "   }");
}

// Test PTP message with a managment TLV
TEST(Msg2JsonTest, MngTlv)
{
    uint8_t buf[70];
    Message m;
    USER_DESCRIPTION_t t;
    t.userDescription.textField = "test123";
    EXPECT_TRUE(m.setAction(SET, USER_DESCRIPTION, &t));
    EXPECT_EQ(m.getBuildTlvId(), USER_DESCRIPTION);
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(m.getMsgLen(), 62);
    buf[46] = RESPONSE;
    ASSERT_EQ(m.parse(buf, 62), MNG_PARSE_ERROR_OK);
    EXPECT_STREQ(msg2json(m).c_str(),
        "{\n"
        "  \"sequenceId\" : 1,\n"
        "  \"sdoId\" : 0,\n"
        "  \"domainNumber\" : 0,\n"
        "  \"versionPTP\" : 2,\n"
        "  \"minorVersionPTP\" : 0,\n"
        "  \"unicastFlag\" : true,\n"
        "  \"PTPProfileSpecific\" : 0,\n"
        "  \"messageType\" : \"Management\",\n"
        "  \"sourcePortIdentity\" :\n"
        "  {\n"
        "    \"clockIdentity\" : \"000000.0000.000000\",\n"
        "    \"portNumber\" : 0\n"
        "  },\n"
        "  \"targetPortIdentity\" :\n"
        "  {\n"
        "    \"clockIdentity\" : \"ffffff.ffff.ffffff\",\n"
        "    \"portNumber\" : 65535\n"
        "  },\n"
        "  \"actionField\" : \"RESPONSE\",\n"
        "  \"tlvType\" : \"MANAGEMENT\",\n"
        "  \"managementId\" : \"USER_DESCRIPTION\",\n"
        "  \"dataField\" :\n"
        "  {\n"
        "    \"userDescription\" : \"test123\"\n"
        "  }\n"
        "}");
}

// Test PTP message with a managment error TLV
TEST(Msg2JsonTest, MngErrTlv)
{
    Message m;
    uint8_t buf[80];
    EXPECT_TRUE(m.setAction(GET, PRIORITY1));
    EXPECT_EQ(m.getBuildTlvId(), PRIORITY1);
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    buf[46] = RESPONSE;
    // Create error managementTLV
    uint16_t *mt = (uint16_t *)(buf + 48);
    char displayData[] = "test 123";
    mt[0] = 0x0200; // cpu_to_net16(MANAGEMENT_ERROR_STATUS); // tlvType
    mt[1] = 0x1200; // length = 18
    mt[3] = mt[2]; // cp managementId from "MANAGEMENT TLV"
    mt[2] = 0x0400; // cpu_to_net16(WRONG_VALUE); // managementErrorId
    mt[4] = 0; // reserved
    mt[5] = 0; // reserved
    uint8_t *d = buf + 60; // displayData
    d[0] = 8; // displayData.lengthField
    buf[3] = 70; // header.messageLength
    memcpy(d + 1, displayData, 8); // displayData.textField
    EXPECT_EQ(m.parse(buf, 70), MNG_PARSE_ERROR_MSG);
    EXPECT_STREQ(msg2json(m).c_str(),
        "{\n"
        "  \"sequenceId\" : 1,\n"
        "  \"sdoId\" : 0,\n"
        "  \"domainNumber\" : 0,\n"
        "  \"versionPTP\" : 2,\n"
        "  \"minorVersionPTP\" : 0,\n"
        "  \"unicastFlag\" : true,\n"
        "  \"PTPProfileSpecific\" : 0,\n"
        "  \"messageType\" : \"Management\",\n"
        "  \"sourcePortIdentity\" :\n"
        "  {\n"
        "    \"clockIdentity\" : \"000000.0000.000000\",\n"
        "    \"portNumber\" : 0\n"
        "  },\n"
        "  \"targetPortIdentity\" :\n"
        "  {\n"
        "    \"clockIdentity\" : \"ffffff.ffff.ffffff\",\n"
        "    \"portNumber\" : 65535\n"
        "  },\n"
        "  \"actionField\" : \"RESPONSE\",\n"
        "  \"tlvType\" : \"MANAGEMENT_ERROR_STATUS\",\n"
        "  \"managementId\" : \"PRIORITY1\",\n"
        "  \"managementErrorId\" : \"WRONG_VALUE\",\n"
        "  \"displayData\" : \"test 123\"\n"
        "}");
}

// Test PTP signaling message
void addTlv(size_t &curLen, uint8_t *buf, tlvType_e type,
    uint8_t *tlv, size_t len)
{
    uint16_t *cur = (uint16_t *)(buf + curLen);
    *cur++ = cpu_to_net16(type);
    *cur++ = cpu_to_net16(len);
    memcpy(cur, tlv, len);
    curLen += len + 4;
}
TEST(Msg2JsonTest, Signaling)
{
    uint8_t buf[600];
    Message msg;
    EXPECT_EQ(msg.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(msg.getMsgLen(), 54);
    // signaling = 36 header + 10 targetPortIdentity = 44
    buf[0] = (buf[0] & 0xf0) | Signaling; // messageType
    buf[32] = 5; // controlField
    size_t curLen = 44;
    MsgParams a = msg.getParams();
    a.rcvSignaling = true;
    a.filterSignaling = false;
    EXPECT_TRUE(msg.updateParams(a));
    uint8_t m0[4] = {0x20, 6, 119}; // PRIORITY2 priority2 = 119
    addTlv(curLen, buf, MANAGEMENT, m0, sizeof m0);
    uint8_t m1[4] = {0x20, 7, 7};  // DOMAIN domainNumber = 7
    addTlv(curLen, buf, MANAGEMENT, m1, sizeof m1);
    uint8_t m2[10] = {0x12, 0x34, 0x15, 7, 0xf3, 0xa3, 0x17, 3, 5, 1};
    addTlv(curLen, buf, ORGANIZATION_EXTENSION, m2, sizeof m2);
    uint8_t m3[10] = {0x23, 0x13, 0x27, 8, 7, 0xb2, 9, 7, 0xb5, 3};
    addTlv(curLen, buf, ORGANIZATION_EXTENSION_PROPAGATE, m3, sizeof m3);
    uint8_t m4[10] = {0x45, 2, 0x29, 9, 0x19, 0xc5, 3, 9, 0xd7, 5};
    addTlv(curLen, buf, ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE, m4, sizeof m4);
    // PRIORITY1, WRONG_LENGTH, "test"
    uint8_t m5[18] = {0, 3, 0x20, 5, 0, 0, 0, 0, 4, 116, 101, 115, 116};
    addTlv(curLen, buf, MANAGEMENT_ERROR_STATUS, m5, sizeof m5);
    uint8_t m6[22] = {4, 0x80, 0, 0x76, 0x5c, 0xbb, 0xcb, 0xe3, 0xd4, 0x12,
            0x57, 0x89, 0x19, 0x33, 0x24, 5, 97, 108, 116, 101, 114
        };
    addTlv(curLen, buf, ALTERNATE_TIME_OFFSET_INDICATOR, m6, sizeof m6);
    uint8_t m7[2] = {15, 7};
    addTlv(curLen, buf, L1_SYNC, m7, sizeof m7);
    uint8_t m8[2] = {15, 15};
    addTlv(curLen, buf, PORT_COMMUNICATION_AVAILABILITY, m8, sizeof m8);
    uint8_t m9[8] = {0, 1, 0, 4, 0x12, 0x34, 0x56, 0x78};
    addTlv(curLen, buf, PROTOCOL_ADDRESS, m9, sizeof m9);
    uint8_t m10[4] = {0x99, 0x1a, 0x11, 0xbd};
    addTlv(curLen, buf, CUMULATIVE_RATE_RATIO, m10, sizeof m10);
    uint8_t m11[16] = {196, 125, 70, 255, 254, 32, 172, 174, 5, 7, 9, 1,
            172, 201, 3, 45
        };
    addTlv(curLen, buf, PATH_TRACE, m11, sizeof m11);
    uint8_t m12[78] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1,
            4, 0, 0, 0x90, 8, 0x20, 0x11, 0, 0x36, 0xf9, 0xdf, 0xb8,
            0x45, 0x38, 0xaf, 0xb7, 0x17, 0x94, 0xd2, 0xa1, 0x99, 0x1a, 0x11,
            0xbd, 0, 0x98, 0x41, 0, 2, 0x4e, 0x38, 0xd0, 0, 0,
            11, 0xc7, 0, 0x81, 4, 8, 0x22, 8, 0, 0, 0, 0,
            0x12, 0x43, 0x5b, 0x4a, 0xf4, 0xd4, 0x1e, 0x48, 0xbd, 0xde,
            0xfa, 0x5c, 0, 0x81, 0x90, 0x58, 0x24, 0x20, 0x38, 0x1a, 0, 0
        };
    addTlv(curLen, buf, SLAVE_RX_SYNC_TIMING_DATA, m12, sizeof m12);
    uint8_t m13[56] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1, 7, 0,
            11, 0xe6, 0x81, 1, 0x14, 0, 2, 0x24, 4, 0, 0x81, 0x12, 0x14, 0, 2,
            0x20, 4, 0, 0xbe, 0xbd, 0xe0, 0,
            7, 3, 0x81, 0x12, 0x14, 0x50, 0xb0, 0x20, 4, 0, 0x98, 0x42, 0x14,
            0x50, 0xb0, 0x20, 4, 0, 0xbe, 0x95, 0x4e, 0xf0
        };
    addTlv(curLen, buf, SLAVE_RX_SYNC_COMPUTED_DATA, m13, sizeof m13);
    uint8_t m14[36] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1, 9, 0,
            2, 0xf1, 0, 2, 9, 8, 2, 0x20,
            0x36, 0x61, 0x20, 0x10,
            9, 0xf3, 0, 0x20, 0, 0x90, 8, 0x40,
            0x36, 0x61, 0x6d, 0x7c
        };
    addTlv(curLen, buf, SLAVE_TX_EVENT_TIMESTAMPS, m14, sizeof m14);
    uint8_t m15[88] = {171, 231, 0, 0, 0x80, 0x2c, 0x40, 0, 0x48, 0x10, 5, 0,
            64, 234, 32, 6, 102, 102, 102, 102, 0x91, 0x3c, 0x40, 0, 0x4e,
            0x80, 5, 0, 65, 0, 143, 2, 184, 81, 235, 133, 0x91, 0x60, 0, 0,
            0x4e, 0xB5, 5, 0, 65, 210, 102, 120, 12, 99, 231, 109, 0x91,
            0x60, 0, 0, 0x4e, 0x90, 0x90, 0x65, 65, 149, 197, 130, 9, 172,
            244, 31, 0x91, 0x6a, 0x54, 4, 0x4e, 0x94, 0x90, 0x65, 65, 199,
            136, 144, 30, 119, 223, 59
        };
    addTlv(curLen, buf, ENHANCED_ACCURACY_METRICS, m15, sizeof m15);
    uint8_t m16[70] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1,
            8, 0x6e, 0x84, 0x10, 9, 0x42, 8, 0xa4, 0x39, 0x50, 0x44,
            0xd3, 0x48, 1, 0x20, 0x40, 0x10, 0x10, 0, 0, 0, 0x40, 8,
            0x42, 8, 0x80, 0x36, 0xa0, 0xf5, 2,
            0xc, 0x49, 0x80, 0x41, 0x21, 0x12, 8, 0xa4, 0x38, 0xde,
            0xa5, 0x3a, 0x8c, 0x42, 0xa2, 0x40, 0x10, 2, 0, 0, 0xc5,
            0, 8, 0x42, 8, 0xa4, 0x37, 6, 0x2c, 0xe2
        };
    addTlv(curLen, buf, SLAVE_DELAY_TIMING_DATA_NP, m16, sizeof m16);
    // header.messageLength
    buf[2] = curLen >> 8;
    buf[3] = curLen & 0xff;
    ASSERT_EQ(msg.parse(buf, curLen), MNG_PARSE_ERROR_SIG);
    EXPECT_STREQ(msg2json(msg).c_str(),
        "{\n"
        "  \"sequenceId\" : 1,\n"
        "  \"sdoId\" : 0,\n"
        "  \"domainNumber\" : 0,\n"
        "  \"versionPTP\" : 2,\n"
        "  \"minorVersionPTP\" : 0,\n"
        "  \"unicastFlag\" : true,\n"
        "  \"PTPProfileSpecific\" : 0,\n"
        "  \"messageType\" : \"Signaling\",\n"
        "  \"sourcePortIdentity\" :\n"
        "  {\n"
        "    \"clockIdentity\" : \"000000.0000.000000\",\n"
        "    \"portNumber\" : 0\n"
        "  },\n"
        "  \"targetPortIdentity\" :\n"
        "  {\n"
        "    \"clockIdentity\" : \"ffffff.ffff.ffffff\",\n"
        "    \"portNumber\" : 65535\n"
        "  },\n"
        "  \"TLVs\" :\n"
        "  [\n"
        "    {\n"
        "      \"tlvType\" : \"MANAGEMENT\",\n"
        "      \"managementId\" : \"PRIORITY2\",\n"
        "      \"dataField\" :\n"
        "      {\n"
        "        \"priority2\" : 119\n"
        "      }\n"
        "    },\n"
        "    {\n"
        "      \"tlvType\" : \"MANAGEMENT\",\n"
        "      \"managementId\" : \"DOMAIN\",\n"
        "      \"dataField\" :\n"
        "      {\n"
        "        \"domainNumber\" : 7\n"
        "      }\n"
        "    },\n"
        "    {\n"
        "      \"tlvType\" : \"ORGANIZATION_EXTENSION\",\n"
        "      \"organizationId\" : \"12:34:15\",\n"
        "      \"organizationSubType\" : \"07:f3:a3\",\n"
        "      \"dataField\" : \"17:03:05:01\"\n"
        "    },\n"
        "    {\n"
        "      \"tlvType\" : \"ORGANIZATION_EXTENSION_PROPAGATE\",\n"
        "      \"organizationId\" : \"23:13:27\",\n"
        "      \"organizationSubType\" : \"08:07:b2\",\n"
        "      \"dataField\" : \"09:07:b5:03\"\n"
        "    },\n"
        "    {\n"
        "      \"tlvType\" : \"ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE\",\n"
        "      \"organizationId\" : \"45:02:29\",\n"
        "      \"organizationSubType\" : \"09:19:c5\",\n"
        "      \"dataField\" : \"03:09:d7:05\"\n"
        "    },\n"
        "    {\n"
        "      \"tlvType\" : \"MANAGEMENT_ERROR_STATUS\",\n"
        "      \"managementId\" : \"PRIORITY1\",\n"
        "      \"managementErrorId\" : \"WRONG_LENGTH\",\n"
        "      \"displayData\" : \"test\"\n"
        "    },\n"
        "    {\n"
        "      \"tlvType\" : \"ALTERNATE_TIME_OFFSET_INDICATOR\",\n"
        "      \"keyField\" : 4,\n"
        "      \"currentOffset\" : -2147453348,\n"
        "      \"jumpSeconds\" : -1144265772,\n"
        "      \"timeOfNextJump\" : 20167171584804,\n"
        "      \"displayName\" : \"alter\"\n"
        "    },\n"
        "    {\n"
        "      \"tlvType\" : \"L1_SYNC\",\n"
        "      \"txCoherentIsRequired\" : true,\n"
        "      \"rxCoherentIsRequired\" : true,\n"
        "      \"congruentIsRequired\" : true,\n"
        "      \"optParamsEnabled\" : true,\n"
        "      \"isTxCoherent\" : true,\n"
        "      \"isRxCoherent\" : true,\n"
        "      \"isCongruent\" : true\n"
        "    },\n"
        "    {\n"
        "      \"tlvType\" : \"PORT_COMMUNICATION_AVAILABILITY\",\n"
        "      \"syncMessageAvailability\" :\n"
        "      {\n"
        "        \"multicastCapable\" : true,\n"
        "        \"unicastCapable\" : true,\n"
        "        \"unicastNegotiationCapableEnable\" : true,\n"
        "        \"unicastNegotiationCapable\" : true\n"
        "      },\n"
        "      \"delayRespMessageAvailability\" :\n"
        "      {\n"
        "        \"multicastCapable\" : true,\n"
        "        \"unicastCapable\" : true,\n"
        "        \"unicastNegotiationCapableEnable\" : true,\n"
        "        \"unicastNegotiationCapable\" : true\n"
        "      }\n"
        "    },\n"
        "    {\n"
        "      \"tlvType\" : \"PROTOCOL_ADDRESS\",\n"
        "      \"portProtocolAddress\" :\n"
        "      {\n"
        "        \"networkProtocol\" : \"UDP_IPv4\",\n"
        "        \"addressField\" : \"12:34:56:78\"\n"
        "      }\n"
        "    },\n"
        "    {\n"
        "      \"tlvType\" : \"CUMULATIVE_RATE_RATIO\",\n"
        "      \"scaledCumulativeRateRatio\" : -1726344771\n"
        "    },\n"
        "    {\n"
        "      \"tlvType\" : \"PATH_TRACE\",\n"
        "      \"pathSequence\" :\n"
        "      [\n"
        "        \"c47d46.fffe.20acae\",\n"
        "        \"050709.01ac.c9032d\"\n"
        "      ]\n"
        "    },\n"
        "    {\n"
        "      \"tlvType\" : \"SLAVE_RX_SYNC_TIMING_DATA\",\n"
        "      \"syncSourcePortIdentity\" :\n"
        "      {\n"
        "        \"clockIdentity\" : \"c47d46.fffe.20acae\",\n"
        "        \"portNumber\" : 1\n"
        "      },\n"
        "      \"list\" :\n"
        "      [\n"
        "        {\n"
        "          \"sequenceId\" : 1024,\n"
        "          \"syncOriginTimestamp\" : 618611609856.922345400,\n"
        "          \"totalCorrectionField\" : 4987929788222329505,\n"
        "          \"scaledCumulativeRateOffset\" : -1726344771,\n"
        "          \"syncEventIngressTimestamp\" : 653925548622.953155584\n"
        "        },\n"
        "        {\n"
        "          \"sequenceId\" : 3015,\n"
        "          \"syncOriginTimestamp\" : 554118423048.000000000,\n"
        "          \"totalCorrectionField\" : 1315995893615566408,\n"
        "          \"scaledCumulativeRateOffset\" : -1109460388,\n"
        "          \"syncEventIngressTimestamp\" : 556472476704.941228032\n"
        "        }\n"
        "      ]\n"
        "    },\n"
        "    {\n"
        "      \"tlvType\" : \"SLAVE_RX_SYNC_COMPUTED_DATA\",\n"
        "      \"sourcePortIdentity\" :\n"
        "      {\n"
        "        \"clockIdentity\" : \"c47d46.fffe.20acae\",\n"
        "        \"portNumber\" : 1\n"
        "      },\n"
        "      \"scaledNeighborRateRatioValid\" : true,\n"
        "      \"meanPathDelayValid\" : true,\n"
        "      \"offsetFromMasterValid\" : true,\n"
        "      \"list\" :\n"
        "      [\n"
        "        {\n"
        "          \"sequenceId\" : 3046,\n"
        "          \"offsetFromMaster\" : -9151010977571666944,\n"
        "          \"meanPathDelay\" : -9146225902967847936,\n"
        "          \"scaledNeighborRateRatio\" : -1094852608\n"
        "        },\n"
        "        {\n"
        "          \"sequenceId\" : 1795,\n"
        "          \"offsetFromMaster\" : -9146225556451228672,\n"
        "          \"meanPathDelay\" : -7475390094696774656,\n"
        "          \"scaledNeighborRateRatio\" : -1097511184\n"
        "        }\n"
        "      ]\n"
        "    },\n"
        "    {\n"
        "      \"tlvType\" : \"SLAVE_TX_EVENT_TIMESTAMPS\",\n"
        "      \"sourcePortIdentity\" :\n"
        "      {\n"
        "        \"clockIdentity\" : \"c47d46.fffe.20acae\",\n"
        "        \"portNumber\" : 1\n"
        "      },\n"
        "      \"eventMessageType\" : \"Delay_Resp\",\n"
        "      \"list\" :\n"
        "      [\n"
        "        {\n"
        "          \"sequenceId\" : 753,\n"
        "          \"eventEgressTimestamp\" : 8741454368.912334864\n"
        "        },\n"
        "        {\n"
        "          \"sequenceId\" : 2547,\n"
        "          \"eventEgressTimestamp\" : 137448392768.912354684\n"
        "        }\n"
        "      ]\n"
        "    },\n"
        "    {\n"
        "      \"tlvType\" : \"ENHANCED_ACCURACY_METRICS\",\n"
        "      \"bcHopCount\" : 171,\n"
        "      \"tcHopCount\" : 231,\n"
        "      \"maxGmInaccuracy\" : -9210916767926319872,\n"
        "      \"varGmInaccuracy\" : 53504.200000,\n"
        "      \"maxTransientInaccuracy\" : -7981434069546171136,\n"
        "      \"varTransientInaccuracy\" : 135648.340000,\n"
        "      \"maxDynamicInaccuracy\" : -7971371339125291776,\n"
        "      \"varDynamicInaccuracy\" : 1234821169.561000,\n"
        "      \"maxStaticInstanceInaccuracy\" : -7971371339127680923,\n"
        "      \"varStaticInstanceInaccuracy\" : 91316354.418900,\n"
        "      \"maxStaticMediumInaccuracy\" : -7968464213203709851,\n"
        "      \"varStaticMediumInaccuracy\" : 789651516.936500\n"
        "    },\n"
        "    {\n"
        "      \"tlvType\" : \"SLAVE_DELAY_TIMING_DATA_NP\",\n"
        "      \"sourcePortIdentity\" :\n"
        "      {\n"
        "        \"clockIdentity\" : \"c47d46.fffe.20acae\",\n"
        "        \"portNumber\" : 1\n"
        "      },\n"
        "      \"list\" :\n"
        "      [\n"
        "        {\n"
        "          \"sequenceId\" : 2158,\n"
        "          \"delayOriginTimestamp\" : 145204409665700.961561811,\n"
        "          \"totalCorrectionField\" : 5188463705227001856,\n"
        "          \"delayResponseTimestamp\" : 275016452224.916518146\n"
        "        },\n"
        "        {\n"
        "          \"sequenceId\" : 3145,\n"
        "          \"delayOriginTimestamp\" : 141017216059556.954115386,\n"
        "          \"totalCorrectionField\" : -8339925163906564096,\n"
        "          \"delayResponseTimestamp\" : 216603929217188.923151586\n"
        "        }\n"
        "      ]\n"
        "    }\n"
        "  ]\n"
        "}");
}

// Tests CLOCK_DESCRIPTION structure
TEST(Tlv2JsonTest, CLOCK_DESCRIPTION)
{
    CLOCK_DESCRIPTION_t t;
    t.clockType = ordinaryClock;
    t.physicalLayerProtocol.textField = "IEEE 802.3";
    t.physicalAddressLength = 6;
    Binary physicalAddress = Binary("\xc4\x7d\x46\x20\xac\xae", 6);
    t.physicalAddress = physicalAddress;
    PortAddress_t portAddress = { IEEE_802_3, 6, physicalAddress };
    t.protocolAddress = portAddress;
    t.manufacturerIdentity[0] = 1;
    t.manufacturerIdentity[1] = 2;
    t.manufacturerIdentity[2] = 3;
    t.productDescription.textField = ";;";
    t.revisionData.textField = ";;";
    t.userDescription.textField = "test123";
    memcpy(t.profileIdentity, "\x1\x2\x3\x4\x5\x6", 6);
    EXPECT_STREQ(tlv2json(CLOCK_DESCRIPTION, &t).c_str(),
        "{\n"
        "  \"clockType\" : 32768,\n"
        "  \"physicalLayerProtocol\" : \"IEEE 802.3\",\n"
        "  \"physicalAddress\" : \"c4:7d:46:20:ac:ae\",\n"
        "  \"protocolAddress\" :\n"
        "  {\n"
        "    \"networkProtocol\" : \"IEEE_802_3\",\n"
        "    \"addressField\" : \"c4:7d:46:20:ac:ae\"\n"
        "  },\n"
        "  \"manufacturerIdentity\" : \"01:02:03\",\n"
        "  \"productDescription\" : \";;\",\n"
        "  \"revisionData\" : \";;\",\n"
        "  \"userDescription\" : \"test123\",\n"
        "  \"profileIdentity\" : \"01:02:03:04:05:06\"\n"
        "}");
}

// Tests USER_DESCRIPTION structure
TEST(Tlv2JsonTest, USER_DESCRIPTION)
{
    USER_DESCRIPTION_t t;
    t.userDescription.textField = "test123";
    EXPECT_STREQ(tlv2json(USER_DESCRIPTION, &t).c_str(),
        "{\n"
        "  \"userDescription\" : \"test123\"\n"
        "}");
}

// Tests INITIALIZE structure
TEST(Tlv2JsonTest, INITIALIZE)
{
    INITIALIZE_t t;
    t.initializationKey = 0x1234;
    EXPECT_STREQ(tlv2json(INITIALIZE, &t).c_str(),
        "{\n"
        "  \"initializationKey\" : 4660\n"
        "}");
    EXPECT_STREQ(tlv2json(INITIALIZE, &t, 3).c_str(),
        "   {\n"
        "     \"initializationKey\" : 4660\n"
        "   }");
}

// Tests FAULT_LOG structure
TEST(Tlv2JsonTest, FAULT_LOG)
{
    FAULT_LOG_t t;
    t.numberOfFaultRecords = 2;
    FaultRecord_t r;
    r.faultRecordLength = 50;
    r.faultTime = 9.000709;
    r.severityCode = F_Critical;
    r.faultName.textField = "error 1";
    r.faultValue.textField = "test123";
    r.faultDescription.textField = "This is first record";
    t.faultRecords.push_back(r);
    r.faultRecordLength = 55;
    r.faultTime = 1791.003019;
    r.severityCode = F_Warning;
    r.faultName.textField = "error 2";
    r.faultValue.textField = "test321";
    r.faultDescription.textField = "This is the second record";
    t.faultRecords.push_back(r);
    EXPECT_STREQ(tlv2json(FAULT_LOG, &t).c_str(),
        "{\n"
        "  \"numberOfFaultRecords\" : 2,\n"
        "  \"faultRecords\" :\n"
        "  [\n"
        "    {\n"
        "      \"faultRecordLength\" : 50,\n"
        "      \"faultTime\" : 9.000709000,\n"
        "      \"severityCode\" : \"Critical\",\n"
        "      \"faultName\" : \"error 1\",\n"
        "      \"faultValue\" : \"test123\",\n"
        "      \"faultDescription\" : \"This is first record\"\n"
        "    },\n"
        "    {\n"
        "      \"faultRecordLength\" : 55,\n"
        "      \"faultTime\" : 1791.003018999,\n"
        "      \"severityCode\" : \"Warning\",\n"
        "      \"faultName\" : \"error 2\",\n"
        "      \"faultValue\" : \"test321\",\n"
        "      \"faultDescription\" : \"This is the second record\"\n"
        "    }\n"
        "  ]\n"
        "}");
}

// Tests DEFAULT_DATA_SET structure
TEST(Tlv2JsonTest, DEFAULT_DATA_SET)
{
    DEFAULT_DATA_SET_t t;
    t.flags = 0x3;
    t.numberPorts = 1;
    t.priority1 = 153;
    t.clockQuality.clockClass = 255;
    t.clockQuality.clockAccuracy = Accurate_Unknown;
    t.clockQuality.offsetScaledLogVariance = 0xffff;
    t.priority2 = 137;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    t.clockIdentity = clockId;
    t.domainNumber = 0;
    EXPECT_STREQ(tlv2json(DEFAULT_DATA_SET, &t).c_str(),
        "{\n"
        "  \"twoStepFlag\" : true,\n"
        "  \"slaveOnly\" : true,\n"
        "  \"numberPorts\" : 1,\n"
        "  \"priority1\" : 153,\n"
        "  \"clockQuality\" :\n"
        "  {\n"
        "    \"clockClass\" : 255,\n"
        "    \"clockAccuracy\" : \"Unknown\",\n"
        "    \"offsetScaledLogVariance\" : 65535\n"
        "  },\n"
        "  \"priority2\" : 137,\n"
        "  \"clockIdentity\" : \"c47d46.fffe.20acae\",\n"
        "  \"domainNumber\" : 0\n"
        "}");
}

// Tests CURRENT_DATA_SET structure
TEST(Tlv2JsonTest, CURRENT_DATA_SET)
{
    CURRENT_DATA_SET_t t;
    t.stepsRemoved = 0x1234;
    t.offsetFromMaster.scaledNanoseconds = 0x321047abcd541285LL;
    t.meanPathDelay.scaledNanoseconds = 0x0906050403020100LL;
    EXPECT_STREQ(tlv2json(CURRENT_DATA_SET, &t).c_str(),
        "{\n"
        "  \"stepsRemoved\" : 4660,\n"
        "  \"offsetFromMaster\" : 3607462104733586053,\n"
        "  \"meanPathDelay\" : 650212710990086400\n"
        "}");
}

// Tests PARENT_DATA_SET structure
TEST(Tlv2JsonTest, PARENT_DATA_SET)
{
    PARENT_DATA_SET_t t;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 5 };
    t.parentPortIdentity = portIdentity;
    t.flags = 1;
    t.observedParentOffsetScaledLogVariance = 0xffff;
    t.observedParentClockPhaseChangeRate = 0x7fffffff;
    t.grandmasterPriority1 = 255;
    t.grandmasterClockQuality.clockClass = 255;
    t.grandmasterClockQuality.clockAccuracy = Accurate_Unknown;
    t.grandmasterClockQuality.offsetScaledLogVariance = 0xffff;
    t.grandmasterPriority2 = 255;
    t.grandmasterIdentity = clockId;
    EXPECT_STREQ(tlv2json(PARENT_DATA_SET, &t).c_str(),
        "{\n"
        "  \"parentPortIdentity\" :\n"
        "  {\n"
        "    \"clockIdentity\" : \"c47d46.fffe.20acae\",\n"
        "    \"portNumber\" : 5\n"
        "  },\n"
        "  \"parentStats\" : true,\n"
        "  \"observedParentOffsetScaledLogVariance\" : 65535,\n"
        "  \"observedParentClockPhaseChangeRate\" : 2147483647,\n"
        "  \"grandmasterPriority1\" : 255,\n"
        "  \"grandmasterClockQuality\" :\n"
        "  {\n"
        "    \"clockClass\" : 255,\n"
        "    \"clockAccuracy\" : \"Unknown\",\n"
        "    \"offsetScaledLogVariance\" : 65535\n"
        "  },\n"
        "  \"grandmasterPriority2\" : 255,\n"
        "  \"grandmasterIdentity\" : \"c47d46.fffe.20acae\"\n"
        "}");
}

// Tests TIME_PROPERTIES_DATA_SET structure
TEST(Tlv2JsonTest, TIME_PROPERTIES_DATA_SET)
{
    TIME_PROPERTIES_DATA_SET_t t;
    t.currentUtcOffset = 37;
    t.flags = F_PTP; // ptpTimescale bit
    t.timeSource = INTERNAL_OSCILLATOR;
    EXPECT_STREQ(tlv2json(TIME_PROPERTIES_DATA_SET, &t).c_str(),
        "{\n"
        "  \"currentUtcOffset\" : 37,\n"
        "  \"leap61\" : false,\n"
        "  \"leap59\" : false,\n"
        "  \"currentUtcOffsetValid\" : false,\n"
        "  \"ptpTimescale\" : true,\n"
        "  \"timeTraceable\" : false,\n"
        "  \"frequencyTraceable\" : false,\n"
        "  \"timeSource\" : \"INTERNAL_OSCILLATOR\"\n"
        "}");
}

// Tests PORT_DATA_SET structure
TEST(Tlv2JsonTest, PORT_DATA_SET)
{
    PORT_DATA_SET_t t;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    t.portIdentity = portIdentity;
    t.portState = LISTENING;
    t.logMinDelayReqInterval = 0;
    t.peerMeanPathDelay.scaledNanoseconds = 0;
    t.logAnnounceInterval = 1;
    t.announceReceiptTimeout = 3;
    t.logSyncInterval = 0;
    t.delayMechanism = P2P;
    t.logMinPdelayReqInterval = 0;
    t.versionNumber = 2;
    EXPECT_STREQ(tlv2json(PORT_DATA_SET, &t).c_str(),
        "{\n"
        "  \"portIdentity\" :\n"
        "  {\n"
        "    \"clockIdentity\" : \"c47d46.fffe.20acae\",\n"
        "    \"portNumber\" : 1\n"
        "  },\n"
        "  \"portState\" : \"LISTENING\",\n"
        "  \"logMinDelayReqInterval\" : 0,\n"
        "  \"peerMeanPathDelay\" : 0,\n"
        "  \"logAnnounceInterval\" : 1,\n"
        "  \"announceReceiptTimeout\" : 3,\n"
        "  \"logSyncInterval\" : 0,\n"
        "  \"delayMechanism\" : \"P2P\",\n"
        "  \"logMinPdelayReqInterval\" : 0,\n"
        "  \"versionNumber\" : 2\n"
        "}");
};

// Tests PRIORITY1 structure
TEST(Tlv2JsonTest, PRIORITY1)
{
    PRIORITY1_t t;
    t.priority1 = 153;
    EXPECT_STREQ(tlv2json(PRIORITY1, &t).c_str(),
        "{\n"
        "  \"priority1\" : 153\n"
        "}");
}

// Tests PRIORITY2 structure
TEST(Tlv2JsonTest, PRIORITY2)
{
    PRIORITY2_t t;
    t.priority2 = 137;
    EXPECT_STREQ(tlv2json(PRIORITY2, &t).c_str(),
        "{\n"
        "  \"priority2\" : 137\n"
        "}");
}

// Tests DOMAIN structure
TEST(Tlv2JsonTest, DOMAIN)
{
    DOMAIN_t t;
    t.domainNumber = 7;
    EXPECT_STREQ(tlv2json(DOMAIN, &t).c_str(),
        "{\n"
        "  \"domainNumber\" : 7\n"
        "}");
}

// Tests SLAVE_ONLY structure
TEST(Tlv2JsonTest, SLAVE_ONLY)
{
    SLAVE_ONLY_t t;
    t.flags = 1;
    EXPECT_STREQ(tlv2json(SLAVE_ONLY, &t).c_str(),
        "{\n"
        "  \"slaveOnly\" : true\n"
        "}");
}

// Tests LOG_ANNOUNCE_INTERVAL structure
TEST(Tlv2JsonTest, LOG_ANNOUNCE_INTERVAL)
{
    LOG_ANNOUNCE_INTERVAL_t t;
    t.logAnnounceInterval = 1;
    EXPECT_STREQ(tlv2json(LOG_ANNOUNCE_INTERVAL, &t).c_str(),
        "{\n"
        "  \"logAnnounceInterval\" : 1\n"
        "}");
}

// Tests ANNOUNCE_RECEIPT_TIMEOUT structure
TEST(Tlv2JsonTest, ANNOUNCE_RECEIPT_TIMEOUT)
{
    ANNOUNCE_RECEIPT_TIMEOUT_t t;
    t.announceReceiptTimeout = 3;
    EXPECT_STREQ(tlv2json(ANNOUNCE_RECEIPT_TIMEOUT, &t).c_str(),
        "{\n"
        "  \"announceReceiptTimeout\" : 3\n"
        "}");
}

// Tests LOG_SYNC_INTERVAL structure
TEST(Tlv2JsonTest, LOG_SYNC_INTERVAL)
{
    LOG_SYNC_INTERVAL_t t;
    t.logSyncInterval = 7;
    EXPECT_STREQ(tlv2json(LOG_SYNC_INTERVAL, &t).c_str(),
        "{\n"
        "  \"logSyncInterval\" : 7\n"
        "}");
}

// Tests VERSION_NUMBER structure
TEST(Tlv2JsonTest, VERSION_NUMBER)
{
    VERSION_NUMBER_t t;
    t.versionNumber = 2;
    EXPECT_STREQ(tlv2json(VERSION_NUMBER, &t).c_str(),
        "{\n"
        "  \"versionNumber\" : 2\n"
        "}");
}

// Tests TIME structure
TEST(Tlv2JsonTest, TIME)
{
    TIME_t t;
    t.currentTime = 13.15;
    EXPECT_STREQ(tlv2json(TIME, &t).c_str(),
        "{\n"
        "  \"currentTime\" : 13.150000000\n"
        "}");
}

// Tests CLOCK_ACCURACY structure
TEST(Tlv2JsonTest, CLOCK_ACCURACY)
{
    CLOCK_ACCURACY_t t;
    t.clockAccuracy = Accurate_Unknown;
    EXPECT_STREQ(tlv2json(CLOCK_ACCURACY, &t).c_str(),
        "{\n"
        "  \"clockAccuracy\" : \"Unknown\"\n"
        "}");
}

// Tests UTC_PROPERTIES structure
TEST(Tlv2JsonTest, UTC_PROPERTIES)
{
    UTC_PROPERTIES_t t;
    t.currentUtcOffset = -0x5433;
    t.flags = 7;
    EXPECT_STREQ(tlv2json(UTC_PROPERTIES, &t).c_str(),
        "{\n"
        "  \"currentUtcOffset\" : -21555,\n"
        "  \"leap61\" : true,\n"
        "  \"leap59\" : true,\n"
        "  \"currentUtcOffsetValid\" : true\n"
        "}");
}

// Tests TRACEABILITY_PROPERTIES structure
TEST(Tlv2JsonTest, TRACEABILITY_PROPERTIES)
{
    TRACEABILITY_PROPERTIES_t t;
    t.flags = F_TTRA | F_FTRA;
    EXPECT_STREQ(tlv2json(TRACEABILITY_PROPERTIES, &t).c_str(),
        "{\n"
        "  \"timeTraceable\" : true,\n"
        "  \"frequencyTraceable\" : true\n"
        "}");
}

// Tests TIMESCALE_PROPERTIES structure
TEST(Tlv2JsonTest, TIMESCALE_PROPERTIES)
{
    TIMESCALE_PROPERTIES_t t;
    t.flags = F_PTP;
    t.timeSource = HAND_SET;
    EXPECT_STREQ(tlv2json(TIMESCALE_PROPERTIES, &t).c_str(),
        "{\n"
        "  \"ptpTimescale\" : true,\n"
        "  \"timeSource\" : \"HAND_SET\"\n"
        "}");
}

// Tests UNICAST_NEGOTIATION_ENABLE structure
TEST(Tlv2JsonTest, UNICAST_NEGOTIATION_ENABLE)
{
    UNICAST_NEGOTIATION_ENABLE_t t;
    t.flags = 1;
    EXPECT_STREQ(tlv2json(UNICAST_NEGOTIATION_ENABLE, &t).c_str(),
        "{\n"
        "  \"unicastNegotiationPortDS\" : true\n"
        "}");
}

// Tests PATH_TRACE_LIST structure
TEST(Tlv2JsonTest, PATH_TRACE_LIST)
{
    PATH_TRACE_LIST_t t;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    t.pathSequence.push_back(clockId);
    ClockIdentity_t c2 = {12, 4, 19, 97, 11, 74, 12, 74};
    t.pathSequence.push_back(c2);
    EXPECT_STREQ(tlv2json(PATH_TRACE_LIST, &t).c_str(),
        "{\n"
        "  \"pathSequence\" :\n"
        "  [\n"
        "    \"c47d46.fffe.20acae\",\n"
        "    \"0c0413.610b.4a0c4a\"\n"
        "  ]\n"
        "}");
}

// Tests PATH_TRACE_ENABLE structure
TEST(Tlv2JsonTest, PATH_TRACE_ENABLE)
{
    PATH_TRACE_ENABLE_t t;
    t.flags = 1;
    EXPECT_STREQ(tlv2json(PATH_TRACE_ENABLE, &t).c_str(),
        "{\n"
        "  \"pathTraceDS\" : true\n"
        "}");
}

// Tests GRANDMASTER_CLUSTER_TABLE structure
TEST(Tlv2JsonTest, GRANDMASTER_CLUSTER_TABLE)
{
    GRANDMASTER_CLUSTER_TABLE_t t;
    t.logQueryInterval = -19;
    t.actualTableSize = 2;
    Binary physicalAddress = Binary("\xc4\x7d\x46\x20\xac\xae", 6);
    PortAddress_t portAddress0 = { IEEE_802_3, 6, physicalAddress };
    t.PortAddress.push_back(portAddress0);
    Binary ip("\x12\x34\x56\x78", 4);
    PortAddress_t portAddress1 = { UDP_IPv4, 4, ip };
    t.PortAddress.push_back(portAddress1);
    EXPECT_STREQ(tlv2json(GRANDMASTER_CLUSTER_TABLE, &t).c_str(),
        "{\n"
        "  \"logQueryInterval\" : -19,\n"
        "  \"actualTableSize\" : 2,\n"
        "  \"PortAddress\" :\n"
        "  [\n"
        "    {\n"
        "      \"networkProtocol\" : \"IEEE_802_3\",\n"
        "      \"addressField\" : \"c4:7d:46:20:ac:ae\"\n"
        "    },\n"
        "    {\n"
        "      \"networkProtocol\" : \"UDP_IPv4\",\n"
        "      \"addressField\" : \"12:34:56:78\"\n"
        "    }\n"
        "  ]\n"
        "}");
}

// Tests UNICAST_MASTER_TABLE structure
TEST(Tlv2JsonTest, UNICAST_MASTER_TABLE)
{
    UNICAST_MASTER_TABLE_t t;
    t.logQueryInterval = -19;
    t.actualTableSize = 2;
    Binary physicalAddress = Binary("\xc4\x7d\x46\x20\xac\xae", 6);
    PortAddress_t portAddress0 = { IEEE_802_3, 6, physicalAddress };
    t.PortAddress.push_back(portAddress0);
    Binary ip("\x12\x34\x56\x78", 4);
    PortAddress_t portAddress1 = { UDP_IPv4, 4, ip };
    t.PortAddress.push_back(portAddress1);
    EXPECT_STREQ(tlv2json(UNICAST_MASTER_TABLE, &t).c_str(),
        "{\n"
        "  \"logQueryInterval\" : -19,\n"
        "  \"actualTableSize\" : 2,\n"
        "  \"PortAddress\" :\n"
        "  [\n"
        "    {\n"
        "      \"networkProtocol\" : \"IEEE_802_3\",\n"
        "      \"addressField\" : \"c4:7d:46:20:ac:ae\"\n"
        "    },\n"
        "    {\n"
        "      \"networkProtocol\" : \"UDP_IPv4\",\n"
        "      \"addressField\" : \"12:34:56:78\"\n"
        "    }\n"
        "  ]\n"
        "}");
}

// Tests UNICAST_MASTER_MAX_TABLE_SIZE structure
TEST(Tlv2JsonTest, UNICAST_MASTER_MAX_TABLE_SIZE)
{
    UNICAST_MASTER_MAX_TABLE_SIZE_t t;
    t.maxTableSize = 0x2143;
    EXPECT_STREQ(tlv2json(UNICAST_MASTER_MAX_TABLE_SIZE, &t).c_str(),
        "{\n"
        "  \"maxTableSize\" : 8515\n"
        "}");
}

// Tests ACCEPTABLE_MASTER_TABLE structure
TEST(Tlv2JsonTest, ACCEPTABLE_MASTER_TABLE)
{
    ACCEPTABLE_MASTER_TABLE_t t;
    t.actualTableSize = 2;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t a0 = { clockId, 1 };
    t.list.push_back({a0, 127});
    PortIdentity_t a1 = { { 9, 8, 7, 6, 5, 4, 1, 7}, 2 };
    t.list.push_back({a1, 111});
    EXPECT_STREQ(tlv2json(ACCEPTABLE_MASTER_TABLE, &t).c_str(),
        "{\n"
        "  \"actualTableSize\" : 2,\n"
        "  \"list\" :\n"
        "  [\n"
        "    {\n"
        "      \"acceptablePortIdentity\" :\n"
        "      {\n"
        "        \"clockIdentity\" : \"c47d46.fffe.20acae\",\n"
        "        \"portNumber\" : 1\n"
        "      },\n"
        "      \"alternatePriority1\" : 127\n"
        "    },\n"
        "    {\n"
        "      \"acceptablePortIdentity\" :\n"
        "      {\n"
        "        \"clockIdentity\" : \"090807.0605.040107\",\n"
        "        \"portNumber\" : 2\n"
        "      },\n"
        "      \"alternatePriority1\" : 111\n"
        "    }\n"
        "  ]\n"
        "}");
}

// Tests ACCEPTABLE_MASTER_TABLE_ENABLED structure
TEST(Tlv2JsonTest, ACCEPTABLE_MASTER_TABLE_ENABLED)
{
    ACCEPTABLE_MASTER_TABLE_ENABLED_t t;
    t.flags = 1;
    EXPECT_STREQ(tlv2json(ACCEPTABLE_MASTER_TABLE_ENABLED, &t).c_str(),
        "{\n"
        "  \"acceptableMasterPortDS\" : true\n"
        "}");
}

// Tests ACCEPTABLE_MASTER_MAX_TABLE_SIZE structure
TEST(Tlv2JsonTest, ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
{
    ACCEPTABLE_MASTER_MAX_TABLE_SIZE_t t;
    t.maxTableSize = 0x67ba;
    EXPECT_STREQ(tlv2json(ACCEPTABLE_MASTER_MAX_TABLE_SIZE, &t).c_str(),
        "{\n"
        "  \"maxTableSize\" : 26554\n"
        "}");
}

// Tests ALTERNATE_MASTER structure
TEST(Tlv2JsonTest, ALTERNATE_MASTER)
{
    ALTERNATE_MASTER_t t;
    t.flags = 1;
    t.logAlternateMulticastSyncInterval = -17;
    t.numberOfAlternateMasters = 210;
    EXPECT_STREQ(tlv2json(ALTERNATE_MASTER, &t).c_str(),
        "{\n"
        "  \"transmitAlternateMulticastSync\" : true,\n"
        "  \"logAlternateMulticastSyncInterval\" : -17,\n"
        "  \"numberOfAlternateMasters\" : 210\n"
        "}");
}

// Tests ALTERNATE_TIME_OFFSET_ENABLE structure
TEST(Tlv2JsonTest, ALTERNATE_TIME_OFFSET_ENABLE)
{
    ALTERNATE_TIME_OFFSET_ENABLE_t t;
    t.keyField = 7;
    t.flags = 1;
    EXPECT_STREQ(tlv2json(ALTERNATE_TIME_OFFSET_ENABLE, &t).c_str(),
        "{\n"
        "  \"keyField\" : 7,\n"
        "  \"alternateTimescaleOffsetsDS\" : true\n"
        "}");
}

// Tests ALTERNATE_TIME_OFFSET_NAME structure
TEST(Tlv2JsonTest, ALTERNATE_TIME_OFFSET_NAME)
{
    ALTERNATE_TIME_OFFSET_NAME_t t;
    t.keyField = 11;
    t.displayName.textField = "123";
    EXPECT_STREQ(tlv2json(ALTERNATE_TIME_OFFSET_NAME, &t).c_str(),
        "{\n"
        "  \"keyField\" : 11,\n"
        "  \"displayName\" : \"123\"\n"
        "}");
}

// Tests ALTERNATE_TIME_OFFSET_MAX_KEY structure
TEST(Tlv2JsonTest, ALTERNATE_TIME_OFFSET_MAX_KEY)
{
    ALTERNATE_TIME_OFFSET_MAX_KEY_t t;
    t.maxKey = 9;
    EXPECT_STREQ(tlv2json(ALTERNATE_TIME_OFFSET_MAX_KEY, &t).c_str(),
        "{\n"
        "  \"maxKey\" : 9\n"
        "}");
}

// Tests ALTERNATE_TIME_OFFSET_PROPERTIES structure
TEST(Tlv2JsonTest, ALTERNATE_TIME_OFFSET_PROPERTIES)
{
    ALTERNATE_TIME_OFFSET_PROPERTIES_t t;
    t.keyField = 13;
    t.currentOffset = -2145493247;
    t.jumpSeconds = -2147413249;
    t.timeOfNextJump = 0x912478321891LL;
    EXPECT_STREQ(tlv2json(ALTERNATE_TIME_OFFSET_PROPERTIES, &t).c_str(),
        "{\n"
        "  \"keyField\" : 13,\n"
        "  \"currentOffset\" : -2145493247,\n"
        "  \"jumpSeconds\" : -2147413249,\n"
        "  \"timeOfNextJump\" : 159585821399185\n"
        "}");
}

// Tests TRANSPARENT_CLOCK_PORT_DATA_SET structure
TEST(Tlv2JsonTest, TRANSPARENT_CLOCK_PORT_DATA_SET)
{
    TRANSPARENT_CLOCK_PORT_DATA_SET_t t;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    t.portIdentity = portIdentity;
    t.flags = 1;
    t.logMinPdelayReqInterval = -21;
    t.peerMeanPathDelay.scaledNanoseconds = 0xdcf87240dcd12301LL;
    EXPECT_STREQ(tlv2json(TRANSPARENT_CLOCK_PORT_DATA_SET, &t).c_str(),
        "{\n"
        "  \"portIdentity\" :\n"
        "  {\n"
        "    \"clockIdentity\" : \"c47d46.fffe.20acae\",\n"
        "    \"portNumber\" : 1\n"
        "  },\n"
        "  \"transparentClockPortDS\" : true,\n"
        "  \"logMinPdelayReqInterval\" : -21,\n"
        "  \"peerMeanPathDelay\" : -2524141968232996095\n"
        "}");
}

// Tests LOG_MIN_PDELAY_REQ_INTERVAL structure
TEST(Tlv2JsonTest, LOG_MIN_PDELAY_REQ_INTERVAL)
{
    LOG_MIN_PDELAY_REQ_INTERVAL_t t;
    t.logMinPdelayReqInterval = 9;
    EXPECT_STREQ(tlv2json(LOG_MIN_PDELAY_REQ_INTERVAL, &t).c_str(),
        "{\n"
        "  \"logMinPdelayReqInterval\" : 9\n"
        "}");
}

// Tests TRANSPARENT_CLOCK_DEFAULT_DATA_SET structure
TEST(Tlv2JsonTest, TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
{
    TRANSPARENT_CLOCK_DEFAULT_DATA_SET_t t;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    t.clockIdentity = clockId;
    t.numberPorts = 0x177a;
    t.delayMechanism = NO_MECHANISM;
    t.primaryDomain = 18;
    EXPECT_STREQ(tlv2json(TRANSPARENT_CLOCK_DEFAULT_DATA_SET, &t).c_str(),
        "{\n"
        "  \"clockIdentity\" : \"c47d46.fffe.20acae\",\n"
        "  \"numberPorts\" : 6010,\n"
        "  \"delayMechanism\" : \"NO_MECHANISM\",\n"
        "  \"primaryDomain\" : 18\n"
        "}");
}

// Tests PRIMARY_DOMAIN structure
TEST(Tlv2JsonTest, PRIMARY_DOMAIN)
{
    PRIMARY_DOMAIN_t t;
    t.primaryDomain = 17;
    EXPECT_STREQ(tlv2json(PRIMARY_DOMAIN, &t).c_str(),
        "{\n"
        "  \"primaryDomain\" : 17\n"
        "}");
}

// Tests DELAY_MECHANISM structure
TEST(Tlv2JsonTest, DELAY_MECHANISM)
{
    DELAY_MECHANISM_t t;
    t.delayMechanism = P2P;
    EXPECT_STREQ(tlv2json(DELAY_MECHANISM, &t).c_str(),
        "{\n"
        "  \"delayMechanism\" : \"P2P\"\n"
        "}");
}

// Tests EXTERNAL_PORT_CONFIGURATION_ENABLED structure
TEST(Tlv2JsonTest, EXTERNAL_PORT_CONFIGURATION_ENABLED)
{
    EXTERNAL_PORT_CONFIGURATION_ENABLED_t t;
    t.flags = 1;
    EXPECT_STREQ(tlv2json(EXTERNAL_PORT_CONFIGURATION_ENABLED, &t).c_str(),
        "{\n"
        "  \"externalPortConfiguration\" : true\n"
        "}");
}

// Tests MASTER_ONLY structure
TEST(Tlv2JsonTest, MASTER_ONLY)
{
    MASTER_ONLY_t t;
    t.flags = 1;
    EXPECT_STREQ(tlv2json(MASTER_ONLY, &t).c_str(),
        "{\n"
        "  \"masterOnly\" : true\n"
        "}");
}

// Tests HOLDOVER_UPGRADE_ENABLE structure
TEST(Tlv2JsonTest, HOLDOVER_UPGRADE_ENABLE)
{
    HOLDOVER_UPGRADE_ENABLE_t t;
    t.flags = 1;
    EXPECT_STREQ(tlv2json(HOLDOVER_UPGRADE_ENABLE, &t).c_str(),
        "{\n"
        "  \"holdoverUpgradeDS\" : true\n"
        "}");
}

// Tests EXT_PORT_CONFIG_PORT_DATA_SET structure
TEST(Tlv2JsonTest, EXT_PORT_CONFIG_PORT_DATA_SET)
{
    EXT_PORT_CONFIG_PORT_DATA_SET_t t;
    t.flags = 1;
    t.desiredState = PASSIVE;
    EXPECT_STREQ(tlv2json(EXT_PORT_CONFIG_PORT_DATA_SET, &t).c_str(),
        "{\n"
        "  \"acceptableMasterPortDS\" : true,\n"
        "  \"desiredState\" : \"PASSIVE\"\n"
        "}");
}

// Tests SMPTE organization extension
TEST(Tlv2JsonTest, SMPTE_ORGANIZATION_EXTENSION)
{
    Message msg;
    uint8_t b[100] = {0xd, 2, 0, 0x64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0x74, 0xda, 0x38, 0xff, 0xfe, 0xf6, 0x98, 0x5e, 0, 1, 0, 0, 4,
            0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 3,
            3, 3, 0, 0, 3, 0, 0x30, 0x68, 0x97, 0xe8, 0, 0, 1, 0, 0, 0, 0x1e, 0,
            0, 0, 1, 1
        };
    MsgParams a = msg.getParams();
    a.rcvSMPTEOrg = 1;
    EXPECT_TRUE(msg.updateParams(a));
    ASSERT_EQ(msg.parse(b, sizeof b), MNG_PARSE_ERROR_SMPTE);
    ASSERT_TRUE(msg.isLastMsgSMPTE());
    EXPECT_STREQ(msg2json(msg).c_str(),
        "{\n"
        "  \"sequenceId\" : 0,\n"
        "  \"sdoId\" : 0,\n"
        "  \"domainNumber\" : 0,\n"
        "  \"versionPTP\" : 2,\n"
        "  \"minorVersionPTP\" : 0,\n"
        "  \"unicastFlag\" : false,\n"
        "  \"PTPProfileSpecific\" : 0,\n"
        "  \"messageType\" : \"Management\",\n"
        "  \"sourcePortIdentity\" :\n"
        "  {\n"
        "    \"clockIdentity\" : \"74da38.fffe.f6985e\",\n"
        "    \"portNumber\" : 1\n"
        "  },\n"
        "  \"targetPortIdentity\" :\n"
        "  {\n"
        "    \"clockIdentity\" : \"ffffff.ffff.ffffff\",\n"
        "    \"portNumber\" : 65535\n"
        "  },\n"
        "  \"actionField\" : \"COMMAND\",\n"
        "  \"tlvType\" : \"ORGANIZATION_EXTENSION\",\n"
        "  \"managementId\" : \"SMPTE_MNG_ID\",\n"
        "  \"organizationId\" : \"68:97:e8\",\n"
        "  \"organizationSubType\" : \"00:00:01\",\n"
        "  \"defaultSystemFrameRate_numerator\" : 30,\n"
        "  \"defaultSystemFrameRate_denominator\" : 1,\n"
        "  \"masterLockingStatus\" : \"FREE_RUN\",\n"
        "  \"timeAddressFlags\" : 0,\n"
        "  \"currentLocalOffset\" : 0,\n"
        "  \"jumpSeconds\" : 0,\n"
        "  \"timeOfNextJump\" : 0,\n"
        "  \"timeOfNextJam\" : 0,\n"
        "  \"timeOfPreviousJam\" : 0,\n"
        "  \"previousJamLocalOffset\" : 0,\n"
        "  \"daylightSaving\" : 0,\n"
        "  \"leapSecondJump\" : 0\n"
        "}");
}

// Tests TIME_STATUS_NP structure
TEST(Tlv2JsonTest, TIME_STATUS_NP)
{
    TIME_STATUS_NP_t t;
    t.master_offset = 0;
    t.ingress_time = 0;
    t.cumulativeScaledRateOffset = 0;
    t.scaledLastGmPhaseChange = 0;
    t.gmTimeBaseIndicator = 0;
    t.nanoseconds_msb = 0;
    t.nanoseconds_lsb = 0;
    t.fractional_nanoseconds = 0;
    t.gmPresent = 0;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    t.gmIdentity = clockId;
    EXPECT_STREQ(tlv2json(TIME_STATUS_NP, &t).c_str(),
        "{\n"
        "  \"master_offset\" : 0,\n"
        "  \"ingress_time\" : 0,\n"
        "  \"cumulativeScaledRateOffset\" : 0,\n"
        "  \"scaledLastGmPhaseChange\" : 0,\n"
        "  \"gmTimeBaseIndicator\" : 0,\n"
        "  \"nanoseconds_msb\" : 0,\n"
        "  \"nanoseconds_lsb\" : 0,\n"
        "  \"fractional_nanoseconds\" : 0,\n"
        "  \"gmPresent\" : 0,\n"
        "  \"gmIdentity\" : \"c47d46.fffe.20acae\"\n"
        "}");
}

// Tests GRANDMASTER_SETTINGS_NP structure
TEST(Tlv2JsonTest, GRANDMASTER_SETTINGS_NP)
{
    GRANDMASTER_SETTINGS_NP_t t;
    t.clockQuality.clockClass = 255;
    t.clockQuality.clockAccuracy = Accurate_Unknown;
    t.clockQuality.offsetScaledLogVariance = 0xffff;
    t.currentUtcOffset = 37;
    t.flags = F_PTP;
    t.timeSource = INTERNAL_OSCILLATOR;
    EXPECT_STREQ(tlv2json(GRANDMASTER_SETTINGS_NP, &t).c_str(),
        "{\n"
        "  \"clockQuality\" :\n"
        "  {\n"
        "    \"clockClass\" : 255,\n"
        "    \"clockAccuracy\" : \"Unknown\",\n"
        "    \"offsetScaledLogVariance\" : 65535\n"
        "  },\n"
        "  \"currentUtcOffset\" : 37,\n"
        "  \"leap61\" : false,\n"
        "  \"leap59\" : false,\n"
        "  \"currentUtcOffsetValid\" : false,\n"
        "  \"ptpTimescale\" : true,\n"
        "  \"timeTraceable\" : false,\n"
        "  \"frequencyTraceable\" : false,\n"
        "  \"timeSource\" : \"INTERNAL_OSCILLATOR\"\n"
        "}");
}

// Tests PORT_DATA_SET_NP structure
TEST(Tlv2JsonTest, PORT_DATA_SET_NP)
{
    PORT_DATA_SET_NP_t t;
    t.neighborPropDelayThresh = 20000000;
    t.asCapable = 1;
    EXPECT_STREQ(tlv2json(PORT_DATA_SET_NP, &t).c_str(),
        "{\n"
        "  \"neighborPropDelayThresh\" : 20000000,\n"
        "  \"asCapable\" : 1\n"
        "}");
}

// Tests SUBSCRIBE_EVENTS_NP structure
TEST(Tlv2JsonTest, SUBSCRIBE_EVENTS_NP)
{
    SUBSCRIBE_EVENTS_NP_t t;
    t.duration = 0x1234;
    t.setEvent(NOTIFY_PORT_STATE);
    t.setEvent(NOTIFY_TIME_SYNC);
    t.setEvent(NOTIFY_PARENT_DATA_SET);
    t.setEvent(NOTIFY_CMLDS);
    EXPECT_STREQ(tlv2json(SUBSCRIBE_EVENTS_NP, &t).c_str(),
        "{\n"
        "  \"duration\" : 4660,\n"
        "  \"NOTIFY_PORT_STATE\" : true,\n"
        "  \"NOTIFY_TIME_SYNC\" : true,\n"
        "  \"NOTIFY_PARENT_DATA_SET\" : true,\n"
        "  \"NOTIFY_CMLDS\" : true\n"
        "}");
}

// Tests PORT_PROPERTIES_NP structure
TEST(Tlv2JsonTest, PORT_PROPERTIES_NP)
{
    PORT_PROPERTIES_NP_t t;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    t.portIdentity = portIdentity;
    t.portState = LISTENING;
    t.timestamping = TS_HARDWARE;
    t.interface.textField = "enp0s25";
    EXPECT_STREQ(tlv2json(PORT_PROPERTIES_NP, &t).c_str(),
        "{\n"
        "  \"portIdentity\" :\n"
        "  {\n"
        "    \"clockIdentity\" : \"c47d46.fffe.20acae\",\n"
        "    \"portNumber\" : 1\n"
        "  },\n"
        "  \"portState\" : \"LISTENING\",\n"
        "  \"timestamping\" : \"HARDWARE\",\n"
        "  \"interface\" : \"enp0s25\"\n"
        "}");
}

// Tests PORT_STATS_NP structure
TEST(Tlv2JsonTest, PORT_STATS_NP)
{
    PORT_STATS_NP_t t;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    t.portIdentity = portIdentity;
    t.rxMsgType[STAT_SYNC] = 0;
    t.rxMsgType[STAT_DELAY_REQ] = 0;
    t.rxMsgType[STAT_PDELAY_REQ] = 0;
    t.rxMsgType[STAT_PDELAY_RESP] = 0;
    t.rxMsgType[STAT_FOLLOW_UP] = 0;
    t.rxMsgType[STAT_DELAY_RESP] = 0;
    t.rxMsgType[STAT_PDELAY_RESP_FOLLOW_UP] = 0;
    t.rxMsgType[STAT_ANNOUNCE] = 0;
    t.rxMsgType[STAT_SIGNALING] = 0;
    t.rxMsgType[STAT_MANAGEMENT] = 0;
    t.txMsgType[STAT_SYNC] = 0;
    t.txMsgType[STAT_DELAY_REQ] = 0;
    t.txMsgType[STAT_PDELAY_REQ] = 63346;
    t.txMsgType[STAT_PDELAY_RESP] = 0;
    t.txMsgType[STAT_FOLLOW_UP] = 0;
    t.txMsgType[STAT_DELAY_RESP] = 0;
    t.txMsgType[STAT_PDELAY_RESP_FOLLOW_UP] = 0;
    t.txMsgType[STAT_ANNOUNCE] = 0;
    t.txMsgType[STAT_SIGNALING] = 0;
    t.txMsgType[STAT_MANAGEMENT] = 0;
    EXPECT_STREQ(tlv2json(PORT_STATS_NP, &t).c_str(),
        "{\n"
        "  \"portIdentity\" :\n"
        "  {\n"
        "    \"clockIdentity\" : \"c47d46.fffe.20acae\",\n"
        "    \"portNumber\" : 1\n"
        "  },\n"
        "  \"rx_Sync\" : 0,\n"
        "  \"rx_Delay_Req\" : 0,\n"
        "  \"rx_Pdelay_Req\" : 0,\n"
        "  \"rx_Pdelay_Resp\" : 0,\n"
        "  \"rx_Follow_Up\" : 0,\n"
        "  \"rx_Delay_Resp\" : 0,\n"
        "  \"rx_Pdelay_Resp_Follow_Up\" : 0,\n"
        "  \"rx_Announce\" : 0,\n"
        "  \"rx_Signaling\" : 0,\n"
        "  \"rx_Management\" : 0,\n"
        "  \"tx_Sync\" : 0,\n"
        "  \"tx_Delay_Req\" : 0,\n"
        "  \"tx_Pdelay_Req\" : 63346,\n"
        "  \"tx_Pdelay_Resp\" : 0,\n"
        "  \"tx_Follow_Up\" : 0,\n"
        "  \"tx_Delay_Resp\" : 0,\n"
        "  \"tx_Pdelay_Resp_Follow_Up\" : 0,\n"
        "  \"tx_Announce\" : 0,\n"
        "  \"tx_Signaling\" : 0,\n"
        "  \"tx_Management\" : 0\n"
        "}");
}

// Tests SYNCHRONIZATION_UNCERTAIN_NP structure
TEST(Tlv2JsonTest, SYNCHRONIZATION_UNCERTAIN_NP)
{
    SYNCHRONIZATION_UNCERTAIN_NP_t t;
    t.val = SYNC_UNCERTAIN_DONTCARE;
    EXPECT_STREQ(tlv2json(SYNCHRONIZATION_UNCERTAIN_NP, &t).c_str(),
        "{\n"
        "  \"val\" : 255\n"
        "}");
}

// Tests PORT_SERVICE_STATS_NP structure
TEST(Tlv2JsonTest, PORT_SERVICE_STATS_NP)
{
    PORT_SERVICE_STATS_NP_t t;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    t.portIdentity = portIdentity;
    t.announce_timeout = 9041;
    t.sync_timeout = 0;
    t.delay_timeout = 63346;
    t.unicast_service_timeout = 0;
    t.unicast_request_timeout = 0;
    t.master_announce_timeout = 0;
    t.master_sync_timeout = 0;
    t.qualification_timeout = 0;
    t.sync_mismatch = 0;
    t.followup_mismatch = 0;
    EXPECT_STREQ(tlv2json(PORT_SERVICE_STATS_NP, &t).c_str(),
        "{\n"
        "  \"portIdentity\" :\n"
        "  {\n"
        "    \"clockIdentity\" : \"c47d46.fffe.20acae\",\n"
        "    \"portNumber\" : 1\n"
        "  },\n"
        "  \"announce_timeout\" : 9041,\n"
        "  \"sync_timeout\" : 0,\n"
        "  \"delay_timeout\" : 63346,\n"
        "  \"unicast_service_timeout\" : 0,\n"
        "  \"unicast_request_timeout\" : 0,\n"
        "  \"master_announce_timeout\" : 0,\n"
        "  \"master_sync_timeout\" : 0,\n"
        "  \"qualification_timeout\" : 0,\n"
        "  \"sync_mismatch\" : 0,\n"
        "  \"followup_mismatch\" : 0\n"
        "}");
}

// Tests UNICAST_MASTER_TABLE_NP structure
TEST(Tlv2JsonTest, UNICAST_MASTER_TABLE_NP)
{
    UNICAST_MASTER_TABLE_NP_t t;
    t.actualTableSize = 1;
    LinuxptpUnicastMaster_t r;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    r.portIdentity = portIdentity;
    r.clockQuality.clockClass = 255;
    r.clockQuality.clockAccuracy = Accurate_Unknown;
    r.clockQuality.offsetScaledLogVariance = 0xffff;
    r.selected = 1;
    r.portState = UC_HAVE_SYDY;
    r.priority1 = 126;
    r.priority2 = 134;
    Binary physicalAddress = Binary("\xc4\x7d\x46\x20\xac\xae", 6);
    PortAddress_t portAddress = { IEEE_802_3, 6, physicalAddress };
    r.portAddress = portAddress;
    t.unicastMasters.push_back(r);
    EXPECT_STREQ(tlv2json(UNICAST_MASTER_TABLE_NP, &t).c_str(),
        "{\n"
        "  \"actualTableSize\" : 1,\n"
        "  \"unicastMasters\" :\n"
        "  [\n"
        "    {\n"
        "      \"portIdentity\" :\n"
        "      {\n"
        "        \"clockIdentity\" : \"c47d46.fffe.20acae\",\n"
        "        \"portNumber\" : 1\n"
        "      },\n"
        "      \"clockQuality\" :\n"
        "      {\n"
        "        \"clockClass\" : 255,\n"
        "        \"clockAccuracy\" : \"Unknown\",\n"
        "        \"offsetScaledLogVariance\" : 65535\n"
        "      },\n"
        "      \"selected\" : true,\n"
        "      \"portState\" : \"HAVE_SYDY\",\n"
        "      \"priority1\" : 126,\n"
        "      \"priority2\" : 134,\n"
        "      \"portAddress\" :\n"
        "      {\n"
        "        \"networkProtocol\" : \"IEEE_802_3\",\n"
        "        \"addressField\" : \"c4:7d:46:20:ac:ae\"\n"
        "      }\n"
        "    }\n"
        "  ]\n"
        "}");
}

// Tests PORT_HWCLOCK_NP structure
TEST(Tlv2JsonTest, PORT_HWCLOCK_NP)
{
    PORT_HWCLOCK_NP_t t;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    t.portIdentity = portIdentity;
    t.phc_index = 1;
    t.flags = 7;
    EXPECT_STREQ(tlv2json(PORT_HWCLOCK_NP, &t).c_str(),
        "{\n"
        "  \"portIdentity\" :\n"
        "  {\n"
        "    \"clockIdentity\" : \"c47d46.fffe.20acae\",\n"
        "    \"portNumber\" : 1\n"
        "  },\n"
        "  \"phc_index\" : 1,\n"
        "  \"flags\" : 7\n"
        "}");
}

// Tests POWER_PROFILE_SETTINGS_NP structure
TEST(Tlv2JsonTest, POWER_PROFILE_SETTINGS_NP)
{
    POWER_PROFILE_SETTINGS_NP_t t;
    t.version = IEEE_C37_238_VERSION_2011;
    t.grandmasterID = 56230;
    t.grandmasterTimeInaccuracy = 4124796349;
    t.networkTimeInaccuracy = 3655058877;
    t.totalTimeInaccuracy = 4223530875;
    EXPECT_STREQ(tlv2json(POWER_PROFILE_SETTINGS_NP, &t).c_str(),
        "{\n"
        "  \"version\" : \"2011\",\n"
        "  \"grandmasterID\" : 56230,\n"
        "  \"grandmasterTimeInaccuracy\" : 4124796349,\n"
        "  \"networkTimeInaccuracy\" : 3655058877,\n"
        "  \"totalTimeInaccuracy\" : 4223530875\n"
        "}");
}

// Tests CMLDS_INFO_NP structure
TEST(Tlv2JsonTest, CMLDS_INFO_NP)
{
    CMLDS_INFO_NP_t t;
    t.meanLinkDelay.scaledNanoseconds = 201548321LL;
    t.scaledNeighborRateRatio = 1842;
    t.as_capable = 1;
    EXPECT_STREQ(tlv2json(CMLDS_INFO_NP, &t).c_str(),
        "{\n"
        "  \"meanLinkDelay\" : 201548321,\n"
        "  \"scaledNeighborRateRatio\" : 1842,\n"
        "  \"as_capable\" : 1\n"
        "}");
}
