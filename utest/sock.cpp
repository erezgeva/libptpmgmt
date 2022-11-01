/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2022 Erez Geva */

/** @file
 * @brief socket classes unit test
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2022 Erez Geva
 *
 */

#include "sock.h"

using namespace ptpmgmt;

class SockUnixTest : public ::testing::Test, public SockUnix
{
  protected:
    void SetUp() override {
        useTestMode(true);
    }
    void TearDown() override {
        useTestMode(false);
    }
};

// Tests getHomeDir method
// const std::string &getHomeDir()
// const char *getHomeDir_c()
TEST_F(SockUnixTest, MethodGetHomeDir)
{
    EXPECT_STREQ(getHomeDir().c_str(), "/home/usr");
    EXPECT_STREQ(getHomeDir_c(), "/home/usr");
    useRoot(true);
    EXPECT_STREQ(getHomeDir().c_str(), "/root");
    EXPECT_STREQ(getHomeDir_c(), "/root");
}

// Tests setDefSelfAddress method
//  bool setDefSelfAddress(const std::string &rootBase = "",
//      const std::string &useDef = "");
// const std::string &getSelfAddress() const
// const char *getSelfAddress_c() const
TEST_F(SockUnixTest, MethodSetDefSelfAddress)
{
    EXPECT_TRUE(setDefSelfAddress());
    EXPECT_STREQ(getSelfAddress().c_str(), "/home/usr/.pmc.111");
    EXPECT_STREQ(getSelfAddress_c(), "/home/usr/.pmc.111");
    EXPECT_TRUE(setDefSelfAddress("/r/", "/u/"));
    EXPECT_STREQ(getSelfAddress().c_str(), "/u/111");
    EXPECT_STREQ(getSelfAddress_c(), "/u/111");
    useRoot(true);
    EXPECT_TRUE(setDefSelfAddress());
    EXPECT_STREQ(getSelfAddress().c_str(), "/var/run/pmc.111");
    EXPECT_STREQ(getSelfAddress_c(), "/var/run/pmc.111");
    EXPECT_TRUE(setDefSelfAddress("/r/", "/u/"));
    EXPECT_STREQ(getSelfAddress().c_str(), "/r/111");
    EXPECT_STREQ(getSelfAddress_c(), "/r/111");
}

// Tests setSelfAddress method
// bool setSelfAddress(const std::string &string)
TEST_F(SockUnixTest, MethodSetSelfAddress)
{
    EXPECT_TRUE(setSelfAddress("/tes1"));
    EXPECT_STREQ(getSelfAddress().c_str(), "/tes1");
    EXPECT_STREQ(getSelfAddress_c(), "/tes1");
}

// Tests init method
// bool init()
TEST_F(SockUnixTest, MethodInit)
{
    EXPECT_TRUE(setSelfAddress("/me"));
    EXPECT_TRUE(init());
}

// Tests close method
// void close()
TEST_F(SockUnixTest, MethodClose)
{
    EXPECT_TRUE(setSelfAddress("/me"));
    EXPECT_TRUE(init());
    close();
}

// Tests getFd method
// int getFd() const
// int fileno() const
TEST_F(SockUnixTest, MethodGetFd)
{
    EXPECT_TRUE(setSelfAddress("/me"));
    EXPECT_TRUE(init());
    int fd = getFd();
    EXPECT_GE(fd, 0);
    EXPECT_EQ(fd, fileno());
}

// Tests poll method
// bool poll(uint64_t timeout_ms = 0) const
TEST_F(SockUnixTest, MethodPoll)
{
    EXPECT_TRUE(setSelfAddress("/me"));
    EXPECT_TRUE(init());
    EXPECT_TRUE(poll());
    EXPECT_TRUE(poll(0));
    EXPECT_TRUE(poll(2000));
}

// Tests poll method
// bool tpoll(uint64_t &timeout_ms) const
TEST_F(SockUnixTest, MethodTpoll)
{
    EXPECT_TRUE(setSelfAddress("/me"));
    EXPECT_TRUE(init());
    uint64_t to = 1500;
    EXPECT_TRUE(tpoll(to));
    EXPECT_EQ(to, 500);
    EXPECT_TRUE(tpoll(to));
    EXPECT_EQ(to, 0);
    EXPECT_TRUE(tpoll(to));
    EXPECT_EQ(to, 0);
}

// Tests setPeerAddress method
// bool setPeerAddress(const std::string &string)
// const char *getPeerAddress_c() const
// const std::string &getPeerAddress() const
TEST_F(SockUnixTest, MethodSetPeerAddress)
{
    EXPECT_TRUE(setPeerAddress("/tes1"));
    EXPECT_STREQ(getPeerAddress_c(), "/tes1");
    EXPECT_STREQ(getPeerAddress().c_str(), "/tes1");
}

// Tests setPeerAddress method
// bool setPeerAddress(const ConfigFile &cfg, const std::string &section = "")
TEST_F(SockUnixTest, MethodSetPeerAddressCfg)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_TRUE(setPeerAddress(f));
    EXPECT_STREQ(getPeerAddress_c(), "/var/run/dummy");
    EXPECT_TRUE(setPeerAddress(f, "dumm"));
    EXPECT_STREQ(getPeerAddress_c(), "/var/run/dummy2");
}

// Tests send method
// bool send(const void *msg, size_t len)
TEST_F(SockUnixTest, MethodSend)
{
    EXPECT_TRUE(setSelfAddress("/me"));
    EXPECT_TRUE(setPeerAddress("/peer"));
    EXPECT_TRUE(init());
    EXPECT_TRUE(send("\x1\x2\x3\x4\x5", 5));
}

// Tests send with Buf method
// bool send(Buf &buf, size_t len)
// bool sendBuf(Buf &buf, size_t len)
TEST_F(SockUnixTest, MethodSendBuf)
{
    EXPECT_TRUE(setSelfAddress("/me"));
    EXPECT_TRUE(setPeerAddress("/peer"));
    EXPECT_TRUE(init());
    Buf b(10);
    memcpy(b.get(), "\x1\x2\x3\x4\x5", 5);
    EXPECT_TRUE(send(b, 5));
    EXPECT_TRUE(sendBuf(b, 5));
}

// Tests sendTo method
// bool sendTo(const void *msg, size_t len, const std::string &addrStr) const
TEST_F(SockUnixTest, MethodSendTo)
{
    EXPECT_TRUE(setSelfAddress("/me"));
    EXPECT_TRUE(init());
    EXPECT_TRUE(sendTo("\x1\x2\x3\x4\x5", 5, "/peer"));
}

// Tests sendTo with Buf method
// bool sendTo(Buf &buf, size_t len, const std::string &addrStr) const
TEST_F(SockUnixTest, MethodSendToBuf)
{
    EXPECT_TRUE(setSelfAddress("/me"));
    EXPECT_TRUE(init());
    Buf b(10);
    memcpy(b.get(), "\x1\x2\x3\x4\x5", 5);
    EXPECT_TRUE(sendTo(b, 5, "/peer"));
}

// Tests rcv method
// ssize_t rcv(void *buf, size_t bufSize, bool block = false)
TEST_F(SockUnixTest, MethodRcv)
{
    EXPECT_TRUE(setSelfAddress("/me"));
    EXPECT_TRUE(setPeerAddress("/peer"));
    EXPECT_TRUE(init());
    uint8_t buf[10];
    EXPECT_EQ(rcv(buf, sizeof buf), 5);
    EXPECT_EQ(memcmp(buf, "\x2\x4\x5\x6\x7", 5), 0);
    EXPECT_EQ(rcv(buf, sizeof buf, true), 5);
    EXPECT_EQ(memcmp(buf, "\x1\x4\x5\x6\x7", 5), 0);
}

// Tests rcv method
// ssize_t rcv(Buf &buf, bool block = false)
// ssize_t rcvBuf(Buf &buf, bool block = false)
TEST_F(SockUnixTest, MethodRcvBuf)
{
    EXPECT_TRUE(setSelfAddress("/me"));
    EXPECT_TRUE(setPeerAddress("/peer"));
    EXPECT_TRUE(init());
    Buf b(10);
    EXPECT_EQ(rcv(b), 5);
    EXPECT_EQ(memcmp(b.get(), "\x2\x4\x5\x6\x7", 5), 0);
    EXPECT_EQ(rcv(b, true), 5);
    EXPECT_EQ(memcmp(b.get(), "\x1\x4\x5\x6\x7", 5), 0);
    EXPECT_EQ(rcvBuf(b), 5);
    EXPECT_EQ(memcmp(b.get(), "\x2\x4\x5\x6\x7", 5), 0);
    EXPECT_EQ(rcvBuf(b, true), 5);
    EXPECT_EQ(memcmp(b.get(), "\x1\x4\x5\x6\x7", 5), 0);
}

// Tests rcvFrom method
// ssize_t rcvFrom(void *buf, size_t bufSize, std::string &from,
//     bool block = false) const;
// ssize_t rcvFrom(void *buf, size_t bufSize, bool block = false)
// const std::string &getLastFrom() const
TEST_F(SockUnixTest, MethodRcvFrom)
{
    EXPECT_TRUE(setSelfAddress("/me"));
    EXPECT_TRUE(init());
    uint8_t buf[10];
    EXPECT_EQ(rcvFrom(buf, sizeof buf), 5);
    EXPECT_EQ(memcmp(buf, "\x2\x4\x5\x6\x7", 5), 0);
    EXPECT_STREQ(getLastFrom().c_str(), "/peer");
    EXPECT_EQ(rcvFrom(buf, sizeof buf, true), 5);
    EXPECT_EQ(memcmp(buf, "\x1\x4\x5\x6\x7", 5), 0);
    EXPECT_STREQ(getLastFrom().c_str(), "/peer");
    std::string from;
    EXPECT_EQ(rcvFrom(buf, sizeof buf, from), 5);
    EXPECT_EQ(memcmp(buf, "\x2\x4\x5\x6\x7", 5), 0);
    EXPECT_STREQ(from.c_str(), "/peer");
    EXPECT_EQ(rcvFrom(buf, sizeof buf, from, true), 5);
    EXPECT_EQ(memcmp(buf, "\x1\x4\x5\x6\x7", 5), 0);
    EXPECT_STREQ(from.c_str(), "/peer");
}

// Tests rcvFrom with Buf method

// ssize_t rcvFrom(Buf &buf, std::string &from, bool block = false) const
// ssize_t rcvFrom(Buf &buf, bool block = false)
// ssize_t rcvBufFrom(Buf &buf, bool block = false)

TEST_F(SockUnixTest, MethodRcvFromBuf)
{
    EXPECT_TRUE(setSelfAddress("/me"));
    EXPECT_TRUE(init());
    Buf b(10);
    EXPECT_EQ(rcvFrom(b), 5);
    EXPECT_EQ(memcmp(b.get(), "\x2\x4\x5\x6\x7", 5), 0);
    EXPECT_STREQ(getLastFrom().c_str(), "/peer");
    EXPECT_EQ(rcvFrom(b, true), 5);
    EXPECT_EQ(memcmp(b.get(), "\x1\x4\x5\x6\x7", 5), 0);
    EXPECT_STREQ(getLastFrom().c_str(), "/peer");
    EXPECT_EQ(rcvBufFrom(b), 5);
    EXPECT_EQ(memcmp(b.get(), "\x2\x4\x5\x6\x7", 5), 0);
    EXPECT_STREQ(getLastFrom().c_str(), "/peer");
    EXPECT_EQ(rcvBufFrom(b, true), 5);
    EXPECT_EQ(memcmp(b.get(), "\x1\x4\x5\x6\x7", 5), 0);
    EXPECT_STREQ(getLastFrom().c_str(), "/peer");
    std::string from;
    EXPECT_EQ(rcvFrom(b, from), 5);
    EXPECT_EQ(memcmp(b.get(), "\x2\x4\x5\x6\x7", 5), 0);
    EXPECT_STREQ(from.c_str(), "/peer");
    EXPECT_EQ(rcvFrom(b, from, true), 5);
    EXPECT_EQ(memcmp(b.get(), "\x1\x4\x5\x6\x7", 5), 0);
    EXPECT_STREQ(from.c_str(), "/peer");
}

class SockIp4Test : public ::testing::Test, public SockIp4
{
  protected:
    void SetUp() override {
        useTestMode(true);
    }
    void TearDown() override {
        useTestMode(false);
    }
};

// Tests setIfUsingIndex method
// bool setIfUsingIndex(int ifIndex)
// bool setUdpTtl(uint8_t udp_ttl)
// bool init()
// int getFd() const
// int fileno() const
TEST_F(SockIp4Test, MethodSetIfUsingIndex)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setUdpTtl(7));
    EXPECT_TRUE(init());
    int fd = getFd();
    EXPECT_GE(fd, 0);
    EXPECT_EQ(fd, fileno());
}

// Tests setIfUsingName method
// bool setIfUsingName(const std::string &ifName)
// void close()
TEST_F(SockIp4Test, MethodSetIfUsingName)
{
    EXPECT_TRUE(setIfUsingName("eth0"));
    EXPECT_TRUE(setUdpTtl(7));
    EXPECT_TRUE(init());
    close();
}

// Tests setIf method
// bool setIf(const IfInfo &ifObj)
TEST_F(SockIp4Test, MethodSetIf)
{
    IfInfo i;
    EXPECT_TRUE(i.initUsingIndex(7));
    EXPECT_TRUE(setIf(i));
    EXPECT_TRUE(setUdpTtl(7));
    EXPECT_TRUE(init());
}

// Tests setUdpTtl method
// bool setUdpTtl(const ConfigFile &cfg, const std::string &section = "")
TEST_F(SockIp4Test, MethodSetUdpTtl)
{
    IfInfo i;
    EXPECT_TRUE(i.initUsingIndex(7));
    EXPECT_TRUE(setIf(i));
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_TRUE(setUdpTtl(f, "dumm"));
    EXPECT_TRUE(init());
}

// Tests setAll method
// bool setAll(const IfInfo &ifObj, const ConfigFile &cfg,
//     const std::string &section = "") => setAll(i, f, "dumm")
TEST_F(SockIp4Test, MethodSetAll)
{
    IfInfo i;
    EXPECT_TRUE(i.initUsingIndex(7));
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_TRUE(setAll(i, f, "dumm"));
    EXPECT_TRUE(init());
}

// Tests setAllInit method
// bool setAllInit(const IfInfo &ifObj, const ConfigFile &cfg,
//     const std::string &section = "") => setAllInit(i, f, "dumm")
TEST_F(SockIp4Test, MethodSetAllInit)
{
    IfInfo i;
    EXPECT_TRUE(i.initUsingIndex(7));
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_TRUE(setAllInit(i, f, "dumm"));
}

// Tests poll method
// bool poll(uint64_t timeout_ms = 0) const
TEST_F(SockIp4Test, MethodPoll)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setUdpTtl(7));
    EXPECT_TRUE(init());
    EXPECT_TRUE(poll());
    EXPECT_TRUE(poll(0));
    EXPECT_TRUE(poll(2000));
}

// Tests poll method
// bool tpoll(uint64_t &timeout_ms) const
TEST_F(SockIp4Test, MethodTpoll)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setUdpTtl(7));
    EXPECT_TRUE(init());
    uint64_t to = 1500;
    EXPECT_TRUE(tpoll(to));
    EXPECT_EQ(to, 500);
    EXPECT_TRUE(tpoll(to));
    EXPECT_EQ(to, 0);
    EXPECT_TRUE(tpoll(to));
    EXPECT_EQ(to, 0);
}

// Tests send method
// bool send(const void *msg, size_t len)
TEST_F(SockIp4Test, MethodSend)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setUdpTtl(7));
    EXPECT_TRUE(init());
    EXPECT_TRUE(send("\x1\x2\x3\x4\x5", 5));
}

// Tests send with Buf method
// bool send(Buf &buf, size_t len)
// bool sendBuf(Buf &buf, size_t len)
TEST_F(SockIp4Test, MethodSendBuf)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setUdpTtl(7));
    EXPECT_TRUE(init());
    Buf b(10);
    memcpy(b.get(), "\x1\x2\x3\x4\x5", 5);
    EXPECT_TRUE(send(b, 5));
    EXPECT_TRUE(sendBuf(b, 5));
}

// Tests rcv method
// ssize_t rcv(void *buf, size_t bufSize, bool block = false)
TEST_F(SockIp4Test, MethodRcv)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setUdpTtl(7));
    EXPECT_TRUE(init());
    uint8_t buf[10];
    EXPECT_EQ(rcv(buf, sizeof buf), 5);
    EXPECT_EQ(memcmp(buf, "\x2\x4\x5\x6\x7", 5), 0);
    EXPECT_EQ(rcv(buf, sizeof buf, true), 5);
    EXPECT_EQ(memcmp(buf, "\x1\x4\x5\x6\x7", 5), 0);
}

// Tests rcv method
// ssize_t rcv(Buf &buf, bool block = false)
// ssize_t rcvBuf(Buf &buf, bool block = false)
TEST_F(SockIp4Test, MethodRcvBuf)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setUdpTtl(7));
    EXPECT_TRUE(init());
    Buf b(10);
    EXPECT_EQ(rcv(b), 5);
    EXPECT_EQ(memcmp(b.get(), "\x2\x4\x5\x6\x7", 5), 0);
    EXPECT_EQ(rcv(b, true), 5);
    EXPECT_EQ(memcmp(b.get(), "\x1\x4\x5\x6\x7", 5), 0);
    EXPECT_EQ(rcvBuf(b), 5);
    EXPECT_EQ(memcmp(b.get(), "\x2\x4\x5\x6\x7", 5), 0);
    EXPECT_EQ(rcvBuf(b, true), 5);
    EXPECT_EQ(memcmp(b.get(), "\x1\x4\x5\x6\x7", 5), 0);
}

class SockIp6Test : public ::testing::Test, public SockIp6
{
  protected:
    void SetUp() override {
        useTestMode(true);
    }
    void TearDown() override {
        useTestMode(false);
    }
};

// Tests setIfUsingIndex method
// bool setIfUsingIndex(int ifIndex)
// bool setUdpTtl(uint8_t udp_ttl)
// bool setScope(uint8_t udp6_scope)
// bool init()
// int getFd() const
// int fileno() const
TEST_F(SockIp6Test, MethodSetIfUsingIndex)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setUdpTtl(7));
    EXPECT_TRUE(setScope(15));
    EXPECT_TRUE(init());
    int fd = getFd();
    EXPECT_GE(fd, 0);
    EXPECT_EQ(fd, fileno());
}

// Tests setIfUsingName method
// bool setIfUsingName(const std::string &ifName)
// void close()
TEST_F(SockIp6Test, MethodSetIfUsingName)
{
    EXPECT_TRUE(setIfUsingName("eth0"));
    EXPECT_TRUE(setUdpTtl(7));
    EXPECT_TRUE(setScope(15));
    EXPECT_TRUE(init());
    close();
}

// Tests setIf method
// bool setIf(const IfInfo &ifObj)
TEST_F(SockIp6Test, MethodSetIf)
{
    IfInfo i;
    EXPECT_TRUE(i.initUsingIndex(7));
    EXPECT_TRUE(setIf(i));
    EXPECT_TRUE(setUdpTtl(7));
    EXPECT_TRUE(setScope(15));
    EXPECT_TRUE(init());
}

// Tests setUdpTtl method
// bool setUdpTtl(const ConfigFile &cfg, const std::string &section = "")
// bool setScope(const ConfigFile &cfg, const std::string &section = "")
TEST_F(SockIp6Test, MethodSetUdpTtl)
{
    IfInfo i;
    EXPECT_TRUE(i.initUsingIndex(7));
    EXPECT_TRUE(setIf(i));
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_TRUE(setUdpTtl(f, "dumm"));
    EXPECT_TRUE(setScope(f, "dumm"));
    EXPECT_TRUE(init());
}

// Tests setAll method
// bool setAll(const IfInfo &ifObj, const ConfigFile &cfg,
//     const std::string &section = "")
TEST_F(SockIp6Test, MethodSetAll)
{
    IfInfo i;
    EXPECT_TRUE(i.initUsingIndex(7));
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_TRUE(setAll(i, f, "dumm"));
    EXPECT_TRUE(init());
}

// Tests setAllInit method
// bool setAllInit(const IfInfo &ifObj, const ConfigFile &cfg,
//     const std::string &section = "")
TEST_F(SockIp6Test, MethodSetAllInit)
{
    IfInfo i;
    EXPECT_TRUE(i.initUsingIndex(7));
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_TRUE(setAllInit(i, f, "dumm"));
}

// Tests poll method
// bool poll(uint64_t timeout_ms = 0) const
TEST_F(SockIp6Test, MethodPoll)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setUdpTtl(7));
    EXPECT_TRUE(setScope(15));
    EXPECT_TRUE(init());
    EXPECT_TRUE(poll());
    EXPECT_TRUE(poll(0));
    EXPECT_TRUE(poll(2000));
}

// Tests poll method
// bool tpoll(uint64_t &timeout_ms) const
TEST_F(SockIp6Test, MethodTpoll)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setUdpTtl(7));
    EXPECT_TRUE(setScope(15));
    EXPECT_TRUE(init());
    uint64_t to = 1500;
    EXPECT_TRUE(tpoll(to));
    EXPECT_EQ(to, 500);
    EXPECT_TRUE(tpoll(to));
    EXPECT_EQ(to, 0);
    EXPECT_TRUE(tpoll(to));
    EXPECT_EQ(to, 0);
}

// Tests send method
// bool send(const void *msg, size_t len)
TEST_F(SockIp6Test, MethodSend)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setUdpTtl(7));
    EXPECT_TRUE(setScope(15));
    EXPECT_TRUE(init());
    EXPECT_TRUE(send("\x1\x2\x3\x4\x5", 5));
}

// Tests send with Buf method
// bool send(Buf &buf, size_t len)
// bool sendBuf(Buf &buf, size_t len)
TEST_F(SockIp6Test, MethodSendBuf)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setUdpTtl(7));
    EXPECT_TRUE(setScope(15));
    EXPECT_TRUE(init());
    Buf b(10);
    memcpy(b.get(), "\x1\x2\x3\x4\x5", 5);
    EXPECT_TRUE(send(b, 5));
    EXPECT_TRUE(sendBuf(b, 5));
}

// Tests rcv method
// ssize_t rcv(void *buf, size_t bufSize, bool block = false)
TEST_F(SockIp6Test, MethodRcv)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setUdpTtl(7));
    EXPECT_TRUE(setScope(15));
    EXPECT_TRUE(init());
    uint8_t buf[10];
    EXPECT_EQ(rcv(buf, sizeof buf), 5);
    EXPECT_EQ(memcmp(buf, "\x2\x4\x5\x6\x7", 5), 0);
    EXPECT_EQ(rcv(buf, sizeof buf, true), 5);
    EXPECT_EQ(memcmp(buf, "\x1\x4\x5\x6\x7", 5), 0);
}

// Tests rcv method
// ssize_t rcv(Buf &buf, bool block = false)
// ssize_t rcvBuf(Buf &buf, bool block = false)
TEST_F(SockIp6Test, MethodRcvBuf)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setUdpTtl(7));
    EXPECT_TRUE(setScope(15));
    EXPECT_TRUE(init());
    Buf b(10);
    EXPECT_EQ(rcv(b), 5);
    EXPECT_EQ(memcmp(b.get(), "\x2\x4\x5\x6\x7", 5), 0);
    EXPECT_EQ(rcv(b, true), 5);
    EXPECT_EQ(memcmp(b.get(), "\x1\x4\x5\x6\x7", 5), 0);
    EXPECT_EQ(rcvBuf(b), 5);
    EXPECT_EQ(memcmp(b.get(), "\x2\x4\x5\x6\x7", 5), 0);
    EXPECT_EQ(rcvBuf(b, true), 5);
    EXPECT_EQ(memcmp(b.get(), "\x1\x4\x5\x6\x7", 5), 0);
}

class SockRawTest : public ::testing::Test, public SockRaw
{
  protected:
    void SetUp() override {
        useTestMode(true);
    }
    void TearDown() override {
        useTestMode(false);
    }
};

// Tests setIfUsingIndex method
// bool setIfUsingIndex(int ifIndex)
// bool setPtpDstMac(const uint8_t *ptp_dst_mac, size_t len)
// bool setSocketPriority(uint8_t socket_priority)
// bool init()
// int getFd() const
// int fileno() const
TEST_F(SockRawTest, MethodSetIfUsingIndex)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    const uint8_t mac[6] = { 1, 27, 23, 15, 12 };
    EXPECT_TRUE(setPtpDstMac(mac, sizeof mac));
    EXPECT_TRUE(setSocketPriority(7));
    EXPECT_TRUE(init());
    int fd = getFd();
    EXPECT_GE(fd, 0);
    EXPECT_EQ(fd, fileno());
}

// Tests setIfUsingName method
// bool setIfUsingName(const std::string &ifName)
// bool setPtpDstMac(const Binary &ptp_dst_mac)
// void close()
TEST_F(SockRawTest, MethodSetIfUsingName)
{
    EXPECT_TRUE(setIfUsingName("eth0"));
    EXPECT_TRUE(setPtpDstMac(Binary("\x1\x1b\x17\xf\xc", 6)));
    EXPECT_TRUE(setSocketPriority(7));
    EXPECT_TRUE(init());
    close();
}

// Tests setPtpDstMacStr method
// bool setPtpDstMacStr(const std::string &string)
TEST_F(SockRawTest, MethodSetPtpDstMacStr)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setPtpDstMacStr("1:1b:17:f:c:0"));
    EXPECT_TRUE(setSocketPriority(7));
    EXPECT_TRUE(init());
}

// Tests setIf method
// bool setIf(const IfInfo &ifObj)
TEST_F(SockRawTest, MethodSetIf)
{
    IfInfo i;
    EXPECT_TRUE(i.initUsingIndex(7));
    EXPECT_TRUE(setIf(i));
    EXPECT_TRUE(setPtpDstMacStr("1:1b:17:f:c:0"));
    EXPECT_TRUE(setSocketPriority(7));
    EXPECT_TRUE(init());
}

// Tests setPtpDstMac method
// bool setPtpDstMac(const ConfigFile &cfg, const std::string &section = "")
// bool setSocketPriority(const ConfigFile &cfg, const std::string &section = "")
TEST_F(SockRawTest, setPtpDstMac)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_TRUE(setPtpDstMac(f, "dumm"));
    EXPECT_TRUE(setSocketPriority(f, "dumm"));
    EXPECT_TRUE(init());
}

// Tests setAll method
// bool setAll(const IfInfo &ifObj, const ConfigFile &cfg,
//     const std::string &section = "")
TEST_F(SockRawTest, MethodSetAll)
{
    IfInfo i;
    EXPECT_TRUE(i.initUsingIndex(7));
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_TRUE(setAll(i, f, "dumm"));
    EXPECT_TRUE(init());
}

// Tests setAllInit method
// bool setAllInit(const IfInfo &ifObj, const ConfigFile &cfg,
//     const std::string &section = "")
TEST_F(SockRawTest, MethodSetAllInit)
{
    IfInfo i;
    EXPECT_TRUE(i.initUsingIndex(7));
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    EXPECT_TRUE(setAllInit(i, f, "dumm"));
}

// Tests poll method
// bool poll(uint64_t timeout_ms = 0) const
TEST_F(SockRawTest, MethodPoll)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setPtpDstMacStr("1:1b:17:f:c:0"));
    EXPECT_TRUE(setSocketPriority(7));
    EXPECT_TRUE(init());
    EXPECT_TRUE(poll());
    EXPECT_TRUE(poll(0));
    EXPECT_TRUE(poll(2000));
}

// Tests poll method
// bool tpoll(uint64_t &timeout_ms) const
TEST_F(SockRawTest, MethodTpoll)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setPtpDstMacStr("1:1b:17:f:c:0"));
    EXPECT_TRUE(setSocketPriority(7));
    EXPECT_TRUE(init());
    uint64_t to = 1500;
    EXPECT_TRUE(tpoll(to));
    EXPECT_EQ(to, 500);
    EXPECT_TRUE(tpoll(to));
    EXPECT_EQ(to, 0);
    EXPECT_TRUE(tpoll(to));
    EXPECT_EQ(to, 0);
}

// Tests send method
// bool send(const void *msg, size_t len)
TEST_F(SockRawTest, MethodSend)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setPtpDstMacStr("1:1b:17:f:c:0"));
    EXPECT_TRUE(setSocketPriority(7));
    EXPECT_TRUE(init());
    EXPECT_TRUE(send("\x1\x2\x3\x4\x5", 5));
}

// Tests send with Buf method
// bool send(Buf &buf, size_t len)
// bool sendBuf(Buf &buf, size_t len)
TEST_F(SockRawTest, MethodSendBuf)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setPtpDstMacStr("1:1b:17:f:c:0"));
    EXPECT_TRUE(setSocketPriority(7));
    EXPECT_TRUE(init());
    Buf b(10);
    memcpy(b.get(), "\x1\x2\x3\x4\x5", 5);
    EXPECT_TRUE(send(b, 5));
    EXPECT_TRUE(sendBuf(b, 5));
}

// Tests rcv method
// ssize_t rcv(void *buf, size_t bufSize, bool block = false)
TEST_F(SockRawTest, MethodRcv)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setPtpDstMacStr("1:1b:17:f:c:0"));
    EXPECT_TRUE(setSocketPriority(7));
    EXPECT_TRUE(init());
    uint8_t buf[10];
    EXPECT_EQ(rcv(buf, sizeof buf), 5);
    EXPECT_EQ(memcmp(buf, "\x2\x4\x5\x6\x7", 5), 0);
    EXPECT_EQ(rcv(buf, sizeof buf, true), 5);
    EXPECT_EQ(memcmp(buf, "\x1\x4\x5\x6\x7", 5), 0);
}

// Tests rcv method
// ssize_t rcv(Buf &buf, bool block = false)
// ssize_t rcvBuf(Buf &buf, bool block = false)
TEST_F(SockRawTest, MethodRcvBuf)
{
    EXPECT_TRUE(setIfUsingIndex(7));
    EXPECT_TRUE(setPtpDstMacStr("1:1b:17:f:c:0"));
    EXPECT_TRUE(setSocketPriority(7));
    EXPECT_TRUE(init());
    Buf b(10);
    EXPECT_EQ(rcv(b), 5);
    EXPECT_EQ(memcmp(b.get(), "\x2\x4\x5\x6\x7", 5), 0);
    EXPECT_EQ(rcv(b, true), 5);
    EXPECT_EQ(memcmp(b.get(), "\x1\x4\x5\x6\x7", 5), 0);
    EXPECT_EQ(rcvBuf(b), 5);
    EXPECT_EQ(memcmp(b.get(), "\x2\x4\x5\x6\x7", 5), 0);
    EXPECT_EQ(rcvBuf(b, true), 5);
    EXPECT_EQ(memcmp(b.get(), "\x1\x4\x5\x6\x7", 5), 0);
}
