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
// const char *getHomeDir(ptpmgmt_sk sk)
Test(SockUnixTest, MethodGetHomeDir)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockUnix);
    useTestMode(true);
    bool r1 = strcmp(sk->getHomeDir(sk), "/home/usr") == 0;
    useRoot(true);
    bool r2 = strcmp(sk->getHomeDir(sk), "/root") == 0;
    useTestMode(false);
    cr_expect(r1);
    cr_expect(r2);
    sk->free(sk);
}

// Tests setDefSelfAddress method
// bool setDefSelfAddress(ptpmgmt_sk sk, const char *rootBase, const char *useDef)
// const char *getSelfAddress(const_ptpmgmt_sk sk)
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
// bool setSelfAddress(ptpmgmt_sk sk, const char *string)
// bool setSelfAddressA(ptpmgmt_sk sk, const char *string)
// bool isSelfAddressAbstract(const_ptpmgmt_sk sk)
Test(SockUnixTest, MethodSetSelfAddress)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockUnix);
    cr_expect(sk->setSelfAddress(sk, "/tes1"));
    cr_expect(eq(str, (char *)sk->getSelfAddress(sk), "/tes1"));
    cr_expect(not(sk->isSelfAddressAbstract(sk)));
    // Using abstract socket address
    cr_expect(sk->setSelfAddressA(sk, "tes1"));
    cr_expect(eq(str, (char *)sk->getSelfAddress(sk) + 1, "tes1"));
    cr_expect(eq(str, (char *)sk->getSelfAddress(sk), "")); // First char is 0
    cr_expect(sk->isSelfAddressAbstract(sk));
    sk->free(sk);
}

// Tests init method
// bool init(ptpmgmt_sk sk)
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
// void close(ptpmgmt_sk sk)
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
// int getFd(const_ptpmgmt_sk sk)
// int fileno(const_ptpmgmt_sk sk)
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
// bool poll(ptpmgmt_sk sk, uint64_t timeout_ms)
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
// bool tpoll(ptpmgmt_sk sk, uint64_t *timeout_ms)
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
// bool setPeerAddress(ptpmgmt_sk sk, const char *string)
// bool setPeerAddressA(ptpmgmt_sk sk, const char *string)
// const char *getPeerAddress(ptpmgmt_sk sk)
// bool isPeerAddressAbstract(const_ptpmgmt_sk sk)
Test(SockUnixTest, MethodSetPeerAddress)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockUnix);
    cr_expect(sk->setPeerAddress(sk, "/tes1"));
    cr_expect(zero(strcmp(sk->getPeerAddress(sk), "/tes1")));
    cr_expect(not(sk->isPeerAddressAbstract(sk)));
    // Using abstract socket address
    cr_expect(sk->setPeerAddressA(sk, "tes1"));
    cr_expect(eq(str, (char *)sk->getPeerAddress(sk) + 1, "tes1"));
    cr_expect(eq(str, (char *)sk->getPeerAddress(sk), "")); // First char is 0
    cr_expect(sk->isPeerAddressAbstract(sk));
    sk->free(sk);
}

// Tests setPeerAddressCfg method
// bool setPeerAddressCfg(ptpmgmt_sk sk, const_ptpmgmt_cfg cfg, const char *section)
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
// bool send(ptpmgmt_sk sk, const void *msg, size_t len)
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
// bool sendTo(ptpmgmt_sk sk, const void *msg, size_t len, const char *addrStr)
// bool sendToA(ptpmgmt_sk sk, const void *msg, size_t len, const char *addrStr)
Test(SockUnixTest, MethodSendTo)
{
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockUnix);
    useTestMode(true);
    cr_expect(sk->setSelfAddress(sk, "/me"));
    cr_expect(sk->init(sk));
    cr_expect(sk->sendTo(sk, "\x1\x2\x3\x4\x5", 5, "/peer"));
    // Using abstract socket address
    cr_expect(sk->sendToA(sk, "\x1\x2\x3\x4\x5", 5, "peer"));
    sk->close(sk);
    useTestMode(false);
    sk->free(sk);
}

// Tests rcv method
// ssize_t rcv(ptpmgmt_sk sk, void *buf, size_t bufSize, bool block)
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
// ssize_t rcvFrom(ptpmgmt_sk sk, void *buf, size_t bufSize, char *from,
//     size_t *fromSize, bool block);
// ssize_t rcvFromA(ptpmgmt_sk sk, void *buf, size_t bufSize, bool block)
// const char *getLastFrom(ptpmgmt_sk sk)
// bool isLastFromAbstract(const_ptpmgmt_sk sk)
Test(SockUnixTest, MethodRcvFrom)
{
    uint8_t buf[10];
    char from[30];
    ptpmgmt_sk sk = ptpmgmt_sk_alloc(ptpmgmt_SockUnix);
    useTestMode(true);
    cr_expect(sk->setSelfAddress(sk, "/me"));
    cr_expect(sk->init(sk));
    cr_expect(sk->rcvFromA(sk, buf, sizeof buf, false) == 5);
    cr_expect(zero(memcmp(buf, "\x2\x4\x5\x6\x7", 5)));
    cr_expect(zero(strcmp(sk->getLastFrom(sk), "/peer")));
    cr_expect(not(sk->isLastFromAbstract(sk)));
    cr_expect(sk->rcvFromA(sk, buf, sizeof buf, true) == 5);
    cr_expect(zero(memcmp(buf, "\x1\x4\x5\x6\x7", 5)));
    cr_expect(zero(strcmp(sk->getLastFrom(sk), "/peer")));
    cr_expect(not(sk->isLastFromAbstract(sk)));
    size_t fromSize = sizeof from;
    cr_expect(sk->rcvFrom(sk, buf, sizeof buf, from, &fromSize, false) == 5);
    cr_expect(zero(memcmp(buf, "\x2\x4\x5\x6\x7", 5)));
    cr_expect(zero(strcmp(from, "/peer")));
    cr_expect(eq(sz, fromSize, 5));
    fromSize = sizeof from;
    cr_expect(sk->rcvFrom(sk, buf, sizeof buf, from, &fromSize, true) == 5);
    cr_expect(zero(memcmp(buf, "\x1\x4\x5\x6\x7", 5)));
    cr_expect(zero(strcmp(from, "/peer")));
    cr_expect(eq(sz, fromSize, 5));
    sk->close(sk);
    sk->free(sk);
    useTestMode(false);
}

// Tests setIfUsingIndex method
// bool setIfUsingIndex(ptpmgmt_sk sk, int ifIndex)
// bool setUdpTtl(ptpmgmt_sk sk, uint8_t udp_ttl)
// bool init(ptpmgmt_sk sk)
// int getFd(const_ptpmgmt_sk sk)
// int fileno(const_ptpmgmt_sk sk)
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
// bool setIfUsingName(ptpmgmt_sk sk, const char *ifName)
// void close(ptpmgmt_sk sk)
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
// bool setIf(ptpmgmt_sk sk, const_ptpmgmt_ifInfo ifObj)
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
// bool setUdpTtl(ptpmgmt_sk sk, const_ptpmgmt_cfg cfg, const char *section)
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
// bool setAll(ptpmgmt_sk sk, const_ptpmgmt_ifInfo ifObj, const_ptpmgmt_cfg cfg,
//     const char *section)
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
// bool setAllInit(ptpmgmt_sk sk, const_ptpmgmt_ifInfo ifObj,
//     const_ptpmgmt_cfg cfg, const char *section)
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
// bool poll(ptpmgmt_sk sk, uint64_t timeout_ms)
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
// bool tpoll(ptpmgmt_sk sk, uint64_t *timeout_ms)
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
// bool send(ptpmgmt_sk sk, const void *msg, size_t len)
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
// ssize_t rcv(ptpmgmt_sk sk, void *buf, size_t bufSize, bool block)
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
// bool setIfUsingIndex(ptpmgmt_sk sk, int ifIndex)
// bool setUdpTtl(ptpmgmt_sk sk, uint8_t udp_ttl)
// bool setScope(ptpmgmt_sk sk, uint8_t udp6_scope)
// bool init(ptpmgmt_sk sk)
// int getFd(const_ptpmgmt_sk sk)
// int fileno(const_ptpmgmt_sk sk)
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
// bool setIfUsingName(ptpmgmt_sk sk, const char *ifName)
// void close(ptpmgmt_sk sk)
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
// bool setIf(ptpmgmt_sk sk, const_ptpmgmt_ifInfo ifObj)
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
// bool setUdpTtl(ptpmgmt_sk sk, const_ptpmgmt_cfg cfg, const char *section)
// bool setScope(ptpmgmt_sk sk, const_ptpmgmt_cfg cfg, const char *section)
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
// bool setAll(ptpmgmt_sk sk, const_ptpmgmt_ifInfo ifObj, const_ptpmgmt_cfg cfg,
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
// bool setAllInit(ptpmgmt_sk sk, const_ptpmgmt_ifInfo ifObj,
//     const_ptpmgmt_cfg cfg, const char *section)
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
// bool poll(ptpmgmt_sk sk, uint64_t timeout_ms)
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
// bool tpoll(ptpmgmt_sk sk, uint64_t *timeout_ms)
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
// bool send(ptpmgmt_sk sk, const void *msg, size_t len)
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
// ssize_t rcv(ptpmgmt_sk sk, void *buf, size_t bufSize, bool block)
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
// bool setIfUsingIndex(ptpmgmt_sk sk, int ifIndex)
// bool setPtpDstMac(ptpmgmt_sk sk, const uint8_t *ptp_dst_mac, size_t len)
// bool setSocketPriority(ptpmgmt_sk sk, uint8_t socket_priority)
// bool init(ptpmgmt_sk sk)
// int getFd(const_ptpmgmt_sk sk)
// int fileno(const_ptpmgmt_sk sk)
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
// bool setIfUsingName(ptpmgmt_sk sk, const char *ifName)
// bool setPtpDstMac(ptpmgmt_sk sk, const uint8_t *ptp_dst_mac, size_t len)
// void close(ptpmgmt_sk sk)
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
// bool setPtpDstMacStr(ptpmgmt_sk sk, const char *string)
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
// bool setIf(ptpmgmt_sk sk, const_ptpmgmt_ifInfo ifObj)
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
// bool setPtpDstMac(ptpmgmt_sk sk, const_ptpmgmt_cfg cfg, const char *section)
// bool setSocketPriority(ptpmgmt_sk sk, const_ptpmgmt_cfg cfg, const char *section)
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
// bool setAll(ptpmgmt_sk sk, const_ptpmgmt_ifInfo ifObj, const_ptpmgmt_cfg cfg,
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
// bool setAllInit(ptpmgmt_sk sk, const_ptpmgmt_ifInfo ifObj,
//     const_ptpmgmt_cfg cfg, const char *section)
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
// bool poll(ptpmgmt_sk sk, uint64_t timeout_ms)
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
// bool tpoll(ptpmgmt_sk sk, uint64_t *timeout_ms)
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
// bool send(ptpmgmt_sk sk, const void *msg, size_t len)
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
// ssize_t rcv(ptpmgmt_sk sk, void *buf, size_t bufSize, bool block)
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
