/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file connect_msg.cpp
 * @brief Common connect message implementation.
 * Implements common functions and (de-)serialization
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#include <common/connect_msg.hpp>
#include <common/print.hpp>
#include <common/serialize.hpp>

using namespace JClkLibCommon;
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

PARSE_RXBUFFER_TYPE(CommonConnectMessage::parseBuffer)
{
    if(!Message::parseBuffer(LxContext))
        return false;
    if(!PARSE_RX(FIELD, get_sessionId(), LxContext))
        return false;
    if(!PARSE_RX(ARRAY, clientId, LxContext))
        return false;
    return true;
}

BUILD_TXBUFFER_TYPE(CommonConnectMessage::makeBuffer) const
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

TRANSMIT_MESSAGE_TYPE(CommonConnectMessage::transmitMessage)
{
    PrintDebug("[CommonConnectMessage]::transmitMessage ");
    if(!presendMessage(&TxContext))
        return false;
    return TxContext.sendBuffer();
}
