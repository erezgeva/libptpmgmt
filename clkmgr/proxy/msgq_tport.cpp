/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy POSIX message queue transport implementation.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "common/print.hpp"
#include "common/sighandler.hpp"
#include "common/util.hpp"
#include "proxy/message.hpp"
#include "proxy/msgq_tport.hpp"

#include <cstdio>
#include <errno.h>
#include <string.h>

__CLKMGR_NAMESPACE_USE;

using namespace std;

static PosixMessageQueue mqNativeListenerDesc;

LISTENER_CONTEXT_PROCESS_MESSAGE_TYPE(
    ProxyMessageQueueListenerContext::processMessage)
{
    ProxyMessage *msg = dynamic_cast<decltype(msg)>(bmsg);
    PrintDebug("Processing received proxy message");
    if(msg == nullptr) {
        PrintError("Wrong message type");
        return false;
    }
    return msg->processMessage(*this, txcontext);
}

CREATE_TRANSMIT_CONTEXT_TYPE(
    ProxyMessageQueueListenerContext::CreateTransmitterContext)
{
    string id((const char *)clientId.data());
    PosixMessageQueue txd;
    if(!txd.TxOpen(id, false)) {
        PrintErrorCode("Failed to open message queue " + id);
        return nullptr;
    }
    PrintDebug("Successfully connected to client " + id);
    return new ProxyMessageQueueTransmitterContext(std::move(txd));
}

bool ProxyMessageQueue::initTransport()
{
    PrintDebug("Initializing Message Queue Proxy Transport...");
    if(!mqNativeListenerDesc.RxOpen(mqProxyName, MAX_CLIENT_COUNT)) {
        PrintErrorCode("mq_open failed");
        return false;
    }
    if(InvalidTransportWorkDesc ==
    (mqListenerDesc = registerWork((TransportWork) {
    MqListenerWork,
    TransportWorkArg(new ProxyMessageQueueListenerContext(mqNativeListenerDesc))
    })))
    return false;
    PrintDebug("Proxy Message queue opened");
    return true;
}

bool ProxyMessageQueue::stopTransport()
{
    PrintDebug("Stopping Message Queue Proxy Transport");
    mq_unlink(mqProxyName.c_str());
    if(mqListenerDesc != InvalidTransportWorkDesc &&
        !InterruptWorker(mqListenerDesc))
        PrintError("Interrupt worker failed");
    return true;
}

bool ProxyMessageQueue::finalizeTransport()
{
    return mqNativeListenerDesc.close();
}
