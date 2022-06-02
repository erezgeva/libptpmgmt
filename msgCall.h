/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief messages dispatcher and builder classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2022 Erez Geva
 */

#ifndef __PTPMGMT_MSG_CALL_H
#define __PTPMGMT_MSG_CALL_H

#include <string>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <memory>
#include "msg.h"

#ifndef SWIG
namespace ptpmgmt
{
#endif

#ifndef SWIG
/** @cond internal
 * Doxygen do not know how to proccess.
 * proccess these classes in mngIds.h
 */
class BaseMngDispatchCallback
{
  protected:
    virtual ~BaseMngDispatchCallback() = default;
#define _ptpmCaseUF(n)\
    virtual void n##_h(const Message &msg, const n##_t &tlv,\
        const char*idStr) const {}
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
    /* Per tlv ID a virtual call-back for user */
#include "ids.h"
};
class BaseMngBuildCallback
{
  protected:
    virtual ~BaseMngBuildCallback() = default;
#define _ptpmCaseUFB(n)\
    virtual bool n##_b(const Message &msg, n##_t &tlv) {return false;}
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
    /* Per tlv ID a virtual call-back for user */
#include "ids.h"
};
#endif /* SWIG */
/**< @endcond */

#ifndef SWIG_MessageDispatcher
/**
 * @brief dispatch received PTP management message TLV
 * @note Do not handle signaling messages!
 * @note You must inherit this class to use it!
 * @note Class implemented in Python, Perl, Lua, PHP and Ruby.
 *       See testing for examples.
 *       Currently TCL is excluded.
 * @note callHadler() has an implementation per script language.
 *       So, it can call virtual function define in the script language itself.
 */
class MessageDispatcher : public BaseMngDispatchCallback
{
  public:
    #ifdef SWIG_MessageDispatcher1
    SWIG_MessageDispatcher1
    #endif
    /**
     * Call handler based on Message last received message
     * @param[in] msg Message object
     */
    void callHadler(const Message &msg);
    #ifdef SWIG_MessageDispatcher2
    SWIG_MessageDispatcher2
    #endif
    /**
     * Call handler based on supplied TLV
     * @param[in] msg Message object
     * @param[in] tlv_id TLV ID
     * @param[in] tlv pointer to a TLV of TLV ID
     * @note caller @b MUST @/b ensure the TLV is of TLV ID!
     */
    void callHadler(const Message &msg, mng_vals_e tlv_id,
        const BaseMngTlv *tlv);
    #ifdef SWIG_MessageDispatcher3
    SWIG_MessageDispatcher3
    #endif
    #ifndef SWIG
    /**
     * Handler called if there is no TLV data
     * It could be an empty TLV or not set
     * @param[in] msg Message object
     */
    virtual void noTlv(const Message &msg) const {}
    /**
     * Handler called if TLV does not have a callback.
     * i.e. inherit class did not implement a proper method for this TLV
     * @param[in] msg Message object
     * @param[in] idStr string of the tlv_id
     */
    virtual void noTlvCallBack(const Message &msg, const char *idStr) const {}
    #endif /* SWIG */
};
#endif /* SWIG_MessageDispatcher */

#ifdef SWIG_MessageBulder_START
SWIG_MessageBulder_START
#endif
/**
 * @brief build TLV to send a PTP management message
 * @note You must inherit this class to use it!
 * @note Class implemented in Python, Perl, Lua, PHP and Ruby.
 *       See testing for examples.
 *       Currently TCL is excluded.
 * @note As this class allocate a new TLV, and store it.
 *       It call Message.clearData() on destructor,
 *       to ensure Message does not use the TLV.
 *       Do not call Message.build() after deleting this object.
 * @note Class allocate TLV object and store it,
 * @note Lua do @b NOT @/b support destructors for native classes.
 *       You should call Message.clearData() yourself.
 * @note buildTlv() has an implementation per script language.
 *       So, it can call virtual function define in the script language itself.
 */
class MessageBulder : public BaseMngBuildCallback
{
  private:
    std::unique_ptr<BaseMngTlv> m_tlv; /**< Store allocated TLV for send */
    Message &m_msg; /**< Message Object to send message */

  public:
    /**
     * Get reference to the Message object
     * @return reference to the msg Message object
     */
    Message &getMsg() { return m_msg; }
    /**
     * Construct a message TLV builder
     * @param[in] msg Message object
     * @note This constructor is used internaly.
     *       You must inherit this class to use it!
     */
    MessageBulder(Message &msg) : m_msg(msg) {}
    virtual ~MessageBulder() { m_msg.clearData(); }
    /**
     * Allocate a management TLV, use a call-back to set its values and
     *  assign it with the Message object ready for sending
     * @param[in] actionField action type
     * @param[in] tlv_id TLV ID to send
     * @return true if setAction() succes
     * @note This function calls Message.setAction
     * @note For empty TLV or GET action,
     *       the function will call setAction without TLV
     */
    bool buildTlv(actionField_e actionField, mng_vals_e tlv_id);
};
#ifdef SWIG_MessageBulder_END
SWIG_MessageBulder_END
#endif

#ifndef SWIG
}; /* namespace ptpmgmt */
#endif

#endif /* __PTPMGMT_MSG_CALL_H */
