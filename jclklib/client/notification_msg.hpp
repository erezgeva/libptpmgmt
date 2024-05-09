/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file notification_msg.hpp
 * @brief Client notification message class. Implements client specific notification message function.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
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

		static void addClientState(ClientState *newClientState);
		static void deleteClientState(ClientState *newClientState);

	protected:
		ClientNotificationMessage() : MESSAGE_NOTIFY() {}

	private:
		inline static std::vector<ClientState *> ClientStateArray;

		// proxy_data can be 1 only. it is ok.
		JClkLibCommon::ptp_event proxy_data = {};

	};
}

#endif/*CLIENT_NOTIFICATION_MSG_HPP*/
