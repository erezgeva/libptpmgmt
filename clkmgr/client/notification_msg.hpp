/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client notification message class.
 * Implements client specific notification message function.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLIENT_NOTIFICATION_MSG_HPP
#define CLIENT_NOTIFICATION_MSG_HPP

#include "client/message.hpp"
#include "common/notification_msg.hpp"

__CLKMGR_NAMESPACE_BEGIN

class ClientNotificationMessage : public NotificationMessage
{
  public:
    bool processMessage(Listener &rxContext, Transmitter *&txContext) override;
    bool makeBuffer(Transmitter &txContext) const override;
    bool parseBuffer(Listener &rxContext) override;
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_NOTIFICATION_MSG_HPP */
