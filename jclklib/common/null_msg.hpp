/*! \file null_msg.hpp
    \brief Non-functional message class for debug

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#ifndef COMMON_NULL_MSG_HPP
#define COMMON_NULL_MSG_HPP

#include <common/message.hpp>

namespace JClkLibCommon
{
	class CommonNullMessage : virtual public Message
	{
	protected:
	public:
		static msgId_t getMsgId() { return NULL_MSG; }
		static MAKE_RXBUFFER_TYPE(buildMessage);
#define MESSAGE_NULL() JClkLibCommon::Message(JClkLibCommon::NULL_MSG)
		CommonNullMessage() : MESSAGE_NULL() {};
		static bool initMessage();
	};
}

#endif/*COMMON_NULL_MSG_HPP*/
