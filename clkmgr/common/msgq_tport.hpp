/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Common POSIX message queue transport class.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef COMMON_MSGQ_TPORT_HPP
#define COMMON_MSGQ_TPORT_HPP

#include "common/transport.hpp"

#include <mqueue.h>
#include <string>

#define MESSAGE_QUEUE_PREFIX "/clkmgr"

__CLKMGR_NAMESPACE_BEGIN

class PosixMessageQueue
{
  private:
    static const mqd_t invalidMq = (mqd_t) -1;
    mqd_t mq = invalidMq;
    bool rx = false; // Receive or Transmit
    std::string name;
  public:
    PosixMessageQueue() = default;
    ~PosixMessageQueue() { close(); remove(); }
    // Move Constructor
    PosixMessageQueue(PosixMessageQueue &&other) noexcept;
    // Move Assignment Operator
    PosixMessageQueue &operator=(PosixMessageQueue &&other) noexcept;
    // Delete Copy Constructor
    PosixMessageQueue(const PosixMessageQueue &) = delete;
    // Delete Copy Assignment Operator
    PosixMessageQueue &operator=(const PosixMessageQueue &) = delete;
    // Receive POSIX message queue
    bool RxOpen(const std::string &name, size_t maxMsg);
    // Transmit POSIX message queue
    bool TxOpen(const std::string &name, bool block = true);
    // Is the queue exist
    bool exist() const { return mq != invalidMq; }
    // Receive with Receive only queue
    bool send(const void *ptr, size_t size) const;
    // Transmit Transmit only queue
    bool receive(const void *ptr, size_t length) const;
    // Remove receive queue
    bool remove();
    // Close queue
    bool close();
    // To string
    operator std::string() const { return std::to_string(mq); }
    // Get string
    std::string str() const { return std::to_string(mq); }
};

class MessageQueueListenerContext : virtual public TransportListenerContext
{
  protected:
    friend class MessageQueue;
    const PosixMessageQueue &mqListenerDesc;
    MessageQueueListenerContext(const PosixMessageQueue &q) : mqListenerDesc(q) {}
  public:
    virtual ~MessageQueueListenerContext() = default;
};

class MessageQueueTransmitterContext : virtual public
    TransportTransmitterContext
{
    friend class MessageQueue;
  private:
    const PosixMessageQueue &mqTransmitterDesc;
  protected:
    MessageQueueTransmitterContext(const PosixMessageQueue &q) :
        mqTransmitterDesc(q) {}
  public:
    virtual ~MessageQueueTransmitterContext() = default;
    virtual SEND_BUFFER_TYPE(sendBuffer);
};

class MessageQueue : public Transport
{
  protected:
    static std::string const mqProxyName;
    static TransportWorkDesc mqListenerDesc;
    static bool MqListenerWork(TransportContext *mqListenerContext);
    static bool MqTransmit(TransportContext *mqTransmitterContext, Message *msg);
  public:
    static bool initTransport() { return true; };
    static bool stopTransport() { return true; };
    static bool finalizeTransport() { return true; };
};

__CLKMGR_NAMESPACE_END

#endif /* COMMON_MSGQ_TPORT_HPP */
