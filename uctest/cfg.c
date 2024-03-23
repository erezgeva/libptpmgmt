/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief ConfigFile class unit tests
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
    cr_expect(not(zero(ptr, f)));
    // Check default values
    cr_expect(eq(i8, f->transportSpecific(f, NULL), 0));
    cr_expect(eq(i8, f->domainNumber(f, NULL), 0));
    cr_expect(eq(i8, f->udp6_scope(f, NULL), 0xe));
    cr_expect(eq(i8, f->udp_ttl(f, NULL), 1));
    cr_expect(eq(i8, f->socket_priority(f, NULL), 0));
    cr_expect(eq(i8, f->network_transport(f, NULL), '4'));
    cr_expect(eq(str, (char *)f->uds_address(f, NULL), "/var/run/ptp4l"));
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
    cr_expect(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    // Check file configuration
    cr_expect(eq(i8, f->transportSpecific(f, NULL), 7));
    cr_expect(eq(i8, f->domainNumber(f, NULL), 5));
    cr_expect(eq(i8, f->udp6_scope(f, NULL), 0xd));
    cr_expect(eq(i8, f->udp_ttl(f, NULL), 3));
    cr_expect(eq(i8, f->socket_priority(f, NULL), 11));
    cr_expect(eq(i8, f->network_transport(f, NULL), '6'));
    cr_expect(eq(str, (char *)f->uds_address(f, NULL), "/var/run/dummy"));
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
    cr_expect(not(zero(ptr, f)));
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
    cr_expect(not(zero(ptr, f)));
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
    cr_expect(not(zero(ptr, f)));
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
    cr_expect(not(zero(ptr, f)));
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
    cr_expect(not(zero(ptr, f)));
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
    cr_expect(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    cr_expect(eq(i8, f->network_transport(f, "dumm"), '4'));
    cr_expect(eq(i8, f->network_transport(f, "non"), '6'));
    f->free(f);
}

// Tests get Unix Domain socket address parameter
// const char *uds_address(ptpmgmt_cfg _this, const char *section)
Test(ConfigFileTest, MethodUDSAddress)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_expect(not(zero(ptr, f)));
    cr_expect(f->read_cfg(f, "utest/testing.cfg"));
    cr_expect(eq(str, (char *)f->uds_address(f, "dumm"), "/var/run/dummy2"));
    cr_expect(eq(str, (char *)f->uds_address(f, "non"), "/var/run/dummy"));
    f->free(f);
}

// Tests get PTP destination MAC address
// const void *ptp_dst_mac(ptpmgmt_cfg _this, const char *section)
Test(ConfigFileTest, MethodPTPDstMac)
{
    ptpmgmt_cfg f = ptpmgmt_cfg_alloc();
    cr_expect(not(zero(ptr, f)));
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
    cr_expect(not(zero(ptr, f)));
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
