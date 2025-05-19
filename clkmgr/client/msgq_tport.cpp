/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client POSIX message queue transport class.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "client/msgq_tport.hpp"
#include "client/connect_msg.hpp"
#include "client/subscribe_msg.hpp"
#include "common/print.hpp"

#include <cstring>
#include <unistd.h>

__CLKMGR_NAMESPACE_USE;

using namespace std;

static MessageQueueListenerContext listenerQueue;
static PosixMessageQueue mqNativeClientTransmitterDesc;

DECLARE_STATIC(ClientMessageQueue::mqListenerName, "");
DECLARE_STATIC(ClientMessageQueue::txContext);

bool ClientMessageQueue::initTransport()
{
    /* Two outstanding messages per client */
    PrintDebug("Initializing Message Queue Client Transport...");
    mqListenerName += mqProxyName + ".";
    mqListenerName += to_string(getpid());
    if(!listenerQueue.init(mqListenerName, 2)) {
        PrintError("Failed to open listener queue");
        return false;
    }
    if(!mqNativeClientTransmitterDesc.TxOpen(mqProxyName)) {
        PrintErrorCode("Failed to open transmitter queue: " + mqProxyName);
        return false;
    }
    txContext.reset(new ClientMessageQueueTransmitterContext(
            mqNativeClientTransmitterDesc));
    PrintDebug("Client Message queue opened");
    return true;
}

bool ClientMessageQueue::stopTransport()
{
    PrintDebug("Stopping Message Queue Client Transport");
    PrintDebug("mqListenerName = " + mqListenerName);
    if(!listenerQueue.stopTransport()) {
        PrintError("stop listenerQueue failed");
        return false;
    }
    return true;
}

bool ClientMessageQueue::finalizeTransport()
{
    PrintDebug("mqNativeListenerDesc = " + listenerQueue.getQueueName());
    PrintDebug("mqNativeClientTransmitterDesc = " +
        mqNativeClientTransmitterDesc.str());
    return listenerQueue.finalize() && mqNativeClientTransmitterDesc.close();
}

bool ClientMessageQueue::stop()
{
    listenerQueue.stopSignal();
    return true;
}

bool ClientMessageQueue::writeTransportClientId(Message *msg)
{
    msgId_t msgId = msg->get_msgId();
    switch(msgId) {
        case CONNECT_MSG : {
            ClientConnectMessage *cmsg = dynamic_cast<decltype(cmsg)>(msg);
            if(cmsg == nullptr) {
                PrintErrorCode("[ClientMessageQueue] ClientConnectMessage "
                    "cmsg is nullptr!!");
                return false;
            }
            memcpy(cmsg->getClientId().data(), mqListenerName.c_str(),
                TRANSPORT_CLIENTID_LENGTH);
            break;
        }
        case SUBSCRIBE_MSG : {
            ClientSubscribeMessage *cmsg = dynamic_cast<decltype(cmsg)>(msg);
            if(cmsg == nullptr) {
                PrintErrorCode("[ClientMessageQueue] ClientSubscribeMessage "
                    "cmsg is nullptr!!");
                return false;
            }
            PrintDebug("[ClientMessageQueue] [SUBSCRIBE] : "
                "subscription->event Mask : " +
                to_string(cmsg->getSubscription().get_event_mask()));
            break;
        }
        default: {
            PrintErrorCode("Unknown msgID type");
            return false;
        }
    }
    return true;
}

bool ClientMessageQueue::sendMessage(Message *msg)
{
    auto context = txContext.get();
    msg->presendMessage(context);
    if(!mqNativeClientTransmitterDesc.send(context->getc_buffer().data(),
            context->getc_offset())) {
        PrintErrorCode("Failed to transmit client message");
        return false;
    }
    PrintDebug("[ClientMessageQueue]::sendMessage successful ");
    return true;
}
