/*! \file notification_msg.hpp
    \brief Proxy connect message class. Implements proxy specific connect message function.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <cstdint>

#ifndef PROXY_NOTIFICATION_MSG_HPP
#define PROXY_NOTIFICATION_MSG_HPP

#include <proxy/message.hpp>
#include <common/notification_msg.hpp>
#include <common/jclklib_import.hpp>

namespace JClkLibProxy
{
	class ProxyNotificationMessage : virtual public ProxyMessage,
					 virtual public JClkLibCommon::NotificationMessage
	{
	public:
		bool processMessage(ClockConfiguration &config);
		bool generateResponse(std::uint8_t *msgBuffer, std::size_t &length,
				      const ClockStatus &status);

	protected:
		ProxyNotificationMessage() : MESSAGE_NOTIFY() {}
	};
}

#endif/*PROXY_NOTIFICATION_MSG_HPP*/
