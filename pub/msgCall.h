/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Messages dispatcher and builder classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 */

#ifndef __PTPMGMT_MSG_CALL_H
#define __PTPMGMT_MSG_CALL_H

#ifdef __cplusplus
#include "callDef.h"

__PTPMGMT_NAMESPACE_BEGIN

/**
 * @brief Dispatch received PTP management message TLV
 * @note Do not handle signalling messages!
 * @note You must inherit this class to use it!
 * @note callHadler() has an implementation in lua, tcl and go languages,
 *       other languages uses swig to translate this class.
 */
class MessageDispatcher : public BaseMngDispatchCallback
{
  private:
    /* To improve backward compatibility
       Call virtual functions in inline functions,
       so we compile them in the application
       and avoid using virtual function table in the library */
    void cNoTlv(const Message &msg) { noTlv(msg); }
    void cNoTlvCB(const Message &msg, const char *idStr) {
        noTlvCallBack(msg, idStr);
    }
  public:
    /**
     * Construct a message TLV dispatcher
     */
    MessageDispatcher() = default;
    /**
     * Call handler based on Message last received message
     * @param[in] msg Message object
     */
    void callHadler(const Message &msg);
    /**
     * Call handler based on supplied TLV
     * @param[in] msg Message object
     * @param[in] tlv_id TLV ID
     * @param[in] tlv pointer to a TLV of TLV ID
     * @note caller @b MUST @/b ensure the TLV ID match the actual TLV!
     */
    void callHadler(const Message &msg, mng_vals_e tlv_id,
        const BaseMngTlv *tlv);
    /**
     * Handler called if there is no TLV data
     * It could be an empty TLV or unkown
     * @param[in] msg Message object
     */
    virtual void noTlv(const Message &msg) {}
    /**
     * Handler called if TLV does not have a callback.
     * i.e. inherit class did not implement a proper method for this TLV
     * @param[in] msg Message object
     * @param[in] idStr string of the tlv_id
     */
    virtual void noTlvCallBack(const Message &msg, const char *idStr) {}
};

/**
 * @brief Build TLV to send a PTP management message
 * @note You must inherit this class to use it!
 * @note Class allocate TLV object and store it,
 * @note buildTlv() has an implementation in lua, tcl and go languages,
 *       other languages uses swig to translate this class.
 */
class MessageBuilder : public BaseMngBuildCallback
{
  private:
    std::unique_ptr<BaseMngTlv> m_tlv; /**< Store allocated TLV for send */
    Message &m_msg; /**< Message Object to send message */

  public:
    /**
     * Construct a message TLV builder
     * @param[in] msg Message object
     */
    MessageBuilder(Message &msg);
    /**
     * Get reference to the Message object
     * @return reference to the msg Message object
     */
    Message &getMsg();
    /**
     * Clear the tlv in the Message object
     */
    void clear();
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

__PTPMGMT_NAMESPACE_END
#else /* __cplusplus */
#include "c/msgCall.h"
#endif /* __cplusplus */

#endif /* __PTPMGMT_MSG_CALL_H */
