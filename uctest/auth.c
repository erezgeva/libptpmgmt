/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Message class with authentication TLV wrapper unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include "msg.h"

// Test set authntication using the main configuration
// bool useAuth_cfg(ptpmgmt_msg msg, const_ptpmgmt_cfg cfg, const char *section)
Test(MessageAuthTest, MethodUseAuthCfg)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_assert(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->useAuth_cfg(m, f, NULL));
    m->free(m);
    f->free(f);
}

// Test set authntication using the SA file
// bool useAuth(ptpmgmt_msg msg, const_ptpmgmt_safile sa, uint8_t spp, uint32_t key)
Test(MessageAuthTest, MethodUseAuth)
{
    ptpmgmt_safile s = ptpmgmt_safile_alloc();
    cr_assert(not(zero(ptr, s)));
    cr_expect(s->read_sa(s, "utest/sa_file.cfg"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->useAuth(m, s, 2, 10));
    m->free(m);
    s->free(s);
}

// Test change authntication key and spp
// bool changeAuth(ptpmgmt_msg msg, uint8_t spp, uint32_t key)
// int usedAuthSppID(const_ptpmgmt_msg msg)
Test(MessageAuthTest, MethodChangeAuth)
{
    ptpmgmt_safile s = ptpmgmt_safile_alloc();
    cr_assert(not(zero(ptr, s)));
    cr_expect(s->read_sa(s, "utest/sa_file.cfg"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->useAuth(m, s, 2, 10));
    cr_expect(eq(int, m->usedAuthSppID(m), 2));
    cr_expect(eq(u32, m->usedAuthKeyID(m), 10));
    cr_expect(m->changeAuth(m, 1, 2));
    cr_expect(eq(int, m->usedAuthSppID(m), 1));
    cr_expect(eq(u32, m->usedAuthKeyID(m), 2));
    m->free(m);
    s->free(s);
}

// Test change authntication key
// bool changeAuthKey(ptpmgmt_msg msg, uint32_t key)
// uint32_t usedAuthKeyID(const_ptpmgmt_msg msg)
Test(MessageAuthTest, MethodChangeAuthKey)
{
    ptpmgmt_safile s = ptpmgmt_safile_alloc();
    cr_assert(not(zero(ptr, s)));
    cr_expect(s->read_sa(s, "utest/sa_file.cfg"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->useAuth(m, s, 2, 10));
    cr_expect(eq(u32, m->usedAuthKeyID(m), 10));
    cr_expect(m->changeAuthKey(m, 5));
    cr_expect(eq(u32, m->usedAuthKeyID(m), 5));
    m->free(m);
    s->free(s);
}

// Test disable authntication
// bool haveAuth(const_ptpmgmt_msg msg)
// bool disableAuth(ptpmgmt_msg msg)
Test(MessageAuthTest, MethodDisableAuth)
{
    ptpmgmt_safile s = ptpmgmt_safile_alloc();
    cr_assert(not(zero(ptr, s)));
    cr_expect(s->read_sa(s, "utest/sa_file.cfg"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->useAuth(m, s, 2, 10));
    cr_expect(m->haveAuth(m));
    cr_expect(m->disableAuth(m));
    cr_expect(not(m->haveAuth(m)));
    m->free(m);
    s->free(s);
}

// Test get SA
// const_ptpmgmt_safile getSa(ptpmgmt_msg msg)
Test(MessageAuthTest, MethodGetSa)
{
    ptpmgmt_safile s = ptpmgmt_safile_alloc();
    cr_assert(not(zero(ptr, s)));
    cr_expect(s->read_sa(s, "utest/sa_file.cfg"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    cr_expect(m->useAuth(m, s, 2, 10));
    cr_expect(m->haveAuth(m));
    const_ptpmgmt_safile a = m->getSa(m);
    cr_expect(a->have_key(a, 1, 2));
    m->free(m);
    s->free(s);
}

// Test build get send buffer size
// ssize_t getMsgPlanedLen(const_ptpmgmt_msg m)
Test(MessageAuthTest, MethodGetMsgPlanedLen)
{
    ptpmgmt_safile s = ptpmgmt_safile_alloc();
    cr_assert(not(zero(ptr, s)));
    cr_expect(s->read_sa(s, "utest/sa_file.cfg"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    struct ptpmgmt_PRIORITY1_t p;
    p.priority1 = 1;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY1, &p));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    cr_expect(eq(int, m->getSendAction(m), PTPMGMT_SET));
    cr_expect(eq(sz, m->getMsgPlanedLen(m), 56));
    cr_expect(m->useAuth(m, s, 2, 10));
    cr_expect(m->haveAuth(m));
    cr_expect(eq(sz, m->getMsgPlanedLen(m), 82));
    m->free(m);
    s->free(s);
}

// Test build
// enum ptpmgmt_MNG_PARSE_ERROR_e build(const_ptpmgmt_msg m, void *buf,
//     size_t bufSize, uint16_t sequence)
Test(MessageAuthTest, MethodBuild)
{
    ptpmgmt_safile s = ptpmgmt_safile_alloc();
    cr_assert(not(zero(ptr, s)));
    cr_expect(s->read_sa(s, "utest/sa_file.cfg"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    ptpmgmt_pMsgParams pm = m->getParams(m);
    pm->minorVersion = 1; // Authentication need IEEE 1588-2019
    cr_expect(m->updateParams(m, pm));
    struct ptpmgmt_PRIORITY1_t p;
    p.priority1 = 0x7f;
    cr_expect(m->setAction(m, PTPMGMT_SET, PTPMGMT_PRIORITY1, &p));
    cr_expect(eq(int, m->getBuildTlvId(m), PTPMGMT_PRIORITY1));
    cr_expect(eq(int, m->getSendAction(m), PTPMGMT_SET));
    cr_expect(eq(sz, m->getMsgPlanedLen(m), 56));
    cr_expect(m->useAuth(m, s, 2, 10));
    cr_expect(m->haveAuth(m));
    ssize_t plen = m->getMsgPlanedLen(m);
    cr_expect(eq(int, plen, 82));
    uint8_t buf[90];
    cr_expect(eq(int, m->build(m, buf, sizeof buf, 137),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    cr_expect(eq(int, m->getMsgLen(m), plen));
    // byte 4 is message length
    uint8_t ret[82] = { 13, 0x12, 0, 82, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x89, 4, 0x7f, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1, 1, 1, 0, 0, 1, 0, 4, 0x20,
            5, 0x7f, 0, // 56
            // Authentication TLV
            // byte 4 is TLV length - 4
            // byte 5 is spp
            // byte 10 is key ID (last 4 bytes)
            0x80, 9, 0, 22, 2, 0, 0, 0, 0, 10,
            // ICV of 16 bytes
            0x79, 0x8b, 0xf9, 0xd2, 7, 0x25, 0xc4, 0x11, 0xec, 0xf9, 0x15, 0xac,
            0xaa, 0x10, 0x9b, 0x20
        };
    cr_expect(zero(memcmp(buf, ret, sizeof ret)));
    m->free(m);
    s->free(s);
}
// Test parse managment message with auth TLV
// enum ptpmgmt_MNG_PARSE_ERROR_e parse(const_ptpmgmt_msg msg, const void *buf,
//     ssize_t msgSize)
Test(MessageAuthTest, MethodParse)
{
    ptpmgmt_safile s = ptpmgmt_safile_alloc();
    cr_assert(not(zero(ptr, s)));
    cr_expect(s->read_sa(s, "utest/sa_file.cfg"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    ptpmgmt_pMsgParams pm = m->getParams(m);
    pm->minorVersion = 1; // Authentication need IEEE 1588-2019
    cr_expect(m->updateParams(m, pm));
    cr_expect(m->useAuth(m, s, 2, 10));
    cr_expect(m->haveAuth(m));
    uint8_t p1[82] = { 13, 0x12, 0, 82, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x89, 4, 0x7f, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1, 1, 2, 0, 0, 1, 0, 4, 0x20,
            5, 0x7f, 0, // 56
            // Authentication TLV
            0x80, 9, 0, 22, 2, 0, 0, 0, 0, 10,
            // ICV of 16 bytes
            0x78, 0x87, 0x56, 0xc2, 0xf1, 0x57, 0x42, 0x92, 0x14, 0xaa, 0x6b, 0xaa,
            0xf, 0x69, 0x4d, 0x8f
        };
    cr_expect(eq(int, m->parse(m, p1, sizeof p1), PTPMGMT_MNG_PARSE_ERROR_OK));
    m->free(m);
    s->free(s);
}

// Test receive Signaling with authentication
Test(MessageAuthTest, MethodSig)
{
    ptpmgmt_safile s = ptpmgmt_safile_alloc();
    cr_assert(not(zero(ptr, s)));
    cr_expect(s->read_sa(s, "utest/sa_file.cfg"));
    ptpmgmt_msg m = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, m)));
    ptpmgmt_pMsgParams pm = m->getParams(m);
    pm->minorVersion = 1; // Authentication need IEEE 1588-2019
    pm->rcvSignaling = true;
    cr_expect(m->updateParams(m, pm));
    cr_expect(m->useAuth(m, s, 2, 10));
    cr_expect(m->haveAuth(m));
    uint8_t p1[78] = {12, 0x12, 0, 78, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 5, 0x7f, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0, 1, 0, 4,
            0x20, 5, 1, 0, // 52
            // Authentication TLV
            0x80, 9, 0, 22, 2, 0, 0, 0, 0, 10,
            // ICV of 16 bytes
            0xa2, 0xd2, 0x82, 0x95, 0x96, 0xf1, 0x4f, 0xb3, 4, 0xe4, 0xc7,
            0xdd, 0x5e, 0x51, 0x74, 0x63
        };
    cr_expect(eq(int, m->parse(m, p1, sizeof p1), PTPMGMT_MNG_PARSE_ERROR_SIG));
    cr_expect(m->isLastMsgSig(m));
    m->free(m);
    s->free(s);
}
