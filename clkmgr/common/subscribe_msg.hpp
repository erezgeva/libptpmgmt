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
    bool parseBufferComm() override final;
    bool makeBufferComm(Buffer &buff) const override final;

  protected:
    SubscribeMessage() = default;
    size_t timeBaseIndex = 0;

  public:
    msgId_t get_msgId() const override final { return SUBSCRIBE_MSG; }

    // Seems the Clock manager subscription is left in the client
    // TODO Why do we send it in the Subscribe Message?
    const ClkMgrSubscription &getSubscription() const { return subscription; }
    // void setSubscription(const ClkMgrSubscription &newsub);

    std::string toString() const override;

    /**
     * Set the time base index.
     * @param[in] index The new time base index to set.
     */
    void set_timeBaseIndex(int index) { timeBaseIndex = index; }
};

__CLKMGR_NAMESPACE_END

#endif /* COMMON_SUBSCRIBE_MSG_HPP */
