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
    // Client ID Is not in the Subscribe Message
    // ClientId clientId;
    bool parseBufferComm() override final;
    bool makeBufferComm(Transmitter &txContext) const override final;

  protected:
    SubscribeMessage() = default;

  public:
    msgId_t get_msgId() const override final { return SUBSCRIBE_MSG; }

    // Seems the Clock manager subscription is left in the client
    // Why do we send it in the Subscribe Message?
    const ClkMgrSubscription &getSubscription() const { return subscription; }
    // void setSubscription(const ClkMgrSubscription &newsub);

    // Client ID Is not in the Subscribe Message
    // const ClientId &getClientId() const { return clientId; }

    std::string toString() const override;
};

__CLKMGR_NAMESPACE_END

#endif /* COMMON_SUBSCRIBE_MSG_HPP */
