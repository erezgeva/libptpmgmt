/*! \file notification_msg.hpp
    \brief Proxy notification message class. Implements proxy specific notification message function.

    (C) Copyright Intel Corporation 2024. All rights reserved. Intel Confidential.
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
		ProxyNotificationMessage() : MESSAGE_NOTIFY() {}
		virtual PROCESS_MESSAGE_TYPE(processMessage);
		bool generateResponse(std::uint8_t *msgBuffer, std::size_t &length,
				      const ClockStatus &status);
		virtual BUILD_TXBUFFER_TYPE(makeBuffer) const;

		/**
		 * @brief Create the ProxyNotificationMessage object
		 * @param msg msg structure to be fill up
		 * @param LxContext proxy transport listener context
		 * @return true
		 */
		static MAKE_RXBUFFER_TYPE(buildMessage);

		/**
		 * @brief Add proxy's NOTIFY_MESSAGE type and its builder to transport layer.
		 * @return true
		 */
		static bool initMessage();
	};
}

#endif/*PROXY_NOTIFICATION_MSG_HPP*/
