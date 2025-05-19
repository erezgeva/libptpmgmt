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
#include <future>

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
  private:
    PosixMessageQueue m_listenerQueue;
    std::promise<bool> m_promise;
    std::future<bool> m_retVal;
    std::atomic_bool m_exitVal;
    std::thread m_thread;

    bool isFutureSet();

  public:
    MessageQueueListenerContext() : m_retVal(m_promise.get_future()),
        m_exitVal(false) {}
    bool init(const std::string &name, size_t maxMsg);
    void dispatchLoop();
    void stopSignal() { m_exitVal.store(true); }
    bool finalize();
    bool stopTransport();
    std::thread &getThread() { return m_thread; }
    bool MqListenerWork();
    std::string getQueueName() const { return m_listenerQueue.str(); }
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
    virtual bool sendBuffer();
};

class MessageQueue : public Transport
{
  protected:
    static std::string const mqProxyName;
    static bool MqTransmit(TransportContext *mqTransmitterContext, Message *msg);
  public:
    static bool initTransport() { return true; };
    static bool stopTransport() { return true; };
    static bool finalizeTransport() { return true; };
};

__CLKMGR_NAMESPACE_END

#endif /* COMMON_MSGQ_TPORT_HPP */
