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
#include "proxy/connect_chrony.hpp"
#include "proxy/connect_ptp4l.hpp"
#include "common/ptp_event.hpp"
#include "common/serialize.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

extern map<int, ptp_event> ptp4lEvents;

bool ProxySubscribeMessage::makeBufferTail(Transmitter &txContext) const
{
    PrintDebug("[ProxySubscribeMessage]::makeBufferTail");
    ptp_event event = ptp4lEvents[timeBaseIndex];
    // Add event data into the message
    return WRITE_TX(FIELD, event, txContext);
}

bool ProxySubscribeMessage::parseBufferTail()
{
    PrintDebug("[ProxySubscribeMessage]::parseBufferTail");
    sessionId_t sID = get_sessionId();
    if(!Client::existClient(sID)) {
        PrintError("Session ID " + to_string(sID) + " is not registered");
        return false;
    }
    ConnectPtp4l::subscribe_ptp4l(timeBaseIndex, sID);
    #ifdef HAVE_LIBCHRONY
    ConnectChrony::subscribe_chrony(timeBaseIndex, sID);
    #endif
    PrintDebug("[ProxySubscribeMessage]::parseBufferTail - "
        "Use current client session ID: " + to_string(sID));
    set_msgAck(ACK_SUCCESS);
    return true;
}
