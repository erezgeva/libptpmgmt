/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file transport.hpp
 * @brief Proxy transport base class for JClkLib. It provides common functionality for derived transport classes.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
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
