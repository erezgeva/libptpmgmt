/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Messages dispatcher and builder classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
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
Message &MessageBuilder::getMsg()
{
    return m_msg;
}
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

extern "C" {
    // C interfaces
#define _ptpmCaseUF(n) void (*n##_h)(void *cookie, ptpmgmt_msg msg,\
    const struct ptpmgmt_##n##_t *tlv, const char *idStr);
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
    // The three first function are identical to struct ptpmgmt_dispatcher_t
    struct ptpmgmt_dispatcher_full_t {
        void (*free)(ptpmgmt_dispatcher dsp);
        ptpmgmt_dispatcher_noTlv_callback noTlv;
        ptpmgmt_dispatcher_noTlvCallBack_callback noTlvCallBack;
#include "ids.h"
    };
    void ptpmgmt_dispatcher_free(ptpmgmt_dispatcher me)
    {
        free(me);
    }
    ptpmgmt_dispatcher ptpmgmt_dispatcher_alloc()
    {
        static const size_t sz = sizeof(ptpmgmt_dispatcher_full_t);
        ptpmgmt_dispatcher me = (ptpmgmt_dispatcher)malloc(sz);
        if(me == nullptr)
            return nullptr;
        memset(me, 0, sz);
        me->free = ptpmgmt_dispatcher_free;
        return me;
    }
#define _ptpmCaseUF(n)\
case PTPMGMT_##n:\
    d->n##_h = (void (*)(void*, ptpmgmt_msg,\
                const ptpmgmt_##n##_t*, const char*))callback;\
    return true;
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
    bool ptpmgmt_dispatcher_assign(ptpmgmt_dispatcher d_,
        enum ptpmgmt_mng_vals_e tlv_id, ptpmgmt_dispatcher_callback callback)
    {
        ptpmgmt_dispatcher_full_t *d = (ptpmgmt_dispatcher_full_t *)d_;
        if(d == nullptr || callback == (ptpmgmt_dispatcher_callback)nullptr)
            return false;
        switch(tlv_id) {
#include "ids.h"
            default:
                return false;
        }
    }
    bool ptpmgmt_dispatcher_assign_noTlv(ptpmgmt_dispatcher d,
        ptpmgmt_dispatcher_noTlv_callback callback)
    {
        if(d == nullptr || callback == nullptr)
            return false;
        d->noTlv = callback;
        return true;
    }
    bool ptpmgmt_dispatcher_assign_noTlvCallBack(ptpmgmt_dispatcher d,
        ptpmgmt_dispatcher_noTlvCallBack_callback callback)
    {
        if(d == nullptr || callback == nullptr)
            return false;
        d->noTlvCallBack = callback;
        return true;
    }
#define _ptpmCaseUF(n)\
case PTPMGMT_##n:if(d->n##_h != nullptr){\
        d->n##_h(cookie, msg, (const ptpmgmt_##n##_t*)tlv, #n);}else{\
        if(d->noTlvCallBack != nullptr){d->noTlvCallBack(cookie, msg, #n);}}break;
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
    void ptpmgmt_callHadler_tlv(void *cookie, const_ptpmgmt_dispatcher d_,
        ptpmgmt_msg msg, ptpmgmt_mng_vals_e tlv_id, const void *tlv)
    {
        ptpmgmt_dispatcher_full_t *d = (ptpmgmt_dispatcher_full_t *)d_;
        if(d == nullptr)
            return;
        if(tlv == nullptr ||
            (msg != nullptr && !msg->isValidId(msg, tlv_id)) ||
            ptpmgmt_msg_isEmpty((tlv_id))) {
            if(d->noTlv != nullptr)
                d->noTlv(cookie, msg);
            return;
        }
        switch(tlv_id) {
#include "ids.h"
            default:
                if(d->noTlv != nullptr)
                    d->noTlv(cookie, msg);
                break;
        }
    }
    void ptpmgmt_callHadler(void *cookie, const_ptpmgmt_dispatcher d,
        ptpmgmt_msg msg)
    {
        if(d == nullptr || msg == nullptr)
            return;
        ptpmgmt_callHadler_tlv(cookie, d, msg, msg->getTlvId(msg),
            msg->getData(msg));
    }
}
