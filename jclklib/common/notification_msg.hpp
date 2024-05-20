/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file notification_msg.hpp
 * @brief Common notification message class. Implements common functions and (de-)serialization
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#include <cstdint>

#ifndef COMMON_NOTIFICATION_MSG_HPP
#define COMMON_NOTIFICATION_MSG_HPP

#include <common/message.hpp>
#include <common/jclklib_import.hpp>
#include <common/transport.hpp>

namespace JClkLibCommon
{
	class NotificationMessage : virtual public Message
	{
	public:
		virtual TRANSMIT_MESSAGE_TYPE(transmitMessage);
		static msgId_t getMsgId() { return SUBSCRIBE_MSG; }

		//const jcl_eventcount	&getEventCount();
		//const jcl_event		&getEvent();
		bool			 isEnable() { return waitEnable == 0x1; }
	protected:
#define MESSAGE_NOTIFY() JClkLibCommon::Message(JClkLibCommon::NOTIFY_MESSAGE)
		NotificationMessage() : MESSAGE_NOTIFY() , waitEnable(0) {}
	private:
		std::uint32_t	waitEnable :1;
		//jcl_event	event;
		//jcl_eventcount	eventCount;
	};
}

#endif/*COMMON_NOTIFICATION_MSG_HPP*/
