/*! \file subscribe_msg.hpp
    \brief Common subscribe message class. Implements common functions and (de-)serialization

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#ifndef COMMON_SUBSCRIBE_MSG_HPP
#define COMMON_SUBSCRIBE_MSG_HPP

#include <common/message.hpp>
#include <common/jclklib_import.hpp>

namespace JClkLibCommon
{
	class SubscribeMessage : public Message
	{
	public:
		static msgId_t getMsgId() { return SUBSCRIBE_MSG; }
		static MAKE_RXBUFFER_TYPE(buildMessage);

		const jcl_subscription &getSubscription();
	protected:
#define MESSAGE_SUBSCRIBE() JClkLibCommon::Message(JClkLibCommon::SUBSCRIBE_MSG)
		SubscribeMessage() : MESSAGE_SUBSCRIBE() {}
	private:
		jcl_subscription subscription;
	};
}

#endif/*COMMON_SUBSCRIBE_MSG_HPP*/
