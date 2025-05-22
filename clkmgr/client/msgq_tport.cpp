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
#include "client/connect_msg.hpp"
#include "client/subscribe_msg.hpp"
#include "common/print.hpp"

#include <unistd.h>

__CLKMGR_NAMESPACE_USE;

using namespace std;

static Listener rxContext;
static Transmitter txContext;

bool ClientQueue::init()
{
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
    return true;
}

bool ClientQueue::stop()
{
    rxContext.stopSignal();
    PrintDebug("Stopping client queue");
    PrintDebug("Listener queue = " + rxContext.getQueueName());
    if(!rxContext.stop()) {
        PrintError("stop rxContext failed");
        return false;
    }
    return true;
}

bool ClientQueue::finalize()
{
    PrintDebug("Listener queue = " + rxContext.getQueueName());
    PrintDebug("Transmitter Queue = " + txContext.getQueueName());
    return rxContext.finalize() && txContext.finalize();
}

bool ClientQueue::sendMessage(Message *msg)
{
    if(!msg->writeClientId(rxContext))
        PrintErrorCode("Unknown msgID type");
    msg->presendMessage(txContext);
    if(!txContext.sendBuffer())
        return false;
    PrintDebug("[ClientQueue]::sendMessage successful ");
    return true;
}
