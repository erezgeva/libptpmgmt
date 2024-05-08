/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file null_msg.hpp
 * @brief Proxy null message class. Used for debug only. Should not be used.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
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
