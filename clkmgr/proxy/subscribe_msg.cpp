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

bool ProxySubscribeMessage::makeBuffer(Transmitter &txContext) const
{
    PrintDebug("[ProxySubscribeMessage]::makeBuffer");
    if(!SubscribeMessage::makeBuffer(txContext))
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

bool ProxySubscribeMessage::parseBuffer(Listener &rxContext)
{
    PrintDebug("[ProxySubscribeMessage]::parseBuffer ");
    if(!SubscribeMessage::parseBuffer(rxContext))
        return false;
    if(!PARSE_RX(FIELD, timeBaseIndex, rxContext))
        return false;
    ConnectPtp4l::subscribe_ptp4l(timeBaseIndex, get_sessionId());
    #ifdef HAVE_LIBCHRONY
    ConnectChrony::subscribe_chrony(std::move(timeBaseIndex), get_sessionId());
    #endif
    return true;
}

/*
This is to process the subscription from the clkmgr client runtime
via POSIX msg queue.
*/
bool ProxySubscribeMessage::processMessage(Listener &rxContext,
    Transmitter *&txContext)
{
    sessionId_t sID = get_sessionId();
    PrintDebug("[ProxySubscribeMessage]::processMessage - "
        "Use current client session ID: " + to_string(sID));
    if(sID == InvalidSessionId) {
        PrintError("Session ID *should be* invalid for received "
            "proxy connect message");
        return false;
    }
    txContext = Client::getTxContext(sID);
    if(txContext == nullptr) {
        PrintError("Session ID " + to_string(sID) + " do not have Transmitter");
        return false;
    }
    set_msgAck(ACK_SUCCESS);
    return true;
}
