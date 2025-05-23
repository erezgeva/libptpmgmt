/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy notification message implementation.
 * Implements proxy specific notification message function.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "proxy/notification_msg.hpp"
#include "common/ptp_event.hpp"
#include "common/serialize.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

extern map<int, ptp_event> ptp4lEvents;

bool ProxyNotificationMessage::makeBuffer(Transmitter &txContext) const
{
    PrintDebug("[ProxyNotificationMessage]::makeBuffer");
    if(!makeBufferBase(txContext))
        return false;
    ptp_event event = ptp4lEvents[timeBaseIndex];
    // Add timeBaseIndex into the message
    if(!WRITE_TX(FIELD, timeBaseIndex, txContext))
        return false;
    // Add event data into the message
    if(!WRITE_TX(FIELD, event, txContext))
        return false;
    return true;
}

bool ProxyNotificationMessage::processMessage(Listener &rxContext,
    Transmitter *&txContext)
{
    return true;
}
