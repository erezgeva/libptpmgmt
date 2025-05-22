/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Common subscribe message class.
 * Implements common functions and (de-)serialization
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef COMMON_SUBSCRIBE_MSG_HPP
#define COMMON_SUBSCRIBE_MSG_HPP

#include "pub/clkmgr/subscription.h"
#include "common/message.hpp"

__CLKMGR_NAMESPACE_BEGIN

class CommonSubscribeMessage : virtual public Message
{
  private:
    ClkMgrSubscription subscription;
    ClientId clientId;

  public:
    static msgId_t getMsgId() { return SUBSCRIBE_MSG; }
    virtual bool parseBuffer(Listener &LxContext);
    virtual bool transmitMessage(Transmitter &TxContext);
    virtual bool makeBuffer(Transmitter &TxContext) const;
    ClkMgrSubscription &getSubscription()
    { return subscription; }
    ClientId &getClientId()
    { return clientId; }
    void setSubscription(ClkMgrSubscription &newsub);

    virtual std::string toString();
  protected:
    CommonSubscribeMessage() : Message(SUBSCRIBE_MSG) {}
};

__CLKMGR_NAMESPACE_END

#endif /* COMMON_SUBSCRIBE_MSG_HPP */
