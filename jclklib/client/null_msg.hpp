/*! \file null_msg.hpp
   \brief Non-functional message class for debug

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <client/message.hpp>
#include <common/null_msg.hpp>

#ifndef CLIENT_NULL_MSG_HPP
#define CLIENT_NULL_MSG_HPP

namespace JClkLibClient
{
	class ClientNullMessage : public JClkLibClient::ClientMessage,
				  public JClkLibCommon::CommonNullMessage
	{
	protected:
	public:
		static bool initMessage() { return true; }
		ClientNullMessage() {}
	};
}

#endif/*CLIENT_NULL_MSG_HPP*/
