/*! \file transport.hpp
    \brief Transport base class for JClkLib Proxy. It is extended for specific transports such as POSIX message queue.
    
    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <cstdint>
#include <functional>
#include <vector>
#include <memory>
#include <future>
#include <thread>

#ifndef PROXY_TRANSPORT_HPP
#define PROXY_TRANSPORT_HPP

#include <proxy/clock_config.hpp>
#include <common/transport.hpp>
#include <proxy/message.hpp>
#include <proxy/connect_msg.hpp>
#include <common/util.hpp>

namespace JClkLibProxy
{
	class ProxyTransportTransmitterContext : virtual public JClkLibCommon::TransportTransmitterContext {
	protected:
		ProxyTransportTransmitterContext() {}
	public:
		virtual ~ProxyTransportTransmitterContext() = default;
	};

	class ProxyTransportListenerContext : virtual public JClkLibCommon::TransportListenerContext {
	public:
		virtual ~ProxyTransportListenerContext() = default;
	};
	

	class ProxyTransport : public JClkLibCommon::Transport
	{
	public:
		static bool init();
		static bool stop();
		static bool finalize();
#define SEND_PROXY_MESSAGE(name)					\
		bool name (const JClkLibProxy::ProxyMessage *msg)
		static SEND_PROXY_MESSAGE(sendMessage) { return false; }
	};
}

#endif/*PROXY_TRANSPORT_HPP*/
