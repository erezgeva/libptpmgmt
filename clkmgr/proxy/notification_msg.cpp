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
    uint8_t clockTypeValue = static_cast<uint8_t>(clockType);
    if(!WRITE_TX(clockTypeValue, buff))
        return false;
    if(clockType == PTPClock) {
        // Send PTP event data
        ptp_event ptpEvent;
        Client::getPTPEvent(timeBaseIndex, ptpEvent);
        if(!WRITE_TX(ptpEvent, buff))
            return false;
        PrintDebug("[ProxyNotificationMessage] Sent PTP event data");
    }
    if(clockType == SysClock) {
        // Send Chrony event data
        chrony_event chronyEvent;
        Client::getChronyEvent(timeBaseIndex, chronyEvent);
        if(!WRITE_TX(chronyEvent, buff))
            return false;
        PrintDebug("[ProxyNotificationMessage] Sent Chrony event data");
    }
    return true;
}
