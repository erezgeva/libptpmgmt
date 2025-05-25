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

string SubscribeMessage::toString() const
{
    string name = MSG_EXTRACT_CLASS_NAME;
    name += "\n";
    name += Message::toString();
    //name += "Client ID: " + string((char *)clientId.data()) + "\n";
    return name;
}

bool SubscribeMessage::parseBufferComm()
{
    PrintDebug("[SubscribeMessage]::parseBufferComm");
    sessionId_t sessionId;
    if(!PARSE_RX(FIELD, sessionId, rxContext) ||
        !PARSE_RX(FIELD, subscription, rxContext))
        return false;
    set_sessionId(sessionId);
    return true;
}

bool SubscribeMessage::makeBufferComm(Transmitter &txContext) const
{
    PrintDebug("[SubscribeMessage]::makeBufferComm - sessionId : " +
        to_string(get_sessionId()));
    PrintDebug("[SubscribeMessage]::makeBufferComm - subscription event : " +
        to_string(subscription.get_event_mask()) + ", composite event : " +
        to_string(subscription.get_composite_event_mask()));
    return WRITE_TX(FIELD, get_sessionId(), txContext) &&
        WRITE_TX(FIELD, subscription, txContext);
}

#if 0
void SubscribeMessage::setSubscription(const ClkMgrSubscription &newsub)
{
    PrintDebug("[SubscribeMessage]::setSubscription");
    subscription = newsub;
}
#endif
