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

__CLKMGR_NAMESPACE_BEGIN

class NotificationMessage : public Message
{
  private:
    uint32_t waitEnable : 1;
  protected:
    NotificationMessage() : waitEnable(0) {}
  public:
    static msgId_t get_ClassMsgId() { return NOTIFY_MESSAGE; }
    msgId_t get_msgId() const override final { return NOTIFY_MESSAGE; }
    bool transmitMessage(Transmitter &TxContext) override;
    bool isEnable() { return waitEnable == 0x1; }
};

__CLKMGR_NAMESPACE_END

#endif /* COMMON_NOTIFICATION_MSG_HPP */
