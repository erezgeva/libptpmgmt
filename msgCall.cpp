/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief messages dispatcher and builder classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2022 Erez Geva
 */

#include "msgCall.h"

namespace ptpmgmt
{

MessageDispatcher::MessageDispatcher() : m_msg(nullptr), m_tlv(nullptr)
{
}
MessageDispatcher::MessageDispatcher(const Message &msg, bool _callHadler)
{
    callHadler(msg, _callHadler);
}
void MessageDispatcher::callHadler(const Message &msg, bool _callHadler)
{
    callHadler(msg, msg.getTlvId(), msg.getData(), _callHadler);
}
void MessageDispatcher::callHadler(const Message &msg, mng_vals_e tlv_id,
    const BaseMngTlv *tlv, bool _callHadler)
{
    m_tlv_id = tlv_id;
    m_tlv = tlv;
    m_msg = &msg;
    if(_callHadler)
        callHadler();
}
void MessageDispatcher::callHadler() const
{
    if(m_msg == nullptr)
        return;
    if(m_tlv == nullptr) {
        noTlv(*m_msg);
        return;
    }
#define _ptpmCaseUF(n) case n:\
        n##_h(*m_msg, *dynamic_cast<const n##_t*>(m_tlv));\
        break;
    switch(m_tlv_id) {
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
#include "ids.h"
        default:
            noTlv(*m_msg);
            break;
    }
}
bool MessageBulder::buildTlv(actionField_e actionField, mng_vals_e tlv_id)
{
    if(actionField == GET || m_msg.isEmpty(tlv_id))
        return m_msg.setAction(actionField, tlv_id);
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

}; /* namespace ptpmgmt */

// g++ -E -Wdate-time -Wall -std=c++11 -g -Og  msgCall.cpp | gvim -
