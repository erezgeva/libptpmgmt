/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy POSIX message queue transport implementation.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "proxy/msgq_tport.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

static PosixMessageQueue mqNativeListenerDesc;

bool ProxyMessageQueueListenerContext::processMessage(Message *bmsg,
    TransportTransmitterContext *&txcontext)
{
    ProxyMessage *msg = dynamic_cast<decltype(msg)>(bmsg);
    PrintDebug("Processing received proxy message");
    if(msg == nullptr) {
        PrintError("Wrong message type");
        return false;
    }
    return msg->processMessage(*this, txcontext);
}

TransportTransmitterContext
*ProxyMessageQueueListenerContext::CreateTransmitterContext(
    TransportClientId &clientId)
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
        (mqListenerDesc = registerWork(MqListenerWork,
                    new ProxyMessageQueueListenerContext(mqNativeListenerDesc))))
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
