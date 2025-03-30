/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy POSIX message queue transport class.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef PROXY_MSGQ_TPORT_HPP
#define PROXY_MSGQ_TPORT_HPP

#include "common/msgq_tport.hpp"
#include "common/util.hpp"
#include "proxy/transport.hpp"

#include <cstdint>
#include <string>

#define MAX_CLIENT_COUNT    (8)

__CLKMGR_NAMESPACE_BEGIN

class ProxyMessageQueueListenerContext : virtual public
    MessageQueueListenerContext,
    virtual public ProxyTransportListenerContext
{
    friend class ProxyMessageQueue;
  protected:
    virtual LISTENER_CONTEXT_PROCESS_MESSAGE_TYPE(processMessage);
    ProxyMessageQueueListenerContext(mqd_t mqListenerDesc);
  public:
    virtual ~ProxyMessageQueueListenerContext() = default;
    virtual CREATE_TRANSMIT_CONTEXT_TYPE(CreateTransmitterContext);
};

class ProxyMessageQueueTransmitterContext  : virtual public
    MessageQueueTransmitterContext,
    virtual public ProxyTransportTransmitterContext
{
    friend class ProxyMessageQueue;
    friend class ProxyMessageQueueListenerContext;
  protected:
    ProxyMessageQueueTransmitterContext(mqd_t mqTransmitterDesc);
  public:
    virtual ~ProxyMessageQueueTransmitterContext() = default;
};

class ProxyMessageQueue : public MessageQueue,
    public ProxyTransport
{
  public:
    static bool initTransport();
    static bool stopTransport();
    static bool finalizeTransport();
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_MSGQ_TPORT_HPP */
