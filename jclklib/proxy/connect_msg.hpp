/*! \file connect_msg.hpp
    \brief Proxy connect message class. Implements proxy specific connect message function.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#ifndef PROXY_CONNECT_MSG_HPP
#define PROXY_CONNECT_MSG_HPP

#include <proxy/message.hpp>
#include <common/connect_msg.hpp>

namespace JClkLibProxy
{
	class ProxyConnectMessage : virtual public ProxyMessage, virtual public JClkLibCommon::CommonConnectMessage
	{
	protected:
		ProxyConnectMessage() : MESSAGE_CONNECT() {};
	public:
		virtual PROCESS_MESSAGE_TYPE(processMessage);
		bool generateResponse(uint8_t *msgBuffer, std::size_t &length,
				      const ClockStatus &status)
		{ return false; }
                static MAKE_RXBUFFER_TYPE(buildMessage);
		static bool initMessage();
	};
}

#endif/*PROXY_CONNECT_MSG_HPP*/
