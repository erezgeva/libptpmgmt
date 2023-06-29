/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Signals structures unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include <arpa/inet.h>
#include "sig.h"
#include "msg.h" // Use message to test the signal structures

#define clockId ("\xc4\x7d\x46\xff\xfe\x20\xac\xae")
#define bufSize (400)

enum ptpmgmt_MNG_PARSE_ERROR_e setUp(size_t *curLen, ptpmgmt_msg *m0,
    ptpmgmt_pMsgParams *a, uint8_t *buf)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    // signaling = 36 header + 10 targetPortIdentity = 44
    *curLen = 44;
    enum ptpmgmt_MNG_PARSE_ERROR_e ret = m->build(m, buf, bufSize, 1);
    buf[0] = (buf[0] & 0xf0) | ptpmgmt_Signaling; // messageType
    buf[32] = 5; // controlField
    *a = m->getParams(m);
    (*a)->rcvSignaling = true;
    *m0 = m;
    return ret;
}
void addTlv(uint8_t *buf, size_t *curLen, enum ptpmgmt_tlvType_e type,
    uint8_t *msg, size_t len)
{
    uint16_t *cur = (uint16_t *)(buf + *curLen);
    *cur++ = htons(type);
    *cur++ = htons(len);
    memcpy(cur, msg, len);
    *curLen += len + 4;
}

// Tests one managment TLV
Test(SigTest, OneManagmentTlvs)
{
    size_t curLen;
    ptpmgmt_msg m;
    ptpmgmt_pMsgParams a;
    uint8_t buf[bufSize];
    cr_assert(eq(int, setUp(&curLen, &m, &a, buf), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[4] = {0x20, 5, 137}; // PRIORITY1 priority1 = 137
    addTlv(buf, &curLen, PTPMGMT_MANAGEMENT, mb, sizeof mb);
    a->filterSignaling = false;
    cr_expect(m->updateParams(m, a));
    cr_assert(eq(int, m->parse(m, buf, curLen), PTPMGMT_MNG_PARSE_ERROR_SIG));
    cr_expect(m->isLastMsgSig(m));
    cr_expect(eq(int, m->getSigTlvsCount(m), 1));
    cr_assert(eq(int, m->getSigTlvType(m, 0), PTPMGMT_MANAGEMENT));
    cr_expect(eq(int, m->getSigMngTlvType(m, 0), PTPMGMT_PRIORITY1));
    const struct ptpmgmt_PRIORITY1_t *p =
        (const struct ptpmgmt_PRIORITY1_t *)m->getSigMngTlv(m, 0);
    cr_assert(not(zero(ptr, (void *)p)));
    cr_expect(eq(int, p->priority1, 137));
    m->free(m);
}

// Tests two managment TLVs
Test(SigTest, TwoManagmentTlvs)
{
    size_t curLen;
    ptpmgmt_msg m;
    ptpmgmt_pMsgParams a;
    uint8_t buf[bufSize];
    cr_assert(eq(int, setUp(&curLen, &m, &a, buf), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t m1[4] = {0x20, 6, 119}; // PRIORITY2 priority2 = 119
    uint8_t m2[4] = {0x20, 7, 7};  // DOMAIN domainNumber = 7
    addTlv(buf, &curLen, PTPMGMT_MANAGEMENT, m1, sizeof m1);
    addTlv(buf, &curLen, PTPMGMT_MANAGEMENT, m2, sizeof m2);
    a->filterSignaling = false;
    cr_expect(m->updateParams(m, a));
    cr_assert(eq(int, m->parse(m, buf, curLen), PTPMGMT_MNG_PARSE_ERROR_SIG));
    cr_expect(m->isLastMsgSig(m));
    cr_expect(eq(int, m->getSigTlvsCount(m), 2));
    cr_assert(eq(int, m->getSigTlvType(m, 0), PTPMGMT_MANAGEMENT));
    cr_expect(eq(int, m->getSigMngTlvType(m, 0), PTPMGMT_PRIORITY2));
    const struct ptpmgmt_PRIORITY2_t *p1 =
        (const struct ptpmgmt_PRIORITY2_t *)m->getSigMngTlv(m, 0);
    cr_assert(not(zero(ptr, (void *)p1)));
    cr_expect(eq(int, p1->priority2, 119));
    cr_assert(eq(int, m->getSigTlvType(m, 1), PTPMGMT_MANAGEMENT));
    cr_expect(eq(int, m->getSigMngTlvType(m, 1), PTPMGMT_DOMAIN));
    const struct ptpmgmt_DOMAIN_t *p2 =
        (const struct ptpmgmt_DOMAIN_t *)m->getSigMngTlv(m, 1);
    cr_assert(not(zero(ptr, (void *)p2)));
    cr_expect(eq(int, p2->domainNumber, 7));
    m->free(m);
}

// Tests organization and two managment TLVs
Test(SigTest, OrgTwoManagmentTlvs)
{
    size_t curLen;
    ptpmgmt_msg m;
    ptpmgmt_pMsgParams a;
    uint8_t buf[bufSize];
    cr_assert(eq(int, setUp(&curLen, &m, &a, buf), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t m1[4] = {0x20, 6, 119}; // PRIORITY2 priority2 = 119
    // ORGANIZATION_EXTENSION_PROPAGATE
    uint8_t m2[10] = {0x12, 0x34, 0x56, 7, 0x19, 0xa3, 0x17, 0x23, 0x75, 0x31};
    uint8_t m3[4] = {0x20, 7, 7};  // DOMAIN domainNumber = 7
    addTlv(buf, &curLen, PTPMGMT_MANAGEMENT, m1, sizeof m1);
    addTlv(buf, &curLen, PTPMGMT_ORGANIZATION_EXTENSION_PROPAGATE, m2, sizeof m2);
    addTlv(buf, &curLen, PTPMGMT_MANAGEMENT, m3, sizeof m3);
    a->filterSignaling = false;
    cr_expect(m->updateParams(m, a));
    cr_assert(eq(int, m->parse(m, buf, curLen), PTPMGMT_MNG_PARSE_ERROR_SIG));
    cr_expect(m->isLastMsgSig(m));
    cr_expect(eq(int, m->getSigTlvsCount(m), 3));
    cr_assert(eq(int, m->getSigTlvType(m, 0), PTPMGMT_MANAGEMENT));
    cr_expect(eq(int, m->getSigMngTlvType(m, 0), PTPMGMT_PRIORITY2));
    const struct ptpmgmt_PRIORITY2_t *p1 =
        (const struct ptpmgmt_PRIORITY2_t *)m->getSigMngTlv(m, 0);
    cr_assert(not(zero(ptr, (void *)p1)));
    cr_expect(eq(int, p1->priority2, 119));
    cr_assert(eq(int, m->getSigTlvType(m, 1),
            PTPMGMT_ORGANIZATION_EXTENSION_PROPAGATE));
    const struct ptpmgmt_ORGANIZATION_EXTENSION_t *p2 =
        (const struct ptpmgmt_ORGANIZATION_EXTENSION_t *)m->getSigTlv(m, 1);
    cr_assert(not(zero(ptr, (void *)p2)));
    cr_expect(eq(int, memcmp(p2->organizationId, "\x12\x34\x56", 3), 0));
    cr_expect(eq(int, memcmp(p2->organizationSubType, "\x7\x19\xa3", 3), 0));
    cr_expect(zero(memcmp(p2->dataField, "\x17\x23\x75\x31", 4)));
    cr_assert(eq(int, m->getSigTlvType(m, 2), PTPMGMT_MANAGEMENT));
    cr_expect(eq(int, m->getSigMngTlvType(m, 2), PTPMGMT_DOMAIN));
    const struct ptpmgmt_DOMAIN_t *p3 =
        (const struct ptpmgmt_DOMAIN_t *)m->getSigMngTlv(m, 2);
    cr_assert(not(zero(ptr, (void *)p3)));
    cr_expect(eq(int, p3->domainNumber, 7));
    m->free(m);
}

// Tests all organization TLVs
Test(SigTest, AllOrgTlvs)
{
    size_t curLen;
    ptpmgmt_msg m;
    ptpmgmt_pMsgParams a;
    uint8_t buf[bufSize];
    cr_assert(eq(int, setUp(&curLen, &m, &a, buf), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t m1[10] = {0x12, 0x34, 0x15, 7, 0xf3, 0xa3, 0x17, 3, 5, 1};
    uint8_t m2[10] = {0x23, 0x13, 0x27, 8, 7, 0xb2, 9, 7, 0xb5, 3};
    uint8_t m3[10] = {0x45, 2, 0x29, 9, 0x19, 0xc5, 3, 9, 0xd7, 5};
    addTlv(buf, &curLen, PTPMGMT_ORGANIZATION_EXTENSION, m1, sizeof m1);
    addTlv(buf, &curLen, PTPMGMT_ORGANIZATION_EXTENSION_PROPAGATE, m2, sizeof m2);
    addTlv(buf, &curLen, PTPMGMT_ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE, m3,
        sizeof m3);
    a->filterSignaling = false;
    cr_expect(m->updateParams(m, a));
    cr_assert(eq(int, m->parse(m, buf, curLen), PTPMGMT_MNG_PARSE_ERROR_SIG));
    cr_expect(m->isLastMsgSig(m));
    cr_expect(eq(int, m->getSigTlvsCount(m), 3));
    cr_assert(eq(int, m->getSigTlvType(m, 0), PTPMGMT_ORGANIZATION_EXTENSION));
    const struct ptpmgmt_ORGANIZATION_EXTENSION_t *p0 =
        (const struct ptpmgmt_ORGANIZATION_EXTENSION_t *)m->getSigTlv(m, 0);
    cr_assert(not(zero(ptr, (void *)p0)));
    cr_expect(eq(int, memcmp(p0->organizationId, "\x12\x34\x15", 3), 0));
    cr_expect(eq(int, memcmp(p0->organizationSubType, "\x7\xf3\xa3", 3), 0));
    cr_expect(zero(memcmp(p0->dataField, "\x17\x3\x05\x1", 4)));
    cr_assert(eq(int, m->getSigTlvType(m, 1),
            PTPMGMT_ORGANIZATION_EXTENSION_PROPAGATE));
    const struct ptpmgmt_ORGANIZATION_EXTENSION_t *p1 =
        (const struct ptpmgmt_ORGANIZATION_EXTENSION_t *)m->getSigTlv(m, 1);
    cr_assert(not(zero(ptr, (void *)p1)));
    cr_expect(eq(int, memcmp(p1->organizationId, "\x23\x13\x27", 3), 0));
    cr_expect(eq(int, memcmp(p1->organizationSubType, "\x8\x07\xb2", 3), 0));
    cr_expect(zero(memcmp(p1->dataField, "\x09\x7\xb5\x3", 4)));
    cr_assert(eq(int, m->getSigTlvType(m, 2),
            PTPMGMT_ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE));
    const struct ptpmgmt_ORGANIZATION_EXTENSION_t *p2 =
        (const struct ptpmgmt_ORGANIZATION_EXTENSION_t *)m->getSigTlv(m, 2);
    cr_assert(not(zero(ptr, (void *)p2)));
    cr_expect(eq(int, memcmp(p2->organizationId, "\x45\x02\x29", 3), 0));
    cr_expect(eq(int, memcmp(p2->organizationSubType, "\x9\x19\xc5", 3), 0));
    cr_expect(zero(memcmp(p2->dataField, "\x03\x9\xd7\x5", 4)));
    m->free(m);
}

// Tests filter with organization TLVs
Test(SigTest, FilterWithOrgTlvs)
{
    size_t curLen;
    ptpmgmt_msg m;
    ptpmgmt_pMsgParams a;
    uint8_t buf[bufSize];
    cr_assert(eq(int, setUp(&curLen, &m, &a, buf), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t m1[10] = {0x12, 0x34, 0x15, 7, 0xf3, 0xa3, 0x17, 3, 5, 1};
    uint8_t m2[10] = {0x23, 0x13, 0x27, 8, 7, 0xb2, 9, 7, 0xb5, 3};
    uint8_t m3[10] = {0x45, 2, 0x29, 9, 0x19, 0xc5, 3, 9, 0xd7, 5};
    addTlv(buf, &curLen, PTPMGMT_ORGANIZATION_EXTENSION, m1, sizeof m1);
    addTlv(buf, &curLen, PTPMGMT_ORGANIZATION_EXTENSION_PROPAGATE, m2, sizeof m2);
    addTlv(buf, &curLen, PTPMGMT_ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE, m3,
        sizeof m3);
    a->filterSignaling = true;
    a->allowSigTlv(a, PTPMGMT_ORGANIZATION_EXTENSION);
    a->allowSigTlv(a, PTPMGMT_ORGANIZATION_EXTENSION_PROPAGATE);
    a->allowSigTlv(a, PTPMGMT_ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE);
    cr_expect(a->isSigTlv(a, PTPMGMT_ORGANIZATION_EXTENSION));
    cr_expect(a->isSigTlv(a, PTPMGMT_ORGANIZATION_EXTENSION_PROPAGATE));
    cr_expect(a->isSigTlv(a, PTPMGMT_ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE));
    cr_expect(eq(int, a->countSigTlvs(a), 3));
    // Filter the middle TLV :-)
    a->removeSigTlv(a, PTPMGMT_ORGANIZATION_EXTENSION_PROPAGATE);
    cr_expect(a->isSigTlv(a, PTPMGMT_ORGANIZATION_EXTENSION));
    cr_expect(not(a->isSigTlv(a, PTPMGMT_ORGANIZATION_EXTENSION_PROPAGATE)));
    cr_expect(a->isSigTlv(a, PTPMGMT_ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE));
    cr_expect(eq(int, a->countSigTlvs(a), 2));
    cr_expect(m->updateParams(m, a));
    cr_assert(eq(int, m->parse(m, buf, curLen), PTPMGMT_MNG_PARSE_ERROR_SIG));
    cr_expect(m->isLastMsgSig(m));
    cr_expect(eq(int, m->getSigTlvsCount(m), 2));
    cr_assert(eq(int, m->getSigTlvType(m, 0), PTPMGMT_ORGANIZATION_EXTENSION));
    const struct ptpmgmt_ORGANIZATION_EXTENSION_t *p0 =
        (const struct ptpmgmt_ORGANIZATION_EXTENSION_t *)m->getSigTlv(m, 0);
    cr_assert(not(zero(ptr, (void *)p0)));
    cr_expect(eq(int, memcmp(p0->organizationId, "\x12\x34\x15", 3), 0));
    cr_expect(eq(int, memcmp(p0->organizationSubType, "\x7\xf3\xa3", 3), 0));
    cr_expect(zero(memcmp(p0->dataField, "\x17\x3\x05\x1", 4)));
    cr_assert(eq(int, m->getSigTlvType(m, 1),
            PTPMGMT_ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE));
    const struct ptpmgmt_ORGANIZATION_EXTENSION_t *p2 =
        (const struct ptpmgmt_ORGANIZATION_EXTENSION_t *)m->getSigTlv(m, 1);
    cr_assert(not(zero(ptr, (void *)p2)));
    cr_expect(eq(int, memcmp(p2->organizationId, "\x45\x02\x29", 3), 0));
    cr_expect(eq(int, memcmp(p2->organizationSubType, "\x9\x19\xc5", 3), 0));
    cr_expect(zero(memcmp(p2->dataField, "\x03\x9\xd7\x5", 4)));
    m->free(m);
}

// Tests managment error, alternate time offset,
//  layer 1 synchronization, port communication availability,
//  protocol address and cumulative rate ratio TLVs
Test(SigTest, MngErrMoreTlvs)
{
    size_t curLen;
    ptpmgmt_msg m;
    ptpmgmt_pMsgParams a;
    uint8_t buf[bufSize];
    cr_assert(eq(int, setUp(&curLen, &m, &a, buf), PTPMGMT_MNG_PARSE_ERROR_OK));
    // PRIORITY1, WRONG_LENGTH, "test"
    uint8_t m0[18] = {0, 3, 0x20, 5, 0, 0, 0, 0, 4, 116, 101, 115, 116};
    addTlv(buf, &curLen, PTPMGMT_MANAGEMENT_ERROR_STATUS, m0, sizeof m0);
    uint8_t m1[22] = {4, 0x80, 0, 0x76, 0x5c, 0xbb, 0xcb, 0xe3, 0xd4, 0x12,
            0x57, 0x89, 0x19, 0x33, 0x24, 5, 97, 108, 116, 101, 114
        };
    addTlv(buf, &curLen, PTPMGMT_ALTERNATE_TIME_OFFSET_INDICATOR, m1, sizeof m1);
    uint8_t m2[2] = {15, 7};
    addTlv(buf, &curLen, PTPMGMT_L1_SYNC, m2, sizeof m2);
    uint8_t m3[2] = {15, 15};
    addTlv(buf, &curLen, PTPMGMT_PORT_COMMUNICATION_AVAILABILITY, m3, sizeof m3);
    uint8_t m4[8] = {0, 1, 0, 4, 0x12, 0x34, 0x56, 0x78};
    addTlv(buf, &curLen, PTPMGMT_PROTOCOL_ADDRESS, m4, sizeof m4);
    uint8_t m5[4] = {0x99, 0x1a, 0x11, 0xbd};
    addTlv(buf, &curLen, PTPMGMT_CUMULATIVE_RATE_RATIO, m5, sizeof m5);
    a->filterSignaling = false;
    cr_expect(m->updateParams(m, a));
    cr_assert(eq(int, m->parse(m, buf, curLen), PTPMGMT_MNG_PARSE_ERROR_SIG));
    cr_expect(m->isLastMsgSig(m));
    cr_expect(eq(int, m->getSigTlvsCount(m), 6));
    cr_assert(eq(int, m->getSigTlvType(m, 0), PTPMGMT_MANAGEMENT_ERROR_STATUS));
    const struct ptpmgmt_MANAGEMENT_ERROR_STATUS_t *p0 =
        (const struct ptpmgmt_MANAGEMENT_ERROR_STATUS_t *)m->getSigTlv(m, 0);
    cr_assert(not(zero(ptr, (void *)p0)));
    cr_expect(eq(int, p0->managementId, PTPMGMT_PRIORITY1));
    cr_expect(eq(int, p0->managementErrorId, PTPMGMT_WRONG_LENGTH));
    cr_expect(eq(int, p0->displayData.lengthField, 4));
    cr_expect(eq(str, (char *)p0->displayData.textField, "test"));
    cr_assert(eq(int, m->getSigTlvType(m, 1),
            PTPMGMT_ALTERNATE_TIME_OFFSET_INDICATOR));
    const struct ptpmgmt_ALTERNATE_TIME_OFFSET_INDICATOR_t *p1 =
        (const struct ptpmgmt_ALTERNATE_TIME_OFFSET_INDICATOR_t *)
        m->getSigTlv(m, 1);
    cr_assert(not(zero(ptr, (void *)p1)));
    cr_expect(eq(int, p1->keyField, 4));
    cr_expect(eq(int, p1->currentOffset, -2147453348));
    cr_expect(eq(int, p1->jumpSeconds, -1144265772));
    cr_expect(eq(u64, p1->timeOfNextJump, 0x125789193324));
    cr_expect(eq(int, p1->displayName.lengthField, 5));
    cr_expect(eq(str, (char *)p1->displayName.textField, "alter"));
    cr_assert(eq(int, m->getSigTlvType(m, 2), PTPMGMT_L1_SYNC));
    const struct ptpmgmt_L1_SYNC_t *p2 =
        (const struct ptpmgmt_L1_SYNC_t *)m->getSigTlv(m, 2);
    cr_assert(not(zero(ptr, (void *)p2)));
    cr_expect(eq(int, p2->flags1, 15));
    cr_expect(eq(int, p2->flags2, 7));
    cr_assert(eq(int, m->getSigTlvType(m, 3),
            PTPMGMT_PORT_COMMUNICATION_AVAILABILITY));
    const struct ptpmgmt_PORT_COMMUNICATION_AVAILABILITY_t *p3 =
        (const struct ptpmgmt_PORT_COMMUNICATION_AVAILABILITY_t *)
        m->getSigTlv(m, 3);
    cr_assert(not(zero(ptr, (void *)p3)));
    cr_expect(eq(int, p3->syncMessageAvailability, 15));
    cr_expect(eq(int, p3->delayRespMessageAvailability, 15));
    cr_assert(eq(int, m->getSigTlvType(m, 4), PTPMGMT_PROTOCOL_ADDRESS));
    const struct ptpmgmt_PROTOCOL_ADDRESS_t *p4 =
        (const struct ptpmgmt_PROTOCOL_ADDRESS_t *)m->getSigTlv(m, 4);
    cr_assert(not(zero(ptr, (void *)p4)));
    cr_expect(eq(int, p4->portProtocolAddress.networkProtocol, ptpmgmt_UDP_IPv4));
    cr_expect(eq(u16, p4->portProtocolAddress.addressLength, 4));
    cr_expect(zero(memcmp(p4->portProtocolAddress.addressField, "\x12\x34\x56\x78",
                4)));
    cr_assert(eq(int, m->getSigTlvType(m, 5), PTPMGMT_CUMULATIVE_RATE_RATIO));
    const struct ptpmgmt_CUMULATIVE_RATE_RATIO_t *p5 =
        (const struct ptpmgmt_CUMULATIVE_RATE_RATIO_t *)m->getSigTlv(m, 5);
    cr_assert(not(zero(ptr, (void *)p5)));
    cr_expect(eq(i32, p5->scaledCumulativeRateRatio, -1726344771));
    m->free(m);
}

// Tests vector TLVs
Test(SigTest, VectorTlvs)
{
    size_t curLen;
    ptpmgmt_msg m;
    ptpmgmt_pMsgParams a;
    uint8_t buf[bufSize];
    cr_assert(eq(int, setUp(&curLen, &m, &a, buf), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t m0[16] = {196, 125, 70, 255, 254, 32, 172, 174, 5, 7, 9, 1,
            172, 201, 3, 45
        };
    addTlv(buf, &curLen, PTPMGMT_PATH_TRACE, m0, sizeof m0);
    uint8_t m1[78] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1,
            4, 0, 0, 0x90, 8, 0x20, 0x11, 0, 0x36, 0xf9, 0xdf, 0xb8,
            0x45, 0x38, 0xaf, 0xb7, 0x17, 0x94, 0xd2, 0xa1, 0x99, 0x1a, 0x11,
            0xbd, 0, 0x98, 0x41, 0, 2, 0x4e, 0x38, 0xd0, 0, 0,
            11, 0xc7, 0, 0x81, 4, 8, 0x22, 8, 0, 0, 0, 0,
            0x12, 0x43, 0x5b, 0x4a, 0xf4, 0xd4, 0x1e, 0x48, 0xbd, 0xde,
            0xfa, 0x5c, 0, 0x81, 0x90, 0x58, 0x24, 0x20, 0x38, 0x1a, 0, 0
        };
    addTlv(buf, &curLen, PTPMGMT_SLAVE_RX_SYNC_TIMING_DATA, m1, sizeof m1);
    uint8_t m2[56] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1, 7, 0,
            11, 0xe6, 0x81, 1, 0x14, 0, 2, 0x24, 4, 0, 0x81, 0x12, 0x14, 0, 2,
            0x20, 4, 0, 0xbe, 0xbd, 0xe0, 0,
            7, 3, 0x81, 0x12, 0x14, 0x50, 0xb0, 0x20, 4, 0, 0x98, 0x42, 0x14,
            0x50, 0xb0, 0x20, 4, 0, 0xbe, 0x95, 0x4e, 0xf0
        };
    addTlv(buf, &curLen, PTPMGMT_SLAVE_RX_SYNC_COMPUTED_DATA, m2, sizeof m2);
    uint8_t m3[36] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1, 9, 0,
            2, 0xf1, 0, 2, 9, 8, 2, 0x20,
            0x36, 0x61, 0x20, 0x10,
            9, 0xf3, 0, 0x20, 0, 0x90, 8, 0x40,
            0x36, 0x61, 0x6d, 0x7c
        };
    addTlv(buf, &curLen, PTPMGMT_SLAVE_TX_EVENT_TIMESTAMPS, m3, sizeof m3);
    a->filterSignaling = false;
    cr_expect(m->updateParams(m, a));
    cr_assert(eq(int, m->parse(m, buf, curLen), PTPMGMT_MNG_PARSE_ERROR_SIG));
    cr_expect(m->isLastMsgSig(m));
    cr_expect(eq(int, m->getSigTlvsCount(m), 4));
    cr_assert(eq(int, m->getSigTlvType(m, 0), PTPMGMT_PATH_TRACE));
    const struct ptpmgmt_PATH_TRACE_t *p0 =
        (const struct ptpmgmt_PATH_TRACE_t *)m->getSigTlv(m, 0);
    cr_assert(not(zero(ptr, (void *)p0)));
    cr_expect(zero(memcmp(p0->pathSequence[0].v, clockId, 8)));
    cr_expect(zero(memcmp(p0->pathSequence[1].v, "\x5\x7\x9\x1\xac\xc9\x3\x2d",
                8)));
    cr_expect(zero(memcmp(p0->pathSequence[2].v, "\0\0\0\0\0\0\0\0", 8)));
    cr_assert(eq(int, m->getSigTlvType(m, 1), PTPMGMT_SLAVE_RX_SYNC_TIMING_DATA));
    const struct ptpmgmt_SLAVE_RX_SYNC_TIMING_DATA_t *p1 =
        (const struct ptpmgmt_SLAVE_RX_SYNC_TIMING_DATA_t *)m->getSigTlv(m, 1);
    cr_assert(not(zero(ptr, (void *)p1)));
    cr_expect(eq(u16, p1->syncSourcePortIdentity.portNumber, 1));
    cr_expect(zero(memcmp(p1->syncSourcePortIdentity.clockIdentity.v, clockId, 8)));
    cr_expect(eq(u16, p1->list[0].sequenceId, 1024));
    cr_expect(eq(u64, p1->list[0].syncOriginTimestamp.secondsField, 618611609856));
    cr_expect(eq(u32, p1->list[0].syncOriginTimestamp.nanosecondsField, 922345400));
    cr_expect(eq(i64, p1->list[0].totalCorrectionField.scaledNanoseconds,
            0x4538afb71794d2a1));
    cr_expect(eq(int, p1->list[0].scaledCumulativeRateOffset, -1726344771));
    cr_expect(eq(u64, p1->list[0].syncEventIngressTimestamp.secondsField,
            653925548622));
    cr_expect(eq(u32, p1->list[0].syncEventIngressTimestamp.nanosecondsField,
            953155584));
    cr_expect(eq(u16, p1->list[1].sequenceId, 3015));
    cr_expect(eq(u64, p1->list[1].syncOriginTimestamp.secondsField, 554118423048));
    cr_expect(eq(int, p1->list[1].syncOriginTimestamp.nanosecondsField, 0));
    cr_expect(eq(i64, p1->list[1].totalCorrectionField.scaledNanoseconds,
            0x12435b4af4d41e48));
    cr_expect(eq(int, p1->list[1].scaledCumulativeRateOffset, -1109460388));
    cr_expect(eq(u64, p1->list[1].syncEventIngressTimestamp.secondsField,
            556472476704));
    cr_expect(eq(u32, p1->list[1].syncEventIngressTimestamp.nanosecondsField,
            941228032));
    cr_expect(zero(u16, p1->list[2].sequenceId));
    cr_expect(zero(u64, p1->list[2].syncOriginTimestamp.secondsField));
    cr_expect(zero(int, p1->list[2].syncOriginTimestamp.nanosecondsField));
    cr_expect(zero(i64, p1->list[2].totalCorrectionField.scaledNanoseconds));
    cr_expect(zero(int, p1->list[2].scaledCumulativeRateOffset));
    cr_expect(zero(u64, p1->list[2].syncEventIngressTimestamp.secondsField));
    cr_expect(zero(u32, p1->list[2].syncEventIngressTimestamp.nanosecondsField));
    cr_assert(eq(int, m->getSigTlvType(m, 2), PTPMGMT_SLAVE_RX_SYNC_COMPUTED_DATA));
    const struct ptpmgmt_SLAVE_RX_SYNC_COMPUTED_DATA_t *p2 =
        (const struct ptpmgmt_SLAVE_RX_SYNC_COMPUTED_DATA_t *)m->getSigTlv(m, 2);
    cr_assert(not(zero(ptr, (void *)p2)));
    cr_expect(eq(u16, p2->sourcePortIdentity.portNumber, 1));
    cr_expect(zero(memcmp(p2->sourcePortIdentity.clockIdentity.v, clockId, 8)));
    cr_expect(eq(u16, p2->list[0].sequenceId, 3046));
    cr_expect(eq(i64, p2->list[0].offsetFromMaster.scaledNanoseconds,
            0x8101140002240400));
    cr_expect(eq(i64, p2->list[0].meanPathDelay.scaledNanoseconds,
            0x8112140002200400));
    cr_expect(eq(int, p2->list[0].scaledNeighborRateRatio, -1094852608));
    cr_expect(eq(u16, p2->list[1].sequenceId, 1795));
    cr_expect(eq(i64, p2->list[1].offsetFromMaster.scaledNanoseconds,
            0x81121450b0200400));
    cr_expect(eq(i64, p2->list[1].meanPathDelay.scaledNanoseconds,
            0x98421450b0200400));
    cr_expect(eq(int, p2->list[1].scaledNeighborRateRatio, -1097511184));
    cr_assert(eq(int, m->getSigTlvType(m, 3), PTPMGMT_SLAVE_TX_EVENT_TIMESTAMPS));
    const struct ptpmgmt_SLAVE_TX_EVENT_TIMESTAMPS_t *p3 =
        (const struct ptpmgmt_SLAVE_TX_EVENT_TIMESTAMPS_t *)m->getSigTlv(m, 3);
    cr_assert(not(zero(ptr, (void *)p3)));
    cr_expect(eq(u16, p3->sourcePortIdentity.portNumber, 1));
    cr_expect(zero(memcmp(p3->sourcePortIdentity.clockIdentity.v, clockId, 8)));
    cr_expect(eq(int, p3->eventMessageType, ptpmgmt_Delay_Resp));
    cr_expect(eq(u16, p3->list[0].sequenceId, 753));
    cr_expect(eq(u64, p3->list[0].eventEgressTimestamp.secondsField, 8741454368));
    cr_expect(eq(u32, p3->list[0].eventEgressTimestamp.nanosecondsField,
            912334864));
    cr_expect(eq(u16, p3->list[1].sequenceId, 2547));
    cr_expect(eq(u64, p3->list[1].eventEgressTimestamp.secondsField, 137448392768));
    cr_expect(eq(u32, p3->list[1].eventEgressTimestamp.nanosecondsField,
            912354684));
    m->free(m);
}

// Tests enhanced accuracy_metrics TLV
Test(SigTest, AccuracyTlv)
{
    size_t curLen;
    ptpmgmt_msg m;
    ptpmgmt_pMsgParams a;
    uint8_t buf[bufSize];
    cr_assert(eq(int, setUp(&curLen, &m, &a, buf), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[88] = {171, 231, 0, 0, 0x80, 0x2c, 0x40, 0, 0x48, 0x10, 5, 0,
            64, 234, 32, 6, 102, 102, 102, 102, 0x91, 0x3c, 0x40, 0, 0x4e,
            0x80, 5, 0, 65, 0, 143, 2, 184, 81, 235, 133, 0x91, 0x60, 0, 0,
            0x4e, 0xB5, 5, 0, 65, 210, 102, 120, 12, 99, 231, 109, 0x91,
            0x60, 0, 0, 0x4e, 0x90, 0x90, 0x65, 65, 149, 197, 130, 9, 172,
            244, 31, 0x91, 0x6a, 0x54, 4, 0x4e, 0x94, 0x90, 0x65, 65, 199,
            136, 144, 30, 119, 223, 59
        };
    addTlv(buf, &curLen, PTPMGMT_ENHANCED_ACCURACY_METRICS, mb, sizeof mb);
    a->filterSignaling = false;
    cr_expect(m->updateParams(m, a));
    cr_assert(eq(int, m->parse(m, buf, curLen), PTPMGMT_MNG_PARSE_ERROR_SIG));
    cr_expect(m->isLastMsgSig(m));
    cr_expect(eq(int, m->getSigTlvsCount(m), 1));
    cr_assert(eq(int, m->getSigTlvType(m, 0), PTPMGMT_ENHANCED_ACCURACY_METRICS));
    const struct ptpmgmt_ENHANCED_ACCURACY_METRICS_t *p =
        (const struct ptpmgmt_ENHANCED_ACCURACY_METRICS_t *)m->getSigTlv(m, 0);
    cr_assert(not(zero(ptr, (void *)p)));
    cr_expect(eq(int, p->bcHopCount, 171));
    cr_expect(eq(int, p->tcHopCount, 231));
    cr_expect(eq(i64, p->maxGmInaccuracy.scaledNanoseconds, 9235827305783231744ul));
    cr_expect(eq(int, p->varGmInaccuracy, 53504.2));
    cr_expect(eq(i64, p->maxTransientInaccuracy.scaledNanoseconds,
            10465310004163380480ul));
    cr_expect(eq(int, p->varTransientInaccuracy, 135648.34));
    cr_expect(eq(i64, p->maxDynamicInaccuracy.scaledNanoseconds,
            10475372734584259840ul));
    cr_expect(eq(int, p->varDynamicInaccuracy, 1234821169.561));
    cr_expect(eq(i64, p->maxStaticInstanceInaccuracy.scaledNanoseconds,
            10475372734581870693ul));
    cr_expect(eq(int, p->varStaticInstanceInaccuracy, 91316354.4189));
    cr_expect(eq(i64, p->maxStaticMediumInaccuracy.scaledNanoseconds,
            10478279860505841765ul));
    cr_expect(eq(int, p->varStaticMediumInaccuracy, 789651516.9365));
    m->free(m);
}

// Tests Linuxptp TLVs
Test(SigTest, LinuxptpTlvs)
{
    size_t curLen;
    ptpmgmt_msg m;
    ptpmgmt_pMsgParams a;
    uint8_t buf[bufSize];
    cr_assert(eq(int, setUp(&curLen, &m, &a, buf), PTPMGMT_MNG_PARSE_ERROR_OK));
    uint8_t mb[70] = {196, 125, 70, 255, 254, 32, 172, 174, 0, 1,
            8, 0x6e, 0x84, 0x10, 9, 0x42, 8, 0xa4, 0x39, 0x50, 0x44,
            0xd3, 0x48, 1, 0x20, 0x40, 0x10, 0x10, 0, 0, 0, 0x40, 8,
            0x42, 8, 0x80, 0x36, 0xa0, 0xf5, 2,
            0xc, 0x49, 0x80, 0x41, 0x21, 0x12, 8, 0xa4, 0x38, 0xde,
            0xa5, 0x3a, 0x8c, 0x42, 0xa2, 0x40, 0x10, 2, 0, 0, 0xc5,
            0, 8, 0x42, 8, 0xa4, 0x37, 6, 0x2c, 0xe2
        };
    addTlv(buf, &curLen, PTPMGMT_SLAVE_DELAY_TIMING_DATA_NP, mb, sizeof mb);
    a->filterSignaling = false;
    a->implementSpecific = ptpmgmt_noImplementSpecific;
    cr_expect(m->updateParams(m, a));
    cr_assert(eq(int, m->parse(m, buf, curLen), PTPMGMT_MNG_PARSE_ERROR_SIG));
    cr_expect(m->isLastMsgSig(m));
    cr_expect(eq(int, m->getSigTlvsCount(m), 0));
    a->implementSpecific = ptpmgmt_linuxptp;
    cr_expect(m->updateParams(m, a));
    cr_assert(eq(int, m->parse(m, buf, curLen), PTPMGMT_MNG_PARSE_ERROR_SIG));
    cr_expect(m->isLastMsgSig(m));
    cr_expect(eq(int, m->getSigTlvsCount(m), 1));
    cr_assert(eq(int, m->getSigTlvType(m, 0), PTPMGMT_SLAVE_DELAY_TIMING_DATA_NP));
    const struct ptpmgmt_SLAVE_DELAY_TIMING_DATA_NP_t *p =
        (const struct ptpmgmt_SLAVE_DELAY_TIMING_DATA_NP_t *)m->getSigTlv(m, 0);
    cr_assert(not(zero(ptr, (void *)p)));
    cr_expect(eq(u16, p->sourcePortIdentity.portNumber, 1));
    cr_expect(zero(memcmp(p->sourcePortIdentity.clockIdentity.v, clockId, 8)));
    cr_expect(eq(u16, p->list[0].sequenceId, 2158));
    cr_expect(eq(u64, p->list[0].delayOriginTimestamp.secondsField,
            145204409665700));
    cr_expect(eq(u32, p->list[0].delayOriginTimestamp.nanosecondsField, 961561811));
    cr_expect(eq(i64, p->list[0].totalCorrectionField.scaledNanoseconds,
            5188463705227001856l));
    cr_expect(eq(u64, p->list[0].delayResponseTimestamp.secondsField,
            275016452224));
    cr_expect(eq(u32, p->list[0].delayResponseTimestamp.nanosecondsField,
            916518146));
    cr_expect(eq(u16, p->list[1].sequenceId, 3145));
    cr_expect(eq(u64, p->list[1].delayOriginTimestamp.secondsField,
            141017216059556));
    cr_expect(eq(u32, p->list[1].delayOriginTimestamp.nanosecondsField, 954115386));
    cr_expect(eq(i64, p->list[1].totalCorrectionField.scaledNanoseconds,
            10106818909802987520ul));
    cr_expect(eq(u64, p->list[1].delayResponseTimestamp.secondsField,
            216603929217188));
    cr_expect(eq(u32, p->list[1].delayResponseTimestamp.nanosecondsField,
            923151586));
    cr_expect(zero(u16, p->list[2].sequenceId));
    cr_expect(zero(u64, p->list[2].delayOriginTimestamp.secondsField));
    cr_expect(zero(u32, p->list[2].delayOriginTimestamp.nanosecondsField));
    cr_expect(zero(i64, p->list[2].totalCorrectionField.scaledNanoseconds));
    cr_expect(zero(u64, p->list[2].delayResponseTimestamp.secondsField));
    cr_expect(zero(u32, p->list[2].delayResponseTimestamp.nanosecondsField));
    m->free(m);
}

// Tests enhanced accuracy_metrics TLV
Test(SMPTETest, SMPTE_Org)
{
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    ptpmgmt_pMsgParams a = m->getParams(m);
    uint8_t b[100] = {0xd, 2, 0, 0x64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0x74, 0xda, 0x38, 0xff, 0xfe, 0xf6, 0x98, 0x5e, 0, 1, 0, 0, 4,
            0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 3,
            3, 3, 0, 0, 3, 0, 0x30, 0x68, 0x97, 0xe8, 0, 0, 1, 0, 0, 0, 0x1e, 0,
            0, 0, 1, 1
        };
    a->rcvSMPTEOrg = 1;
    cr_expect(m->updateParams(m, a));
    cr_assert(eq(int, m->parse(m, b, sizeof b), PTPMGMT_MNG_PARSE_ERROR_SMPTE));
    cr_assert(eq(int, m->getTlvId(m), PTPMGMT_SMPTE_MNG_ID));
    cr_assert(eq(int, m->getReplyAction(m), PTPMGMT_COMMAND));
    const void *t = m->getData(m);
    cr_assert(not(zero(ptr, (void *)t)));
    struct ptpmgmt_SMPTE_ORGANIZATION_EXTENSION_t *o =
        (struct ptpmgmt_SMPTE_ORGANIZATION_EXTENSION_t *)t;
    cr_assert(not(zero(ptr, (void *)o)));
    cr_assert(eq(int, memcmp(o->organizationId, "\x68\x97\xe8", 3),
            0)); //always SMPTE OUI
    cr_assert(eq(int, memcmp(o->organizationSubType, "\x0\x0\x1", 3),
            0)); // SM TLV version
    cr_assert(eq(int, o->defaultSystemFrameRate_numerator, 30));
    cr_assert(eq(int, o->defaultSystemFrameRate_denominator, 1));
    cr_assert(eq(int, o->masterLockingStatus, PTPMGMT_SMPTE_FREE_RUN));
    cr_assert(eq(int, o->timeAddressFlags, 0));
    cr_assert(eq(int, o->currentLocalOffset, 0));
    cr_assert(eq(int, o->jumpSeconds, 0));
    cr_assert(eq(u64, o->timeOfNextJump, 0));
    cr_assert(eq(int, o->timeOfNextJam, 0));
    cr_assert(eq(int, o->timeOfPreviousJam, 0));
    cr_assert(eq(int, o->previousJamLocalOffset, 0));
    cr_assert(eq(int, o->daylightSaving, 0));
    cr_assert(eq(int, o->leapSecondJump, 0));
    m->free(m);
}
