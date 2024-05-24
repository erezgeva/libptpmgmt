/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file msgq_tport.cpp
 * @brief Proxy POSIX message queue transport implementation.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <mqueue.h>
#include <string.h>
#include <sys/stat.h>
#include <thread>

#include <common/print.hpp>
#include <common/sighandler.hpp>
#include <common/util.hpp>
#include <proxy/message.hpp>
#include <proxy/msgq_tport.hpp>

using namespace JClkLibCommon;
using namespace JClkLibProxy;
using namespace std;

#define NSEC_PER_MSEC	(1000000 /*ns*/)
#define NSEC_PER_SEC	(1000000000 /*ns*/)

LISTENER_CONTEXT_PROCESS_MESSAGE_TYPE(ProxyMessageQueueListenerContext::processMessage)
{
    ProxyMessage *msg = dynamic_cast<decltype(msg)>(bmsg);

    PrintDebug("Processing received proxy message");
    if (msg == NULL) {
        PrintError("Wrong message type");
        return false;
    }

    return msg->processMessage(*this,txcontext);
}

CREATE_TRANSMIT_CONTEXT_TYPE(ProxyMessageQueueListenerContext::CreateTransmitterContext)
{
    mqd_t txd = mq_open((char *)clientId.data(), TX_QUEUE_FLAGS | O_NONBLOCK);
    if (txd == -1) {
        PrintErrorCode("Failed to open message queue " + string((const char*)clientId.data()));
        return NULL;
    }
    PrintDebug("Successfully connected to client " + string((const char*)clientId.data()));
    return new ProxyMessageQueueTransmitterContext(txd);
}

ProxyMessageQueueListenerContext::ProxyMessageQueueListenerContext(mqd_t mqListenerDesc)
    : MessageQueueListenerContext(mqListenerDesc)
{
}

ProxyMessageQueueTransmitterContext::ProxyMessageQueueTransmitterContext(mqd_t mqTransmitterDesc)
    : MessageQueueTransmitterContext(mqTransmitterDesc)
{
}

bool ProxyMessageQueue::initTransport()
{
    struct mq_attr mq_attr;

    mq_attr.mq_flags = 0;
    mq_attr.mq_maxmsg = MAX_CLIENT_COUNT;
    mq_attr.mq_msgsize = (decltype(mq_attr.mq_msgsize)) std::tuple_size<TransportBuffer>::value;

    PrintDebug("Initializing Message Queue Proxy Transport...");
    mqNativeListenerDesc = mq_open(mqProxyName.c_str(), RX_QUEUE_FLAGS, RX_QUEUE_MODE, &mq_attr);
    if (mqNativeListenerDesc == -1) {
        cout << "mq_open failed " << strerror(errno) << endl;
        return false;
    }

    if (InvalidTransportWorkDesc ==
        (mqListenerDesc = registerWork(move((TransportWork){MqListenerWork,
        TransportWorkArg(new ProxyMessageQueueListenerContext(mqNativeListenerDesc))}))))
        return false;
	    
    PrintDebug("Proxy Message queue opened");

    return true;
}

bool ProxyMessageQueue::stopTransport()
{
    PrintDebug("Stopping Message Queue Proxy Transport");
    mq_unlink(mqProxyName.c_str());
    if (mqListenerDesc != InvalidTransportWorkDesc && !InterruptWorker(mqListenerDesc))
        PrintError("Interrupt worker failed");

    return true;
}

bool ProxyMessageQueue::finalizeTransport()
{
    return mq_close(mqNativeListenerDesc) != -1;
}
