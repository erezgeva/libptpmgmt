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
 * A new ClientSession object and a corresponding TxContext
 * (with the transmit msq) is created in the proxy.
 *
 * @param LxContext proxy listener
 * @param TxContext proxy transmitter
 * @return true
 */
bool ProxyConnectMessage::processMessage(Listener &LxContext,
    Transmitter *&TxContext)
{
    sessionId_t newSessionId = get_sessionId();
    PrintDebug("Processing proxy connect message");
    if(newSessionId != InvalidSessionId) {
        auto clientSession = Client::GetClientSession(newSessionId);
        if(clientSession)
            TxContext = clientSession.get()->get_transmitContext();
        if(TxContext) {
            PrintDebug("Receive Connect msg as liveness check.");
            set_msgAck(ACK_SUCCESS);
            return true;
        }
        PrintError("Session ID not exists: " + to_string(newSessionId));
        return false;
    }
    newSessionId = Client::CreateClientSession();
    PrintDebug("Created new client session ID: " + to_string(newSessionId));
    this->set_sessionId(newSessionId);
    TxContext = Client::CreateTransmitterContext(getClientId());
    if(TxContext == nullptr)
        return false;
    Client::GetClientSession(newSessionId).get()->set_transmitContext(TxContext);
    set_msgAck(ACK_SUCCESS);
    return true;
}

bool ProxyConnectMessage::makeBuffer(Transmitter &TxContext) const
{
    PrintDebug("[ProxyConnectMessage]::makeBuffer");
    if(!ConnectMessage::makeBuffer(TxContext))
        return false;
    JsonConfigParser parser = JsonConfigParser::getInstance();
    size_t mapSize = parser.size();
    if(!WRITE_TX(FIELD, mapSize, TxContext))
        return false;;
    for(const auto &row : parser) {
        TimeBaseCfg cfg = row.base;
        if(!WRITE_TX(FIELD, cfg, TxContext))
            return false;
    }
    return true;
}
