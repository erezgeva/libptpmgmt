/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy connect message implementation.
 * Implements proxy specific connect message function.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "proxy/connect_msg.hpp"
#include "proxy/config_parser.hpp"
#include "proxy/client.hpp"
#include "common/serialize.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

/**
 * @brief process the connect msg from client-runtime
 *
 * This function will be called when the transport layer
 * in proxy receive a CONNECT_MSG type from client-runtime.
 * In this case, proxy transport layer will rx a buffer in the
 * message queue and call this function when
 * the enum ID corresponding to the CONNECT_MSG is received.
 * A new ClientSession object
 * (with the transmit msq) is created in the proxy.
 *
 * @return true
 */
bool ProxyConnectMessage::parseBufferTail()
{
    sessionId_t newSessionId = get_sessionId();
    PrintDebug("Processing proxy connect message");
    if(newSessionId != InvalidSessionId) {
        if(Client::existClient(newSessionId)) {
            PrintDebug("Receive Connect msg as liveness check.");
            set_msgAck(ACK_SUCCESS);
            return true;
        }
        PrintError("Session ID not exists: " + to_string(newSessionId));
        return false;
    }
    newSessionId = Client::CreateClientSession(getClientId());
    if(newSessionId == InvalidSessionId) {
        PrintError("Fail to allocate new session");
        return false;
    }
    PrintDebug("Created new client session ID: " + to_string(newSessionId));
    set_sessionId(newSessionId);
    set_msgAck(ACK_SUCCESS);
    return true;
}

bool ProxyConnectMessage::makeBufferTail(Transmitter &txContext) const
{
    PrintDebug("[ProxyConnectMessage]::makeBufferTail");
    JsonConfigParser parser = JsonConfigParser::getInstance();
    size_t mapSize = parser.size();
    if(!WRITE_TX(FIELD, mapSize, txContext))
        return false;;
    for(const auto &row : parser) {
        TimeBaseCfg cfg = row.base;
        if(!WRITE_TX(FIELD, cfg, txContext))
            return false;
    }
    return true;
}
