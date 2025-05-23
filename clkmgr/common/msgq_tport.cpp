/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Common queue implementation.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "common/msgq_tport.hpp"
#include "common/message.hpp"
#include "common/sighandler.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

const uint16_t EXIT_TIMEOUT = 200; // milliseconds

Queue::Queue(Queue &&other) noexcept
    : mq(other.mq), rx(other.rx), name(std::move(other.name))
{
    other.mq = invalidMq;
    other.rx = false;
    other.name.clear();
}

Queue &Queue::operator=(Queue &&other) noexcept
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

bool Queue::RxOpen(const string &n, size_t maxMsg)
{
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = maxMsg;
    attr.mq_msgsize = Buffer::size();
    mq = mq_open(n.c_str(), O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR | S_IWGRP, &attr);
    if(exist()) {
        rx = true;
        name = n;
        clientId = name;
        return true;
    }
    return false;
}

bool Queue::TxOpen(const string &n, bool block)
{
    mq = mq_open(n.c_str(), O_WRONLY | (block ? 0 : O_NONBLOCK));
    return exist();
}

bool Queue::send(const void *ptr, size_t size) const
{
    return exist() && !rx ? mq_send(mq, (char *)ptr, size, 0) != -1 : false;
}

ssize_t Queue::receive(const void *ptr, size_t length) const
{
    return exist() && rx ?  mq_receive(mq, (char *)ptr, length, nullptr) : -1;
}

bool Queue::remove()
{
    if(!name.empty() && mq_unlink(name.c_str()) != -1) {
        name.clear();
        return true;
    }
    return false;
}

bool Queue::close()
{
    if(exist() && mq_close(mq) != -1) {
        mq = invalidMq;
        return true;
    }
    return false;
}

Listener &Listener::getSingleListenerInstance()
{
    static Listener listener;
    return listener;
}

bool Listener::isFutureSet()
{
    return m_retVal.valid() &&
        m_retVal.wait_for(chrono::milliseconds(0)) == future_status::ready;
}

void Listener::dispatchLoop()
{
    bool promiseVal = false;
    if(EnableSyscallInterruptSignal())
        PrintDebug("Listener Thread started");
    else
        PrintError("Listener Thread fail"
            " enabling interrupt signal");
    while(!m_exitVal.load()) {
        if(!MqListenerWork())
            goto done;
    }
    promiseVal = true;
done:
    PrintDebug("Listener thread exited");
    m_promise.set_value_at_thread_exit(promiseVal);
    return;
}

static void staticDispatchLoop(Listener *me)
{
    me->dispatchLoop();
}

bool Listener::init(const string &name, size_t maxMsg)
{
    if(!m_listenerQueue.RxOpen(name, maxMsg)) {
        PrintError("Failed to open listener queue: " + name);
        return false;
    }
    m_thread = thread(staticDispatchLoop, this);
    PrintDebug("Thread started");
    if(isFutureSet()) {
        m_thread.join();
        PrintError("Thread exited early");
        return false;
    }
    return true;
}

bool Listener::finalize()
{
    if(m_retVal.wait_for(chrono::milliseconds(EXIT_TIMEOUT)) !=
        future_status::ready) {
        PrintError("Thread Join Timeout");
        return false;
    }
    m_thread.join();
    m_listenerQueue.close();
    m_listenerQueue.remove();
    return true;
}

bool Listener::stop()
{
    PrintDebug("Stopping listener queue");
    m_exitVal.store(true);
    // Thread has exited, no need to interrupt
    if(isFutureSet())
        return true;
    PrintDebug("Sending interrupt to Queue worker");
    return SendSyscallInterruptSignal(m_thread);
}

bool Listener::MqListenerWork()
{
    ssize_t len = m_listenerQueue.receive(m_buffer, size());
    if(len < 0 && errno != EINTR) {
        PrintError("MQ Receive Failed", errno);
        return false;
    }
    // The data length is zero or
    //  we have an interrupted by a signal (EINTR)
    if(len <= 0)
        return true;
    setLen(len); // Set the length in the buffer for parsing
    PrintDebug("Receive complete");
    DumpOctetArray("Received Message", data(), size());
    Message *msg = Message::parseBuffer(*this);
    if(msg == nullptr)
        return false;
    PrintDebug("Received message " + msg->toString());
    // Echo the message back with ACK disposition
    return msg->get_msgAck() != ACK_NONE ? msg->transmitMessage() : true;
}

bool Transmitter::sendBuffer()
{
    if(!m_transmitterQueue.send(m_buffer, getOffset())) {
        PrintErrorCode("Failed to send buffer");
        return false;
    }
    return true;
}

bool Transmitter::open(const std::string &name, bool block)
{
    if(m_transmitterQueue.TxOpen(name, block)) {
        PrintDebug("Successfully connected to client " + name);
        return true;
    }
    PrintErrorCode("Failed to open message queue " + name);
    return false;
}

bool Transmitter::finalize()
{
    return m_transmitterQueue.close();
}
