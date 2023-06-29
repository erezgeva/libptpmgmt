/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief socket classes unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include <string.h>
#include <stdio.h>
#include "sock.h"

TestSuite(SockUnixTest, .init = initLibSys);
TestSuite(SockIp4Test, .init = initLibSys);
TestSuite(SockIp6Test, .init = initLibSys);
TestSuite(SockRawTest, .init = initLibSys);

// Tests getHomeDir method
// const char *getHomeDir()
Test(SockUnixTest, MethodGetHomeDir)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockUnix);
    useTestMode(true);
    bool r1 = strcmp(sk->getHomeDir(sk), "/home/usr") == 0;
    useRoot(true);
    bool r2 = strcmp(sk->getHomeDir(sk), "/root") == 0;
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    sk->free(sk);
}

// Tests setDefSelfAddress method
//  bool setDefSelfAddress(const char *rootBase = "",
//      const char *useDef = "")
// const char *getSelfAddress()
Test(SockUnixTest, MethodSetDefSelfAddress)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockUnix);
    useTestMode(true);
    bool r1 = sk->setDefSelfAddress(sk, NULL, NULL);
    bool r2 = strcmp(sk->getSelfAddress(sk), "/home/usr/.pmc.111") == 0;
    bool r3 = sk->setDefSelfAddress(sk, "/r/", "/u/");
    bool r4 = strcmp(sk->getSelfAddress(sk), "/u/111") == 0;
    useRoot(true);
    bool r5 = sk->setDefSelfAddress(sk, NULL, NULL);
    bool r6 = strcmp(sk->getSelfAddress(sk), "/var/run/pmc.111") == 0;
    bool r7 = sk->setDefSelfAddress(sk, "/r/", "/u/");
    bool r8 = strcmp(sk->getSelfAddress(sk), "/r/111") == 0;
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    cr_expect(r6);
    cr_expect(r7);
    cr_expect(r8);
    sk->free(sk);
}

// Tests setSelfAddress method
// bool setSelfAddress(const char *string)
Test(SockUnixTest, MethodSetSelfAddress)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockUnix);
    useTestMode(true);
    bool r1 = sk->setSelfAddress(sk, "/tes1");
    char *r2 = (char *)sk->getSelfAddress(sk);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(eq(str, r2, "/tes1"));
    sk->free(sk);
}

// Tests init method
// bool init()
Test(SockUnixTest, MethodInit)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockUnix);
    useTestMode(true);
    bool r1 = sk->setSelfAddress(sk, "/me");
    bool r2 = sk->init(sk);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    sk->free(sk);
}

// Tests close method
// void close()
Test(SockUnixTest, MethodClose)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockUnix);
    useTestMode(true);
    bool r1 = sk->setSelfAddress(sk, "/me");
    bool r2 = sk->init(sk);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    sk->free(sk);
}

// Tests getFd method
// int getFd()
// int fileno()
Test(SockUnixTest, MethodGetFd)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockUnix);
    useTestMode(true);
    bool r1 = sk->setSelfAddress(sk, "/me");
    bool r2 = sk->init(sk);
    int fd = sk->getFd(sk);
    bool r4 = fd == sk->fileno(sk);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(ge(int, fd, 0));
    cr_expect(r4);
    sk->free(sk);
}

// Tests poll method
// bool poll(uint64_t timeout_ms = 0)
Test(SockUnixTest, MethodPoll)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockUnix);
    useTestMode(true);
    bool r1 = sk->setSelfAddress(sk, "/me");
    bool r2 = sk->init(sk);
    bool r3 = sk->poll(sk, 0);
    bool r4 = sk->poll(sk, 0);
    bool r5 = sk->poll(sk, 2000);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    sk->free(sk);
}

// Tests poll method
// bool tpoll(uint64_t *timeout_ms)
Test(SockUnixTest, MethodTpoll)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockUnix);
    useTestMode(true);
    bool r1 = sk->setSelfAddress(sk, "/me");
    bool r2 = sk->init(sk);
    uint64_t to = 1500;
    bool r3 = sk->tpoll(sk, &to);
    bool r4 = to == 500;
    bool r5 = sk->tpoll(sk, &to);
    bool r6 = to ==  0;
    bool r7 = sk->tpoll(sk, &to);
    bool r8 = to ==  0;
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    cr_expect(r6);
    cr_expect(r7);
    cr_expect(r8);
    sk->free(sk);
}

// Tests setPeerAddress method
// bool setPeerAddress(const char *string)
// const char *getPeerAddress()
Test(SockUnixTest, MethodSetPeerAddress)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockUnix);
    useTestMode(true);
    bool r1 = sk->setPeerAddress(sk, "/tes1");
    bool r2 = strcmp(sk->getPeerAddress(sk), "/tes1") == 0;
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    sk->free(sk);
}

// Tests setPeerAddressCfg method
// bool setPeerAddressCfg(const_ptpmgmt_cfg cfg, const char *section)
Test(SockUnixTest, MethodSetPeerAddressCfg)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockUnix);
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    useTestMode(true);
    bool r1 = f->read_cfg(f, "utest/testing.cfg");
    bool r2 = sk->setPeerAddressCfg(sk, f, NULL);
    bool r3 = strcmp(sk->getPeerAddress(sk), "/var/run/dummy") == 0;
    bool r4 = sk->setPeerAddressCfg(sk, f, "dumm");
    bool r5 = strcmp(sk->getPeerAddress(sk), "/var/run/dummy2") == 0;
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    f->free(f);
    sk->free(sk);
}

// Tests send method
// bool send(const void *msg, size_t len)
Test(SockUnixTest, MethodSend)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockUnix);
    useTestMode(true);
    bool r1 = sk->setSelfAddress(sk, "/me");
    bool r2 = sk->setPeerAddress(sk, "/peer");
    bool r3 = sk->init(sk);
    bool r4 = sk->send(sk, "\x1\x2\x3\x4\x5", 5);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    sk->free(sk);
}

// Tests sendTo method
// bool sendTo(const void *msg, size_t len, const char *addrStr)
Test(SockUnixTest, MethodSendTo)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockUnix);
    useTestMode(true);
    bool r1 = sk->setSelfAddress(sk, "/me");
    bool r2 = sk->init(sk);
    bool r3 = sk->sendTo(sk, "\x1\x2\x3\x4\x5", 5, "/peer");
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    sk->free(sk);
}

// Tests rcv method
// ssize_t rcv(void *buf, size_t bufSize, bool block = false)
Test(SockUnixTest, MethodRcv)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockUnix);
    useTestMode(true);
    bool r1 = sk->setSelfAddress(sk, "/me");
    bool r2 = sk->setPeerAddress(sk, "/peer");
    bool r3 = sk->init(sk);
    uint8_t buf[10];
    bool r4 = sk->rcv(sk, buf, sizeof buf, false) == 5;
    bool r5 = memcmp(buf, "\x2\x4\x5\x6\x7", 5) == 0;
    bool r6 = sk->rcv(sk, buf, sizeof buf, true) == 5;
    bool r7 = memcmp(buf, "\x1\x4\x5\x6\x7", 5) == 0;
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    cr_expect(r6);
    cr_expect(r7);
    sk->free(sk);
}

// Tests rcvFrom method
// ssize_t rcvFrom(void *buf, size_t bufSize, std::string &from,
//     bool block = false)
// ssize_t rcvFrom(void *buf, size_t bufSize, bool block = false)
// const char *getLastFrom()
Test(SockUnixTest, MethodRcvFrom)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockUnix);
    useTestMode(true);
    bool r1 = sk->setSelfAddress(sk, "/me");
    bool r2 = sk->init(sk);
    uint8_t buf[10];
    bool r3 = sk->rcvFromA(sk, buf, sizeof buf, false) == 5;
    bool r4 = memcmp(buf, "\x2\x4\x5\x6\x7", 5) == 0;
    bool r5 = strcmp(sk->getLastFrom(sk), "/peer") == 0;
    bool r6 = sk->rcvFromA(sk, buf, sizeof buf, true) == 5;
    bool r7 = memcmp(buf, "\x1\x4\x5\x6\x7", 5) == 0;
    bool r8 = strcmp(sk->getLastFrom(sk), "/peer") == 0;
    char from[30];
    bool r9 = sk->rcvFrom(sk, buf, sizeof buf, from, sizeof from, false) == 5;
    bool r10 = memcmp(buf, "\x2\x4\x5\x6\x7", 5) == 0;
    bool r11 = strcmp(from, "/peer") == 0;
    bool r12 = sk->rcvFrom(sk, buf, sizeof buf, from, sizeof from, true) == 5;
    bool r13 = memcmp(buf, "\x1\x4\x5\x6\x7", 5) == 0;
    bool r14 = strcmp(from, "/peer") == 0;
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    cr_expect(r6);
    cr_expect(r7);
    cr_expect(r8);
    cr_expect(r9);
    cr_expect(r10);
    cr_expect(r11);
    cr_expect(r12);
    cr_expect(r13);
    cr_expect(r14);
    sk->free(sk);
}

// Tests setIfUsingIndex method
// bool setIfUsingIndex(int ifIndex)
// bool setUdpTtl(uint8_t udp_ttl)
// bool init()
// int getFd()
// int fileno()
Test(SockIp4Test, MethodSetIfUsingIndex)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockIp4);
    useTestMode(true);
    bool r1 = sk->setIfUsingIndex(sk, 7);
    bool r2 = sk->setUdpTtl(sk, 7);
    bool r3 = sk->init(sk);
    int fd = sk->getFd(sk);
    bool r5 = fd == sk->fileno(sk);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(ge(int, fd, 0));
    cr_expect(r5);
    sk->free(sk);
}

// Tests setIfUsingName method
// bool setIfUsingName(const char *ifName)
// void close()
Test(SockIp4Test, MethodSetIfUsingName)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockIp4);
    useTestMode(true);
    bool r1 = sk->setIfUsingName(sk, "eth0");
    bool r2 = sk->setUdpTtl(sk, 7);
    bool r3 = sk->init(sk);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    sk->free(sk);
}

// Tests setIf method
// bool setIf(const_ptpmgmt_ifInfo ifObj)
Test(SockIp4Test, MethodSetIf)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockIp4);
    ptpmgmt_ifInfo i = ptpmgmt_ifInfo_alloc();
    useTestMode(true);
    bool r1 = i->initUsingIndex(i, 7);
    bool r2 = sk->setIf(sk, i);
    bool r3 = sk->setUdpTtl(sk, 7);
    bool r4 = sk->init(sk);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    sk->free(sk);
}

// Tests setUdpTtl method
// bool setUdpTtl(const_ptpmgmt_cfg cfg, const char *section)
Test(SockIp4Test, MethodSetUdpTtl)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockIp4);
    ptpmgmt_ifInfo i = ptpmgmt_ifInfo_alloc();
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    useTestMode(true);
    bool r1 = i->initUsingIndex(i, 7);
    bool r2 = sk->setIf(sk, i);
    bool r3 = f->read_cfg(f, "utest/testing.cfg");
    bool r4 = sk->setUdpTtlCfg(sk, f, "dumm");
    bool r5 = sk->init(sk);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    sk->free(sk);
}

// Tests setAll method
// bool setAll(const_ptpmgmt_ifInfo ifObj, const_ptpmgmt_cfg cfg,
//     const char *section) => setAll(i, f, "dumm")
Test(SockIp4Test, MethodSetAll)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockIp4);
    ptpmgmt_ifInfo i = ptpmgmt_ifInfo_alloc();
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    useTestMode(true);
    bool r1 = i->initUsingIndex(i, 7);
    bool r2 = f->read_cfg(f, "utest/testing.cfg");
    bool r3 = sk->setAll(sk, i, f, "dumm");
    bool r4 = sk->init(sk);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    sk->free(sk);
}

// Tests setAllInit method
// bool setAllInit(const_ptpmgmt_ifInfo ifObj, const_ptpmgmt_cfg cfg,
//     const char *section) => setAllInit(i, f, "dumm")
Test(SockIp4Test, MethodSetAllInit)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockIp4);
    ptpmgmt_ifInfo i = ptpmgmt_ifInfo_alloc();
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    useTestMode(true);
    bool r1 = i->initUsingIndex(i, 7);
    bool r2 = f->read_cfg(f, "utest/testing.cfg");
    bool r3 = sk->setAllInit(sk, i, f, "dumm");
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    sk->free(sk);
}

// Tests poll method
// bool poll(uint64_t timeout_ms = 0)
Test(SockIp4Test, MethodPoll)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockIp4);
    useTestMode(true);
    bool r1 = sk->setIfUsingIndex(sk, 7);
    bool r2 = sk->setUdpTtl(sk, 7);
    bool r3 = sk->init(sk);
    bool r4 = sk->poll(sk, 0);
    bool r5 = sk->poll(sk, 0);
    bool r6 = sk->poll(sk, 2000);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    cr_expect(r6);
    sk->free(sk);
}

// Tests poll method
// bool tpoll(uint64_t &timeout_ms)
Test(SockIp4Test, MethodTpoll)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockIp4);
    useTestMode(true);
    bool r1 = sk->setIfUsingIndex(sk, 7);
    bool r2 = sk->setUdpTtl(sk, 7);
    bool r3 = sk->init(sk);
    uint64_t to = 1500;
    bool r4 = sk->tpoll(sk, &to);
    bool r5 = to == 500;
    bool r6 = sk->tpoll(sk, &to);
    bool r7 = to == 0;
    bool r8 = sk->tpoll(sk, &to);
    bool r9 = to == 0;
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    cr_expect(r6);
    cr_expect(r7);
    cr_expect(r8);
    cr_expect(r9);
    sk->free(sk);
}

// Tests send method
// bool send(const void *msg, size_t len)
Test(SockIp4Test, MethodSend)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockIp4);
    useTestMode(true);
    bool r1 = sk->setIfUsingIndex(sk, 7);
    bool r2 = sk->setUdpTtl(sk, 7);
    bool r3 = sk->init(sk);
    bool r4 = sk->send(sk, "\x1\x2\x3\x4\x5", 5);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    sk->free(sk);
}

// Tests rcv method
// ssize_t rcv(void *buf, size_t bufSize, bool block = false)
Test(SockIp4Test, MethodRcv)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockIp4);
    useTestMode(true);
    bool r1 = sk->setIfUsingIndex(sk, 7);
    bool r2 = sk->setUdpTtl(sk, 7);
    bool r3 = sk->init(sk);
    uint8_t buf[10];
    bool r4 = sk->rcv(sk, buf, sizeof buf, false) == 5;
    bool r5 = memcmp(buf, "\x2\x4\x5\x6\x7", 5) == 0;
    bool r6 = sk->rcv(sk, buf, sizeof buf, true) == 5;
    bool r7 = memcmp(buf, "\x1\x4\x5\x6\x7", 5) == 0;
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    cr_expect(r6);
    cr_expect(r7);
    sk->free(sk);
}

// Tests setIfUsingIndex method
// bool setIfUsingIndex(int ifIndex)
// bool setUdpTtl(uint8_t udp_ttl)
// bool setScope(uint8_t udp6_scope)
// bool init()
// int getFd()
// int fileno()
Test(SockIp6Test, MethodSetIfUsingIndex)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockIp6);
    useTestMode(true);
    bool r1 = sk->setIfUsingIndex(sk, 7);
    bool r2 = sk->setUdpTtl(sk, 7);
    bool r3 = sk->setScope(sk, 15);
    bool r4 = sk->init(sk);
    int fd = sk->getFd(sk);
    bool r5 = fd == sk->fileno(sk);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(ge(int, fd, 0));
    cr_expect(r5);
    sk->free(sk);
}

// Tests setIfUsingName method
// bool setIfUsingName(const char *ifName)
// void close()
Test(SockIp6Test, MethodSetIfUsingName)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockIp6);
    useTestMode(true);
    bool r1 = sk->setIfUsingName(sk, "eth0");
    bool r2 = sk->setUdpTtl(sk, 7);
    bool r3 = sk->setScope(sk, 15);
    bool r4 = sk->init(sk);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    sk->free(sk);
}

// Tests setIf method
// bool setIf(const_ptpmgmt_ifInfo ifObj)
Test(SockIp6Test, MethodSetIf)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockIp6);
    ptpmgmt_ifInfo i = ptpmgmt_ifInfo_alloc();
    useTestMode(true);
    bool r1 = i->initUsingIndex(i, 7);
    bool r2 = sk->setIf(sk, i);
    bool r3 = sk->setUdpTtl(sk, 7);
    bool r4 = sk->setScope(sk, 15);
    bool r5 = sk->init(sk);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    sk->free(sk);
}

// Tests setUdpTtl method
// bool setUdpTtl(const_ptpmgmt_cfg cfg, const char *section)
// bool setScope(const_ptpmgmt_cfg cfg, const char *section)
Test(SockIp6Test, MethodSetUdpTtl)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockIp6);
    ptpmgmt_ifInfo i = ptpmgmt_ifInfo_alloc();
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    useTestMode(true);
    bool r1 = i->initUsingIndex(i, 7);
    bool r2 = sk->setIf(sk, i);
    bool r3 = f->read_cfg(f, "utest/testing.cfg");
    bool r4 = sk->setUdpTtlCfg(sk, f, "dumm");
    bool r5 = sk->setScopeCfg(sk, f, "dumm");
    bool r6 = sk->init(sk);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    cr_expect(r6);
    sk->free(sk);
}

// Tests setAll method
// bool setAll(const_ptpmgmt_ifInfo ifObj, const_ptpmgmt_cfg cfg,
//     const char *section)
Test(SockIp6Test, MethodSetAll)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockIp6);
    ptpmgmt_ifInfo i = ptpmgmt_ifInfo_alloc();
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    useTestMode(true);
    bool r1 = i->initUsingIndex(i, 7);
    bool r2 = f->read_cfg(f, "utest/testing.cfg");
    bool r3 = sk->setAll(sk, i, f, "dumm");
    bool r4 = sk->init(sk);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    sk->free(sk);
}

// Tests setAllInit method
// bool setAllInit(const_ptpmgmt_ifInfo ifObj, const_ptpmgmt_cfg cfg,
//     const char *section)
Test(SockIp6Test, MethodSetAllInit)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockIp6);
    ptpmgmt_ifInfo i = ptpmgmt_ifInfo_alloc();
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    useTestMode(true);
    bool r1 = i->initUsingIndex(i, 7);
    bool r2 = f->read_cfg(f, "utest/testing.cfg");
    bool r3 = sk->setAllInit(sk, i, f, "dumm");
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    sk->free(sk);
}

// Tests poll method
// bool poll(uint64_t timeout_ms = 0)
Test(SockIp6Test, MethodPoll)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockIp6);
    useTestMode(true);
    bool r1 = sk->setIfUsingIndex(sk, 7);
    bool r2 = sk->setUdpTtl(sk, 7);
    bool r3 = sk->setScope(sk, 15);
    bool r4 = sk->init(sk);
    bool r5 = sk->poll(sk, 0);
    bool r6 = sk->poll(sk, 0);
    bool r7 = sk->poll(sk, 2000);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    cr_expect(r6);
    cr_expect(r7);
    sk->free(sk);
}

// Tests poll method
// bool tpoll(uint64_t &timeout_ms)
Test(SockIp6Test, MethodTpoll)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockIp6);
    useTestMode(true);
    bool r1 = sk->setIfUsingIndex(sk, 7);
    bool r2 = sk->setUdpTtl(sk, 7);
    bool r3 = sk->setScope(sk, 15);
    bool r4 = sk->init(sk);
    uint64_t to = 1500;
    bool r5 = sk->tpoll(sk, &to);
    bool r6 = to == 500;
    bool r7 = sk->tpoll(sk, &to);
    bool r8 = to == 0;
    bool r9 = sk->tpoll(sk, &to);
    bool r10 = to == 0;
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    cr_expect(r6);
    cr_expect(r7);
    cr_expect(r8);
    cr_expect(r9);
    cr_expect(r10);
    sk->free(sk);
}

// Tests send method
// bool send(const void *msg, size_t len)
Test(SockIp6Test, MethodSend)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockIp6);
    useTestMode(true);
    bool r1 = sk->setIfUsingIndex(sk, 7);
    bool r2 = sk->setUdpTtl(sk, 7);
    bool r3 = sk->setScope(sk, 15);
    bool r4 = sk->init(sk);
    bool r5 = sk->send(sk, "\x1\x2\x3\x4\x5", 5);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    sk->free(sk);
}

// Tests rcv method
// ssize_t rcv(void *buf, size_t bufSize, bool block = false)
Test(SockIp6Test, MethodRcv)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockIp6);
    useTestMode(true);
    bool r1 = sk->setIfUsingIndex(sk, 7);
    bool r2 = sk->setUdpTtl(sk, 7);
    bool r3 = sk->setScope(sk, 15);
    bool r4 = sk->init(sk);
    uint8_t buf[10];
    bool r5 = sk->rcv(sk, buf, sizeof buf, false) == 5;
    bool r6 = memcmp(buf, "\x2\x4\x5\x6\x7", 5) == 0;
    bool r7 = sk->rcv(sk, buf, sizeof buf, true) == 5;
    bool r8 = memcmp(buf, "\x1\x4\x5\x6\x7", 5) == 0;
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    cr_expect(r6);
    cr_expect(r7);
    cr_expect(r8);
    sk->free(sk);
}

// Tests setIfUsingIndex method
// bool setIfUsingIndex(int ifIndex)
// bool setPtpDstMac(const uint8_t *ptp_dst_mac, size_t len)
// bool setSocketPriority(uint8_t socket_priority)
// bool init()
// int getFd()
// int fileno()
Test(SockRawTest, MethodSetIfUsingIndex)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockRaw);
    useTestMode(true);
    bool r1 = sk->setIfUsingIndex(sk, 7);
    const uint8_t mac[6] = { 1, 27, 23, 15, 12 };
    bool r2 = sk->setPtpDstMac(sk, mac, sizeof mac);
    bool r3 = sk->setSocketPriority(sk, 7);
    bool r4 = sk->init(sk);
    int fd = sk->getFd(sk);
    bool r6 = fd == sk->fileno(sk);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(ge(int, fd, 0));
    cr_expect(r6);
    sk->free(sk);
}

// Tests setIfUsingName method
// bool setIfUsingName(const char *ifName)
// bool setPtpDstMac(const uint8_t *ptp_dst_mac, size_t len)
// void close()
Test(SockRawTest, MethodSetIfUsingName)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockRaw);
    useTestMode(true);
    bool r1 = sk->setIfUsingName(sk, "eth0");
    const uint8_t mac[6] = { 1, 27, 23, 15, 12 };
    bool r2 = sk->setPtpDstMac(sk, mac, sizeof mac);
    bool r3 = sk->setSocketPriority(sk, 7);
    bool r4 = sk->init(sk);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    sk->free(sk);
}

// Tests setPtpDstMacStr method
// bool setPtpDstMacStr(const char *string)
Test(SockRawTest, MethodSetPtpDstMacStr)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockRaw);
    useTestMode(true);
    bool r1 = sk->setIfUsingIndex(sk, 7);
    bool r2 = sk->setPtpDstMacStr(sk, "1:1b:17:f:c:0");
    bool r3 = sk->setSocketPriority(sk, 7);
    bool r4 = sk->init(sk);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    sk->free(sk);
}

// Tests setIf method
// bool setIf(const_ptpmgmt_ifInfo ifObj)
Test(SockRawTest, MethodSetIf)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockRaw);
    ptpmgmt_ifInfo i = ptpmgmt_ifInfo_alloc();
    useTestMode(true);
    bool r1 = i->initUsingIndex(i, 7);
    bool r2 = sk->setIf(sk, i);
    bool r3 = sk->setPtpDstMacStr(sk, "1:1b:17:f:c:0");
    bool r4 = sk->setSocketPriority(sk, 7);
    bool r5 = sk->init(sk);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    sk->free(sk);
}

// Tests setPtpDstMac method
// bool setPtpDstMac(const_ptpmgmt_cfg cfg, const char *section)
// bool setSocketPriority(const_ptpmgmt_cfg cfg, const char *section)
Test(SockRawTest, setPtpDstMac)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockRaw);
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    useTestMode(true);
    bool r1 = sk->setIfUsingIndex(sk, 7);
    bool r2 = f->read_cfg(f, "utest/testing.cfg");
    bool r3 = sk->setPtpDstMacCfg(sk, f, "dumm");
    bool r4 = sk->setSocketPriorityCfg(sk, f, "dumm");
    bool r5 = sk->init(sk);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    sk->free(sk);
}

// Tests setAll method
// bool setAll(const_ptpmgmt_ifInfo ifObj, const_ptpmgmt_cfg cfg,
//     const char *section)
Test(SockRawTest, MethodSetAll)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockRaw);
    ptpmgmt_ifInfo i = ptpmgmt_ifInfo_alloc();
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    useTestMode(true);
    bool r1 = i->initUsingIndex(i, 7);
    bool r2 = f->read_cfg(f, "utest/testing.cfg");
    bool r3 = sk->setAll(sk, i, f, "dumm");
    bool r4 = sk->init(sk);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    sk->free(sk);
}

// Tests setAllInit method
// bool setAllInit(const_ptpmgmt_ifInfo ifObj, const_ptpmgmt_cfg cfg,
//     const char *section)
Test(SockRawTest, MethodSetAllInit)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockRaw);
    ptpmgmt_ifInfo i = ptpmgmt_ifInfo_alloc();
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    useTestMode(true);
    bool r1 = i->initUsingIndex(i, 7);
    bool r2 = f->read_cfg(f, "utest/testing.cfg");
    bool r3 = sk->setAllInit(sk, i, f, "dumm");
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    sk->free(sk);
}

// Tests poll method
// bool poll(uint64_t timeout_ms = 0)
Test(SockRawTest, MethodPoll)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockRaw);
    useTestMode(true);
    bool r1 = sk->setIfUsingIndex(sk, 7);
    bool r2 = sk->setPtpDstMacStr(sk, "1:1b:17:f:c:0");
    bool r3 = sk->setSocketPriority(sk, 7);
    bool r4 = sk->init(sk);
    bool r5 = sk->poll(sk, 0);
    bool r6 = sk->poll(sk, 0);
    bool r7 = sk->poll(sk, 2000);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    cr_expect(r6);
    cr_expect(r7);
    sk->free(sk);
}

// Tests poll method
// bool tpoll(uint64_t &timeout_ms)
Test(SockRawTest, MethodTpoll)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockRaw);
    useTestMode(true);
    bool r1 = sk->setIfUsingIndex(sk, 7);
    bool r2 = sk->setPtpDstMacStr(sk, "1:1b:17:f:c:0");
    bool r3 = sk->setSocketPriority(sk, 7);
    bool r4 = sk->init(sk);
    uint64_t to = 1500;
    bool r5 = sk->tpoll(sk, &to);
    bool r6 = to == 500;
    bool r7 = sk->tpoll(sk, &to);
    bool r8 = to == 0;
    bool r9 = sk->tpoll(sk, &to);
    bool r10 = to == 0;
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    cr_expect(r6);
    cr_expect(r7);
    cr_expect(r8);
    cr_expect(r9);
    cr_expect(r10);
    sk->free(sk);
}

// Tests send method
// bool send(const void *msg, size_t len)
Test(SockRawTest, MethodSend)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockRaw);
    useTestMode(true);
    bool r1 = sk->setIfUsingIndex(sk, 7);
    bool r2 = sk->setPtpDstMacStr(sk, "1:1b:17:f:c:0");
    bool r3 = sk->setSocketPriority(sk, 7);
    bool r4 = sk->init(sk);
    bool r5 = sk->send(sk, "\x1\x2\x3\x4\x5", 5);
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    sk->free(sk);
}

// Tests rcv method
// ssize_t rcv(void *buf, size_t bufSize, bool block = false)
Test(SockRawTest, MethodRcv)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockRaw);
    useTestMode(true);
    bool r1 = sk->setIfUsingIndex(sk, 7);
    bool r2 = sk->setPtpDstMacStr(sk, "1:1b:17:f:c:0");
    bool r3 = sk->setSocketPriority(sk, 7);
    bool r4 = sk->init(sk);
    uint8_t buf[10];
    bool r5 = sk->rcv(sk, buf, sizeof buf, false) == 5;
    bool r6 = memcmp(buf, "\x2\x4\x5\x6\x7", 5) == 0;
    bool r7 = sk->rcv(sk, buf, sizeof buf, true) == 5;
    bool r8 = memcmp(buf, "\x1\x4\x5\x6\x7", 5) == 0;
    sk->close(sk);
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    cr_expect(r3);
    cr_expect(r4);
    cr_expect(r5);
    cr_expect(r6);
    cr_expect(r7);
    cr_expect(r8);
    sk->free(sk);
}
