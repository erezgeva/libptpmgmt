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
    size_t m_offset = 0; // Offset in buffer during parsing and building
    size_t m_rcvSize = 0; // Length of the received message

  protected:
    uint8_t m_buffer[MAX_BUFFER_LENGTH];

    // Set length of received message and reset the offset for parsing
    void setLen(size_t rcvSize) { m_rcvSize = rcvSize; m_offset = 0; }

  public:
    Buffer() = default;
    virtual ~Buffer() = default;
    static size_t size() { return MAX_BUFFER_LENGTH; }

    // Data pointer for read only!
    const uint8_t *data() const { return m_buffer; }
    size_t getOffset() const { return m_offset; }
    void resetOffset() { m_offset = 0; }
    void addOffset(size_t offset) { m_offset += offset; }
    // pointer to data at offset for parser and build
    uint8_t *dataOff() { return m_buffer + m_offset; }
    // Left data to build of a transmit message
    size_t sizeLeft() const { return size() - m_offset; }
    // Left data to parse of a received message
    size_t lenLeft() const { return m_rcvSize - m_offset; }
};

class Queue
{
  private:
    static const mqd_t invalidMq = (mqd_t) -1;
    mqd_t mq = invalidMq;
    bool rx = false; // Receive or Transmit
    std::string name;
    std::string clientId;
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
    ssize_t receive(const void *ptr, size_t length) const;
    // Remove receive queue
    bool remove();
    // Close queue
    bool close();
    // To string
    operator std::string() const { return std::to_string(mq); }
    // Get string
    std::string str() const { return std::to_string(mq); }
    // Get client ID
    const std::string &getClientId() const { return clientId; }
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

    // Create the singlton
    Listener() : m_retVal(m_promise.get_future()), m_exitVal(false) {}

  public:
    static Listener &getSingleListenerInstance();
    virtual ~Listener() = default;
    bool init(const std::string &name, size_t maxMsg);
    void dispatchLoop();
    bool MqListenerWork();
    Buffer &getBuff() { return *this; }
    std::thread &getThread() { return m_thread; }
    std::string getQueueName() const { return m_listenerQueue.str(); }
    const std::string &getClientId() const { return m_listenerQueue.getClientId(); }
};

class Transmitter
{
  private:
    Queue m_transmitterQueue;

  public:
    Transmitter() = default;
    virtual ~Transmitter() = default;
    bool finalize();
    bool sendBuffer(Buffer &buf);
    bool open(const std::string &name, bool block = true);
    std::string getQueueName() const { return m_transmitterQueue.str(); }
    static Transmitter *getTransmitterInstance(sessionId_t sessionId);
};

__CLKMGR_NAMESPACE_END

#endif /* COMMON_MSGQ_TPORT_HPP */
