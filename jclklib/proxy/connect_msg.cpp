/*! \file connect_msg.cpp
    \brief Proxy connect message implementation. Implements proxy specific connect message function.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <proxy/connect_msg.hpp>
#include <proxy/client.hpp>
#include <common/print.hpp>

using namespace std;
using namespace JClkLibProxy;
using namespace JClkLibCommon;

MAKE_RXBUFFER_TYPE(ProxyConnectMessage::buildMessage)
{
	msg = new ProxyConnectMessage();

	return true;
}

bool ProxyConnectMessage::initMessage()
{
        addMessageType(parseMsgMapElement_t(CONNECT_MSG, buildMessage));
        return true;
}

PROCESS_MESSAGE_TYPE(ProxyConnectMessage::processMessage)
{
	sessionId_t newSessionId;

	PrintDebug("Processing proxy connect message");

	if (this->getc_sessionId() != InvalidSessionId) {
		PrintError("Session ID *should be* invalid for received proxy connect message");
		return false;
	}

	newSessionId = Client::CreateClientSession();
	PrintDebug("Created new client session ID: " + to_string(newSessionId));
	this->set_sessionId(newSessionId);
	TxContext = LxContext.CreateTransmitterContext(getClientId());
	Client::GetClientSession(newSessionId).get()->set_transmitContext(TxContext);
	set_msgAck(ACK_SUCCESS);

	return true;
}

