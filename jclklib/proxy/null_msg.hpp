/*! \file null_msg.hpp
    \brief Proxy null message class. Used for debug only. Should not be used.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#ifndef PROXY_NULL_MSG_HPP
#define PROXY_NULL_MSG_HPP

#include <proxy/message.hpp>
#include <common/null_msg.hpp>

namespace JClkLibProxy
{
	class ProxyNullMessage : virtual public ProxyMessage, virtual public JClkLibCommon::CommonNullMessage
	{
	protected:
		ProxyNullMessage() : MESSAGE_NULL() {};
	public:
		bool processMessage(ClockConfiguration &config) { return true; }
		bool generateResponse(uint8_t *msgBuffer, std::size_t &length,
				      const ClockStatus &status)
		{ return false; }
		static bool initMessage() { return true; }
	};
}

#endif/*PROXY_NULL_MSG_HPP*/
