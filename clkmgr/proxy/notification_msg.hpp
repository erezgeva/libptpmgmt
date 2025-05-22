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

#include "proxy/message.hpp"
#include "common/notification_msg.hpp"

__CLKMGR_NAMESPACE_BEGIN

class ProxyNotificationMessage : public NotificationMessage
{
  private:
    int timeBaseIndex = 0;

  public:
    bool processMessage(Listener &rxContext, Transmitter *&txContext) override;
    bool makeBuffer(Transmitter &txContext) const override;

    void setTimeBaseIndex(int newTimeBaseIndex) {
        timeBaseIndex = newTimeBaseIndex;
    }
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_NOTIFICATION_MSG_HPP */
