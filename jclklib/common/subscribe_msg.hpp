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
	class CommonSubscribeMessage : virtual public Message
	{
	private:
		jcl_subscription subscription;
		TransportClientId clientId;

	public:
		static msgId_t getMsgId() { return SUBSCRIBE_MSG; }
		//static MAKE_RXBUFFER_TYPE(buildMessage);
		//const jcl_subscription &getSubscription();
		virtual PARSE_RXBUFFER_TYPE(parseBuffer);
		virtual TRANSMIT_MESSAGE_TYPE(transmitMessage);
		virtual BUILD_TXBUFFER_TYPE(makeBuffer) const;
		jcl_subscription &getSubscription()
		{ return subscription; }
		TransportClientId &getClientId()
		{ return clientId; }
		void setSubscription(jcl_subscription newsub);

		virtual std::string toString();
	protected:
#define MESSAGE_SUBSCRIBE() JClkLibCommon::Message(JClkLibCommon::SUBSCRIBE_MSG)
		CommonSubscribeMessage() : MESSAGE_SUBSCRIBE() {}
	};
}

#endif/*COMMON_SUBSCRIBE_MSG_HPP*/
