/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Common connect message implementation.
 * Implements common functions and (de-)serialization
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "common/connect_msg.hpp"
#include "common/serialize.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

string ConnectMessage::toString()
{
    string name = MSG_EXTRACT_CLASS_NAME;
    name += "\n";
    name += Message::toString();
    name += "Client ID: " + string((char *)clientId.data()) + "\n";
    return name;
}

bool ConnectMessage::parseBuffer(Listener &LxContext)
{
    if(!Message::parseBuffer(LxContext))
        return false;
    sessionId_t sessionId;
    if(!PARSE_RX(FIELD, sessionId, LxContext))
        return false;
    set_sessionId(sessionId);
    if(!PARSE_RX(ARRAY, clientId, LxContext))
        return false;
    return true;
}

bool ConnectMessage::makeBuffer(Transmitter &TxContext) const
{
    auto ret = makeBufferBase(TxContext);
    if(!ret)
        return ret;
    if(!WRITE_TX(FIELD, get_sessionId(), TxContext))
        return false;
    if(!WRITE_TX(ARRAY, clientId, TxContext))
        return false;
    return true;
}

bool ConnectMessage::transmitMessage(Transmitter &TxContext)
{
    PrintDebug("[ConnectMessage]::transmitMessage ");
    if(!presendMessage(TxContext))
        return false;
    return TxContext.sendBuffer();
}
