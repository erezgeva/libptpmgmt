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
#include "common/serialize.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

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
    ptp_event event;
    Client::getPTPEvent(timeBaseIndex, event);
    // Add event data into the message
    return WRITE_TX(event, buff);
}
