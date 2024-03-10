/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief ConfigFile class unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

#include "cfg.h"

using namespace ptpmgmt;

// Tests ConfigFile empty constructor
// ConfigFile()
TEST(ConfigFileTest, MethodEmptyConstructor)
{
    ConfigFile f;
    // Check default values
    EXPECT_EQ(f.transportSpecific(), 0);
    EXPECT_EQ(f.domainNumber(), 0);
    EXPECT_EQ(f.udp6_scope(), 0xe);
    EXPECT_EQ(f.udp_ttl(), 1);
    EXPECT_EQ(f.socket_priority(), 0);
    EXPECT_EQ(f.network_transport(), '4');
    EXPECT_EQ(f.active_key_id(), 0);
    EXPECT_EQ(f.spp(), 0);
    EXPECT_EQ(f.allow_unauth(), 0);
    EXPECT_FALSE(f.haveSpp());
    EXPECT_STREQ(f.uds_address().c_str(), "/var/run/ptp4l");
    EXPECT_TRUE(f.sa_file().empty());
    Binary b;
    EXPECT_TRUE(b.fromMac("1:1b:19:0:0:0"));
    EXPECT_EQ(f.ptp_dst_mac(), b);
    EXPECT_TRUE(b.fromMac("1:80:c2:0:0:e"));
    EXPECT_EQ(f.p2p_dst_mac(), b);
}

// Tests read configuration file
// bool read_cfg(const std::string &file)
TEST(ConfigFileTest, MethodReadConfiguration)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    // Check file configuration
    EXPECT_EQ(f.transportSpecific(), 7);
    EXPECT_EQ(f.domainNumber(), 5);
    EXPECT_EQ(f.udp6_scope(), 0xd);
    EXPECT_EQ(f.udp_ttl(), 3);
    EXPECT_EQ(f.socket_priority(), 11);
    EXPECT_EQ(f.network_transport(), '6');
    EXPECT_EQ(f.active_key_id(), 10);
    EXPECT_EQ(f.spp(), 2);
    EXPECT_EQ(f.allow_unauth(), 2);
    EXPECT_TRUE(f.haveSpp());
    EXPECT_STREQ(f.uds_address().c_str(), "/var/run/dummy");
    EXPECT_STREQ(f.sa_file().c_str(), "utest/sa_file.cfg");
    Binary b;
    EXPECT_TRUE(b.fromMac("1:1b:19:f:c:0"));
    EXPECT_EQ(f.ptp_dst_mac(), b);
    EXPECT_TRUE(b.fromMac("1:80:c2:1:0:e"));
    EXPECT_EQ(f.p2p_dst_mac(), b);
}

// Tests get transport specific parameter
// uint8_t transportSpecific(const std::string &section = "") const
TEST(ConfigFileTest, MethodTransportSpecific)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_EQ(f.transportSpecific("dumm"), 9);
    EXPECT_EQ(f.transportSpecific("non"), 7);
}

// Tests get domain number parameter
// uint8_t domainNumber(const std::string &section = "") const
TEST(ConfigFileTest, MethodDomainNumber)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_EQ(f.domainNumber("dumm"), 3);
    EXPECT_EQ(f.domainNumber("non"), 5);
}

// Tests get UDP IPv6 scope parameter
// uint8_t udp6_scope(const std::string &section = "") const
TEST(ConfigFileTest, MethodUdp6_scope)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_EQ(f.udp6_scope("dumm"), 0xf);
    EXPECT_EQ(f.udp6_scope("non"), 0xd);
}

// Tests get UDP IPv4 Time to live parameter
// uint8_t udp_ttl(const std::string &section = "") const
TEST(ConfigFileTest, MethodUdp_ttl)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_EQ(f.udp_ttl("dumm"), 7);
    EXPECT_EQ(f.udp_ttl("non"), 3);
}

// Tests get socket priority parameter
// uint8_t socket_priority(const std::string &section = "") const
TEST(ConfigFileTest, MethodSocketPriority)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_EQ(f.socket_priority("dumm"), 7);
    EXPECT_EQ(f.socket_priority("non"), 11);
}

// Tests get network transport type parameter
// uint8_t network_transport(const std::string &section = "") const
TEST(ConfigFileTest, MethodNetworkTransport)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_EQ(f.network_transport("dumm"), '4');
    EXPECT_EQ(f.network_transport("non"), '6');
}

// Tests get authentication Key ID
// uint32_t active_key_id(const std::string &section = "") const
TEST(ConfigFileTest, MethoAuthKeyID)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_EQ(f.active_key_id("dumm"), 0x1297);
    EXPECT_EQ(f.active_key_id("non"), 10);
}

// Tests get the authentication security parameter pointer
// uint8_t spp(const std::string &section = "") const
TEST(ConfigFileTest, MethoAuthSPP)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_EQ(f.spp("dumm"), 37);
    EXPECT_EQ(f.spp("non"), 2);
}

// Tests accept unauthenticated response messages parameter
// uint8_t allow_unauth(const std::string &section = "") const
TEST(ConfigFileTest, MethoAllowUnauth)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_EQ(f.allow_unauth("dumm"), 1);
    EXPECT_EQ(f.allow_unauth("non"), 2);
}

// Tests whether the authentication security parameter available
// bool haveSpp(const std::string &section = "") const
TEST(ConfigFileTest, MethoHaveSPP)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_TRUE(f.haveSpp());
    EXPECT_TRUE(f.haveSpp("dumm"));
    EXPECT_TRUE(f.haveSpp("non"));
}

// Tests get Unix Domain socket address parameter
// const std::string &uds_address(const std::string &section = "") const
TEST(ConfigFileTest, MethodUDSAddress)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_STREQ(f.uds_address("dumm").c_str(), "/var/run/dummy2");
    EXPECT_STREQ(f.uds_address("non").c_str(), "/var/run/dummy");
}

// Tests get authentication security association file
// const std::string &sa_file(const std::string &section = "") const
TEST(ConfigFileTest, MethodSaFile)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_STREQ(f.sa_file("dumm").c_str(), "utest/sa_file2.cfg");
    EXPECT_STREQ(f.sa_file("non").c_str(), "utest/sa_file.cfg");
}

// Tests get PTP destination MAC address
// const Binary &ptp_dst_mac(const std::string &section = "") const
TEST(ConfigFileTest, MethodPTPDstMac)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    Binary b;
    EXPECT_TRUE(b.fromMac("1:1b:17:f:c:0"));
    EXPECT_EQ(f.ptp_dst_mac("dumm"), b);
    EXPECT_TRUE(b.fromMac("1:1b:19:f:c:0"));
    EXPECT_EQ(f.ptp_dst_mac("non"), b);
}

// Tests get PTP point to point destination MAC address
// const Binary &p2p_dst_mac(const std::string &section = "") const
TEST(ConfigFileTest, MethodP2pDstMac)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    Binary b;
    EXPECT_TRUE(b.fromMac("1:80:c1:1:0:e"));
    EXPECT_EQ(f.p2p_dst_mac("dumm"), b);
    EXPECT_TRUE(b.fromMac("1:80:c2:1:0:e"));
    EXPECT_EQ(f.p2p_dst_mac("non"), b);
}

// Tests read SA file
// bool read_sa(const std::string &file);
TEST(SaFileTest, MethodReadSA)
{
    SaFile s;
    EXPECT_TRUE(s.read_sa("utest/sa_file.cfg"));
}

// Tests read SA file using file name from configuration
// bool read_sa(const ConfigFile &cfg, const std::string &section = "")
TEST(SaFileTest, MethodReadSACfg)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    SaFile s;
    EXPECT_TRUE(s.read_sa(f));
}

// Tests if SPP exist
// bool have(uint8_t spp) const
TEST(SaFileTest, MethodHaveSpp)
{
    SaFile s;
    ASSERT_TRUE(s.read_sa("utest/sa_file.cfg"));
    EXPECT_FALSE(s.have(17));
    EXPECT_TRUE(s.have(0));
}

// Tests if Key exist
// bool have(uint8_t spp, uint32_t key) const
TEST(SaFileTest, MethodHaveKey)
{
    SaFile s;
    ASSERT_TRUE(s.read_sa("utest/sa_file.cfg"));
    EXPECT_TRUE(s.have(0, 1));
}

// Get SPP
// Spp &spp(uint8_t spp) const
TEST(SaFileTest, MethodSpp)
{
    SaFile s;
    ASSERT_TRUE(s.read_sa("utest/sa_file.cfg"));
    ASSERT_TRUE(s.have(1));
    const Spp &s1 = s.spp(1);
    EXPECT_EQ(s1.keys(), 2);
    EXPECT_EQ(s1.ownID(), 1);
    EXPECT_FALSE(s.have(19));
    Spp s19 = s.spp(19); // Non exist SPP
    EXPECT_EQ(s19.keys(), 0);
    EXPECT_NE(s19.ownID(), 19);
}

// Constructor
// Spp(uint8_t id)
TEST(SppTest, MethodConstructor)
{
    Spp s(15);
    EXPECT_EQ(s.ownID(), 15);
}
// Test adding keys
// bool addkey(uint32_t id, hmac_t type, binary &value, size_t digest,
//     bool replace = true)
TEST(SppTest, MethodAddKey)
{
    Spp s(0);
    Binary k57("abxdefghijklmnop");
    EXPECT_TRUE(s.addKey(0x57, HMAC_SHA256, k57, 16));
    EXPECT_EQ(s.keys(), 1);
    EXPECT_TRUE(s.have(0x57));
    Binary k1(16);
    EXPECT_TRUE(s.addKey(1, HMAC_AES128, k1, 10));
    EXPECT_EQ(s.keys(), 2);
    EXPECT_TRUE(s.have(1));
    EXPECT_FALSE(s.addKey(1, HMAC_AES128, k1, 12, false));
    EXPECT_TRUE(s.addKey(0x57, HMAC_SHA256, k57, 32));
    EXPECT_EQ(s.keys(), 2);
}

// Test if key exist
// bool have(uint32_t key) const
TEST(SppTest, MethodHave)
{
    SaFile s;
    ASSERT_TRUE(s.read_sa("utest/sa_file.cfg"));
    ASSERT_TRUE(s.have(1));
    const Spp &s1 = s.spp(1);
    EXPECT_TRUE(s1.have(1));
}

// Get key digest size (ICV size)
// size_t mac_size(uint32_t key) const
TEST(SppTest, MethodMacSize)
{
    SaFile s;
    ASSERT_TRUE(s.read_sa("utest/sa_file.cfg"));
    ASSERT_TRUE(s.have(1));
    const Spp &s1 = s.spp(1);
    EXPECT_EQ(s1.mac_size(1), 16);
    EXPECT_EQ(s1.mac_size(2), 32);
}

// Get key value
// const Binary &key(uint32_t key) const
TEST(SppTest, MethodKey)
{
    SaFile s;
    ASSERT_TRUE(s.read_sa("utest/sa_file.cfg"));
    ASSERT_TRUE(s.have(2));
    const Spp &s1 = s.spp(2);
    ASSERT_TRUE(s1.have(10));
    uint8_t k10b[16] = {0xfa, 0xf4, 0x8e, 0xba, 1, 0xe7, 0xc5, 0x96, 0x6a,
            0x76, 0xcb, 0x78, 0x7a, 0xed, 0x4e, 0x7b
        };
    Binary k10(k10b, sizeof k10b);
    EXPECT_EQ(s1.key(10), k10);
    ASSERT_TRUE(s1.have(5));
    Binary k5("abcdefghijklmnopqrstuvwxyz");
    EXPECT_EQ(s1.key(5), k5);
    ASSERT_TRUE(s1.have(64));
    uint8_t k64b[37] = {0, 0xcb, 0xcd, 0x9a, 0x7c, 0xa6, 0x33, 0xd8, 0x7e,
            0xf8, 0xc5, 0x59, 0xa3, 0x6b, 0xcf, 0xdd, 0x19, 0x2c, 0x8d, 0xd5,
            0x68, 3, 0x72, 0x35, 0x44, 0x23, 0xff, 0x2e, 0xcf, 0x45, 0x5d,
            0xfb, 0xcb, 0x23, 0x36, 0xb6, 7
        };
    Binary k64(k64b, sizeof k64b);
    EXPECT_EQ(s1.key(64), k64);
}

// Get number of keys
// size_t keys() const
TEST(SppTest, MethodKeys)
{
    SaFile s;
    ASSERT_TRUE(s.read_sa("utest/sa_file.cfg"));
    ASSERT_TRUE(s.have(0));
    const Spp &s0 = s.spp(0);
    EXPECT_EQ(s0.keys(), 1);
    ASSERT_TRUE(s.have(1));
    const Spp &s1 = s.spp(1);
    EXPECT_EQ(s1.keys(), 2);
    ASSERT_TRUE(s.have(2));
    const Spp &s2 = s.spp(2);
    EXPECT_EQ(s2.keys(), 4);
}

// Get key type
// HMAC_t htype(uint32_t key) const
TEST(SppTest, MethodHtype)
{
    SaFile s;
    ASSERT_TRUE(s.read_sa("utest/sa_file.cfg"));
    ASSERT_TRUE(s.have(2));
    const Spp &s2 = s.spp(2);
    EXPECT_EQ(s2.htype(10), HMAC_AES128);
    EXPECT_EQ(s2.htype(25), HMAC_AES256);
    EXPECT_EQ(s2.htype(5), HMAC_SHA256);
}

// Get SPP Own ID
// uint8_t ownID() const
TEST(SppTest, MethodOwnID)
{
    SaFile s;
    ASSERT_TRUE(s.read_sa("utest/sa_file.cfg"));
    ASSERT_TRUE(s.have(0));
    const Spp &s0 = s.spp(0);
    EXPECT_EQ(s0.ownID(), 0);
    ASSERT_TRUE(s.have(1));
    const Spp &s1 = s.spp(1);
    EXPECT_EQ(s1.ownID(), 1);
    ASSERT_TRUE(s.have(2));
    const Spp &s2 = s.spp(2);
    EXPECT_EQ(s2.ownID(), 2);
}
