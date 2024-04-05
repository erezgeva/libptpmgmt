/*! \file message.cpp
    \brief Client message base class. Extended for specific messages such as connect and subscribe.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <client/message.hpp>
#include <client/null_msg.hpp>
#include <client/connect_msg.hpp>
#include <client/subscribe_msg.hpp>
#include <common/print.hpp>

using namespace JClkLibClient;

bool ClientMessage::init()
{
	PrintDebug("Initializing Client Message");
    return JClkLibCommon::_initMessage<ClientNullMessage,ClientConnectMessage, ClientSubscribeMessage>();
}
