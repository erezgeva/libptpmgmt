/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy notification message class.
 * Implements proxy specific notification message function.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef PROXY_NOTIFICATION_MSG_HPP
#define PROXY_NOTIFICATION_MSG_HPP

#include "common/notification_msg.hpp"
#include "pub/clkmgr/types.h"

__CLKMGR_NAMESPACE_BEGIN

class ProxyNotificationMessage : public NotificationMessage
{
  private:
    bool makeBufferTail(Buffer &buff) const override final;
    ClockType clockType = PTPClock;

  public:
    void setClockType(ClockType type) { clockType = type; }
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_NOTIFICATION_MSG_HPP */
