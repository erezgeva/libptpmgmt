/*! \file subscribe_msg.hpp
    \brief Proxy subscribe message class. Implements proxy specific subscribe message function.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#ifndef PROXY_SUBSCRIBE_MSG_HPP
#define PROXY_SUBSCRIBE_MSG_HPP

#include <proxy/message.hpp>
#include <common/subscribe_msg.hpp>

namespace JClkLibProxy
{
	class ProxySubscribeMessage : virtual public ProxyMessage, virtual public JClkLibCommon::SubscribeMessage
	{
	public:
		bool processMessage(ClockConfiguration &config);
		bool generateResponse(uint8_t *msgBuffer, std::size_t &length,
				      const ClockStatus &status);

	protected:
		ProxySubscribeMessage() : MESSAGE_SUBSCRIBE() {};
	};
}

#endif/*SUBSCRIBE_MSG_HPP*/
