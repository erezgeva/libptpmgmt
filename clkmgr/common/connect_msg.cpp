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

string CommonConnectMessage::toString()
{
    string name = ExtractClassName(string(__PRETTY_FUNCTION__),
            string(__FUNCTION__));
    name += "\n";
    name += Message::toString();
    name += "Client ID: " + string((char *)clientId.data()) + "\n";
    return name;
}

bool CommonConnectMessage::parseBuffer(TransportListenerContext &LxContext)
{
    if(!Message::parseBuffer(LxContext))
        return false;
    if(!PARSE_RX(FIELD, get_sessionId(), LxContext))
        return false;
    if(!PARSE_RX(ARRAY, clientId, LxContext))
        return false;
    return true;
}

bool CommonConnectMessage::makeBuffer(TransportTransmitterContext &TxContext)
const
{
    auto ret = Message::makeBuffer(TxContext);
    if(!ret)
        return ret;
    if(!WRITE_TX(FIELD, c_get_val_sessionId(), TxContext))
        return false;
    if(!WRITE_TX(ARRAY, clientId, TxContext))
        return false;
    return true;
}

bool CommonConnectMessage::transmitMessage(TransportTransmitterContext
    &TxContext)
{
    PrintDebug("[CommonConnectMessage]::transmitMessage ");
    if(!presendMessage(&TxContext))
        return false;
    return TxContext.sendBuffer();
}
