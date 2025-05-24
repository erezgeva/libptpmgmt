/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Common queue class.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef COMMON_MSGQ_TPORT_HPP
#define COMMON_MSGQ_TPORT_HPP

#include "common/util.hpp"
#include "common/termin.hpp"

#include <mqueue.h>
#include <future>

__CLKMGR_NAMESPACE_BEGIN

static const size_t MAX_BUFFER_LENGTH = 4096;
static const std::string mqProxyName("/clkmgr");

class Buffer
{
  private:
    uint8_t m_buffer[MAX_BUFFER_LENGTH];
    size_t m_offset = 0;

  public:
    Buffer() = default;
    virtual ~Buffer() = default;

    size_t get_offset() { return m_offset; }
    void set_offset(size_t offset) { m_offset = offset; }

    void resetOffset() { m_offset = 0; }
    void addOffset(size_t offset) { m_offset += offset; }

    uint8_t *data() { return m_buffer; }
    static size_t max_size() { return MAX_BUFFER_LENGTH; }
};

class Queue
{
  private:
    static const mqd_t invalidMq = (mqd_t) -1;
    mqd_t mq = invalidMq;
    bool rx = false; // Receive or Transmit
    std::string name;
  public:
    Queue() = default;
    ~Queue() { close(); remove(); }
    // Move Constructor
    Queue(Queue &&other) noexcept;
    // Move Assignment Operator
    Queue &operator=(Queue &&other) noexcept;
    // Delete Copy Constructor
    Queue(const Queue &) = delete;
    // Delete Copy Assignment Operator
    Queue &operator=(const Queue &) = delete;
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

class Listener : public Buffer, public End
{
  private:
    Queue m_listenerQueue;
    std::promise<bool> m_promise;
    std::future<bool> m_retVal;
    std::atomic_bool m_exitVal;
    std::thread m_thread;

    bool isFutureSet();
    // Called by End::stopAll
    bool stop() override final;
    bool finalize() override final;

  public:
    Listener() : m_retVal(m_promise.get_future()), m_exitVal(false) {}
    virtual ~Listener() = default;
    bool init(const std::string &name, size_t maxMsg);
    void dispatchLoop();
    bool MqListenerWork();
    std::thread &getThread() { return m_thread; }
    std::string getQueueName() const { return m_listenerQueue.str(); }
};

class Transmitter : public Buffer
{
  private:
    Queue m_transmitterQueue;

  public:
    Transmitter() = default;
    virtual ~Transmitter() = default;
    bool finalize();
    bool sendBuffer();
    bool open(const std::string &name, bool block = true);
    std::string getQueueName() const { return m_transmitterQueue.str(); }
};

__CLKMGR_NAMESPACE_END

#endif /* COMMON_MSGQ_TPORT_HPP */
