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
#define _ptpmCaseUF(n)\
    case n: n##_h(msg, *dynamic_cast<const n##_t*>(tlv), #n);\
    if(_noTlv()){noTlvCallBack(msg, #n);}break;
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
void MessageDispatcher::callHadler(const Message &msg, mng_vals_e tlv_id,
    const BaseMngTlv *tlv)
{
    if(tlv == nullptr) {
        noTlv(msg);
        return;
    }
    _noTlvClear(); /* Clear the flag */
    switch(tlv_id) {
#include "ids.h"
        default:
            noTlv(msg);
            break;
    }
}
bool MessageBuilder::buildTlv(actionField_e actionField, mng_vals_e tlv_id)
{
    if(!m_msg.isValidId(tlv_id))
        return false;
    if(actionField == GET || m_msg.isEmpty(tlv_id))
        return m_msg.setAction(actionField, tlv_id);
    switch(actionField) {
        case SET:
            FALLTHROUGH;
        case COMMAND:
            break;
        default:
            return false;
    }
    BaseMngTlv *tlv = nullptr;
#define _ptpmCaseUFB(n) case n: {\
            n##_t *d = new n##_t;\
            if (d == nullptr) return false;\
            if (!n##_b(m_msg, *d)) { delete d; return false; }\
            tlv = d; } break;
    switch(tlv_id) {
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
#include "ids.h"
        default:
            return false;
    }
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
    static void ptpmgmt_dispatcher_free(ptpmgmt_dispatcher me)
    {
        free(me);
    }
    ptpmgmt_dispatcher ptpmgmt_dispatcher_alloc()
    {
        static const size_t sz = sizeof(ptpmgmt_dispatcher_t);
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
    bool ptpmgmt_dispatcher_assign(ptpmgmt_dispatcher d,
        enum ptpmgmt_mng_vals_e tlv_id, ptpmgmt_dispatcher_callback callback)
    {
        if(d == nullptr || callback == (ptpmgmt_dispatcher_callback)nullptr)
            return false;
        switch(tlv_id) {
#include "ids.h"
            default:
                break;
        }
        return false;
    }
    bool ptpmgmt_dispatcher_assign_noTlv(ptpmgmt_dispatcher d,
        ptpmgmt_dispatcher_noTlv_callback callback)
    {
        if(d == nullptr || callback == (ptpmgmt_dispatcher_noTlv_callback)nullptr)
            return false;
        d->noTlv = callback;
        return true;
    }
    bool ptpmgmt_dispatcher_assign_noTlvCallBack(ptpmgmt_dispatcher d,
        ptpmgmt_dispatcher_noTlvCallBack_callback callback)
    {
        if(d == nullptr ||
            callback == (ptpmgmt_dispatcher_noTlvCallBack_callback)nullptr)
            return false;
        d->noTlvCallBack = callback;
        return true;
    }
#define _ptpmCaseUF(n)\
case PTPMGMT_##n:if(d->n##_h != nullptr){\
        d->n##_h(cookie, msg, (const ptpmgmt_##n##_t*)tlv, #n);}else{\
        if(d->noTlvCallBack != nullptr){d->noTlvCallBack(cookie, msg, #n);}}break;
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
    void ptpmgmt_callHadler_tlv(void *cookie, const_ptpmgmt_dispatcher d,
        ptpmgmt_msg msg, ptpmgmt_mng_vals_e tlv_id, const void *tlv)
    {
        if(d == nullptr)
            return;
        if(tlv == nullptr) {
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
