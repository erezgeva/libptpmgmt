/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Common notification message class.
 * Implements common functions and (de-)serialization
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef COMMON_NOTIFICATION_MSG_HPP
#define COMMON_NOTIFICATION_MSG_HPP

#include "common/message.hpp"
#include "common/transport.hpp"

#include <cstdint>

__CLKMGR_NAMESPACE_BEGIN

class NotificationMessage : virtual public Message
{
  public:
    virtual TRANSMIT_MESSAGE_TYPE(transmitMessage);
    static msgId_t getMsgId() { return SUBSCRIBE_MSG; }
    bool isEnable() { return waitEnable == 0x1; }
  protected:
#define MESSAGE_NOTIFY() Message(NOTIFY_MESSAGE)
    NotificationMessage() : MESSAGE_NOTIFY(), waitEnable(0) {}
  private:
    uint32_t   waitEnable : 1;
};

__CLKMGR_NAMESPACE_END

#endif /* COMMON_NOTIFICATION_MSG_HPP */
