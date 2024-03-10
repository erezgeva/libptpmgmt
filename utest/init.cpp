/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief init class unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

#include "init.h"

using namespace ptpmgmt;

const char *argv_u[] = {"me", "-u", "-s", "/peer"};
const size_t argc_u = 4;
const char *argv_4[] = {"me", "-4", "-i", "eth0", "--udp_ttl", "7"};
const size_t argc_4 = 6;
const char *argv_6[] = {"me", "-6", "-i", "eth0", "--udp_ttl", "7",
        "--udp6_scope", "15"
    };
const size_t argc_6 = 8;
const char *argv_2[] = {"me", "-2", "-i", "eth0", "--socket_priority", "7",
        "--ptp_dst_mac", "1:1b:17:f:c:0"
    };
const size_t argc_2 = 8;

// Tests process with unix socket
// int process(const Options &opt)
// const ConfigFile &cfg() const
// const SaFile &sa() const
// bool use_uds() const
// uint8_t allow_unauth() const
// Message &msg()
// SockBase *sk()
// char getNetSelect()
// void close()
TEST(InitTest, MethodProcessUnix)
{
    Options o;
    EXPECT_EQ(o.parse_options(argc_u, (char *const *)argv_u), Options::OPT_DONE);
    Init i;
    useTestMode(true);
    EXPECT_EQ(i.process(o), 0);
    EXPECT_TRUE(i.use_uds());
    EXPECT_EQ(i.allow_unauth(), 0);
    EXPECT_EQ(i.cfg().udp6_scope(), 14);
    EXPECT_FALSE(i.sa().have(1));
    EXPECT_EQ(i.msg().getTlvId(), NULL_PTP_MANAGEMENT);
    EXPECT_EQ(i.msg().getBuildTlvId(), NULL_PTP_MANAGEMENT);
    ASSERT_NE(i.sk(), nullptr);
    EXPECT_EQ(i.getNetSelect(), 'u');
    EXPECT_NE(dynamic_cast<SockUnix *>(i.sk()), nullptr);
    i.close();
    useTestMode(false);
}

// Tests process with ipv4 socket
TEST(InitTest, MethodProcessIPv4)
{
    Options o;
    EXPECT_EQ(o.parse_options(argc_4, (char *const *)argv_4), Options::OPT_DONE);
    Init i;
    useTestMode(true);
    EXPECT_EQ(i.process(o), 0);
    EXPECT_FALSE(i.use_uds());
    EXPECT_EQ(i.cfg().udp6_scope(), 14);
    EXPECT_EQ(i.msg().getTlvId(), NULL_PTP_MANAGEMENT);
    EXPECT_EQ(i.msg().getBuildTlvId(), NULL_PTP_MANAGEMENT);
    ASSERT_NE(i.sk(), nullptr);
    EXPECT_EQ(i.getNetSelect(), '4');
    EXPECT_NE(dynamic_cast<SockIp4 *>(i.sk()), nullptr);
    i.close();
    useTestMode(false);
}

// Tests process with ipv6 socket
TEST(InitTest, MethodProcessIPv6)
{
    Options o;
    EXPECT_EQ(o.parse_options(argc_6, (char *const *)argv_6), Options::OPT_DONE);
    Init i;
    useTestMode(true);
    EXPECT_EQ(i.process(o), 0);
    EXPECT_FALSE(i.use_uds());
    EXPECT_EQ(i.cfg().udp6_scope(), 14);
    EXPECT_EQ(i.msg().getTlvId(), NULL_PTP_MANAGEMENT);
    EXPECT_EQ(i.msg().getBuildTlvId(), NULL_PTP_MANAGEMENT);
    ASSERT_NE(i.sk(), nullptr);
    EXPECT_EQ(i.getNetSelect(), '6');
    EXPECT_NE(dynamic_cast<SockIp6 *>(i.sk()), nullptr);
    i.close();
    useTestMode(false);
}

// Tests process with raw socket
TEST(InitTest, MethodProcessRaw)
{
    Options o;
    EXPECT_EQ(o.parse_options(argc_2, (char *const *)argv_2), Options::OPT_DONE);
    Init i;
    useTestMode(true);
    EXPECT_EQ(i.process(o), 0);
    EXPECT_FALSE(i.use_uds());
    EXPECT_EQ(i.cfg().udp6_scope(), 14);
    EXPECT_EQ(i.msg().getTlvId(), NULL_PTP_MANAGEMENT);
    EXPECT_EQ(i.msg().getBuildTlvId(), NULL_PTP_MANAGEMENT);
    ASSERT_NE(i.sk(), nullptr);
    EXPECT_EQ(i.getNetSelect(), '2');
    EXPECT_NE(dynamic_cast<SockRaw *>(i.sk()), nullptr);
    i.close();
    useTestMode(false);
}
