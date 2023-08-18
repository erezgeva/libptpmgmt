/*! \file transport.hpp
    \brief Client transport base class for JClkLib. It is extended for specific transports such as POSIX message queue.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <cstdint>
#include <functional>
#include <vector>
#include <memory>
#include <future>
#include <thread>

#ifndef CLIENT_TRANSPORT_HPP
#define CLIENT_TRANSPORT_HPP

#include <common/transport.hpp>
#include <common/util.hpp>
#include <client/message.hpp>

namespace JClkLibClient
{
	class ClientTransportContext : virtual public JClkLibCommon::TransportContext {
	public:
		virtual ~ClientTransportContext() = default;
	};

#define SEND_CLIENT_MESSAGE(name)					\
        bool name (::JClkLibCommon::Message *msg)

	class ClientTransport : public JClkLibCommon::Transport
	{
	protected:
                static bool processMessage(JClkLibCommon::Message *msg) { return false; }
	public:
		static bool init();
		static bool stop();
		static bool finalize();
		static void writeTransportClientId(Message0 &msg) {}
		static SEND_CLIENT_MESSAGE(sendMessage) { return false; }
	};
}

#endif/*CLIENT_TRANSPORT_HPP*/
