/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file subscribe_msg.hpp
 * @brief Proxy subscribe message class. Implements proxy specific subscribe message function.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#ifndef PROXY_SUBSCRIBE_MSG_HPP
#define PROXY_SUBSCRIBE_MSG_HPP

#include <proxy/message.hpp>
#include <common/subscribe_msg.hpp>

namespace JClkLibProxy
{
	class ProxySubscribeMessage : virtual public ProxyMessage, virtual public JClkLibCommon::CommonSubscribeMessage
	{
	protected:
		ProxySubscribeMessage() : MESSAGE_SUBSCRIBE() {};
	public:
		//bool processMessage(ClockConfiguration &config);
		virtual PROCESS_MESSAGE_TYPE(processMessage);
		virtual BUILD_TXBUFFER_TYPE(makeBuffer) const;
		bool generateResponse(uint8_t *msgBuffer, std::size_t &length,
				      const ClockStatus &status);
		/**
		 * @brief Create the ProxyConnectMessage object
		 * @param msg msg structure to be fill up
		 * @param LxContext proxy transport listener context
		 * @return true
		 */
		static MAKE_RXBUFFER_TYPE(buildMessage);

		/**
		 * @brief Add proxy's CONNECT_MSG type and its builder to transport layer.
		 * @return true
		 */
		static bool initMessage();
	};
}

#endif/*PROXY_SUBSCRIBE_MSG_HPP*/
