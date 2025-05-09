/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client message base class.
 * Extended for specific messages such as connect and subscribe.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "client/message.hpp"
#include "client/connect_msg.hpp"
#include "client/notification_msg.hpp"
#include "client/null_msg.hpp"
#include "client/subscribe_msg.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

bool ClientMessage::init()
{
    PrintDebug("Initializing Client Message");
    return _initMessage<ClientNullMessage, ClientConnectMessage,
        ClientSubscribeMessage, ClientNotificationMessage>();
}
