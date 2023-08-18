/*! \file notification_msg.hpp
    \brief Common notification message class. Implements common functions and (de-)serialization

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <cstdint>

#ifndef COMMON_NOTIFICATION_MSG_HPP
#define COMMON_NOTIFICATION_MSG_HPP

#include <common/message.hpp>
#include <common/jclklib_import.hpp>

namespace JClkLibCommon
{
	class NotificationMessage : public Message
	{
	public:
		static msgId_t getMsgId() { return SUBSCRIBE_MSG; }
		static MAKE_RXBUFFER_TYPE(buildMessage);

		const jcl_eventcount	&getEventCount();
		const jcl_event		&getEvent();
		bool			 isEnable() { return waitEnable == 0x1; }
	protected:
#define MESSAGE_NOTIFY() JClkLibCommon::Message(JClkLibCommon::NOTIFY_MESSAGE)
		NotificationMessage() : MESSAGE_NOTIFY() {}
	private:
		std::uint32_t	waitEnable :1;
		jcl_event	event;
		jcl_eventcount	eventCount;
	};
}

#endif/*COMMON_NOTIFICATION_MSG_HPP*/
