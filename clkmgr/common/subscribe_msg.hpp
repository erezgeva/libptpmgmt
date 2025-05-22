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

class SubscribeMessage : public Message
{
  private:
    ClkMgrSubscription subscription;
    ClientId clientId;
  protected:
    SubscribeMessage() = default;

  public:
    static msgId_t get_ClassMsgId() { return SUBSCRIBE_MSG; }
    msgId_t get_msgId() const override final { return SUBSCRIBE_MSG; }
    bool parseBuffer(Listener &LxContext) override;
    bool transmitMessage(Transmitter &TxContext) override;
    bool makeBuffer(Transmitter &TxContext) const override;
    ClkMgrSubscription &getSubscription() { return subscription; }
    ClientId &getClientId() { return clientId; }
    void setSubscription(ClkMgrSubscription &newsub);

    std::string toString() override;
};

__CLKMGR_NAMESPACE_END

#endif /* COMMON_SUBSCRIBE_MSG_HPP */
