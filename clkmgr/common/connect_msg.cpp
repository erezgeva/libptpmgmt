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

string ConnectMessage::toString() const
{
    string name = MSG_EXTRACT_CLASS_NAME;
    name += "\n";
    name += Message::toString();
    name += "Client ID: " + string((char *)clientId.data()) + "\n";
    return name;
}

bool ConnectMessage::parseBufferComm()
{
    sessionId_t sessionId;
    if(!PARSE_RX(FIELD, sessionId, rxContext) ||
        !PARSE_RX(ARRAY, clientId, rxContext))
        return false;
    set_sessionId(sessionId);
    return true;
}

bool ConnectMessage::makeBufferComm(Transmitter &txContext) const
{
    return WRITE_TX(FIELD, get_sessionId(), txContext) &&
        WRITE_TX(ARRAY, clientId, txContext);
}
