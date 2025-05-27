/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client queue class.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "client/msgq_tport.hpp"
#include "client/client_state.hpp"
#include "common/message.hpp"
#include "common/termin.hpp"
#include "common/print.hpp"

#include <unistd.h>

__CLKMGR_NAMESPACE_USE;

using namespace std;

static Transmitter txContext;

Transmitter *Transmitter::getTransmitterInstance(sessionId_t sessionId)
{
    return &txContext;
}

bool ClientQueue::init()
{
    Listener &rxContext = Listener::getSingleListenerInstance();
    /* Two outstanding messages per client */
    PrintDebug("Initializing Client Queue ...");
    string mqListenerName(mqProxyName);
    mqListenerName += "." + to_string(getpid());
    if(!rxContext.init(mqListenerName, 2)) {
        PrintError("Failed to open listener queue");
        return false;
    }
    if(!txContext.open(mqProxyName)) {
        PrintErrorCode("Failed to open transmitter queue: " + mqProxyName);
        return false;
    }
    PrintDebug("Client Message queue opened");
    ClientState::getSingleInstance().set_clientID(rxContext.getClientId());
    return true;
}

bool ClientQueue::sendMessage(Message *msg)
{
    if(!msg->transmitMessage())
        return false;
    PrintDebug("[ClientQueue]::sendMessage successful");
    return true;
}

class ClientQueueEnd : public End
{
    bool stop() override final { return true; }
    bool finalize() override final {
        PrintDebug("Transmitter Queue = " + txContext.getQueueName());
        return txContext.finalize();
    }
};
static ClientQueueEnd endClient;
