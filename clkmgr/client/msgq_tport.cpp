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

static Listener rxContext;
static Transmitter txContext;
static std::string mqListenerName;

bool ClientMessageQueue::initTransport()
{
    /* Two outstanding messages per client */
    PrintDebug("Initializing Message Queue Client Transport...");
    mqListenerName += mqProxyName + ".";
    mqListenerName += to_string(getpid());
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

bool ClientMessageQueue::stopTransport()
{
    PrintDebug("Stopping Message Queue Client Transport");
    PrintDebug("mqListenerName = " + mqListenerName);
    if(!rxContext.stopTransport()) {
        PrintError("stop rxContext failed");
        return false;
    }
    return true;
}

bool ClientMessageQueue::finalizeTransport()
{
    PrintDebug("Listener queue = " + rxContext.getQueueName());
    PrintDebug("Transmitter Queue = " + txContext.getQueueName());
    return rxContext.finalize() && txContext.finalize();
}

bool ClientMessageQueue::stop()
{
    rxContext.stopSignal();
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
    msg->presendMessage(txContext);
    if(!txContext.sendBuffer())
        return false;
    PrintDebug("[ClientMessageQueue]::sendMessage successful ");
    return true;
}
