/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief message dispatcher unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include "msgCall.h"

struct flags {
    struct ptpmgmt_PRIORITY1_t tlv;
    const char *idStr_PRIORITY1;
    const char *idStr_noTlvCallBack;
    bool noTlvCalled;
};

static void PRIORITY1_h(void *cookie, ptpmgmt_msg msg,
    const struct ptpmgmt_PRIORITY1_t *tlv, const char *idStr)
{
    struct flags *f = (struct flags *)cookie;
    f->tlv.priority1 = tlv->priority1;
    f->idStr_PRIORITY1 = idStr;
}
static void noTlv(void *cookie, ptpmgmt_msg msg)
{
    struct flags *f = (struct flags *)cookie;
    f->noTlvCalled = true;
}
static void noTlvCallBack(void *cookie, ptpmgmt_msg msg, const char *idStr)
{
    struct flags *f = (struct flags *)cookie;
    f->idStr_noTlvCallBack = idStr;
}

// Tests callHadler method with paresed empty TLV
// ptpmgmt_dispatcher ptpmgmt_dispatcher_alloc()
// bool ptpmgmt_dispatcher_assign(ptpmgmt_dispatcher d,
//     enum ptpmgmt_mng_vals_e tlv_id, ptpmgmt_dispatcher_callback callback)
// bool ptpmgmt_dispatcher_assign_noTlv(ptpmgmt_dispatcher d,
//     ptpmgmt_dispatcher_noTlv_callback callback)
// bool ptpmgmt_dispatcher_assign_noTlvCallBack(ptpmgmt_dispatcher d,
//     ptpmgmt_dispatcher_noTlvCallBack_callback callback)
// void ptpmgmt_callHadler(void *cookie, const_ptpmgmt_dispatcher d,
//     ptpmgmt_msg msg)
Test(MessageDispatcherTest, MethodParsedCallHadlerEmptyTLV)
{
    struct flags f;
    memset(&f, 0, sizeof f);
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, msg)));
    ptpmgmt_dispatcher d = ptpmgmt_dispatcher_alloc();
    cr_assert(not(zero(ptr, d)));
    cr_assert(ptpmgmt_dispatcher_assign(d, PTPMGMT_PRIORITY1,
            (ptpmgmt_dispatcher_callback)PRIORITY1_h));
    cr_assert(ptpmgmt_dispatcher_assign_noTlv(d, noTlv));
    cr_assert(ptpmgmt_dispatcher_assign_noTlvCallBack(d, noTlvCallBack));
    cr_expect(msg->setAction(msg, PTPMGMT_COMMAND, PTPMGMT_ENABLE_PORT, NULL));
    cr_expect(eq(int, msg->getBuildTlvId(msg), PTPMGMT_ENABLE_PORT));
    uint8_t buf[70];
    cr_expect(eq(int, msg->build(msg, buf, sizeof buf, 1),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of command message
    buf[46] = PTPMGMT_ACKNOWLEDGE;
    cr_expect(eq(int, msg->parse(msg, buf, 54), PTPMGMT_MNG_PARSE_ERROR_OK));
    f.noTlvCalled = false;
    // Dispatch ENABLE_PORT
    ptpmgmt_callHadler(&f, d, msg);
    // ENABLE_PORT is an empty code, which do not have a TLV, so noTlv is called!
    cr_expect(f.noTlvCalled);
    d->free(d);
    msg->free(msg);
}

// Tests callHadler with method with provided empty TLV
// void ptpmgmt_callHadler_tlv(void *cookie, const_ptpmgmt_dispatcher d,
//     ptpmgmt_msg msg, enum ptpmgmt_mng_vals_e tlv_id, const void *tlv)
Test(MessageDispatcherTest, MethodProvidedCallHadlerEmptyTLV)
{
    struct flags f;
    memset(&f, 0, sizeof f);
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, msg)));
    ptpmgmt_dispatcher d = ptpmgmt_dispatcher_alloc();
    cr_assert(not(zero(ptr, d)));
    cr_assert(ptpmgmt_dispatcher_assign(d, PTPMGMT_PRIORITY1,
            (ptpmgmt_dispatcher_callback)PRIORITY1_h));
    cr_assert(ptpmgmt_dispatcher_assign_noTlv(d, noTlv));
    cr_assert(ptpmgmt_dispatcher_assign_noTlvCallBack(d, noTlvCallBack));
    f.noTlvCalled = false;
    // Dispatch ENABLE_PORT
    ptpmgmt_callHadler_tlv(&f, d, msg, PTPMGMT_ENABLE_PORT, NULL);
    // ENABLE_PORT is an empty code, which do not have a TLV, so noTlv is called!
    cr_expect(f.noTlvCalled);
    char dummy[2];
    f.noTlvCalled = false;
    // Dispatch ENABLE_PORT
    ptpmgmt_callHadler_tlv(&f, d, msg, PTPMGMT_ENABLE_PORT, dummy);
    cr_expect(f.noTlvCalled);
    d->free(d);
    msg->free(msg);
}

// Tests callHadler method with paresed TLV
Test(MessageDispatcherTest, MethodParsedCallHadlerTLV)
{
    struct flags f;
    memset(&f, 0, sizeof f);
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, msg)));
    ptpmgmt_dispatcher d = ptpmgmt_dispatcher_alloc();
    cr_assert(not(zero(ptr, d)));
    cr_assert(ptpmgmt_dispatcher_assign(d, PTPMGMT_PRIORITY1,
            (ptpmgmt_dispatcher_callback)PRIORITY1_h));
    cr_assert(ptpmgmt_dispatcher_assign_noTlv(d, noTlv));
    cr_assert(ptpmgmt_dispatcher_assign_noTlvCallBack(d, noTlvCallBack));
    struct ptpmgmt_PRIORITY1_t p;
    p.priority1 = 137;
    f.noTlvCalled = false;
    cr_expect(msg->setAction(msg, PTPMGMT_SET, PTPMGMT_PRIORITY1, &p));
    cr_expect(eq(int, msg->getBuildTlvId(msg), PTPMGMT_PRIORITY1));
    uint8_t buf[70];
    cr_expect(eq(int, msg->build(msg, buf, sizeof buf, 137),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    cr_expect(eq(int, msg->parse(msg, buf, 56), PTPMGMT_MNG_PARSE_ERROR_OK));
    // Dispatch PRIORITY1 in parsed message
    ptpmgmt_callHadler(&f, d, msg);
    // PRIORITY1 have callback
    cr_expect(not(f.noTlvCalled));
    cr_expect(eq(u8, f.tlv.priority1, p.priority1));
    cr_expect(eq(str, (char *)f.idStr_PRIORITY1, "PRIORITY1"));
    d->free(d);
    msg->free(msg);
}

// Tests callHadler with method with provided TLV
Test(MessageDispatcherTest, MethodProvidedCallHadlerTLV)
{
    struct flags f;
    memset(&f, 0, sizeof f);
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, msg)));
    ptpmgmt_dispatcher d = ptpmgmt_dispatcher_alloc();
    cr_assert(not(zero(ptr, d)));
    cr_assert(ptpmgmt_dispatcher_assign(d, PTPMGMT_PRIORITY1,
            (ptpmgmt_dispatcher_callback)PRIORITY1_h));
    cr_assert(ptpmgmt_dispatcher_assign_noTlv(d, noTlv));
    cr_assert(ptpmgmt_dispatcher_assign_noTlvCallBack(d, noTlvCallBack));
    struct ptpmgmt_PRIORITY1_t p;
    p.priority1 = 137;
    f.noTlvCalled = false;
    // Dispatch PRIORITY1
    ptpmgmt_callHadler_tlv(&f, d, msg, PTPMGMT_PRIORITY1, &p);
    // PRIORITY1 have callback
    cr_expect(not(f.noTlvCalled));
    cr_expect(eq(u8, f.tlv.priority1, p.priority1));
    cr_expect(eq(str, (char *)f.idStr_PRIORITY1, "PRIORITY1"));
    d->free(d);
    msg->free(msg);
}

// Tests callHadler method with paresed TLV without callback
Test(MessageDispatcherTest, MethodParsedCallHadlerTLVNoCallback)
{
    struct flags f;
    memset(&f, 0, sizeof f);
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, msg)));
    ptpmgmt_dispatcher d = ptpmgmt_dispatcher_alloc();
    cr_assert(not(zero(ptr, d)));
    cr_assert(ptpmgmt_dispatcher_assign(d, PTPMGMT_PRIORITY1,
            (ptpmgmt_dispatcher_callback)PRIORITY1_h));
    cr_assert(ptpmgmt_dispatcher_assign_noTlv(d, noTlv));
    cr_assert(ptpmgmt_dispatcher_assign_noTlvCallBack(d, noTlvCallBack));
    struct ptpmgmt_PRIORITY2_t p;
    p.priority2 = 137;
    f.noTlvCalled = false;
    cr_expect(msg->setAction(msg, PTPMGMT_SET, PTPMGMT_PRIORITY2, &p));
    cr_expect(eq(int, msg->getBuildTlvId(msg), PTPMGMT_PRIORITY2));
    uint8_t buf[70];
    cr_expect(eq(int, msg->build(msg, buf, sizeof buf, 137),
            PTPMGMT_MNG_PARSE_ERROR_OK));
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = PTPMGMT_RESPONSE;
    cr_expect(eq(int, msg->parse(msg, buf, 56), PTPMGMT_MNG_PARSE_ERROR_OK));
    // Dispatch PRIORITY2 in parsed message
    ptpmgmt_callHadler(&f, d, msg);
    // PRIORITY2 do not have callback
    cr_expect(not(f.noTlvCalled));
    cr_expect(eq(u8, f.tlv.priority1, 0));
    cr_expect(zero(ptr, (char *)f.idStr_PRIORITY1));
    cr_expect(eq(str, (char *)f.idStr_noTlvCallBack, "PRIORITY2"));
    d->free(d);
    msg->free(msg);
}

// Tests callHadler with method with provided TLV without callback
Test(MessageDispatcherTest, MethodProvidedCallHadlerTLVNoCallback)
{
    struct flags f;
    memset(&f, 0, sizeof f);
    ptpmgmt_msg msg = ptpmgmt_msg_alloc();
    cr_assert(not(zero(ptr, msg)));
    ptpmgmt_dispatcher d = ptpmgmt_dispatcher_alloc();
    cr_assert(not(zero(ptr, d)));
    cr_assert(ptpmgmt_dispatcher_assign(d, PTPMGMT_PRIORITY1,
            (ptpmgmt_dispatcher_callback)PRIORITY1_h));
    cr_assert(ptpmgmt_dispatcher_assign_noTlv(d, noTlv));
    cr_assert(ptpmgmt_dispatcher_assign_noTlvCallBack(d, noTlvCallBack));
    struct ptpmgmt_PRIORITY2_t p;
    p.priority2 = 137;
    f.noTlvCalled = false;
    // Dispatch PRIORITY2
    ptpmgmt_callHadler_tlv(&f, d, msg, PTPMGMT_PRIORITY2, &p);
    // PRIORITY2 do not have callback
    cr_expect(not(f.noTlvCalled));
    cr_expect(eq(u8, f.tlv.priority1, 0));
    cr_expect(zero(ptr, (char *)f.idStr_PRIORITY1));
    cr_expect(eq(str, (char *)f.idStr_noTlvCallBack, "PRIORITY2"));
    d->free(d);
    msg->free(msg);
}
