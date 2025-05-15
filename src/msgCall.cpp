/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Messages dispatcher and builder classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

#include "msgCall.h"
#include "c/msgCall.h"
#include "comp.h"

__PTPMGMT_NAMESPACE_BEGIN

void MessageDispatcher::callHadler(const Message &msg)
{
    callHadler(msg, msg.getTlvId(), msg.getData());
}
void MessageDispatcher::callHadler(const Message &msg, mng_vals_e tlv_id,
    const BaseMngTlv *tlv)
{
    if(tlv == nullptr || !msg.isValidId(tlv_id) || Message::isEmpty(tlv_id)) {
        cNoTlv(msg);
        return;
    }
    const char *idStr = iCallHandler(msg, tlv_id, tlv);
    if(idStr == nullptr) {
        cNoTlv(msg);
        return;
    }
    if(isNoTlv())
        cNoTlvCB(msg, idStr);
}
MessageBuilder::MessageBuilder(Message &msg) : m_msg(msg) {}
Message &MessageBuilder::getMsg() { return m_msg; }
void MessageBuilder::clear()
{
    m_msg.clearData();
    m_tlv.reset();
}
bool MessageBuilder::buildTlv(actionField_e actionField, mng_vals_e tlv_id)
{
    if(!m_msg.isValidId(tlv_id))
        return false;
    if(actionField == GET || Message::isEmpty(tlv_id))
        return m_msg.setAction(actionField, tlv_id);
    switch(actionField) {
        case SET:
            FALLTHROUGH;
        case COMMAND:
            break;
        default:
            return false;
    }
    BaseMngTlv *tlv = iBuild(m_msg, tlv_id);
    if(tlv == nullptr)
        return false;
    if LIKELY_COND(m_msg.setAction(actionField, tlv_id, tlv)) {
        m_tlv.reset(tlv);
        return true;
    }
    // Should not happen
    delete tlv;
    return false;
}

__PTPMGMT_NAMESPACE_END

__PTPMGMT_C_BEGIN

// C interfaces
#define _ptpmCaseUF(n) void (*n##_h)(void *cookie, ptpmgmt_msg msg,\
    const ptpmgmt_##n##_t *tlv, const char *idStr);
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
struct ptpmgmt_dispatcher_cb_t {
#include "ids.h"
};
static void ptpmgmt_dispatcher_free(ptpmgmt_dispatcher d)
{
    if(d != nullptr) {
        free(d->cbs);
        free(d);
    }
}
#define _ptpmCaseUF(n)\
    case PTPMGMT_##n:\
    df->n##_h = (void (*)(void*, ptpmgmt_msg,\
                const ptpmgmt_##n##_t*, const char*))callback;\
    return true;
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
static bool ptpmgmt_dispatcher_assign(ptpmgmt_dispatcher d,
    ptpmgmt_mng_vals_e tlv_id, ptpmgmt_dispatcher_callback callback)
{
    if(d == nullptr || callback == nullptr)
        return false;
    ptpmgmt_dispatcher_cb_t *df = d->cbs;
    switch(tlv_id) {
#include "ids.h"
        default:
            return false;
    }
}
static bool ptpmgmt_dispatcher_assign_noTlv(ptpmgmt_dispatcher d,
    ptpmgmt_dispatcher_noTlv_callback callback)
{
    if(d == nullptr || callback == nullptr)
        return false;
    d->noTlv = callback;
    return true;
}
static bool ptpmgmt_dispatcher_assign_noTlvCallBack(ptpmgmt_dispatcher d,
    ptpmgmt_dispatcher_noTlvCallBack_callback callback)
{
    if(d == nullptr || callback == nullptr)
        return false;
    d->noTlvCallBack = callback;
    return true;
}
#define _ptpmCaseUF(n)\
    case PTPMGMT_##n:\
    if(df->n##_h != nullptr) \
        df->n##_h(cookie, msg, (const ptpmgmt_##n##_t*)tlv, #n);\
    else if(d->noTlvCallBack != nullptr)\
        d->noTlvCallBack(cookie, msg, #n);\
    break;
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
static void ptpmgmt_dispatcher_callHadler_tlv(const_ptpmgmt_dispatcher d,
    void *cookie, ptpmgmt_msg msg, ptpmgmt_mng_vals_e tlv_id, const void *tlv)
{
    if(d == nullptr)
        return;
    if(tlv == nullptr || (msg != nullptr && !msg->isValidId(msg, tlv_id)) ||
        ptpmgmt_msg_isEmpty(tlv_id)) {
        if(d->noTlv != nullptr)
            d->noTlv(cookie, msg);
        return;
    }
    ptpmgmt_dispatcher_cb_t *df = d->cbs;
    switch(tlv_id) {
#include "ids.h"
        default:
            if(d->noTlv != nullptr)
                d->noTlv(cookie, msg);
            break;
    }
}
static void ptpmgmt_dispatcher_callHadler(const_ptpmgmt_dispatcher d,
    void *cookie, ptpmgmt_msg msg)
{
    if(d == nullptr || msg == nullptr)
        return;
    ptpmgmt_dispatcher_callHadler_tlv(d, cookie, msg, msg->getTlvId(msg),
        msg->getData(msg));
}
ptpmgmt_dispatcher ptpmgmt_dispatcher_alloc()
{
    static const size_t sz = sizeof(ptpmgmt_dispatcher_t);
    static const size_t sz2 = sizeof(ptpmgmt_dispatcher_cb_t);;
    ptpmgmt_dispatcher me = (ptpmgmt_dispatcher)malloc(sz);
    if(me == nullptr)
        return nullptr;
    ptpmgmt_dispatcher_cb_t *cbs = (ptpmgmt_dispatcher_cb_t *)malloc(sz2);
    if(cbs == nullptr) {
        free(me);
        return nullptr;
    }
    memset(me, 0, sz);
    memset(cbs, 0, sz2);
    me->cbs = cbs;
#define C_ASGN(n) me->n = ptpmgmt_dispatcher_##n
    C_ASGN(free);
    C_ASGN(assign);
    C_ASGN(assign_noTlv);
    C_ASGN(assign_noTlvCallBack);
    C_ASGN(callHadler);
    C_ASGN(callHadler_tlv);
    return me;
}
#define _ptpmCaseUFB(n) bool (*n##_b)(void *cookie, ptpmgmt_msg msg,\
    ptpmgmt_##n##_t *tlv, ptpmgmt_tlv_mem tlv_mem);
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
struct ptpmgmt_builder_cb_t {
#include "ids.h"
};
static void ptpmgmt_builder_free(ptpmgmt_builder b)
{
    if(b != nullptr) {
        if(b->tlv_mem != nullptr)
            b->tlv_mem->free(b->tlv_mem);
        free(b->cbs);
        free(b);
    }
}
static ptpmgmt_msg ptpmgmt_builder_getMsg(const_ptpmgmt_builder b)
{
    if(b != nullptr)
        return b->msg;
    return nullptr;
}
static ptpmgmt_tlv_mem ptpmgmt_builder_getTlvMem(const_ptpmgmt_builder b)
{
    if(b != nullptr)
        return b->tlv_mem;
    return nullptr;
}
static void ptpmgmt_builder_clear(ptpmgmt_builder b)
{
    if(b != nullptr) {
        if(b->msg != nullptr)
            b->msg->clearData(b->msg);
        if(b->tlv_mem != nullptr)
            b->tlv_mem->clear(b->tlv_mem);
    }
}
#define _ptpmCaseUFB(n)\
    case PTPMGMT_##n:\
    bf->n##_b = (bool (*)(void*, ptpmgmt_msg, ptpmgmt_##n##_t*,\
                ptpmgmt_tlv_mem))callback;\
    return true;
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
static bool ptpmgmt_builder_assign(ptpmgmt_builder b, ptpmgmt_mng_vals_e tlv_id,
    ptpmgmt_builder_callback callback)
{
    if(b == nullptr || callback == nullptr)
        return false;
    ptpmgmt_builder_cb_t *bf = b->cbs;
    switch(tlv_id) {
#include "ids.h"
        default:
            return false;
    }
}
#define _ptpmCaseUFB(n)\
    case PTPMGMT_##n:\
    if(bf->n##_b != nullptr)\
        ret = bf->n##_b(cookie, b->msg, (ptpmgmt_##n##_t*)tlv, b->tlv_mem);\
    break;
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
static bool ptpmgmt_builder_buildTlv(ptpmgmt_builder b, void *cookie,
    ptpmgmt_actionField_e actionField, ptpmgmt_mng_vals_e tlv_id)
{
    if(b == nullptr || b->msg == nullptr || !b->msg->isValidId(b->msg, tlv_id))
        return false;
    if(actionField == PTPMGMT_GET || ptpmgmt_msg_isEmpty(tlv_id))
        return b->msg->setAction(b->msg, actionField, tlv_id, nullptr);
    switch(actionField) {
        case PTPMGMT_SET:
            FALLTHROUGH;
        case PTPMGMT_COMMAND:
            break;
        default:
            return false;
    }
    if(b->tlv_mem == nullptr || !b->tlv_mem->newTlv(b->tlv_mem, tlv_id))
        return false;
    void *tlv = b->tlv_mem->getTLV(b->tlv_mem);
    if UNLIKELY_COND(tlv == nullptr)
        return false;
    bool ret = false;
    ptpmgmt_builder_cb_t *bf = b->cbs;
    switch(tlv_id) {
#include "ids.h"
        default:
            break;
    }
    return ret && b->msg->setAction(b->msg, actionField, tlv_id, tlv);
}
ptpmgmt_builder ptpmgmt_builder_alloc(ptpmgmt_msg msg)
{
    if(msg == nullptr)
        return nullptr;
    static const size_t sz = sizeof(ptpmgmt_builder_t);
    static const size_t sz2 = sizeof(ptpmgmt_builder_cb_t);
    ptpmgmt_builder me = (ptpmgmt_builder)malloc(sz);
    if(me == nullptr)
        return nullptr;
    ptpmgmt_builder_cb_t *cbs = (ptpmgmt_builder_cb_t *)malloc(sz2);
    if(cbs == nullptr) {
        free(me);
        return nullptr;
    }
    ptpmgmt_tlv_mem tlv = ptpmgmt_tlv_mem_alloc();
    if(tlv == nullptr) {
        free(me);
        free(cbs);
        return nullptr;
    }
    memset(me, 0, sz);
    memset(cbs, 0, sz2);
    me->msg = msg;
    me->tlv_mem = tlv;
    me->cbs = cbs;
#define B_ASGN(n) me->n = ptpmgmt_builder_##n
    B_ASGN(free);
    B_ASGN(getMsg);
    B_ASGN(getTlvMem);
    B_ASGN(clear);
    B_ASGN(assign);
    B_ASGN(buildTlv);
    return me;
}

__PTPMGMT_C_END
