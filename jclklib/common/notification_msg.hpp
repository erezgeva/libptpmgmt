/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file notification_msg.hpp
 * @brief Common notification message class.
 * Implements common functions and (de-)serialization
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#ifndef COMMON_NOTIFICATION_MSG_HPP
#define COMMON_NOTIFICATION_MSG_HPP

#include <cstdint>

#include <common/jclklib_import.hpp>
#include <common/message.hpp>
#include <common/transport.hpp>

namespace JClkLibCommon
{
class NotificationMessage : virtual public Message
{
  public:
    virtual TRANSMIT_MESSAGE_TYPE(transmitMessage);
    static msgId_t getMsgId() { return SUBSCRIBE_MSG; }
    bool isEnable() { return waitEnable == 0x1; }
  protected:
#define MESSAGE_NOTIFY() JClkLibCommon::Message(JClkLibCommon::NOTIFY_MESSAGE)
    NotificationMessage() : MESSAGE_NOTIFY(), waitEnable(0) {}
  private:
    std::uint32_t   waitEnable : 1;
};
}

#endif /* COMMON_NOTIFICATION_MSG_HPP */
