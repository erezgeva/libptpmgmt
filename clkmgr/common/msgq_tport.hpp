/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Common POSIX message queue transport class.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef COMMON_MSGQ_TPORT_HPP
#define COMMON_MSGQ_TPORT_HPP

#include "common/transport.hpp"
#include "common/util.hpp"

#include <cstdint>
#include <mqueue.h>
#include <string>

#define MESSAGE_QUEUE_PREFIX "/clkmgr"

#define TX_QUEUE_FLAGS (O_WRONLY)
#define RX_QUEUE_FLAGS (O_RDONLY | O_CREAT)
#define RX_QUEUE_MODE  (S_IRUSR | S_IWUSR | S_IWGRP)

__CLKMGR_NAMESPACE_BEGIN

class MessageQueueListenerContext : virtual public TransportListenerContext
{
    friend class MessageQueue;
  private:
    mqd_t mqListenerDesc;
  protected:
    MessageQueueListenerContext(mqd_t mqListenerDesc);
  public:
    virtual ~MessageQueueListenerContext() = default;
};

class MessageQueueTransmitterContext : virtual public
    TransportTransmitterContext
{
    friend class MessageQueue;
  private:
    mqd_t mqTransmitterDesc;
  protected:
    MessageQueueTransmitterContext(mqd_t mqTransmitterDesc);
  public:
    virtual ~MessageQueueTransmitterContext() = default;
    virtual SEND_BUFFER_TYPE(sendBuffer);
};

class MessageQueue : public Transport
{
  protected:
    static std::string const mqProxyName;
    static TransportWorkDesc mqListenerDesc;
    static mqd_t mqNativeListenerDesc;
    static bool MqListenerWork(TransportContext *mqListenerContext);
    static bool MqTransmit(TransportContext *mqTransmitterContext, Message *msg);
  public:
    static bool initTransport() { return true; };
    static bool stopTransport() { return true; };
    static bool finalizeTransport() { return true; };
};

__CLKMGR_NAMESPACE_END

#endif /* COMMON_MSGQ_TPORT_HPP */
