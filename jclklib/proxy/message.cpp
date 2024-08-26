/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file message.cpp
 * @brief Proxy message base implementation.
 * Extended for specific messages such as connect and subscribe.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#include <proxy/connect_msg.hpp>
#include <proxy/notification_msg.hpp>
#include <proxy/null_msg.hpp>
#include <proxy/subscribe_msg.hpp>

__CLKMGR_NAMESPACE_USE

bool ProxyMessage::init()
{
    return _initMessage<ProxyNullMessage, ProxyConnectMessage,
        ProxySubscribeMessage, ProxyNotificationMessage>();
}
