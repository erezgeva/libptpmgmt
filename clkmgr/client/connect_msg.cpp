/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client connect message class. Implements client specific functionality.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "pub/clkmgr/timebase_configs.h"
#include "client/connect_msg.hpp"
#include "common/timebase.hpp"
#include "common/serialize.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

ClientState *ClientConnectMessage::currentClientState = nullptr;

/**
 * Create the ClientConnectMessage object
 * @param msg structure to be fill up
 * @param LxContext client run-time transport listener context
 * @return true
 */
bool ClientConnectMessage::buildMessage(Message *&msg,
    TransportListenerContext &LxContext)
{
    msg = new ClientConnectMessage();
    return true;
}

/**
 * @brief Add client's CONNECT_MSG type and its builder to transport layer.
 *
 * This function will be called during init to add a map of CONNECT_MSG
 * type and its corresponding buildMessage function.
 *
 * @return true
 */
bool ClientConnectMessage::initMessage()
{
    addMessageType(parseMsgMapElement_t(CONNECT_MSG, buildMessage));
    return true;
}

bool ClientConnectMessage::parseBuffer(TransportListenerContext &LxContext)
{
    PrintDebug("[ClientConnectMessage]::parseBuffer ");
    if(!CommonConnectMessage::parseBuffer(LxContext))
        return false;
    size_t mapSize = 0;
    if(!PARSE_RX(FIELD, mapSize, LxContext))
        return false;
    for(size_t i = 0; i < mapSize; ++i) {
        TimeBaseCfg newCfg = {};
        if(!PARSE_RX(FIELD, newCfg, LxContext))
            return false;
        TimeBaseConfigurations::addTimeBaseCfg(newCfg);
    }
    return true;
}

void ClientConnectMessage::setClientState(ClientState &newClientState)
{
    currentClientState = &newClientState;
}

/**
 * @brief process the reply for connect msg from proxy.
 *
 * This function will be called when the transport layer
 * in client runtime received a CONNECT_MSG type (an echo reply from
 * proxy)
 * In this case, transport layer will rx a buffer in the client associated
 * listening message queue (listening to proxy) and call this function when
 * the enum ID corresponding to the CONNECT_MSG is received.
 * For client-runtime, this will always be the reply, since connect msg
 * is always send first from the client runtime first. The proxy will
 * echo-reply with a different ACK msg.
 *
 * @param LxContext client run-time transport listener context
 * @param TxContext client run-time transport transmitter context
 * @return true
 */
bool ClientConnectMessage::processMessage(TransportListenerContext &LxContext,
    TransportTransmitterContext *&TxContext)
{
    unique_lock<rtpi::mutex> lock(cv_mtx);
    PrintDebug("Processing client connect message (reply)");
    currentClientState->set_connected(true);
    currentClientState->set_sessionId(this->get_sessionId());
    PrintDebug("Connected with session ID: " +
        to_string(this->get_sessionId()));
    PrintDebug("Current state.sessionId: " +
        to_string(currentClientState->get_sessionId()));
    this->set_msgAck(ACK_NONE);
    cv.notify_one(lock);
    return true;
}
