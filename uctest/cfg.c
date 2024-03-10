/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief ConfigFile class wrapper unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include "cfg.h"

// Tests allocate empty configuration
// ptpmgmt_cfg ptpmgmt_cfg_alloc()
Test(ConfigFileTest, MethodEmptyConstructor)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_assert(not(zero(ptr, f)));
    // Check default values
    cr_expect(eq(i8, f->transportSpecific(f, NULL), 0));
    cr_expect(eq(i8, f->domainNumber(f, NULL), 0));
    cr_expect(eq(i8, f->udp6_scope(f, NULL), 0xe));
    cr_expect(eq(i8, f->udp_ttl(f, NULL), 1));
    cr_expect(eq(i8, f->socket_priority(f, NULL), 0));
    cr_expect(eq(i8, f->network_transport(f, NULL), '4'));
    cr_expect(eq(i32, f->active_key_id(f, NULL), 0));
    cr_expect(eq(i8, f->spp(f, NULL), 0));
    cr_expect(eq(i8, f->allow_unauth(f, NULL), 0));
    cr_expect(not(f->haveSpp(f, NULL)));
    cr_expect(eq(str, (char *)f->uds_address(f, NULL), "/var/run/ptp4l"));
    cr_expect(zero(ptr, (void *)f->sa_file(f, NULL)));
    {
        size_t l;
        struct cr_mem ptp_dst_mac1 =
        { .data = "\x1\x1b\x19\x0\x0\x0", .size = 6 };
        struct cr_mem ptp_dst_mac2 =
        { .data = f->ptp_dst_mac(f, &l, NULL), .size = 6 };
        cr_expect(eq(i32, l, 6));
        struct cr_mem p2p_dst_mac1 =
        { .data = "\x1\x80\xc2\x0\x0\xe", .size = 6 };
        struct cr_mem p2p_dst_mac2 =
        { .data = f->p2p_dst_mac(f, &l, NULL), .size = 6 };
        cr_expect(eq(i32, l, 6));
        cr_expect(cr_user_mem_eq(&ptp_dst_mac2, &ptp_dst_mac1));
        cr_expect(cr_user_mem_eq(&p2p_dst_mac2, &p2p_dst_mac1));
    }
    f->free(f);
}

// Tests read configuration file
// bool read_cfg(ptpmgmt_cfg _this, const char *file)
Test(ConfigFileTest, MethodReadConfiguration)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_assert(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    // Check file configuration
    cr_expect(eq(i8, f->transportSpecific(f, NULL), 7));
    cr_expect(eq(i8, f->domainNumber(f, NULL), 5));
    cr_expect(eq(i8, f->udp6_scope(f, NULL), 0xd));
    cr_expect(eq(i8, f->udp_ttl(f, NULL), 3));
    cr_expect(eq(i8, f->socket_priority(f, NULL), 11));
    cr_expect(eq(i8, f->network_transport(f, NULL), '6'));
    cr_expect(eq(i32, f->active_key_id(f, NULL), 10));
    cr_expect(eq(i8, f->spp(f, NULL), 2));
    cr_expect(eq(i8, f->allow_unauth(f, NULL), 2));
    cr_expect(f->haveSpp(f, NULL));
    cr_expect(eq(str, (char *)f->uds_address(f, NULL), "/var/run/dummy"));
    cr_expect(eq(str, (char *)f->sa_file(f, NULL), "utest/sa_file.cfg"));
    {
        size_t l;
        struct cr_mem ptp_dst_mac1 =
        { .data = "\x1\x1b\x19\xf\xc\x0", .size = 6 };
        struct cr_mem ptp_dst_mac2 =
        { .data = f->ptp_dst_mac(f, &l, NULL), .size = 6 };
        cr_expect(eq(i32, l, 6));
        struct cr_mem p2p_dst_mac1 =
        { .data = "\x1\x80\xc2\x1\x0\xe", .size = 6 };
        struct cr_mem p2p_dst_mac2 =
        { .data = f->p2p_dst_mac(f, &l, NULL), .size = 6 };
        cr_expect(eq(i32, l, 6));
        cr_expect(cr_user_mem_eq(&ptp_dst_mac2, &ptp_dst_mac1));
        cr_expect(cr_user_mem_eq(&p2p_dst_mac2, &p2p_dst_mac1));
    }
    f->free(f);
}

// Tests get transport specific parameter
// uint8_t transportSpecific(ptpmgmt_cfg _this, const char *section)
Test(ConfigFileTest, MethodTransportSpecific)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_assert(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    cr_expect(eq(i8, f->transportSpecific(f, "dumm"), 9));
    cr_expect(eq(i8, f->transportSpecific(f, "non"), 7));
    f->free(f);
}

// Tests get domain number parameter
// uint8_t domainNumber(ptpmgmt_cfg _this, const char *section)
Test(ConfigFileTest, MethodDomainNumber)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_assert(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    cr_expect(eq(i8, f->domainNumber(f, "dumm"), 3));
    cr_expect(eq(i8, f->domainNumber(f, "non"), 5));
    f->free(f);
}

// Tests get UDP IPv6 scope parameter
// uint8_t udp6_scope(ptpmgmt_cfg _this, const char *section)
Test(ConfigFileTest, MethodUdp6_scope)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_assert(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    cr_expect(eq(i8, f->udp6_scope(f, "dumm"), 0xf));
    cr_expect(eq(i8, f->udp6_scope(f, "non"), 0xd));
    f->free(f);
}

// Tests get UDP IPv4 Time to live parameter
// uint8_t udp_ttl(ptpmgmt_cfg _this, const char *section)
Test(ConfigFileTest, MethodUdp_ttl)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_assert(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    cr_expect(eq(i8, f->udp_ttl(f, "dumm"), 7));
    cr_expect(eq(i8, f->udp_ttl(f, "non"), 3));
    f->free(f);
}

// Tests get socket priority parameter
// uint8_t socket_priority(ptpmgmt_cfg _this, const char *section)
Test(ConfigFileTest, MethodSocketPriority)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_assert(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    cr_expect(eq(i8, f->socket_priority(f, "dumm"), 7));
    cr_expect(eq(i8, f->socket_priority(f, "non"), 11));
    f->free(f);
}

// Tests get network transport type parameter
// uint8_t network_transport(ptpmgmt_cfg _this, const char *section)
Test(ConfigFileTest, MethodNetworkTransport)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_assert(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    cr_expect(eq(i8, f->network_transport(f, "dumm"), '4'));
    cr_expect(eq(i8, f->network_transport(f, "non"), '6'));
    f->free(f);
}

// Tests get authentication Key ID
// uint32_t *active_key_id(const_ptpmgmt_cfg cfg, const char *section)
Test(ConfigFileTest, MethoAuthKeyID)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_assert(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    cr_expect(eq(i32, f->active_key_id(f, "dumm"), 0x1297));
    cr_expect(eq(i32, f->active_key_id(f, "non"), 10));
}

// Tests get the authentication security parameter pointer
// uint8_t *spp(const_ptpmgmt_cfg cfg, const char *section)
Test(ConfigFileTest, MethoAuthSPP)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_assert(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    cr_expect(eq(i8, f->spp(f, "dumm"), 37));
    cr_expect(eq(i8, f->spp(f, "non"), 2));
}

// Tests accept unauthenticated response messages parameter
// uint8_t allow_unauth(const_ptpmgmt_cfg cfg, const char *section)
Test(ConfigFileTest, MethoAllowUnauth)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_assert(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    cr_expect(eq(i8, f->allow_unauth(f, "dumm"), 1));
    cr_expect(eq(i8, f->allow_unauth(f, "non"), 2));
}

// Tests whether the authentication security parameter available
// bool haveSpp(const_ptpmgmt_cfg cfg, const char *section)
Test(ConfigFileTest, MethoHaveSPP)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_assert(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    cr_expect(f->haveSpp(f, NULL));
    cr_expect(f->haveSpp(f, "dumm"));
    cr_expect(f->haveSpp(f, "non"));
}

// Tests get Unix Domain socket address parameter
// const char *uds_address(ptpmgmt_cfg _this, const char *section)
Test(ConfigFileTest, MethodUDSAddress)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_assert(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    cr_expect(eq(str, (char *)f->uds_address(f, "dumm"), "/var/run/dummy2"));
    cr_expect(eq(str, (char *)f->uds_address(f, "non"), "/var/run/dummy"));
    f->free(f);
}

// Tests get authentication security association file
// const char *sa_file(const_ptpmgmt_cfg cfg, const char *section)
Test(ConfigFileTest, MethodSaFile)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_assert(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    cr_expect(eq(str, (char *)f->sa_file(f, "dumm"), "utest/sa_file2.cfg"));
    cr_expect(eq(str, (char *)f->sa_file(f, "non"), "utest/sa_file.cfg"));
}

// Tests get PTP destination MAC address
// const void *ptp_dst_mac(ptpmgmt_cfg _this, const char *section)
Test(ConfigFileTest, MethodPTPDstMac)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_assert(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    {
        size_t l;
        struct cr_mem ptp_dst_mac_dumm1 =
        { .data = "\x1\x1b\x17\xf\xc\x0", .size = 6 };
        struct cr_mem ptp_dst_mac_dumm2 =
        { .data = f->ptp_dst_mac(f, &l, "dumm"), .size = 6 };
        cr_expect(eq(i32, l, 6));
        struct cr_mem ptp_dst_mac_non1 =
        { .data = "\x1\x1b\x19\xf\xc\x0", .size = 6 };
        struct cr_mem ptp_dst_mac_non2 =
        { .data = f->ptp_dst_mac(f, &l, "non"), .size = 6 };
        cr_expect(eq(i32, l, 6));
        cr_expect(cr_user_mem_eq(&ptp_dst_mac_dumm2, &ptp_dst_mac_dumm1));
        cr_expect(cr_user_mem_eq(&ptp_dst_mac_non2, &ptp_dst_mac_non1));
    }
    f->free(f);
}

// Tests get PTP point to point destination MAC address
// const void *p2p_dst_mac(ptpmgmt_cfg _this, const char *section)
Test(ConfigFileTest, MethodP2pDstMac)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_assert(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    {
        size_t l;
        struct cr_mem p2p_dst_mac_dumm1 =
        { .data = "\x1\x80\xc1\x1\x0\xe", .size = 6 };
        struct cr_mem p2p_dst_mac_dumm2 =
        { .data = f->p2p_dst_mac(f, &l, "dumm"), .size = 6 };
        cr_expect(eq(i32, l, 6));
        struct cr_mem p2p_dst_mac_non1 =
        { .data = "\x1\x80\xc2\x1\x0\xe", .size = 6 };
        struct cr_mem p2p_dst_mac_non2 =
        { .data = f->p2p_dst_mac(f, &l, "non"), .size = 6 };
        cr_expect(eq(i32, l, 6));
        cr_expect(cr_user_mem_eq(&p2p_dst_mac_dumm2, &p2p_dst_mac_dumm1));
        cr_expect(cr_user_mem_eq(&p2p_dst_mac_non2, &p2p_dst_mac_non1));
    }
    f->free(f);
}

// Tests read SA file
// bool read_sa(ptpmgmt_safile sf, const char *file)
// ptpmgmt_safile ptpmgmt_safile_alloc()
Test(SaFileTest, MethodReadSA)
{
    ptpmgmt_safile s = ptpmgmt_safile_alloc();
    cr_assert(not(zero(ptr, s)));
    cr_expect(s->read_sa(s, "utest/sa_file.cfg"));
    s->free(s);
}

// Tests read SA file using file name from configuration
// bool read_sa_cfg(ptpmgmt_safile sf, const_ptpmgmt_cfg cfg, const char *section)
Test(SaFileTest, MethodReadSACfg)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_assert(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    ptpmgmt_safile s = ptpmgmt_safile_alloc();
    cr_assert(not(zero(ptr, s)));
    cr_expect(s->read_sa_cfg(s, f, NULL));
    s->free(s);
    f->free(f);
}

// Tests if SPP exist
// bool have(const_ptpmgmt_safile sf, uint8_t spp)
Test(SaFileTest, MethodHaveSpp)
{
    ptpmgmt_safile s = ptpmgmt_safile_alloc();
    cr_assert(not(zero(ptr, s)));
    cr_assert(s->read_sa(s, "utest/sa_file.cfg"));
    cr_expect(not(s->have(s, 17)));
    cr_expect(s->have(s, 0));
    s->free(s);
}

// Tests if Key exist
// bool have_key(const_ptpmgmt_safile sf, uint8_t spp, uint32_t key)
Test(SaFileTest, MethodHaveKey)
{
    ptpmgmt_safile s = ptpmgmt_safile_alloc();
    cr_assert(not(zero(ptr, s)));
    cr_assert(s->read_sa(s, "utest/sa_file.cfg"));
    cr_expect(s->have_key(s, 0, 1));
    s->free(s);
}

// Get SPP
// const_ptpmgmt_spp spp(ptpmgmt_safile sf, uint8_t spp)
// ptpmgmt_spp ptpmgmt_spp_alloc_cp(const_ptpmgmt_spp spp)
Test(SaFileTest, MethodSpp)
{
    ptpmgmt_safile s = ptpmgmt_safile_alloc();
    cr_assert(not(zero(ptr, s)));
    cr_assert(s->read_sa(s, "utest/sa_file.cfg"));
    cr_assert(s->have(s, 1));
    ptpmgmt_spp s1 = ptpmgmt_spp_alloc_cp(s->spp(s, 1));
    cr_expect(eq(sz, s1->keys(s1), 2));
    cr_expect(eq(i8, s1->ownID(s1), 1));
    s1->free(s1);
    cr_expect(not(s->have(s, 19)));
    const_ptpmgmt_spp s19 = s->spp(s, 19);
    cr_expect(zero(ptr, (void *)s19));
    s->free(s);
}

// Constructor
// ptpmgmt_spp ptpmgmt_spp_alloc(uint8_t id)
Test(SppTest, MethodConstructor)
{
    ptpmgmt_spp s = ptpmgmt_spp_alloc(15);
    cr_assert(not(zero(ptr, s)));
    cr_expect(eq(i8, s->ownID(s), 15));
    s->free(s);
}

// Test adding keys
// bool addKey(ptpmgmt_spp spp, uint32_t id, enum PTPMGMT_HMAC_t type,
//     const void *value, size_t size, size_t digest, bool replace)
Test(SppTest, MethodAddKey)
{
    ptpmgmt_spp s = ptpmgmt_spp_alloc(0);
    cr_assert(not(zero(ptr, s)));
    char k57[] = "abxdefghijklmnop";
    cr_expect(s->addKey(s, 0x57, PTPMGMT_HMAC_SHA256, k57, sizeof k57, 16, true));
    cr_expect(eq(sz, s->keys(s), 1));
    cr_expect(s->have(s, 0x57));
    uint8_t k1[16] = {0};
    cr_expect(s->addKey(s, 1, PTPMGMT_HMAC_AES128, k1, sizeof k1, 10, true));
    cr_expect(eq(sz, s->keys(s), 2));
    cr_expect(s->have(s, 1));
    cr_expect(not(s->addKey(s, 1, PTPMGMT_HMAC_AES128, k1, sizeof k1, 12, false)));
    cr_expect(s->addKey(s, 0x57, PTPMGMT_HMAC_SHA256, k57, sizeof k57, 32, true));
    cr_expect(eq(sz, s->keys(s), 2));
    s->free(s);
}

// Test if key exist
// bool have(const_ptpmgmt_spp spp, uint32_t key)
Test(SppTest, MethodHave)
{
    ptpmgmt_safile s = ptpmgmt_safile_alloc();
    cr_assert(not(zero(ptr, s)));
    cr_assert(s->read_sa(s, "utest/sa_file.cfg"));
    cr_assert(s->have(s, 1));
    const_ptpmgmt_spp s1 = s->spp(s, 1);
    cr_expect(s1->have(s1, 1));
    s->free(s);
}

// Get key digest size (ICV size)
// size_t mac_size(const_ptpmgmt_spp spp, uint32_t key)
Test(SppTest, MethodMacSize)
{
    ptpmgmt_safile s = ptpmgmt_safile_alloc();
    cr_assert(not(zero(ptr, s)));
    cr_assert(s->read_sa(s, "utest/sa_file.cfg"));
    cr_assert(s->have(s, 1));
    const_ptpmgmt_spp s1 = s->spp(s, 1);
    cr_expect(eq(sz, s1->mac_size(s1, 1), 16));
    cr_expect(eq(sz, s1->mac_size(s1, 2), 32));
    s->free(s);
}

// Get key value
// const void *key(const_ptpmgmt_spp spp, uint32_t key)
// size_t key_size(const_ptpmgmt_spp spp, uint32_t key)
Test(SppTest, MethodKey)
{
    ptpmgmt_safile s = ptpmgmt_safile_alloc();
    cr_assert(not(zero(ptr, s)));
    cr_assert(s->read_sa(s, "utest/sa_file.cfg"));
    cr_assert(s->have(s, 2));
    const_ptpmgmt_spp s1 = s->spp(s, 2);
    cr_assert(s1->have(s1, 10));
    uint8_t k10[16] = {0xfa, 0xf4, 0x8e, 0xba, 1, 0xe7, 0xc5, 0x96, 0x6a,
            0x76, 0xcb, 0x78, 0x7a, 0xed, 0x4e, 0x7b
        };
    cr_assert(eq(sz, s1->key_size(s1, 10), sizeof k10));
    cr_expect(zero(memcmp(s1->key(s1, 10), k10, sizeof k10)));
    cr_assert(s1->have(s1, 5));
    char k5[] = "abcdefghijklmnopqrstuvwxyz";
    cr_assert(eq(sz, s1->key_size(s1, 5), sizeof k5 - 1));
    cr_expect(zero(memcmp(s1->key(s1, 5), k5, sizeof k5 - 1)));
    cr_assert(s1->have(s1, 64));
    uint8_t k64[37] = {0, 0xcb, 0xcd, 0x9a, 0x7c, 0xa6, 0x33, 0xd8, 0x7e,
            0xf8, 0xc5, 0x59, 0xa3, 0x6b, 0xcf, 0xdd, 0x19, 0x2c, 0x8d, 0xd5,
            0x68, 3, 0x72, 0x35, 0x44, 0x23, 0xff, 0x2e, 0xcf, 0x45, 0x5d,
            0xfb, 0xcb, 0x23, 0x36, 0xb6, 7
        };
    cr_assert(eq(sz, s1->key_size(s1, 64), sizeof k64));
    cr_expect(zero(memcmp(s1->key(s1, 64), k64, sizeof k64)));
    s->free(s);
}

// Get number of keys
// size_t keys(const_ptpmgmt_spp spp)
Test(SppTest, MethodKeys)
{
    ptpmgmt_safile s = ptpmgmt_safile_alloc();
    cr_assert(not(zero(ptr, s)));
    cr_assert(s->read_sa(s, "utest/sa_file.cfg"));
    cr_assert(s->have(s, 0));
    const_ptpmgmt_spp s0 = s->spp(s, 0);
    cr_expect(eq(sz, s0->keys(s0), 1));
    cr_assert(s->have(s, 1));
    const_ptpmgmt_spp s1 = s->spp(s, 1);
    cr_expect(eq(sz, s1->keys(s1), 2));
    cr_assert(s->have(s, 2));
    const_ptpmgmt_spp s2 = s->spp(s, 2);
    cr_expect(eq(sz, s2->keys(s2), 4));
    s->free(s);
}

// Get key type
// enum PTPMGMT_HMAC_t htype(const_ptpmgmt_spp spp, uint32_t key)
Test(SppTest, MethodHtype)
{
    ptpmgmt_safile s = ptpmgmt_safile_alloc();
    cr_assert(not(zero(ptr, s)));
    cr_assert(s->read_sa(s, "utest/sa_file.cfg"));
    cr_assert(s->have(s, 2));
    const_ptpmgmt_spp s2 = s->spp(s, 2);
    cr_expect(eq(int, s2->htype(s2, 10), PTPMGMT_HMAC_AES128));
    cr_expect(eq(int, s2->htype(s2, 25), PTPMGMT_HMAC_AES256));
    cr_expect(eq(int, s2->htype(s2, 5), PTPMGMT_HMAC_SHA256));
    s->free(s);
}

// Get SPP Own ID
// uint8_t ownID() const
// uint8_t ownID(const_ptpmgmt_spp spp)
Test(SppTest, MethodOwnID)
{
    ptpmgmt_safile s = ptpmgmt_safile_alloc();
    cr_assert(not(zero(ptr, s)));
    cr_assert(s->read_sa(s, "utest/sa_file.cfg"));
    cr_assert(s->have(s, 0));
    const_ptpmgmt_spp s0 = s->spp(s, 0);
    cr_expect(eq(i8, s0->ownID(s0), 0));
    cr_assert(s->have(s, 1));
    const_ptpmgmt_spp s1 = s->spp(s, 1);
    cr_expect(eq(i8, s1->ownID(s1), 1));
    cr_assert(s->have(s, 2));
    const_ptpmgmt_spp s2 = s->spp(s, 2);
    cr_expect(eq(i8, s2->ownID(s2), 2));
    s->free(s);
}
