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
    return name;
}

bool SubscribeMessage::parseBufferComm()
{
    PrintDebug("[SubscribeMessage]::parseBufferComm");
    sessionId_t sessionId;
    if(!PARSE_RX(FIELD, sessionId, rxBuf) ||
        !PARSE_RX(FIELD, timeBaseIndex, rxBuf))
        return false;
    set_sessionId(sessionId);
    return true;
}

bool SubscribeMessage::makeBufferComm(Buffer &buff) const
{
    PrintDebug("[SubscribeMessage]::makeBufferComm - sessionId : " +
        to_string(get_sessionId()));
    return WRITE_TX(FIELD, get_sessionId(), buff) &&
        WRITE_TX(FIELD, timeBaseIndex, buff);
}
