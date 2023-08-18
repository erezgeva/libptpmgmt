/*! \file message.hpp
    \brief Client message base class. Extended for specific messages such as connect and subscribe.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <cstdint>
#include <map>
#include <memory>

#ifndef CLIENT_MESSAGE_HPP
#define CLIENT_MESSAGE_HPP

#include <client/client_state.hpp>
#include <common/message.hpp>

namespace JClkLibClient
{
        class ClientMessage;
        typedef std::shared_ptr<ClientMessage> MessageX;
        typedef std::unique_ptr<ClientMessage> Message0;

	class ClientMessage : virtual public JClkLibCommon::Message
	{
	protected:
		ClientMessage() : JClkLibCommon::Message() {}
	public:
		static bool init();
	};
}

#endif/*CLIENT_MESSAGE_HPP*/
