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
  protected:
    NotificationMessage() = default;
    size_t timeBaseIndex = 0;
    bool parseBufferComm() override final;
    bool makeBufferComm(Buffer &buff) const override final;

  public:
    msgId_t get_msgId() const override final { return NOTIFY_MESSAGE; }
    void setTimeBaseIndex(int index) { timeBaseIndex = index; }
};

__CLKMGR_NAMESPACE_END

#endif /* COMMON_NOTIFICATION_MSG_HPP */
