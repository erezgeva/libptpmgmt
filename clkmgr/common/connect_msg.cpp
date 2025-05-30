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

struct clientIdEncaps {
    char ClientId[CLIENTID_LENGTH];
};

string ConnectMessage::toString() const
{
    string name = MSG_EXTRACT_CLASS_NAME;
    name += "\n";
    name += Message::toString();
    name += "Client ID: " + clientId + "\n";
    return name;
}

bool ConnectMessage::parseBufferComm()
{
    sessionId_t sessionId;
    clientIdEncaps eClientId;
    if(!PARSE_RX(FIELD, sessionId, rxBuf) ||
        !PARSE_RX(FIELD, eClientId, rxBuf))
        return false;
    clientId = string(eClientId.ClientId, CLIENTID_LENGTH);
    set_sessionId(sessionId);
    return true;
}

bool ConnectMessage::makeBufferComm(Buffer &buff) const
{
    clientIdEncaps eClientId;
    clientId.copy(eClientId.ClientId, CLIENTID_LENGTH);
    return WRITE_TX(FIELD, get_sessionId(), buff) &&
        WRITE_TX(FIELD, eClientId, buff);
}
