/*! \file connect_msg.hpp
    \brief Common connect message class. Implements common functions and (de-)serialization

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <cstdint>

#ifndef COMMON_CONNECT_MSG_HPP
#define COMMON_CONNECT_MSG_HPP

#include <common/message.hpp>
#include <common/transport.hpp>

namespace JClkLibCommon
{
	class CommonConnectMessage : virtual public Message
	{
	private:
		TransportClientId clientId;
	protected:
#define MESSAGE_CONNECT() JClkLibCommon::Message(JClkLibCommon::CONNECT_MSG)
		CommonConnectMessage() : MESSAGE_CONNECT() {}
	public:
		TransportClientId &getClientId() { return clientId; }
		virtual PARSE_RXBUFFER_TYPE(parseBuffer);
		static bool registerBuild();
		virtual TRANSMIT_MESSAGE_TYPE(transmitMessage);
		virtual BUILD_TXBUFFER_TYPE(makeBuffer) const;
		virtual std::string toString();
	};
}

#endif/*COMMON_CONNECT_MSG_HPP*/
