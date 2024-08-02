/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file notification_msg.cpp
 * @brief Common notification message implementation.
 * Implements common functions and (de-)serialization
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#include <common/notification_msg.hpp>
#include <common/print.hpp>
#include <common/serialize.hpp>

using namespace JClkLibCommon;
using namespace std;

TRANSMIT_MESSAGE_TYPE(NotificationMessage::transmitMessage)
{
    PrintDebug("[NotificationMessage]::transmitMessage ");
    if(!presendMessage(&TxContext))
        return false;
    return TxContext.sendBuffer();
}
