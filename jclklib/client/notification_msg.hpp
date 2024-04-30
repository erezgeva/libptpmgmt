/*! \file notification_msg.hpp
    \brief Client notification message class. Implements client specific notification message function.

    (C) Copyright Intel Corporation 2024. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <cstdint>

#ifndef CLIENT_NOTIFICATION_MSG_HPP
#define CLIENT_NOTIFICATION_MSG_HPP

#include <client/message.hpp>
#include <common/notification_msg.hpp>
#include <common/jclklib_import.hpp>

namespace JClkLibClient
{
	class ClientNotificationMessage : virtual public ClientMessage,
					 virtual public JClkLibCommon::NotificationMessage
	{
	public:
		virtual PROCESS_MESSAGE_TYPE(processMessage);
		virtual BUILD_TXBUFFER_TYPE(makeBuffer) const;

		/**
		 * @brief Create the ClientNotificationMessage object
		 * @param msg msg structure to be fill up
		 * @param LxContext client transport listener context
		 * @return true
		 */
		static MAKE_RXBUFFER_TYPE(buildMessage);

		/**
		 * @brief Add client's NOTIFY_MESSAGE type and its builder to transport layer.
		 * @return true
		 */
		static bool initMessage();

		virtual PARSE_RXBUFFER_TYPE(parseBuffer);

	protected:
		ClientNotificationMessage() : MESSAGE_NOTIFY() {}
	};
}

#endif/*CLIENT_NOTIFICATION_MSG_HPP*/
