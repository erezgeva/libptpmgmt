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

/**
 * Create the ProxySubscribeMessage object
 * @param msg msg structure to be fill up
 * @param LxContext proxy transport listener context
 * @return true
 */
bool ProxySubscribeMessage::buildMessage(Message *&msg,
    TransportListenerContext &LxContext)
{
    msg = new ProxySubscribeMessage();
    return true;
}

/**
 * @brief Add proxy's SUBSCRIBE_MSG type and its builder to transport layer.
 *
 * This function will be called during init to add a map of SUBSCRIBE_MSG
 * type and its corresponding buildMessage function.
 *
 * @return true
 */
bool ProxySubscribeMessage::initMessage()
{
    addMessageType(parseMsgMapElement_t(SUBSCRIBE_MSG, buildMessage));
    return true;
}

bool ProxySubscribeMessage::makeBuffer(TransportTransmitterContext &TxContext)
const
{
    PrintDebug("[ProxySubscribeMessage]::makeBuffer");
    if(!CommonSubscribeMessage::makeBuffer(TxContext))
        return false;
    ptp_event event = ptp4lEvents[timeBaseIndex];
    // Add timeBaseIndex into the message
    if(!WRITE_TX(FIELD, timeBaseIndex, TxContext))
        return false;
    // Add event data into the message
    if(!WRITE_TX(FIELD, event, TxContext))
        return false;
    return true;
}

bool ProxySubscribeMessage::parseBuffer(TransportListenerContext &LxContext)
{
    PrintDebug("[ProxySubscribeMessage]::parseBuffer ");
    if(!CommonSubscribeMessage::parseBuffer(LxContext))
        return false;
    if(!PARSE_RX(FIELD, timeBaseIndex, LxContext))
        return false;
    ConnectPtp4l::subscribe_ptp4l(timeBaseIndex, this->getc_sessionId());
    #ifdef HAVE_LIBCHRONY
    ConnectChrony::subscribe_chrony(std::move(timeBaseIndex),
        this->getc_sessionId());
    #endif
    return true;
}

/*
This is to process the subscription from the clkmgr client runtime
via POSIX msg queue.
*/
bool ProxySubscribeMessage::processMessage(TransportListenerContext &LxContext,
    TransportTransmitterContext *&TxContext)
{
    sessionId_t sID;
    sID = this->getc_sessionId();
    PrintDebug("[ProxySubscribeMessage]::processMessage - "
        "Use current client session ID: "
        + to_string(sID));
    if(sID == InvalidSessionId) {
        PrintError("Session ID *should be* invalid for received "
            "proxy connect message");
        return false;
    }
    TxContext = Client::GetClientSession(sID).get()->get_transmitContext();
    set_msgAck(ACK_SUCCESS);
    return true;
}
