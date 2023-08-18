/*! \file msgq_tport.hpp
    \brief Proxy POSIX message queue transport class.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <cstdint>
#include <string>
#include <future>
#include <memory>

#include <mqueue.h>

#ifndef PROXY_MSGQ_TPORT_HPP
#define PROXY_MSGQ_TPORT_HPP

#include <proxy/transport.hpp>
#include <common/msgq_tport.hpp>
#include <common/util.hpp>

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

#endif/*PROXY_MSGQ_TPORT_HPP*/
