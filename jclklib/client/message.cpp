/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file message.cpp
 * @brief Client message base class. Extended for specific messages such as connect and subscribe.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#include <client/message.hpp>
#include <client/null_msg.hpp>
#include <client/connect_msg.hpp>
#include <client/subscribe_msg.hpp>
#include <client/notification_msg.hpp>
#include <common/print.hpp>

using namespace JClkLibClient;

bool ClientMessage::init()
{
	PrintDebug("Initializing Client Message");
    return JClkLibCommon::_initMessage<ClientNullMessage,ClientConnectMessage,
        ClientSubscribeMessage,ClientNotificationMessage>();
}
