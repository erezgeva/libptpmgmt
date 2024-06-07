/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file msgq_tport.cpp
 * @brief Common POSIX message queue transport implementation.
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

#include <common/msgq_tport.hpp>
#include <common/message.hpp>
#include <common/print.hpp>
#include <common/sighandler.hpp>
#include <common/util.hpp>

using namespace JClkLibCommon;
using namespace std;

#define QUEUE_FLAGS (O_RDONLY | O_CREAT)
#define QUEUE_MODE  (S_IRUSR  | S_IWGRP)

#define NSEC_PER_MSEC	(1000000 /*ns*/)
#define NSEC_PER_SEC	(1000000000 /*ns*/)

DECLARE_STATIC(MessageQueue::mqProxyName,MESSAGE_QUEUE_PREFIX);
DECLARE_STATIC(MessageQueue::mqListenerDesc,Transport::InvalidTransportWorkDesc);
DECLARE_STATIC(MessageQueue::mqNativeListenerDesc,-1);

MessageQueueListenerContext::MessageQueueListenerContext(mqd_t mqListenerDesc)
{
    this->mqListenerDesc = mqListenerDesc;
}

MessageQueueTransmitterContext::MessageQueueTransmitterContext(mqd_t mqTransmitterDesc)
{
    this->mqTransmitterDesc = mqTransmitterDesc;
}

int mqRecvWrapper(mqd_t mqDesc, uint8_t *data, size_t length)
{
    int ret;
	
    ret = mq_receive(mqDesc, (char *)data, length, NULL);
    if (ret == -1)
        return -errno;

    return ret;
}

bool MessageQueue::MqListenerWork(TransportContext *mqListenerContext)
{
    MessageQueueListenerContext *context = dynamic_cast<decltype(context)>(mqListenerContext);

    if(!context) {
        PrintError("Internal Error: Received inappropriate context");
        return false; // Return early since context is null and cannot be used.
    }
    if (context->init() && !EnableSyscallInterruptSignal()) {
        PrintError("Unable to enable interrupts in work process context");
        return false;
    }

    int ret = mqRecvWrapper(context->mqListenerDesc, context->get_buffer().data(),
        context->getc_buffer().max_size());
    if (ret < 0) {
        if (ret != -EINTR)
            PrintError("MQ Receive Failed",-ret);
        return ret != -EINTR ? false : true;
    }
    PrintDebug("Receive complete");

    DumpOctetArray("Received Message", context->getc_buffer().data(),
        context->getc_buffer().max_size());

    Transport::processMessage(*context);
	
    return true;
}

SEND_BUFFER_TYPE(MessageQueueTransmitterContext::sendBuffer)
{
    if (mq_send(mqTransmitterDesc, (char *)get_buffer().data(), get_offset(), 0) == -1) {
        PrintErrorCode("Failed to send buffer");
        return false;
    }

    return true;
}
