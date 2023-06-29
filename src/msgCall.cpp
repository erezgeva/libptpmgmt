/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Messages dispatcher and builder classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 */

#include "msgCall.h"
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
    _noTlv(); /* Clear the flag */
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
    if(actionField != SET && actionField != COMMAND)
        return false;
    BaseMngTlv *tlv = nullptr;
#define _ptpmCaseUFB(n) case n: {\
            n##_t *d = new n##_t;\
            if (d == nullptr) return false;\
            if (n##_b(m_msg, *d)) tlv = d; else delete d;\
            break; }
    switch(tlv_id) {
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
#include "ids.h"
        default:
            return false;
    }
    if(tlv == nullptr)
        return false;
    m_tlv.reset(tlv);
    return m_msg.setAction(actionField, tlv_id, tlv);
}

__PTPMGMT_NAMESPACE_END

extern "C" {

#include "c/msgCall.h"

    // C interfaces
#define _ptpmCaseUF(n)\
case PTPMGMT_##n:if(d->n##_h != nullptr){\
        d->n##_h(cookie, msg, (const ptpmgmt_##n##_t*)tlv, #n);}else{\
        if(d->noTlvCallBack != nullptr){d->noTlvCallBack(cookie, msg, #n);}}break;
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
    void ptpmgmt_callHadler_tlv(void *cookie, const_ptpmgmt_dispatcher d,
        ptpmgmt_msg msg, enum ptpmgmt_mng_vals_e tlv_id, const void *tlv)
    {
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
        ptpmgmt_callHadler_tlv(cookie, d, msg, msg->getTlvId(msg),
            msg->getData(msg));
    }
}
