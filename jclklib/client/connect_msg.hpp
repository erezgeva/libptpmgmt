/*! \file connect_msg.hpp
    \brief Client connect message class. Implements client specific functionality.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#ifndef CLIENT_CONNECT_MSG_HPP
#define CLIENT_CONNECT_MSG_HPP

#include <common/connect_msg.hpp>
#include <client/message.hpp>

namespace JClkLibClient
{
	class ClientConnectMessage : virtual public JClkLibCommon::CommonConnectMessage,
				     virtual public ClientMessage
	{
	public:
		ClientConnectMessage() : MESSAGE_CONNECT() {};
		virtual PROCESS_MESSAGE_TYPE(processMessage);
		static MAKE_RXBUFFER_TYPE(buildMessage);
		static bool initMessage();
	};
}

#endif/*CLIENT_CONNECT_MSG_HPP*/
