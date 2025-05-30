/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy subscribe message implementation.
 * Implements proxy specific subscribe message function.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "proxy/subscribe_msg.hpp"
#include "proxy/client.hpp"
#include "common/ptp_event.hpp"
#include "common/serialize.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

extern map<int, ptp_event> ptp4lEvents;

bool ProxySubscribeMessage::parseBufferTail()
{
    PrintDebug("[ProxySubscribeMessage]::parseBufferTail");
    if(!Client::subscribe(timeBaseIndex, get_sessionId()))
        return false;
    set_msgAck(ACK_SUCCESS);
    return true;
}

bool ProxySubscribeMessage::makeBufferTail(Buffer &buff) const
{
    PrintDebug("[ProxySubscribeMessage]::makeBufferTail");
    ptp_event event = ptp4lEvents[timeBaseIndex];
    // Add event data into the message
    return WRITE_TX(FIELD, event, buff);
}
