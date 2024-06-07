/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file msgq_tport.hpp
 * @brief Proxy POSIX message queue transport class.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#ifndef PROXY_MSGQ_TPORT_HPP
#define PROXY_MSGQ_TPORT_HPP

#include <cstdint>
#include <future>
#include <memory>
#include <mqueue.h>
#include <string>

#include <common/msgq_tport.hpp>
#include <common/util.hpp>
#include <proxy/transport.hpp>

#define MAX_CLIENT_COUNT	(8)

namespace JClkLibProxy
{
    class ProxyMessageQueueListenerContext : virtual public JClkLibCommon::MessageQueueListenerContext,
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

    class ProxyMessageQueueTransmitterContext  : virtual public JClkLibCommon::MessageQueueTransmitterContext,
        virtual public ProxyTransportTransmitterContext
    {
        friend class ProxyMessageQueue;
        friend class ProxyMessageQueueListenerContext;
    protected:
        ProxyMessageQueueTransmitterContext(mqd_t mqTransmitterDesc);
    public:
        virtual ~ProxyMessageQueueTransmitterContext() = default;
};

    class ProxyMessageQueue : public JClkLibCommon::MessageQueue, public ProxyTransport
    {
    public:
        static bool initTransport();
        static bool stopTransport();
        static bool finalizeTransport();
    };
}

#endif /* PROXY_MSGQ_TPORT_HPP */
