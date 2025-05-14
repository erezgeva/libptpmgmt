/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Common POSIX message queue transport implementation.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "common/msgq_tport.hpp"
#include "common/sighandler.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

PosixMessageQueue::PosixMessageQueue(PosixMessageQueue &&other) noexcept
    : mq(other.mq), rx(other.rx), name(std::move(other.name))
{
    other.mq = invalidMq;
    other.rx = false;
    other.name.clear();
}

PosixMessageQueue &PosixMessageQueue::operator=(PosixMessageQueue &&other)
noexcept
{
    if(this != &other) {
        close();
        remove();
        mq = other.mq;
        rx = other.rx;
        name = std::move(other.name);
        other.mq = invalidMq;
        other.rx = false;
        other.name.clear();
    }
    return *this;
}

bool PosixMessageQueue::RxOpen(const string &n, size_t maxMsg)
{
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = maxMsg;
    attr.mq_msgsize = sizeof(TransportBuffer);
    mq = mq_open(n.c_str(), O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR | S_IWGRP, &attr);
    if(exist()) {
        rx = true;
        name = n;
        return true;
    }
    return false;
}

bool PosixMessageQueue::TxOpen(const string &n, bool block)
{
    mq = mq_open(n.c_str(), O_WRONLY | (block ? 0 : O_NONBLOCK));
    return exist();
}

bool PosixMessageQueue::send(const void *ptr, size_t size) const
{
    return exist() && !rx ? mq_send(mq, (char *)ptr, size, 0) != -1 : false;
}

bool PosixMessageQueue::receive(const void *ptr, size_t length) const
{
    return exist() &&
        rx ? mq_receive(mq, (char *)ptr, length, nullptr) != -1 : false;
}

bool PosixMessageQueue::remove()
{
    if(!name.empty() && mq_unlink(name.c_str()) != -1) {
        name.clear();
        return true;
    }
    return false;
}

bool PosixMessageQueue::close()
{
    if(exist() && mq_close(mq) != -1) {
        mq = invalidMq;
        return true;
    }
    return false;
}

DECLARE_STATIC(MessageQueue::mqProxyName, MESSAGE_QUEUE_PREFIX);
DECLARE_STATIC(MessageQueue::mqListenerDesc, InvalidTransportWorkDesc);

bool MessageQueue::MqListenerWork(TransportContext *mqListenerContext)
{
    MessageQueueListenerContext *context = dynamic_cast<decltype(context)>
        (mqListenerContext);
    if(context == nullptr) {
        PrintError("Internal Error: Received inappropriate context");
        return false; // Return early since context is null and cannot be used.
    }
    if(context->init() && !EnableSyscallInterruptSignal()) {
        PrintError("Unable to enable interrupts in work process context");
        return false;
    }
    if(!context->mqListenerDesc.receive(context->get_buffer().data(),
            context->getc_buffer().max_size())) {
        if(errno != EINTR) {
            PrintError("MQ Receive Failed", errno);
            return false;
        }
        return true;
    }
    PrintDebug("Receive complete");
    DumpOctetArray("Received Message", context->getc_buffer().data(),
        context->getc_buffer().max_size());
    Transport::processMessage(*context);
    return true;
}

bool MessageQueueTransmitterContext::sendBuffer()
{
    if(!mqTransmitterDesc.send(get_buffer().data(), get_offset())) {
        PrintErrorCode("Failed to send buffer");
        return false;
    }
    return true;
}
