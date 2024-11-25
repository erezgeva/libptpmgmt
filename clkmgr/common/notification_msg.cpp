/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Common notification message implementation.
 * Implements common functions and (de-)serialization
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "common/notification_msg.hpp"
#include "common/print.hpp"
#include "common/serialize.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

TRANSMIT_MESSAGE_TYPE(NotificationMessage::transmitMessage)
{
    PrintDebug("[NotificationMessage]::transmitMessage ");
    if(!presendMessage(&TxContext))
        return false;
    return TxContext.sendBuffer();
}
