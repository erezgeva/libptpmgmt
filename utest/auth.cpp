/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Message class with authentication TLV unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include "msg.h"

using namespace ptpmgmt;

// Test set authntication using the main configuration
// bool useAuth(const ConfigFile &cfg, const std::string &section = "")
TEST(MessageAuthTest, MethodUseAuthCfg)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    Message m;
    EXPECT_TRUE(m.useAuth(f));
}

// Test set authntication using the SA file
// bool useAuth(const SaFile &sa, uint8_t spp, uint32_t key)
TEST(MessageAuthTest, MethodUseAuth)
{
    SaFile s;
    EXPECT_TRUE(s.read_sa("utest/sa_file.cfg"));
    Message m;
    EXPECT_TRUE(m.useAuth(s, 2, 10));
}

// Test change authntication key and spp
// bool changeAuth(uint8_t spp, uint32_t key)
// int usedAuthSppID() const
TEST(MessageAuthTest, MethodChangeAuth)
{
    SaFile s;
    EXPECT_TRUE(s.read_sa("utest/sa_file.cfg"));
    Message m;
    EXPECT_TRUE(m.useAuth(s, 2, 10));
    EXPECT_EQ(m.usedAuthSppID(), 2);
    EXPECT_EQ(m.usedAuthKeyID(), 10);
    EXPECT_TRUE(m.changeAuth(1, 2));
    EXPECT_EQ(m.usedAuthSppID(), 1);
    EXPECT_EQ(m.usedAuthKeyID(), 2);
}

// Test change authntication key
// bool changeAuth(uint32_t key)
// uint32_t usedAuthKeyID() const
TEST(MessageAuthTest, MethodChangeAuthKey)
{
    SaFile s;
    EXPECT_TRUE(s.read_sa("utest/sa_file.cfg"));
    Message m;
    EXPECT_TRUE(m.useAuth(s, 2, 10));
    EXPECT_EQ(m.usedAuthKeyID(), 10);
    EXPECT_TRUE(m.changeAuth(5));
    EXPECT_EQ(m.usedAuthKeyID(), 5);
}

// Test disable authntication
// bool haveAuth() const
// bool disableAuth()
TEST(MessageAuthTest, MethodDisableAuth)
{
    SaFile s;
    EXPECT_TRUE(s.read_sa("utest/sa_file.cfg"));
    Message m;
    EXPECT_TRUE(m.useAuth(s, 2, 10));
    EXPECT_TRUE(m.haveAuth());
    EXPECT_TRUE(m.disableAuth());
    EXPECT_FALSE(m.haveAuth());
}

// Test get SA
// const SaFile &getSa() const
TEST(MessageAuthTest, MethodGetSa)
{
    SaFile s;
    EXPECT_TRUE(s.read_sa("utest/sa_file.cfg"));
    Message m;
    EXPECT_TRUE(m.useAuth(s, 2, 10));
    EXPECT_TRUE(m.haveAuth());
    const SaFile &a = m.getSa();
    EXPECT_TRUE(a.have(1, 2));
}

// Test build get send buffer size
// ssize_t getMsgPlanedLen() const
TEST(MessageAuthTest, MethodGetMsgPlanedLen)
{
    Message m;
    PRIORITY1_t p;
    p.priority1 = 1;
    EXPECT_TRUE(m.setAction(SET, PRIORITY1, &p));
    EXPECT_EQ(m.getBuildTlvId(), PRIORITY1);
    EXPECT_EQ(m.getSendAction(), SET);
    EXPECT_EQ(m.getMsgPlanedLen(), 56);
    SaFile s;
    EXPECT_TRUE(s.read_sa("utest/sa_file.cfg"));
    EXPECT_TRUE(m.useAuth(s, 2, 10));
    EXPECT_TRUE(m.haveAuth());
    EXPECT_EQ(m.getMsgPlanedLen(), 82);
}

// Test build
// MNG_PARSE_ERROR_e build(void *buf, size_t bufSize, uint16_t sequence)
TEST(MessageAuthTest, MethodBuild)
{
    Message m;
    MsgParams pm = m.getParams();
    pm.minorVersion = 1; // Authentication need IEEE 1588-2019
    EXPECT_TRUE(m.updateParams(pm));
    PRIORITY1_t p;
    p.priority1 = 0x7f;
    EXPECT_TRUE(m.setAction(SET, PRIORITY1, &p));
    EXPECT_EQ(m.getBuildTlvId(), PRIORITY1);
    EXPECT_EQ(m.getSendAction(), SET);
    EXPECT_EQ(m.getMsgPlanedLen(), 56);
    SaFile s;
    EXPECT_TRUE(s.read_sa("utest/sa_file.cfg"));
    EXPECT_TRUE(m.useAuth(s, 2, 10));
    EXPECT_TRUE(m.haveAuth());
    ssize_t plen = m.getMsgPlanedLen();
    EXPECT_EQ(plen, 82);
    uint8_t buf[90];
    EXPECT_EQ(m.build(buf, sizeof buf, 137), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(m.getMsgLen(), plen);
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
    EXPECT_EQ(memcmp(buf, ret, sizeof ret), 0);
}

// Test parse managment message with auth TLV
// MNG_PARSE_ERROR_e parse(const void *buf, ssize_t msgSize)
TEST(MessageAuthTest, MethodParse)
{
    Message m;
    MsgParams pm = m.getParams();
    pm.minorVersion = 1; // Authentication need IEEE 1588-2019
    EXPECT_TRUE(m.updateParams(pm));
    SaFile s;
    EXPECT_TRUE(s.read_sa("utest/sa_file.cfg"));
    EXPECT_TRUE(m.useAuth(s, 2, 10));
    EXPECT_TRUE(m.haveAuth());
    uint8_t p1[82] = {13, 0x12, 0, 82, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x89, 4, 0x7f, 0xff, 0xff, 0xff,
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 1, 1, 2, 0, 0, 1, 0, 4, 0x20,
            5, 0x7f, 0, // 56
            // Authentication TLV
            0x80, 9, 0, 22, 2, 0, 0, 0, 0, 10,
            // ICV of 16 bytes
            0x78, 0x87, 0x56, 0xc2, 0xf1, 0x57, 0x42, 0x92, 0x14, 0xaa, 0x6b, 0xaa,
            0xf, 0x69, 0x4d, 0x8f
        };
    EXPECT_EQ(m.parse(p1, sizeof p1), MNG_PARSE_ERROR_OK);
}

// Test receive Signaling with authentication
TEST(MessageAuthTest, MethodSig)
{
    Message m;
    MsgParams pm = m.getParams();
    pm.minorVersion = 1; // Authentication need IEEE 1588-2019
    pm.rcvSignaling = true;
    EXPECT_TRUE(m.updateParams(pm));
    SaFile s;
    EXPECT_TRUE(s.read_sa("utest/sa_file.cfg"));
    EXPECT_TRUE(m.useAuth(s, 2, 10));
    EXPECT_TRUE(m.haveAuth());
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
    EXPECT_EQ(m.parse(p1, sizeof p1), MNG_PARSE_ERROR_SIG);
    EXPECT_TRUE(m.isLastMsgSig());
}

/// printf("Error %s\n", m.err2str_c(m.parse
/// for(size_t i=0; i < 52; i++){printf("0x%x, ", buf[i]);}printf("\n");
