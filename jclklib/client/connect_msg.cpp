/*! \file connect_msg.cpp
    \brief Client connect message class. Implements client specific functionality.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <client/connect_msg.hpp>
#include <common/print.hpp>

using namespace JClkLibClient;
using namespace JClkLibCommon;
using namespace std;

MAKE_RXBUFFER_TYPE(ClientConnectMessage::buildMessage)
{
        msg = new ClientConnectMessage();

        return true;
}

bool ClientConnectMessage::initMessage()
{
        addMessageType(parseMsgMapElement_t(CONNECT_MSG, buildMessage));
        return true;
}

PROCESS_MESSAGE_TYPE(ClientConnectMessage::processMessage)
{
        sessionId_t newSessionId;

        PrintDebug("Processing client connect message (reply)");

	state.set_connected(true);
	state.set_sessionId(this->get_sessionId());

        PrintDebug("Connected with session ID: " + to_string(this->get_sessionId()));

	this->set_msgAck(ACK_NONE);
        return true;
}
