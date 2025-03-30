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
    mqd_t txd = mq_open((char *)clientId.data(), TX_QUEUE_FLAGS | O_NONBLOCK);
    if(txd == -1) {
        PrintErrorCode("Failed to open message queue " + string((
                    const char *)clientId.data()));
        return nullptr;
    }
    PrintDebug("Successfully connected to client " + string((
                const char *)clientId.data()));
    return new ProxyMessageQueueTransmitterContext(txd);
}

ProxyMessageQueueListenerContext::ProxyMessageQueueListenerContext(
    mqd_t mqListenerDesc)
    : MessageQueueListenerContext(mqListenerDesc)
{
}

ProxyMessageQueueTransmitterContext::ProxyMessageQueueTransmitterContext(
    mqd_t mqTransmitterDesc)
    : MessageQueueTransmitterContext(mqTransmitterDesc)
{
}

bool ProxyMessageQueue::initTransport()
{
    struct mq_attr mq_attr;
    mq_attr.mq_flags = 0;
    mq_attr.mq_maxmsg = MAX_CLIENT_COUNT;
    mq_attr.mq_msgsize = sizeof(TransportBuffer);
    PrintDebug("Initializing Message Queue Proxy Transport...");
    mqNativeListenerDesc = mq_open(mqProxyName.c_str(), RX_QUEUE_FLAGS,
            RX_QUEUE_MODE, &mq_attr);
    if(mqNativeListenerDesc == -1) {
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
    return mq_close(mqNativeListenerDesc) != -1;
}
