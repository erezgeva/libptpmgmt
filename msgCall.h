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

/** @cond internal
 * Doxygen do not know how to proccess.
 * proccess these classes in mngIds.h
 */
#ifndef SWIG
class BaseMngDispatchCallback
{
  protected:
    virtual ~BaseMngDispatchCallback() = default;
#define _ptpmCaseUF(n)\
    virtual void n##_h(const Message &msg, const n##_t &data) const {}
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
    /* Per tlv ID a virtual call-back for user */
#include "ids.h"
};
class BaseMngBuildCallback
{
  protected:
    virtual ~BaseMngBuildCallback() = default;
#define _ptpmCaseUFB(n)\
    virtual bool n##_b(const Message &msg, n##_t &data) {return false;}
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
    /* Per tlv ID a virtual call-back for user */
#include "ids.h"
};
#endif /* SWIG */
/**< @endcond */

/**
 * @brief dispatch received PTP management message TLV
 * @note Do not handle signaling messages!
 * @note You must inherite this class to use it!
 */
class MessageDispatcher : public BaseMngDispatchCallback
{
  private:
    mng_vals_e m_tlv_id;
    const Message *m_msg;
    const BaseMngTlv *m_tlv;

  protected:
    MessageDispatcher();
    /**
     * Construct a dispatcher based on message last received message
     * @param[in] msg Message object
     * @param[in] callHadler call handler
     */
    MessageDispatcher(const Message &msg, bool callHadler = true);
    /**
     * Call handler based on Message last received message
     * @param[in] msg Message object
     * @param[in] callHadler call handler
     */

  public:
    void callHadler(const Message &msg, bool callHadler = true);
    /**
     * Call handler based on supplied TLV
     * @param[in] msg Message object
     * @param[in] tlv_id TLV ID
     * @param[in] tlv pointer to a TLV of TLV ID
     * @param[in] callHadler call handler
     * @note caller @b MUST @/b ensure the TLV is of TLV ID!
     */
    void callHadler(const Message &msg, mng_vals_e tlv_id,
        const BaseMngTlv *tlv, bool callHadler = true);
    /**
     * Call handler based on current Message last received message
     */
    void callHadler() const;
    /**
     * Handler called if there is no TLV data
     * It could be an empty TLV or not set
     */
    virtual void noTlv(const Message &msg) const {}
};

/**
 * @brief build TLV to send a PTP management message
 * @note You must inherite this class to use it!
 */
class MessageBulder : public BaseMngBuildCallback
{
  private:
    std::unique_ptr<BaseMngTlv> m_tlv;
    Message &m_msg;

  protected:
    /**
     * Construct a builder based on message last received message
     * @param[in] msg Message object
     */
    MessageBulder(Message &msg) : m_msg(msg) {}

  public:
    /**
     * Allocate a managment TLV use call-back to set its values and
     *  set it with Message ready for sending
     * This function will call setAction
     * @param[in] actionField action type
     * @param[in] tlv_id TLV ID to send
     * @return true if setAction() succes
     * @note For empty TLV the function will call setAction without TLV
     */
    bool buildTlv(actionField_e actionField, mng_vals_e tlv_id);
};

#ifndef SWIG
}; /* namespace ptpmgmt */
#endif

#endif /* __PTPMGMT_MSG_CALL_H */
