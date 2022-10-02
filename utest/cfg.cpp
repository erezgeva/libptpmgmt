/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2022 Erez Geva */

/** @file
 * @brief ConfigFile class unit test
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2022 Erez Geva
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
    EXPECT_STREQ(f.uds_address().c_str(), "/var/run/ptp4l");
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
    EXPECT_STREQ(f.uds_address().c_str(), "/var/run/dummy");
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
// uint8_t udp_ttl(const std::string &section = "") const;
TEST(ConfigFileTest, MethodUdp_ttl)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_EQ(f.udp_ttl("dumm"), 7);
    EXPECT_EQ(f.udp_ttl("non"), 3);
}

// Tests get socket priority parameter
// uint8_t socket_priority(const std::string &section = "") const;
TEST(ConfigFileTest, MethodSocketPriority)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_EQ(f.socket_priority("dumm"), 7);
    EXPECT_EQ(f.socket_priority("non"), 11);
}

// Tests get network transport type parameter
// uint8_t network_transport(const std::string &section = "") const;
TEST(ConfigFileTest, MethodNetworkTransport)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_EQ(f.network_transport("dumm"), '4');
    EXPECT_EQ(f.network_transport("non"), '6');
}

// Tests get Unix Domain socket address parameter
// const std::string &uds_address(const std::string &section = "") const;
TEST(ConfigFileTest, MethodUDSAddress)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_STREQ(f.uds_address("dumm").c_str(), "/var/run/dummy2");
    EXPECT_STREQ(f.uds_address("non").c_str(), "/var/run/dummy");
}

// Tests get PTP destination MAC address
// const Binary &ptp_dst_mac(const std::string &section = "") const;
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
// const Binary &p2p_dst_mac(const std::string &section = "") const;
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
