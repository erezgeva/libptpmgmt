/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief init class unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include "init.h"

TestSuite(InitTest, .init = initLibSys);

char *argv_u[] = {"me", "-u", "-s", "/peer"};
const size_t argc_u = 4;
char *argv_4[] = {"me", "-4", "-i", "eth0", "--udp_ttl", "7"};
const size_t argc_4 = 6;
char *argv_6[] = {"me", "-6", "-i", "eth0", "--udp_ttl", "7",
        "--udp6_scope", "15"
    };
const size_t argc_6 = 8;
char *argv_2[] = {"me", "-2", "-i", "eth0", "--socket_priority", "7",
        "--ptp_dst_mac", "1:1b:17:f:c:0"
    };
const size_t argc_2 = 8;

// Tests proccess with unix socket
// int proccess(ptpmgmt_init i, ptpmgmt_opt o)
// ptpmgmt_cfg cfg(ptpmgmt_init i)
// bool use_uds(ptpmgmt_init i)
// ptpmgmt_msg msg(ptpmgmt_init i)
// ptpmgmt_sk sk(ptpmgmt_init i)
// char getNetSelect(ptpmgmt_init i)
// void close(ptpmgmt_init i)
Test(InitTest, MethodProccessUnix)
{
    ptpmgmt_opt o = ptpmgmt_opt_alloc();
    cr_expect(eq(i32, o->parse_options(o, argc_u, argv_u), PTPMGMT_OPT_DONE));
    ptpmgmt_init i = ptpmgmt_init_alloc();
    useTestMode(true);
    int r1 = i->proccess(i, o);
    bool r2 = i->use_uds(i);
    ptpmgmt_cfg c = i->cfg(i);
    uint8_t r3 = c->udp6_scope(c, NULL);
    ptpmgmt_msg msg = i->msg(i);
    enum ptpmgmt_mng_vals_e r4 = msg->getTlvId(msg);
    ptpmgmt_sk s = i->sk(i);
    char r5 = i->getNetSelect(i);
    useTestMode(false);
    cr_expect(zero(i32, r1));
    cr_expect(r2);
    cr_expect(not(zero(ptr, c)));
    cr_expect(eq(i8, r3, 14));
    cr_expect(eq(i32, r4, PTPMGMT_NULL_PTP_MANAGEMENT));
    cr_expect(not(zero(ptr, s)));
    cr_expect(eq(chr, r5, 'u'));
    i->close(i);
    i->free(i);
    o->free(o);
}

// Tests proccess with ipv4 socket
Test(InitTest, MethodProccessIPv4)
{
    ptpmgmt_opt o = ptpmgmt_opt_alloc();
    cr_expect(eq(i32, o->parse_options(o, argc_4, argv_4), PTPMGMT_OPT_DONE));
    ptpmgmt_init i = ptpmgmt_init_alloc();
    useTestMode(true);
    int r1 = i->proccess(i, o);
    bool r2 = i->use_uds(i);
    ptpmgmt_cfg c = i->cfg(i);
    uint8_t r3 = c->udp6_scope(c, NULL);
    ptpmgmt_msg msg = i->msg(i);
    enum ptpmgmt_mng_vals_e r4 = msg->getTlvId(msg);
    ptpmgmt_sk s = i->sk(i);
    char r5 = i->getNetSelect(i);
    useTestMode(false);
    cr_expect(zero(i32, r1));
    cr_expect(not(r2));
    cr_expect(not(zero(ptr, c)));
    cr_expect(eq(i8, r3, 14));
    cr_expect(eq(i32, r4, PTPMGMT_NULL_PTP_MANAGEMENT));
    cr_expect(not(zero(ptr, s)));
    cr_expect(eq(chr, r5, '4'));
    i->close(i);
    i->free(i);
    o->free(o);
}

// Tests proccess with ipv6 socket
Test(InitTest, MethodProccessIPv6)
{
    ptpmgmt_opt o = ptpmgmt_opt_alloc();
    cr_expect(eq(i32, o->parse_options(o, argc_6, argv_6), PTPMGMT_OPT_DONE));
    ptpmgmt_init i = ptpmgmt_init_alloc();
    useTestMode(true);
    int r1 = i->proccess(i, o);
    bool r2 = i->use_uds(i);
    ptpmgmt_cfg c = i->cfg(i);
    uint8_t r3 = c->udp6_scope(c, NULL);
    ptpmgmt_msg msg = i->msg(i);
    enum ptpmgmt_mng_vals_e r4 = msg->getTlvId(msg);
    ptpmgmt_sk s = i->sk(i);
    char r5 = i->getNetSelect(i);
    useTestMode(false);
    cr_expect(zero(i32, r1));
    cr_expect(not(r2));
    cr_expect(not(zero(ptr, c)));
    cr_expect(eq(i8, r3, 14));
    cr_expect(eq(i32, r4, PTPMGMT_NULL_PTP_MANAGEMENT));
    cr_expect(not(zero(ptr, s)));
    cr_expect(eq(chr, r5, '6'));
    i->close(i);
    i->free(i);
    o->free(o);
}

// Tests proccess with raw socket
Test(InitTest, MethodProccessRaw)
{
    ptpmgmt_opt o = ptpmgmt_opt_alloc();
    cr_expect(eq(i32, o->parse_options(o, argc_2, argv_2), PTPMGMT_OPT_DONE));
    ptpmgmt_init i = ptpmgmt_init_alloc();
    useTestMode(true);
    int r1 = i->proccess(i, o);
    bool r2 = i->use_uds(i);
    ptpmgmt_cfg c = i->cfg(i);
    uint8_t r3 = c->udp6_scope(c, NULL);
    ptpmgmt_msg msg = i->msg(i);
    enum ptpmgmt_mng_vals_e r4 = msg->getTlvId(msg);
    ptpmgmt_sk s = i->sk(i);
    char r5 = i->getNetSelect(i);
    useTestMode(false);
    cr_expect(zero(i32, r1));
    cr_expect(not(r2));
    cr_expect(not(zero(ptr, c)));
    cr_expect(eq(i8, r3, 14));
    cr_expect(eq(i32, r4, PTPMGMT_NULL_PTP_MANAGEMENT));
    cr_expect(not(zero(ptr, s)));
    cr_expect(eq(chr, r5, '2'));
    i->close(i);
    i->free(i);
    o->free(o);
}
