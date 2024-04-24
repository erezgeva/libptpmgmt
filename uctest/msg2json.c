/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief PTP message and TLV to JSON functions unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

#include "config.h"
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include "json.h"

#define ip_v ((uint8_t *)"\x12\x34\x56\x78")
#define physicalAddress_v ((uint8_t *)"\xc4\x7d\x46\x20\xac\xae")

// Test PTP message without TLV
Test(Msg2JsonTest, Empty)
{
    uint8_t buf[60];
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(eq(int, msg->build(msg, buf, sizeof buf, 1),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, msg->getMsgLen(msg), 54));
    buf[46] = PTPMGMT_RESPONSE;
    cr_assert(eq(int, msg->parse(msg, buf, 54), PTPMGMT_MNG_PARSE_ERROR_OK));
    char *ret = ptpmgmt_json_msg2json(msg, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
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
            "}"));
    free(ret);
    ret = ptpmgmt_json_msg2json(msg, 3);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
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
            "   }"));
    free(ret);
    msg->free(msg);
}

// Test PTP message with a managment TLV
Test(Msg2JsonTest, MngTlv)
{
    uint8_t buf[70];
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    struct ptpmgmt_USER_DESCRIPTION_t t;
    char txt[] = "test123";
    t.userDescription.lengthField = strlen(txt);
    t.userDescription.textField = txt;
    cr_expect(msg->setAction(msg, PTPMGMT_SET, PTPMGMT_USER_DESCRIPTION, &t));
    cr_expect(eq(int, msg->getBuildTlvId(msg), PTPMGMT_USER_DESCRIPTION));
    cr_expect(eq(int, msg->build(msg, buf, sizeof buf, 1),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, msg->getMsgLen(msg), 62));
    buf[46] = PTPMGMT_RESPONSE;
    cr_assert(eq(int, msg->parse(msg, buf, 62), PTPMGMT_MNG_PARSE_ERROR_OK));
    char *ret = ptpmgmt_json_msg2json(msg, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
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
            "}"));
    free(ret);
    msg->free(msg);
}

// Test PTP message with a managment error TLV
Test(Msg2JsonTest, MngErrTlv)
{
    uint8_t buf[80];
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(msg->setAction(msg, PTPMGMT_GET, PTPMGMT_PRIORITY1, NULL));
    cr_expect(eq(int, msg->getBuildTlvId(msg), PTPMGMT_PRIORITY1));
    cr_expect(eq(int, msg->build(msg, buf, sizeof buf, 1),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    buf[46] = PTPMGMT_RESPONSE;
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
    cr_expect(eq(int, msg->parse(msg, buf, 70), PTPMGMT_MNG_PARSE_ERROR_MSG));
    char *ret = ptpmgmt_json_msg2json(msg, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
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
            "}"));
    free(ret);
    msg->free(msg);
}

// Test PTP signaling message
static inline void addTlv(size_t *curLen, uint8_t *buf,
    enum ptpmgmt_tlvType_e type, uint8_t *tlv, size_t len)
{
    uint16_t *cur = (uint16_t *)(buf + *curLen);
    *cur++ = htons(type);
    *cur++ = htons(len);
    memcpy(cur, tlv, len);
    *curLen += len + 4;
}
Test(Msg2JsonTest, Signaling)
{
    uint8_t buf[600];
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_expect(eq(int, msg->build(msg, buf, sizeof buf, 1),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, msg->getMsgLen(msg), 54));
    // signaling = 36 header + 10 targetPortIdentity = 44
    buf[0] = (buf[0] & 0xf0) | ptpmgmt_Signaling; // messageType
    buf[32] = 5; // controlField
    size_t curLen = 44;
    ptpmgmt_pMsgParams a = msg->getParams(msg);
    a->rcvSignaling = true;
    a->filterSignaling = false;
    cr_expect(msg->updateParams(msg, a));
    uint8_t m0[4] = {0x20, 6, 119}; // PRIORITY2 priority2 = 119
    addTlv(&curLen, buf, PTPMGMT_MANAGEMENT, m0, sizeof m0);
    uint8_t m1[4] = {0x20, 7, 7}; // DOMAIN domainNumber = 7
    addTlv(&curLen, buf, PTPMGMT_MANAGEMENT, m1, sizeof m1);
    uint8_t m2[10] = {0x12, 0x34, 0x15, 7, 0xf3, 0xa3, 0x17, 3, 5, 1};
    addTlv(&curLen, buf, PTPMGMT_ORGANIZATION_EXTENSION, m2, sizeof m2);
    uint8_t m3[10] = {0x23, 0x13, 0x27, 8, 7, 0xb2, 9, 7, 0xb5, 3};
    addTlv(&curLen, buf, PTPMGMT_ORGANIZATION_EXTENSION_PROPAGATE, m3, sizeof m3);
    uint8_t m4[10] = {0x45, 2, 0x29, 9, 0x19, 0xc5, 3, 9, 0xd7, 5};
    addTlv(&curLen, buf, PTPMGMT_ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE, m4,
        sizeof m4);
    // PRIORITY1, WRONG_LENGTH, "test"
    uint8_t m5[18] = {0, 3, 0x20, 5, 0, 0, 0, 0, 4, 116, 101, 115, 116};
    addTlv(&curLen, buf, PTPMGMT_MANAGEMENT_ERROR_STATUS, m5, sizeof m5);
    uint8_t m6[22] = {4, 0x80, 0, 0x76, 0x5c, 0xbb, 0xcb, 0xe3, 0xd4, 0x12,
            0x57, 0x89, 0x19, 0x33, 0x24, 5, 97, 108, 116, 101, 114
        };
    addTlv(&curLen, buf, PTPMGMT_ALTERNATE_TIME_OFFSET_INDICATOR, m6, sizeof m6);
    uint8_t m7[2] = {15, 7};
    addTlv(&curLen, buf, PTPMGMT_L1_SYNC, m7, sizeof m7);
    uint8_t m8[2] = {15, 15};
    addTlv(&curLen, buf, PTPMGMT_PORT_COMMUNICATION_AVAILABILITY, m8, sizeof m8);
    uint8_t m9[8] = {0, 1, 0, 4, 0x12, 0x34, 0x56, 0x78};
    addTlv(&curLen, buf, PTPMGMT_PROTOCOL_ADDRESS, m9, sizeof m9);
    uint8_t m10[4] = {0x99, 0x1a, 0x11, 0xbd};
    addTlv(&curLen, buf, PTPMGMT_CUMULATIVE_RATE_RATIO, m10, sizeof m10);
    uint8_t m11[16] = {196, 125, 70, 255, 254, 32, 172, 174, 5, 7, 9, 1,
            172, 201, 3, 45
        };
    addTlv(&curLen, buf, PTPMGMT_PATH_TRACE, m11, sizeof m11);
    uint8_t m12[78] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1,
            4, 0, 0, 0x90, 8, 0x20, 0x11, 0, 0x36, 0xf9, 0xdf, 0xb8,
            0x45, 0x38, 0xaf, 0xb7, 0x17, 0x94, 0xd2, 0xa1, 0x99, 0x1a, 0x11,
            0xbd, 0, 0x98, 0x41, 0, 2, 0x4e, 0x38, 0xd0, 0, 0,
            11, 0xc7, 0, 0x81, 4, 8, 0x22, 8, 0, 0, 0, 0,
            0x12, 0x43, 0x5b, 0x4a, 0xf4, 0xd4, 0x1e, 0x48, 0xbd, 0xde,
            0xfa, 0x5c, 0, 0x81, 0x90, 0x58, 0x24, 0x20, 0x38, 0x1a, 0, 0
        };
    addTlv(&curLen, buf, PTPMGMT_SLAVE_RX_SYNC_TIMING_DATA, m12, sizeof m12);
    uint8_t m13[56] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1, 7, 0,
            11, 0xe6, 0x81, 1, 0x14, 0, 2, 0x24, 4, 0, 0x81, 0x12, 0x14, 0, 2,
            0x20, 4, 0, 0xbe, 0xbd, 0xe0, 0,
            7, 3, 0x81, 0x12, 0x14, 0x50, 0xb0, 0x20, 4, 0, 0x98, 0x42, 0x14,
            0x50, 0xb0, 0x20, 4, 0, 0xbe, 0x95, 0x4e, 0xf0
        };
    addTlv(&curLen, buf, PTPMGMT_SLAVE_RX_SYNC_COMPUTED_DATA, m13, sizeof m13);
    uint8_t m14[36] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1, 9, 0,
            2, 0xf1, 0, 2, 9, 8, 2, 0x20,
            0x36, 0x61, 0x20, 0x10,
            9, 0xf3, 0, 0x20, 0, 0x90, 8, 0x40,
            0x36, 0x61, 0x6d, 0x7c
        };
    addTlv(&curLen, buf, PTPMGMT_SLAVE_TX_EVENT_TIMESTAMPS, m14, sizeof m14);
    uint8_t m15[88] = {171, 231, 0, 0, 0x80, 0x2c, 0x40, 0, 0x48, 0x10, 5, 0,
            64, 234, 32, 6, 102, 102, 102, 102, 0x91, 0x3c, 0x40, 0, 0x4e,
            0x80, 5, 0, 65, 0, 143, 2, 184, 81, 235, 133, 0x91, 0x60, 0, 0,
            0x4e, 0xB5, 5, 0, 65, 210, 102, 120, 12, 99, 231, 109, 0x91,
            0x60, 0, 0, 0x4e, 0x90, 0x90, 0x65, 65, 149, 197, 130, 9, 172,
            244, 31, 0x91, 0x6a, 0x54, 4, 0x4e, 0x94, 0x90, 0x65, 65, 199,
            136, 144, 30, 119, 223, 59
        };
    addTlv(&curLen, buf, PTPMGMT_ENHANCED_ACCURACY_METRICS, m15, sizeof m15);
    uint8_t m16[70] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1,
            8, 0x6e, 0x84, 0x10, 9, 0x42, 8, 0xa4, 0x39, 0x50, 0x44,
            0xd3, 0x48, 1, 0x20, 0x40, 0x10, 0x10, 0, 0, 0, 0x40, 8,
            0x42, 8, 0x80, 0x36, 0xa0, 0xf5, 2,
            0xc, 0x49, 0x80, 0x41, 0x21, 0x12, 8, 0xa4, 0x38, 0xde,
            0xa5, 0x3a, 0x8c, 0x42, 0xa2, 0x40, 0x10, 2, 0, 0, 0xc5,
            0, 8, 0x42, 8, 0xa4, 0x37, 6, 0x2c, 0xe2
        };
    addTlv(&curLen, buf, PTPMGMT_SLAVE_DELAY_TIMING_DATA_NP, m16, sizeof m16);
    // header.messageLength
    buf[2] = curLen >> 8;
    buf[3] = curLen & 0xff;
    cr_assert(eq(int, msg->parse(msg, buf, curLen), PTPMGMT_MNG_PARSE_ERROR_SIG));
    char *ret = ptpmgmt_json_msg2json(msg, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
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
            "}"));
    free(ret);
    msg->free(msg);
}

// Tests CLOCK_DESCRIPTION structure
Test(Tlv2JsonTest, CLOCK_DESCRIPTION)
{
    struct ptpmgmt_CLOCK_DESCRIPTION_t t;
    t.clockType = ptpmgmt_ordinaryClock;
    char txt1[] = "IEEE 802.3";
    t.physicalLayerProtocol.lengthField = strlen(txt1);
    t.physicalLayerProtocol.textField = txt1;
    t.physicalAddressLength = 6;
    t.physicalAddress = physicalAddress_v;
    t.protocolAddress.networkProtocol = ptpmgmt_IEEE_802_3;
    t.protocolAddress.addressLength = 6;
    t.protocolAddress.addressField = physicalAddress_v;
    t.manufacturerIdentity[0] = 1;
    t.manufacturerIdentity[1] = 2;
    t.manufacturerIdentity[2] = 3;
    char txt2[] = ";;";
    t.productDescription.lengthField = strlen(txt2);
    t.productDescription.textField = txt2;
    t.revisionData.lengthField = strlen(txt2);
    t.revisionData.textField = txt2;
    char txt3[] = "test123";
    t.userDescription.lengthField = strlen(txt3);
    t.userDescription.textField = txt3;
    t.profileIdentity[0] = 1;
    t.profileIdentity[1] = 2;
    t.profileIdentity[2] = 3;
    t.profileIdentity[3] = 4;
    t.profileIdentity[4] = 5;
    t.profileIdentity[5] = 6;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_CLOCK_DESCRIPTION, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
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
            "}"));
    free(ret);
}

// Tests USER_DESCRIPTION structure
Test(Tlv2JsonTest, USER_DESCRIPTION)
{
    struct ptpmgmt_USER_DESCRIPTION_t t;
    char txt[] = "test123";
    t.userDescription.lengthField = strlen(txt);
    t.userDescription.textField = txt;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_USER_DESCRIPTION, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"userDescription\" : \"test123\"\n"
            "}"));
    free(ret);
}

// Tests INITIALIZE structure
Test(Tlv2JsonTest, INITIALIZE)
{
    struct ptpmgmt_INITIALIZE_t t;
    t.initializationKey = 0x1234;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_INITIALIZE, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"initializationKey\" : 4660\n"
            "}"));
    free(ret);
    ret = ptpmgmt_json_tlv2json(PTPMGMT_INITIALIZE, &t, 3);
    cr_assert(eq(str, (char *)ret,
            "   {\n"
            "     \"initializationKey\" : 4660\n"
            "   }"));
    free(ret);
}

// Tests FAULT_LOG structure
Test(Tlv2JsonTest, FAULT_LOG)
{
    struct ptpmgmt_FAULT_LOG_t t;
    t.numberOfFaultRecords = 2;
    void *x = malloc(sizeof(struct ptpmgmt_FaultRecord_t) * t.numberOfFaultRecords);
    cr_assert(not(zero(ptr, x)));
    t.faultRecords = (struct ptpmgmt_FaultRecord_t *)x;
    t.faultRecords[0].faultRecordLength = 50;
    t.faultRecords[0].faultTime.secondsField = 9;
    t.faultRecords[0].faultTime.nanosecondsField = 709000;
    t.faultRecords[0].severityCode = ptpmgmt_F_Critical;
    char txt1[] = "error 1";
    t.faultRecords[0].faultName.lengthField = strlen(txt1);
    t.faultRecords[0].faultName.textField = txt1;
    char txt2[] = "test123";
    t.faultRecords[0].faultValue.lengthField = strlen(txt2);
    t.faultRecords[0].faultValue.textField = txt2;
    char txt3[] = "This is first record";
    t.faultRecords[0].faultDescription.lengthField = strlen(txt3);
    t.faultRecords[0].faultDescription.textField = txt3;
    t.faultRecords[1].faultRecordLength = 55;
    t.faultRecords[1].faultTime.secondsField = 1791;
    t.faultRecords[1].faultTime.nanosecondsField = 30190000;
    t.faultRecords[1].severityCode = ptpmgmt_F_Warning;
    char txt4[] = "error 2";
    t.faultRecords[1].faultName.lengthField = strlen(txt4);
    t.faultRecords[1].faultName.textField = txt4;
    char txt5[] = "test321";
    t.faultRecords[1].faultValue.lengthField = strlen(txt5);
    t.faultRecords[1].faultValue.textField = txt5;
    char txt6[] = "This is the second record";
    t.faultRecords[1].faultDescription.lengthField = strlen(txt6);
    t.faultRecords[1].faultDescription.textField = txt6;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_FAULT_LOG, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
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
            "      \"faultTime\" : 1791.030190000,\n"
            "      \"severityCode\" : \"Warning\",\n"
            "      \"faultName\" : \"error 2\",\n"
            "      \"faultValue\" : \"test321\",\n"
            "      \"faultDescription\" : \"This is the second record\"\n"
            "    }\n"
            "  ]\n"
            "}"));
    free(x);
    free(ret);
}

// Tests DEFAULT_DATA_SET structure
Test(Tlv2JsonTest, DEFAULT_DATA_SET)
{
    struct ptpmgmt_DEFAULT_DATA_SET_t t;
    t.flags = 0x3;
    t.numberPorts = 1;
    t.priority1 = 153;
    t.clockQuality.clockClass = 255;
    t.clockQuality.clockAccuracy = ptpmgmt_Accurate_Unknown;
    t.clockQuality.offsetScaledLogVariance = 0xffff;
    t.priority2 = 137;
    t.clockIdentity.v[0] = 196;
    t.clockIdentity.v[1] = 125;
    t.clockIdentity.v[2] = 70;
    t.clockIdentity.v[3] = 255;
    t.clockIdentity.v[4] = 254;
    t.clockIdentity.v[5] = 32;
    t.clockIdentity.v[6] = 172;
    t.clockIdentity.v[7] = 174;
    t.domainNumber = 0;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_DEFAULT_DATA_SET, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
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
            "}"));
    free(ret);
}

// Tests CURRENT_DATA_SET structure
Test(Tlv2JsonTest, CURRENT_DATA_SET)
{
    struct ptpmgmt_CURRENT_DATA_SET_t t;
    t.stepsRemoved = 0x1234;
    t.offsetFromMaster.scaledNanoseconds = 0x321047abcd541285LL;
    t.meanPathDelay.scaledNanoseconds = 0x0906050403020100LL;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_CURRENT_DATA_SET, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"stepsRemoved\" : 4660,\n"
            "  \"offsetFromMaster\" : 3607462104733586053,\n"
            "  \"meanPathDelay\" : 650212710990086400\n"
            "}"));
    free(ret);
}

// Tests PARENT_DATA_SET structure
Test(Tlv2JsonTest, PARENT_DATA_SET)
{
    struct ptpmgmt_PARENT_DATA_SET_t t;
    t.parentPortIdentity.clockIdentity.v[0] = 196;
    t.parentPortIdentity.clockIdentity.v[1] = 125;
    t.parentPortIdentity.clockIdentity.v[2] = 70;
    t.parentPortIdentity.clockIdentity.v[3] = 255;
    t.parentPortIdentity.clockIdentity.v[4] = 254;
    t.parentPortIdentity.clockIdentity.v[5] = 32;
    t.parentPortIdentity.clockIdentity.v[6] = 172;
    t.parentPortIdentity.clockIdentity.v[7] = 174;
    t.parentPortIdentity.portNumber = 5;
    t.flags = 1;
    t.observedParentOffsetScaledLogVariance = 0xffff;
    t.observedParentClockPhaseChangeRate = 0x7fffffff;
    t.grandmasterPriority1 = 255;
    t.grandmasterClockQuality.clockClass = 255;
    t.grandmasterClockQuality.clockAccuracy = ptpmgmt_Accurate_Unknown;
    t.grandmasterClockQuality.offsetScaledLogVariance = 0xffff;
    t.grandmasterPriority2 = 255;
    t.grandmasterIdentity.v[0] = 196;
    t.grandmasterIdentity.v[1] = 125;
    t.grandmasterIdentity.v[2] = 70;
    t.grandmasterIdentity.v[3] = 255;
    t.grandmasterIdentity.v[4] = 254;
    t.grandmasterIdentity.v[5] = 32;
    t.grandmasterIdentity.v[6] = 172;
    t.grandmasterIdentity.v[7] = 174;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_PARENT_DATA_SET, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
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
            "}"));
    free(ret);
}

// Tests TIME_PROPERTIES_DATA_SET structure
Test(Tlv2JsonTest, TIME_PROPERTIES_DATA_SET)
{
    struct ptpmgmt_TIME_PROPERTIES_DATA_SET_t t;
    t.currentUtcOffset = 37;
    t.flags = PTPMGMT_F_PTP; // ptpTimescale bit
    t.timeSource = PTPMGMT_INTERNAL_OSCILLATOR;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_TIME_PROPERTIES_DATA_SET, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"currentUtcOffset\" : 37,\n"
            "  \"leap61\" : false,\n"
            "  \"leap59\" : false,\n"
            "  \"currentUtcOffsetValid\" : false,\n"
            "  \"ptpTimescale\" : true,\n"
            "  \"timeTraceable\" : false,\n"
            "  \"frequencyTraceable\" : false,\n"
            "  \"timeSource\" : \"INTERNAL_OSCILLATOR\"\n"
            "}"));
    free(ret);
}

// Tests PORT_DATA_SET structure
Test(Tlv2JsonTest, PORT_DATA_SET)
{
    struct ptpmgmt_PORT_DATA_SET_t t;
    t.portIdentity.clockIdentity.v[0] = 196;
    t.portIdentity.clockIdentity.v[1] = 125;
    t.portIdentity.clockIdentity.v[2] = 70;
    t.portIdentity.clockIdentity.v[3] = 255;
    t.portIdentity.clockIdentity.v[4] = 254;
    t.portIdentity.clockIdentity.v[5] = 32;
    t.portIdentity.clockIdentity.v[6] = 172;
    t.portIdentity.clockIdentity.v[7] = 174;
    t.portIdentity.portNumber = 1;
    t.portState = PTPMGMT_LISTENING;
    t.logMinDelayReqInterval = 0;
    t.peerMeanPathDelay.scaledNanoseconds = 0;
    t.logAnnounceInterval = 1;
    t.announceReceiptTimeout = 3;
    t.logSyncInterval = 0;
    t.delayMechanism = PTPMGMT_P2P;
    t.logMinPdelayReqInterval = 0;
    t.versionNumber = 2;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_PORT_DATA_SET, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
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
            "}"));
    free(ret);
};

// Tests PRIORITY1 structure
Test(Tlv2JsonTest, PRIORITY1)
{
    struct ptpmgmt_PRIORITY1_t t;
    t.priority1 = 153;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_PRIORITY1, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"priority1\" : 153\n"
            "}"));
    free(ret);
}

// Tests PRIORITY2 structure
Test(Tlv2JsonTest, PRIORITY2)
{
    struct ptpmgmt_PRIORITY2_t t;
    t.priority2 = 137;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_PRIORITY2, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"priority2\" : 137\n"
            "}"));
    free(ret);
}

// Tests DOMAIN structure
Test(Tlv2JsonTest, DOMAIN)
{
    struct ptpmgmt_DOMAIN_t t;
    t.domainNumber = 7;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_DOMAIN, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"domainNumber\" : 7\n"
            "}"));
    free(ret);
}

// Tests SLAVE_ONLY structure
Test(Tlv2JsonTest, SLAVE_ONLY)
{
    struct ptpmgmt_SLAVE_ONLY_t t;
    t.flags = 1;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_SLAVE_ONLY, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"slaveOnly\" : true\n"
            "}"));
    free(ret);
}

// Tests LOG_ANNOUNCE_INTERVAL structure
Test(Tlv2JsonTest, LOG_ANNOUNCE_INTERVAL)
{
    struct ptpmgmt_LOG_ANNOUNCE_INTERVAL_t t;
    t.logAnnounceInterval = 1;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_LOG_ANNOUNCE_INTERVAL, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"logAnnounceInterval\" : 1\n"
            "}"));
    free(ret);
}

// Tests ANNOUNCE_RECEIPT_TIMEOUT structure
Test(Tlv2JsonTest, ANNOUNCE_RECEIPT_TIMEOUT)
{
    struct ptpmgmt_ANNOUNCE_RECEIPT_TIMEOUT_t t;
    t.announceReceiptTimeout = 3;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_ANNOUNCE_RECEIPT_TIMEOUT, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"announceReceiptTimeout\" : 3\n"
            "}"));
    free(ret);
}

// Tests LOG_SYNC_INTERVAL structure
Test(Tlv2JsonTest, LOG_SYNC_INTERVAL)
{
    struct ptpmgmt_LOG_SYNC_INTERVAL_t t;
    t.logSyncInterval = 7;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_LOG_SYNC_INTERVAL, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"logSyncInterval\" : 7\n"
            "}"));
    free(ret);
}

// Tests VERSION_NUMBER structure
Test(Tlv2JsonTest, VERSION_NUMBER)
{
    struct ptpmgmt_VERSION_NUMBER_t t;
    t.versionNumber = 2;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_VERSION_NUMBER, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"versionNumber\" : 2\n"
            "}"));
    free(ret);
}

// Tests TIME structure
Test(Tlv2JsonTest, TIME)
{
    struct ptpmgmt_TIME_t t;
    t.currentTime.secondsField = 13;
    t.currentTime.nanosecondsField = 150000000;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_TIME, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"currentTime\" : 13.150000000\n"
            "}"));
    free(ret);
}

// Tests CLOCK_ACCURACY structure
Test(Tlv2JsonTest, CLOCK_ACCURACY)
{
    struct ptpmgmt_CLOCK_ACCURACY_t t;
    t.clockAccuracy = ptpmgmt_Accurate_Unknown;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_CLOCK_ACCURACY, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"clockAccuracy\" : \"Unknown\"\n"
            "}"));
    free(ret);
}

// Tests UTC_PROPERTIES structure
Test(Tlv2JsonTest, UTC_PROPERTIES)
{
    struct ptpmgmt_UTC_PROPERTIES_t t;
    t.currentUtcOffset = -0x5433;
    t.flags = 7;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_UTC_PROPERTIES, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"currentUtcOffset\" : -21555,\n"
            "  \"leap61\" : true,\n"
            "  \"leap59\" : true,\n"
            "  \"currentUtcOffsetValid\" : true\n"
            "}"));
    free(ret);
}

// Tests TRACEABILITY_PROPERTIES structure
Test(Tlv2JsonTest, TRACEABILITY_PROPERTIES)
{
    struct ptpmgmt_TRACEABILITY_PROPERTIES_t t;
    t.flags = PTPMGMT_F_TTRA | PTPMGMT_F_FTRA;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_TRACEABILITY_PROPERTIES, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"timeTraceable\" : true,\n"
            "  \"frequencyTraceable\" : true\n"
            "}"));
    free(ret);
}

// Tests TIMESCALE_PROPERTIES structure
Test(Tlv2JsonTest, TIMESCALE_PROPERTIES)
{
    struct ptpmgmt_TIMESCALE_PROPERTIES_t t;
    t.flags = PTPMGMT_F_PTP;
    t.timeSource = PTPMGMT_HAND_SET;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_TIMESCALE_PROPERTIES, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"ptpTimescale\" : true,\n"
            "  \"timeSource\" : \"HAND_SET\"\n"
            "}"));
    free(ret);
}

// Tests UNICAST_NEGOTIATION_ENABLE structure
Test(Tlv2JsonTest, UNICAST_NEGOTIATION_ENABLE)
{
    struct ptpmgmt_UNICAST_NEGOTIATION_ENABLE_t t;
    t.flags = 1;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_UNICAST_NEGOTIATION_ENABLE, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"unicastNegotiationPortDS\" : true\n"
            "}"));
    free(ret);
}

// Tests PATH_TRACE_LIST structure
Test(Tlv2JsonTest, PATH_TRACE_LIST)
{
    struct ptpmgmt_PATH_TRACE_LIST_t t;
    void *x = malloc(sizeof(struct ptpmgmt_ClockIdentity_t) * 3);
    cr_assert(not(zero(ptr, x)));
    t.pathSequence = (struct ptpmgmt_ClockIdentity_t *)x;
    t.pathSequence[0].v[0] = 196;
    t.pathSequence[0].v[1] = 125;
    t.pathSequence[0].v[2] = 70;
    t.pathSequence[0].v[3] = 255;
    t.pathSequence[0].v[4] = 254;
    t.pathSequence[0].v[5] = 32;
    t.pathSequence[0].v[6] = 172;
    t.pathSequence[0].v[7] = 174;
    t.pathSequence[1].v[0] = 12;
    t.pathSequence[1].v[1] = 4;
    t.pathSequence[1].v[2] = 19;
    t.pathSequence[1].v[3] = 97;
    t.pathSequence[1].v[4] = 11;
    t.pathSequence[1].v[5] = 74;
    t.pathSequence[1].v[6] = 12;
    t.pathSequence[1].v[7] = 74;
    // Mark end of list
    t.pathSequence[2].v[0] = 0;
    t.pathSequence[2].v[1] = 0;
    t.pathSequence[2].v[2] = 0;
    t.pathSequence[2].v[3] = 0;
    t.pathSequence[2].v[4] = 0;
    t.pathSequence[2].v[5] = 0;
    t.pathSequence[2].v[6] = 0;
    t.pathSequence[2].v[7] = 0;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_PATH_TRACE_LIST, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"pathSequence\" :\n"
            "  [\n"
            "    \"c47d46.fffe.20acae\",\n"
            "    \"0c0413.610b.4a0c4a\"\n"
            "  ]\n"
            "}"));
    free(x);
    free(ret);
}

// Tests PATH_TRACE_ENABLE structure
Test(Tlv2JsonTest, PATH_TRACE_ENABLE)
{
    struct ptpmgmt_PATH_TRACE_ENABLE_t t;
    t.flags = 1;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_PATH_TRACE_ENABLE, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"pathTraceDS\" : true\n"
            "}"));
    free(ret);
}

// Tests GRANDMASTER_CLUSTER_TABLE structure
Test(Tlv2JsonTest, GRANDMASTER_CLUSTER_TABLE)
{
    struct ptpmgmt_GRANDMASTER_CLUSTER_TABLE_t t;
    t.logQueryInterval = -19;
    t.actualTableSize = 2;
    void *x = malloc(sizeof(struct ptpmgmt_PortAddress_t) * t.actualTableSize);
    cr_assert(not(zero(ptr, x)));
    t.PortAddress = (struct ptpmgmt_PortAddress_t *)x;
    t.PortAddress[0].networkProtocol = ptpmgmt_IEEE_802_3;
    t.PortAddress[0].addressLength = 6;
    t.PortAddress[0].addressField = physicalAddress_v;
    t.PortAddress[1].networkProtocol = ptpmgmt_UDP_IPv4;
    t.PortAddress[1].addressLength = 4;
    t.PortAddress[1].addressField = ip_v;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_GRANDMASTER_CLUSTER_TABLE, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
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
            "}"));
    free(x);
    free(ret);
}

// Tests UNICAST_MASTER_TABLE structure
Test(Tlv2JsonTest, UNICAST_MASTER_TABLE)
{
    struct ptpmgmt_UNICAST_MASTER_TABLE_t t;
    t.logQueryInterval = -19;
    t.actualTableSize = 2;
    void *x = malloc(sizeof(struct ptpmgmt_PortAddress_t) * t.actualTableSize);
    cr_assert(not(zero(ptr, x)));
    t.PortAddress = (struct ptpmgmt_PortAddress_t *)x;
    t.PortAddress[0].networkProtocol = ptpmgmt_IEEE_802_3;
    t.PortAddress[0].addressLength = 6;
    t.PortAddress[0].addressField = physicalAddress_v;
    t.PortAddress[1].networkProtocol = ptpmgmt_UDP_IPv4;
    t.PortAddress[1].addressLength = 4;
    t.PortAddress[1].addressField = ip_v;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_UNICAST_MASTER_TABLE, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
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
            "}"));
    free(x);
    free(ret);
}

// Tests UNICAST_MASTER_MAX_TABLE_SIZE structure
Test(Tlv2JsonTest, UNICAST_MASTER_MAX_TABLE_SIZE)
{
    struct ptpmgmt_UNICAST_MASTER_MAX_TABLE_SIZE_t t;
    t.maxTableSize = 0x2143;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_UNICAST_MASTER_MAX_TABLE_SIZE, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"maxTableSize\" : 8515\n"
            "}"));
    free(ret);
}

// Tests ACCEPTABLE_MASTER_TABLE structure
Test(Tlv2JsonTest, ACCEPTABLE_MASTER_TABLE)
{
    struct ptpmgmt_ACCEPTABLE_MASTER_TABLE_t t;
    t.actualTableSize = 2;
    void *x = malloc(sizeof(struct ptpmgmt_AcceptableMaster_t) * t.actualTableSize);
    cr_assert(not(zero(ptr, x)));
    t.list = (struct ptpmgmt_AcceptableMaster_t *)x;
    t.list[0].acceptablePortIdentity.clockIdentity.v[0] = 196;
    t.list[0].acceptablePortIdentity.clockIdentity.v[1] = 125;
    t.list[0].acceptablePortIdentity.clockIdentity.v[2] = 70;
    t.list[0].acceptablePortIdentity.clockIdentity.v[3] = 255;
    t.list[0].acceptablePortIdentity.clockIdentity.v[4] = 254;
    t.list[0].acceptablePortIdentity.clockIdentity.v[5] = 32;
    t.list[0].acceptablePortIdentity.clockIdentity.v[6] = 172;
    t.list[0].acceptablePortIdentity.clockIdentity.v[7] = 174;
    t.list[0].acceptablePortIdentity.portNumber = 1;
    t.list[0].alternatePriority1 = 127;
    t.list[1].acceptablePortIdentity.clockIdentity.v[0] = 9;
    t.list[1].acceptablePortIdentity.clockIdentity.v[1] = 8;
    t.list[1].acceptablePortIdentity.clockIdentity.v[2] = 7;
    t.list[1].acceptablePortIdentity.clockIdentity.v[3] = 6;
    t.list[1].acceptablePortIdentity.clockIdentity.v[4] = 5;
    t.list[1].acceptablePortIdentity.clockIdentity.v[5] = 4;
    t.list[1].acceptablePortIdentity.clockIdentity.v[6] = 1;
    t.list[1].acceptablePortIdentity.clockIdentity.v[7] = 7;
    t.list[1].acceptablePortIdentity.portNumber = 2;
    t.list[1].alternatePriority1 = 111;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_ACCEPTABLE_MASTER_TABLE, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
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
            "}"));
    free(x);
    free(ret);
}

// Tests ACCEPTABLE_MASTER_TABLE_ENABLED structure
Test(Tlv2JsonTest, ACCEPTABLE_MASTER_TABLE_ENABLED)
{
    struct ptpmgmt_ACCEPTABLE_MASTER_TABLE_ENABLED_t t;
    t.flags = 1;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_ACCEPTABLE_MASTER_TABLE_ENABLED, &t,
            0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"acceptableMasterPortDS\" : true\n"
            "}"));
    free(ret);
}

// Tests ACCEPTABLE_MASTER_MAX_TABLE_SIZE structure
Test(Tlv2JsonTest, ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
{
    struct ptpmgmt_ACCEPTABLE_MASTER_MAX_TABLE_SIZE_t t;
    t.maxTableSize = 0x67ba;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_ACCEPTABLE_MASTER_MAX_TABLE_SIZE, &t,
            0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"maxTableSize\" : 26554\n"
            "}"));
    free(ret);
}

// Tests ALTERNATE_MASTER structure
Test(Tlv2JsonTest, ALTERNATE_MASTER)
{
    struct ptpmgmt_ALTERNATE_MASTER_t t;
    t.flags = 1;
    t.logAlternateMulticastSyncInterval = -17;
    t.numberOfAlternateMasters = 210;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_ALTERNATE_MASTER, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"transmitAlternateMulticastSync\" : true,\n"
            "  \"logAlternateMulticastSyncInterval\" : -17,\n"
            "  \"numberOfAlternateMasters\" : 210\n"
            "}"));
    free(ret);
}

// Tests ALTERNATE_TIME_OFFSET_ENABLE structure
Test(Tlv2JsonTest, ALTERNATE_TIME_OFFSET_ENABLE)
{
    struct ptpmgmt_ALTERNATE_TIME_OFFSET_ENABLE_t t;
    t.keyField = 7;
    t.flags = 1;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_ALTERNATE_TIME_OFFSET_ENABLE, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"keyField\" : 7,\n"
            "  \"alternateTimescaleOffsetsDS\" : true\n"
            "}"));
    free(ret);
}

// Tests ALTERNATE_TIME_OFFSET_NAME structure
Test(Tlv2JsonTest, ALTERNATE_TIME_OFFSET_NAME)
{
    struct ptpmgmt_ALTERNATE_TIME_OFFSET_NAME_t t;
    t.keyField = 11;
    char txt[] = "123";
    t.displayName.lengthField = strlen(txt);
    t.displayName.textField = txt;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_ALTERNATE_TIME_OFFSET_NAME, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"keyField\" : 11,\n"
            "  \"displayName\" : \"123\"\n"
            "}"));
    free(ret);
}

// Tests ALTERNATE_TIME_OFFSET_MAX_KEY structure
Test(Tlv2JsonTest, ALTERNATE_TIME_OFFSET_MAX_KEY)
{
    struct ptpmgmt_ALTERNATE_TIME_OFFSET_MAX_KEY_t t;
    t.maxKey = 9;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_ALTERNATE_TIME_OFFSET_MAX_KEY, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"maxKey\" : 9\n"
            "}"));
    free(ret);
}

// Tests ALTERNATE_TIME_OFFSET_PROPERTIES structure
Test(Tlv2JsonTest, ALTERNATE_TIME_OFFSET_PROPERTIES)
{
    struct ptpmgmt_ALTERNATE_TIME_OFFSET_PROPERTIES_t t;
    t.keyField = 13;
    t.currentOffset = -2145493247;
    t.jumpSeconds = -2147413249;
    t.timeOfNextJump = 0x912478321891LL;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_ALTERNATE_TIME_OFFSET_PROPERTIES, &t,
            0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"keyField\" : 13,\n"
            "  \"currentOffset\" : -2145493247,\n"
            "  \"jumpSeconds\" : -2147413249,\n"
            "  \"timeOfNextJump\" : 159585821399185\n"
            "}"));
    free(ret);
}

// Tests TRANSPARENT_CLOCK_PORT_DATA_SET structure
Test(Tlv2JsonTest, TRANSPARENT_CLOCK_PORT_DATA_SET)
{
    struct ptpmgmt_TRANSPARENT_CLOCK_PORT_DATA_SET_t t;
    t.portIdentity.clockIdentity.v[0] = 196;
    t.portIdentity.clockIdentity.v[1] = 125;
    t.portIdentity.clockIdentity.v[2] = 70;
    t.portIdentity.clockIdentity.v[3] = 255;
    t.portIdentity.clockIdentity.v[4] = 254;
    t.portIdentity.clockIdentity.v[5] = 32;
    t.portIdentity.clockIdentity.v[6] = 172;
    t.portIdentity.clockIdentity.v[7] = 174;
    t.portIdentity.portNumber = 1;
    t.flags = 1;
    t.logMinPdelayReqInterval = -21;
    t.peerMeanPathDelay.scaledNanoseconds = 0xdcf87240dcd12301LL;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_TRANSPARENT_CLOCK_PORT_DATA_SET, &t,
            0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"portIdentity\" :\n"
            "  {\n"
            "    \"clockIdentity\" : \"c47d46.fffe.20acae\",\n"
            "    \"portNumber\" : 1\n"
            "  },\n"
            "  \"transparentClockPortDS\" : true,\n"
            "  \"logMinPdelayReqInterval\" : -21,\n"
            "  \"peerMeanPathDelay\" : -2524141968232996095\n"
            "}"));
    free(ret);
}

// Tests LOG_MIN_PDELAY_REQ_INTERVAL structure
Test(Tlv2JsonTest, LOG_MIN_PDELAY_REQ_INTERVAL)
{
    struct ptpmgmt_LOG_MIN_PDELAY_REQ_INTERVAL_t t;
    t.logMinPdelayReqInterval = 9;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_LOG_MIN_PDELAY_REQ_INTERVAL, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"logMinPdelayReqInterval\" : 9\n"
            "}"));
    free(ret);
}

// Tests TRANSPARENT_CLOCK_DEFAULT_DATA_SET structure
Test(Tlv2JsonTest, TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
{
    struct ptpmgmt_TRANSPARENT_CLOCK_DEFAULT_DATA_SET_t t;
    t.clockIdentity.v[0] = 196;
    t.clockIdentity.v[1] = 125;
    t.clockIdentity.v[2] = 70;
    t.clockIdentity.v[3] = 255;
    t.clockIdentity.v[4] = 254;
    t.clockIdentity.v[5] = 32;
    t.clockIdentity.v[6] = 172;
    t.clockIdentity.v[7] = 174;
    t.numberPorts = 0x177a;
    t.delayMechanism = PTPMGMT_NO_MECHANISM;
    t.primaryDomain = 18;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_TRANSPARENT_CLOCK_DEFAULT_DATA_SET,
            &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"clockIdentity\" : \"c47d46.fffe.20acae\",\n"
            "  \"numberPorts\" : 6010,\n"
            "  \"delayMechanism\" : \"NO_MECHANISM\",\n"
            "  \"primaryDomain\" : 18\n"
            "}"));
    free(ret);
}

// Tests PRIMARY_DOMAIN structure
Test(Tlv2JsonTest, PRIMARY_DOMAIN)
{
    struct ptpmgmt_PRIMARY_DOMAIN_t t;
    t.primaryDomain = 17;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_PRIMARY_DOMAIN, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"primaryDomain\" : 17\n"
            "}"));
    free(ret);
}

// Tests DELAY_MECHANISM structure
Test(Tlv2JsonTest, DELAY_MECHANISM)
{
    struct ptpmgmt_DELAY_MECHANISM_t t;
    t.delayMechanism = PTPMGMT_P2P;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_DELAY_MECHANISM, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"delayMechanism\" : \"P2P\"\n"
            "}"));
    free(ret);
}

// Tests EXTERNAL_PORT_CONFIGURATION_ENABLED structure
Test(Tlv2JsonTest, EXTERNAL_PORT_CONFIGURATION_ENABLED)
{
    struct ptpmgmt_EXTERNAL_PORT_CONFIGURATION_ENABLED_t t;
    t.flags = 1;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_EXTERNAL_PORT_CONFIGURATION_ENABLED,
            &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"externalPortConfiguration\" : true\n"
            "}"));
    free(ret);
}

// Tests MASTER_ONLY structure
Test(Tlv2JsonTest, MASTER_ONLY)
{
    struct ptpmgmt_MASTER_ONLY_t t;
    t.flags = 1;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_MASTER_ONLY, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"masterOnly\" : true\n"
            "}"));
    free(ret);
}

// Tests HOLDOVER_UPGRADE_ENABLE structure
Test(Tlv2JsonTest, HOLDOVER_UPGRADE_ENABLE)
{
    struct ptpmgmt_HOLDOVER_UPGRADE_ENABLE_t t;
    t.flags = 1;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_HOLDOVER_UPGRADE_ENABLE, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"holdoverUpgradeDS\" : true\n"
            "}"));
    free(ret);
}

// Tests EXT_PORT_CONFIG_PORT_DATA_SET structure
Test(Tlv2JsonTest, EXT_PORT_CONFIG_PORT_DATA_SET)
{
    struct ptpmgmt_EXT_PORT_CONFIG_PORT_DATA_SET_t t;
    t.flags = 1;
    t.desiredState = PTPMGMT_PASSIVE;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_EXT_PORT_CONFIG_PORT_DATA_SET, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"acceptableMasterPortDS\" : true,\n"
            "  \"desiredState\" : \"PASSIVE\"\n"
            "}"));
    free(ret);
}

// Tests SMPTE organization extension
Test(Tlv2JsonTest, SMPTE_ORGANIZATION_EXTENSION)
{
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    uint8_t b[100] = {0xd, 2, 0, 0x64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0x74, 0xda, 0x38, 0xff, 0xfe, 0xf6, 0x98, 0x5e, 0, 1, 0, 0, 4,
            0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 3,
            3, 3, 0, 0, 3, 0, 0x30, 0x68, 0x97, 0xe8, 0, 0, 1, 0, 0, 0, 0x1e, 0,
            0, 0, 1, 1
        };
    ptpmgmt_pMsgParams a = msg->getParams(msg);
    a->rcvSMPTEOrg = 1;
    cr_expect(msg->updateParams(msg, a));
    cr_assert(eq(int, msg->parse(msg, b, sizeof b), PTPMGMT_MNG_PARSE_ERROR_SMPTE));
    cr_assert(msg->isLastMsgSMPTE(msg));
    char *ret = ptpmgmt_json_msg2json(msg, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
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
            "}"));
    msg->free(msg);
    free(ret);
}

// Tests TIME_STATUS_NP structure
Test(Tlv2JsonTest, TIME_STATUS_NP)
{
    struct ptpmgmt_TIME_STATUS_NP_t t;
    t.master_offset = 0;
    t.ingress_time = 0;
    t.cumulativeScaledRateOffset = 0;
    t.scaledLastGmPhaseChange = 0;
    t.gmTimeBaseIndicator = 0;
    t.nanoseconds_msb = 0;
    t.nanoseconds_lsb = 0;
    t.fractional_nanoseconds = 0;
    t.gmPresent = 0;
    t.gmIdentity.v[0] = 196;
    t.gmIdentity.v[1] = 125;
    t.gmIdentity.v[2] = 70;
    t.gmIdentity.v[3] = 255;
    t.gmIdentity.v[4] = 254;
    t.gmIdentity.v[5] = 32;
    t.gmIdentity.v[6] = 172;
    t.gmIdentity.v[7] = 174;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_TIME_STATUS_NP, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
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
            "}"));
    free(ret);
}

// Tests GRANDMASTER_SETTINGS_NP structure
Test(Tlv2JsonTest, GRANDMASTER_SETTINGS_NP)
{
    struct ptpmgmt_GRANDMASTER_SETTINGS_NP_t t;
    t.clockQuality.clockClass = 255;
    t.clockQuality.clockAccuracy = ptpmgmt_Accurate_Unknown;
    t.clockQuality.offsetScaledLogVariance = 0xffff;
    t.currentUtcOffset = 37;
    t.flags = PTPMGMT_F_PTP;
    t.timeSource = PTPMGMT_INTERNAL_OSCILLATOR;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_GRANDMASTER_SETTINGS_NP, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
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
            "}"));
    free(ret);
}

// Tests PORT_DATA_SET_NP structure
Test(Tlv2JsonTest, PORT_DATA_SET_NP)
{
    struct ptpmgmt_PORT_DATA_SET_NP_t t;
    t.neighborPropDelayThresh = 20000000;
    t.asCapable = 1;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_PORT_DATA_SET_NP, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"neighborPropDelayThresh\" : 20000000,\n"
            "  \"asCapable\" : 1\n"
            "}"));
    free(ret);
}

// Tests SUBSCRIBE_EVENTS_NP structure
Test(Tlv2JsonTest, SUBSCRIBE_EVENTS_NP)
{
    struct ptpmgmt_SUBSCRIBE_EVENTS_NP_t t;
    t.duration = 0x1234;
    ptpmgmt_setEvent_lnp(&t, PTPMGMT_NOTIFY_PORT_STATE);
    ptpmgmt_setEvent_lnp(&t, PTPMGMT_NOTIFY_TIME_SYNC);
    ptpmgmt_setEvent_lnp(&t, PTPMGMT_NOTIFY_PARENT_DATA_SET);
    ptpmgmt_setEvent_lnp(&t, PTPMGMT_NOTIFY_CMLDS);
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_SUBSCRIBE_EVENTS_NP, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"duration\" : 4660,\n"
            "  \"NOTIFY_PORT_STATE\" : true,\n"
            "  \"NOTIFY_TIME_SYNC\" : true,\n"
            "  \"NOTIFY_PARENT_DATA_SET\" : true,\n"
            "  \"NOTIFY_CMLDS\" : true\n"
            "}"));
    free(ret);
}

// Tests PORT_PROPERTIES_NP structure
Test(Tlv2JsonTest, PORT_PROPERTIES_NP)
{
    struct ptpmgmt_PORT_PROPERTIES_NP_t t;
    t.portIdentity.clockIdentity.v[0] = 196;
    t.portIdentity.clockIdentity.v[1] = 125;
    t.portIdentity.clockIdentity.v[2] = 70;
    t.portIdentity.clockIdentity.v[3] = 255;
    t.portIdentity.clockIdentity.v[4] = 254;
    t.portIdentity.clockIdentity.v[5] = 32;
    t.portIdentity.clockIdentity.v[6] = 172;
    t.portIdentity.clockIdentity.v[7] = 174;
    t.portIdentity.portNumber = 1;
    t.portState = PTPMGMT_LISTENING;
    t.timestamping = PTPMGMT_TS_HARDWARE;
    char txt[] = "enp0s25";
    t.interface.lengthField = strlen(txt);
    t.interface.textField = txt;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_PORT_PROPERTIES_NP, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"portIdentity\" :\n"
            "  {\n"
            "    \"clockIdentity\" : \"c47d46.fffe.20acae\",\n"
            "    \"portNumber\" : 1\n"
            "  },\n"
            "  \"portState\" : \"LISTENING\",\n"
            "  \"timestamping\" : \"HARDWARE\",\n"
            "  \"interface\" : \"enp0s25\"\n"
            "}"));
    free(ret);
}

// Tests PORT_STATS_NP structure
Test(Tlv2JsonTest, PORT_STATS_NP)
{
    struct ptpmgmt_PORT_STATS_NP_t t;
    t.portIdentity.clockIdentity.v[0] = 196;
    t.portIdentity.clockIdentity.v[1] = 125;
    t.portIdentity.clockIdentity.v[2] = 70;
    t.portIdentity.clockIdentity.v[3] = 255;
    t.portIdentity.clockIdentity.v[4] = 254;
    t.portIdentity.clockIdentity.v[5] = 32;
    t.portIdentity.clockIdentity.v[6] = 172;
    t.portIdentity.clockIdentity.v[7] = 174;
    t.portIdentity.portNumber = 1;
    t.rxMsgType[PTPMGMT_STAT_SYNC] = 0;
    t.rxMsgType[PTPMGMT_STAT_DELAY_REQ] = 0;
    t.rxMsgType[PTPMGMT_STAT_PDELAY_REQ] = 0;
    t.rxMsgType[PTPMGMT_STAT_PDELAY_RESP] = 0;
    t.rxMsgType[PTPMGMT_STAT_FOLLOW_UP] = 0;
    t.rxMsgType[PTPMGMT_STAT_DELAY_RESP] = 0;
    t.rxMsgType[PTPMGMT_STAT_PDELAY_RESP_FOLLOW_UP] = 0;
    t.rxMsgType[PTPMGMT_STAT_ANNOUNCE] = 0;
    t.rxMsgType[PTPMGMT_STAT_SIGNALING] = 0;
    t.rxMsgType[PTPMGMT_STAT_MANAGEMENT] = 0;
    t.txMsgType[PTPMGMT_STAT_SYNC] = 0;
    t.txMsgType[PTPMGMT_STAT_DELAY_REQ] = 0;
    t.txMsgType[PTPMGMT_STAT_PDELAY_REQ] = 63346;
    t.txMsgType[PTPMGMT_STAT_PDELAY_RESP] = 0;
    t.txMsgType[PTPMGMT_STAT_FOLLOW_UP] = 0;
    t.txMsgType[PTPMGMT_STAT_DELAY_RESP] = 0;
    t.txMsgType[PTPMGMT_STAT_PDELAY_RESP_FOLLOW_UP] = 0;
    t.txMsgType[PTPMGMT_STAT_ANNOUNCE] = 0;
    t.txMsgType[PTPMGMT_STAT_SIGNALING] = 0;
    t.txMsgType[PTPMGMT_STAT_MANAGEMENT] = 0;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_PORT_STATS_NP, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
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
            "}"));
    free(ret);
}

// Tests SYNCHRONIZATION_UNCERTAIN_NP structure
Test(Tlv2JsonTest, SYNCHRONIZATION_UNCERTAIN_NP)
{
    struct ptpmgmt_SYNCHRONIZATION_UNCERTAIN_NP_t t;
    t.val = PTPMGMT_SYNC_UNCERTAIN_DONTCARE;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_SYNCHRONIZATION_UNCERTAIN_NP, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"val\" : 255\n"
            "}"));
    free(ret);
}

// Tests PORT_SERVICE_STATS_NP structure
Test(Tlv2JsonTest, PORT_SERVICE_STATS_NP)
{
    struct ptpmgmt_PORT_SERVICE_STATS_NP_t t;
    t.portIdentity.clockIdentity.v[0] = 196;
    t.portIdentity.clockIdentity.v[1] = 125;
    t.portIdentity.clockIdentity.v[2] = 70;
    t.portIdentity.clockIdentity.v[3] = 255;
    t.portIdentity.clockIdentity.v[4] = 254;
    t.portIdentity.clockIdentity.v[5] = 32;
    t.portIdentity.clockIdentity.v[6] = 172;
    t.portIdentity.clockIdentity.v[7] = 174;
    t.portIdentity.portNumber = 1;
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
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_PORT_SERVICE_STATS_NP, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
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
            "}"));
    free(ret);
}

// Tests UNICAST_MASTER_TABLE_NP structure
Test(Tlv2JsonTest, UNICAST_MASTER_TABLE_NP)
{
    struct ptpmgmt_UNICAST_MASTER_TABLE_NP_t t;
    t.actualTableSize = 1;
    void *x = malloc(sizeof(struct ptpmgmt_LinuxptpUnicastMaster_t) *
            t.actualTableSize);
    cr_assert(not(zero(ptr, x)));
    t.unicastMasters = (struct ptpmgmt_LinuxptpUnicastMaster_t *)x;
    t.unicastMasters[0].portIdentity.clockIdentity.v[0] = 196;
    t.unicastMasters[0].portIdentity.clockIdentity.v[1] = 125;
    t.unicastMasters[0].portIdentity.clockIdentity.v[2] = 70;
    t.unicastMasters[0].portIdentity.clockIdentity.v[3] = 255;
    t.unicastMasters[0].portIdentity.clockIdentity.v[4] = 254;
    t.unicastMasters[0].portIdentity.clockIdentity.v[5] = 32;
    t.unicastMasters[0].portIdentity.clockIdentity.v[6] = 172;
    t.unicastMasters[0].portIdentity.clockIdentity.v[7] = 174;
    t.unicastMasters[0].portIdentity.portNumber = 1;
    t.unicastMasters[0].clockQuality.clockClass = 255;
    t.unicastMasters[0].clockQuality.clockAccuracy = ptpmgmt_Accurate_Unknown;
    t.unicastMasters[0].clockQuality.offsetScaledLogVariance = 0xffff;
    t.unicastMasters[0].selected = 1;
    t.unicastMasters[0].portState = PTPMGMT_UC_HAVE_SYDY;
    t.unicastMasters[0].priority1 = 126;
    t.unicastMasters[0].priority2 = 134;
    t.unicastMasters[0].portAddress.networkProtocol = ptpmgmt_IEEE_802_3;
    t.unicastMasters[0].portAddress.addressLength = 6;
    t.unicastMasters[0].portAddress.addressField = physicalAddress_v;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_UNICAST_MASTER_TABLE_NP, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
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
            "}"));
    free(x);
    free(ret);
}

// Tests PORT_HWCLOCK_NP structure
Test(Tlv2JsonTest, PORT_HWCLOCK_NP)
{
    struct ptpmgmt_PORT_HWCLOCK_NP_t t;
    t.portIdentity.clockIdentity.v[0] = 196;
    t.portIdentity.clockIdentity.v[1] = 125;
    t.portIdentity.clockIdentity.v[2] = 70;
    t.portIdentity.clockIdentity.v[3] = 255;
    t.portIdentity.clockIdentity.v[4] = 254;
    t.portIdentity.clockIdentity.v[5] = 32;
    t.portIdentity.clockIdentity.v[6] = 172;
    t.portIdentity.clockIdentity.v[7] = 174;
    t.portIdentity.portNumber = 1;
    t.phc_index = 1;
    t.flags = 7;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_PORT_HWCLOCK_NP, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"portIdentity\" :\n"
            "  {\n"
            "    \"clockIdentity\" : \"c47d46.fffe.20acae\",\n"
            "    \"portNumber\" : 1\n"
            "  },\n"
            "  \"phc_index\" : 1,\n"
            "  \"flags\" : 7\n"
            "}"));
    free(ret);
}

// Tests POWER_PROFILE_SETTINGS_NP structure
Test(Tlv2JsonTest, POWER_PROFILE_SETTINGS_NP)
{
    struct ptpmgmt_POWER_PROFILE_SETTINGS_NP_t t;
    t.version = PTPMGMT_IEEE_C37_238_VERSION_2011;
    t.grandmasterID = 56230;
    t.grandmasterTimeInaccuracy = 4124796349;
    t.networkTimeInaccuracy = 3655058877;
    t.totalTimeInaccuracy = 4223530875;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_POWER_PROFILE_SETTINGS_NP, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"version\" : \"2011\",\n"
            "  \"grandmasterID\" : 56230,\n"
            "  \"grandmasterTimeInaccuracy\" : 4124796349,\n"
            "  \"networkTimeInaccuracy\" : 3655058877,\n"
            "  \"totalTimeInaccuracy\" : 4223530875\n"
            "}"));
    free(ret);
}

// Tests CMLDS_INFO_NP structure
Test(Tlv2JsonTest, CMLDS_INFO_NP)
{
    struct ptpmgmt_CMLDS_INFO_NP_t t;
    t.meanLinkDelay.scaledNanoseconds = 201548321LL;
    t.scaledNeighborRateRatio = 1842;
    t.as_capable = 1;
    char *ret = ptpmgmt_json_tlv2json(PTPMGMT_CMLDS_INFO_NP, &t, 0);
    cr_assert(not(zero(ptr, ret)));
    cr_assert(eq(str, (char *)ret,
            "{\n"
            "  \"meanLinkDelay\" : 201548321,\n"
            "  \"scaledNeighborRateRatio\" : 1842,\n"
            "  \"as_capable\" : 1\n"
            "}"));
    free(ret);
}
