/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file message.cpp
 * @brief Proxy message base implementation. Extended for specific messages such as connect and subscribe.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#include <proxy/null_msg.hpp>
#include <proxy/connect_msg.hpp>
#include <proxy/subscribe_msg.hpp>
#include <proxy/notification_msg.hpp>

using namespace JClkLibProxy;
using namespace JClkLibCommon;

bool ProxyMessage::init()
{
	return _initMessage<ProxyNullMessage, ProxyConnectMessage, ProxySubscribeMessage, ProxyNotificationMessage>();
}
		


