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

string CommonSubscribeMessage::toString()
{
    string name = ExtractClassName(string(__PRETTY_FUNCTION__),
            string(__FUNCTION__));
    name += "\n";
    name += Message::toString();
    //name += "Client ID: " + string((char *)clientId.data()) + "\n";
    return name;
}

bool CommonSubscribeMessage::parseBuffer(TransportListenerContext &LxContext)
{
    PrintDebug("[CommonSubscribeMessage]::parseBuffer ");
    if(!Message::parseBuffer(LxContext))
        return false;
    if(!PARSE_RX(FIELD, get_sessionId(), LxContext))
        return false;
    if(!PARSE_RX(FIELD, subscription, LxContext))
        return false;
    return true;
}

bool CommonSubscribeMessage::makeBuffer(TransportTransmitterContext &TxContext)
const
{
    auto ret = Message::makeBuffer(TxContext);
    if(!ret)
        return ret;
    PrintDebug("[CommonSubscribeMessage]::makeBuffer - sessionId : " +
        to_string(c_get_val_sessionId()));
    if(!WRITE_TX(FIELD, c_get_val_sessionId(), TxContext))
        return false;
    PrintDebug("[CommonSubscribeMessage]::makeBuffer - subscription event : " +
        to_string(subscription.get_event_mask()) + ", composite event : " +
        to_string(subscription.get_composite_event_mask()));
    if(!WRITE_TX(FIELD, subscription, TxContext))
        return false;
    return true;
}

bool CommonSubscribeMessage::transmitMessage(TransportTransmitterContext
    &TxContext)
{
    PrintDebug("[CommonSubscribeMessage]::transmitMessage ");
    if(!presendMessage(&TxContext))
        return false;
    return TxContext.sendBuffer();
}

void setSubscription(ClkMgrSubscription &newsub)
{
    PrintDebug("[CommonSubscribeMessage]::setSubscription ");
}
