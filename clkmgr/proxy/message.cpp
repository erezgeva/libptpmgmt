/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy message base implementation.
 * Extended for specific messages such as connect and subscribe.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "proxy/message.hpp"
#include "proxy/connect_msg.hpp"
#include "proxy/notification_msg.hpp"
#include "proxy/subscribe_msg.hpp"

__CLKMGR_NAMESPACE_USE;

bool clkmgr::proxyMessageRegister()
{
    reg_message_type<ProxyConnectMessage, ProxySubscribeMessage,
                     ProxyNotificationMessage>();
    return true;
}
