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
#include "proxy/client.hpp"
#include "common/serialize.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

bool ProxyNotificationMessage::makeBufferTail(Buffer &buff) const
{
    PrintDebug("[ProxyNotificationMessage]::makeBufferTail");
    unique_lock<rtpi::mutex> local(Client::getTimeBaseLock(timeBaseIndex));
    ptp_event event = Client::getPTPEvent(timeBaseIndex);
    local.unlock(); // Explicitly unlock the mutex
    // Add event data into the message
    return WRITE_TX(FIELD, event, buff);
}
