/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file notification_msg.cpp
 * @brief Common notification message implementation. Implements common functions and (de-)serialization
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#include <common/serialize.hpp>
#include <common/notification_msg.hpp>
#include <common/print.hpp>

using namespace JClkLibCommon;
using namespace std;

TRANSMIT_MESSAGE_TYPE(NotificationMessage::transmitMessage)
{
	PrintDebug("[NotificationMessage]::transmitMessage ");
	if (!presendMessage(&TxContext))
		return false;

	return TxContext.sendBuffer();
}
