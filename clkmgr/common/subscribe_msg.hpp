/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file subscribe_msg.hpp
 * @brief Common subscribe message class.
 * Implements common functions and (de-)serialization
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#ifndef COMMON_SUBSCRIBE_MSG_HPP
#define COMMON_SUBSCRIBE_MSG_HPP

#include <clkmgr/subscription.h>
#include <common/message.hpp>
#include <common/ptp_event.hpp>

__CLKMGR_NAMESPACE_BEGIN

class CommonSubscribeMessage : virtual public Message
{
  private:
    clkmgr_subscription subscription;
    TransportClientId clientId;

  public:
    static msgId_t getMsgId() { return SUBSCRIBE_MSG; }
    //static MAKE_RXBUFFER_TYPE(buildMessage);
    virtual PARSE_RXBUFFER_TYPE(parseBuffer);
    virtual TRANSMIT_MESSAGE_TYPE(transmitMessage);
    virtual BUILD_TXBUFFER_TYPE(makeBuffer) const;
    clkmgr_subscription &getSubscription()
    { return subscription; }
    TransportClientId &getClientId()
    { return clientId; }
    void setSubscription(clkmgr_subscription &newsub);

    virtual std::string toString();
  protected:
#define MESSAGE_SUBSCRIBE() Message(SUBSCRIBE_MSG)
    CommonSubscribeMessage() : MESSAGE_SUBSCRIBE() {}
};

__CLKMGR_NAMESPACE_END

#endif /* COMMON_SUBSCRIBE_MSG_HPP */
