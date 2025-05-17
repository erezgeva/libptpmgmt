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

/**
 * Create the ProxyNotificationMessage object
 * @param msg msg structure to be fill up
 * @param LxContext proxy transport listener context
 * @return true
 */
bool ProxyNotificationMessage::buildMessage(Message *&msg,
    TransportListenerContext &LxContext)
{
    msg = new ProxyNotificationMessage();
    return true;
}

/**
 * @brief Add proxy's NOTIFY_MESSAGE type and its builder to transport layer.
 *
 * This function will be called during init to add a map of NOTIFY_MESSAGE
 * type and its corresponding buildMessage function.
 *
 * @return true
 */
bool ProxyNotificationMessage::initMessage()
{
    addMessageType(parseMsgMapElement_t(NOTIFY_MESSAGE, buildMessage));
    return true;
}

bool ProxyNotificationMessage::makeBuffer(TransportTransmitterContext
    &TxContext) const
{
    PrintDebug("[ProxyNotificationMessage]::makeBuffer");
    if(!Message::makeBuffer(TxContext))
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

bool ProxyNotificationMessage::processMessage(TransportListenerContext
    &LxContext, TransportTransmitterContext *&TxContext)
{
    return true;
}
