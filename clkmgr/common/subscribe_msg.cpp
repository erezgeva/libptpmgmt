/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Common subscribe message implementation.
 * Implements common functions and (de-)serialization
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "common/subscribe_msg.hpp"
#include "common/serialize.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

string SubscribeMessage::toString()
{
    string name = MSG_EXTRACT_CLASS_NAME;
    name += "\n";
    name += Message::toString();
    //name += "Client ID: " + string((char *)clientId.data()) + "\n";
    return name;
}

bool SubscribeMessage::parseBuffer(Listener &rxContext)
{
    PrintDebug("[SubscribeMessage]::parseBuffer ");
    if(!Message::parseBuffer(rxContext))
        return false;
    sessionId_t sessionId;
    if(!PARSE_RX(FIELD, sessionId, rxContext))
        return false;
    set_sessionId(sessionId);
    if(!PARSE_RX(FIELD, subscription, rxContext))
        return false;
    return true;
}

bool SubscribeMessage::makeBuffer(Transmitter &txContext) const
{
    auto ret = makeBufferBase(txContext);
    if(!ret)
        return ret;
    PrintDebug("[SubscribeMessage]::makeBuffer - sessionId : " +
        to_string(get_sessionId()));
    if(!WRITE_TX(FIELD, get_sessionId(), txContext))
        return false;
    PrintDebug("[SubscribeMessage]::makeBuffer - subscription event : " +
        to_string(subscription.get_event_mask()) + ", composite event : " +
        to_string(subscription.get_composite_event_mask()));
    if(!WRITE_TX(FIELD, subscription, txContext))
        return false;
    return true;
}

bool SubscribeMessage::transmitMessage(Transmitter &txContext)
{
    PrintDebug("[SubscribeMessage]::transmitMessage ");
    if(!presendMessage(txContext))
        return false;
    return txContext.sendBuffer();
}

#if 0
void SubscribeMessage::setSubscription(const ClkMgrSubscription &newsub)
{
    PrintDebug("[SubscribeMessage]::setSubscription ");
    subscription = newsub;
}
#endif
