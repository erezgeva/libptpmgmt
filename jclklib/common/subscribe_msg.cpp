/*! \file subscribe_msg.cpp
    \brief Common subscribe message implementation. Implements common functions and (de-)serialization

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <common/subscribe_msg.hpp>
#include <common/serialize.hpp>
#include <common/print.hpp>

using namespace JClkLibCommon;
using namespace std;

/*MAKE_RXBUFFER_TYPE(SubscribeMessage::buildMessage)
{
	// Fill in fields

	return true;
}*/


string CommonSubscribeMessage::toString()
{
	string name = ExtractClassName(string(__PRETTY_FUNCTION__),string(__FUNCTION__));
	name += "\n";
	name += Message::toString();
	name += "Client ID: " + string((char *)clientId.data()) + "\n";

	return name;
}

PARSE_RXBUFFER_TYPE(CommonSubscribeMessage::parseBuffer) {
	PrintDebug("[AZU] CommonSubscribeMessage::parseBuffer ");
	if(!Message::parseBuffer(LxContext))
		return false;

	if (!PARSE_RX(FIELD, get_sessionId(), LxContext))
		return false;
	
	if (!PARSE_RX(FIELD,subscription, LxContext))
		return false;

	return true;
}


BUILD_TXBUFFER_TYPE(CommonSubscribeMessage::makeBuffer) const
{
	auto ret = Message::makeBuffer(TxContext); 
	PrintDebug("[AZU] CommonSubscribeMessage::makeBuffer");
	if (!ret)
		return ret;

	PrintDebug("[AZU] CommonSubscribeMessage::makeBuffer - sessionId : " + to_string(c_get_val_sessionId()));
	if (!WRITE_TX(FIELD, c_get_val_sessionId(), TxContext))
		return false;

	PrintDebug("[AZU] CommonSubscribeMessage::makeBuffer - sessionId : " + to_string(c_get_val_sessionId()));
	if (!WRITE_TX(FIELD,subscription,TxContext))
		return false;

	return true;
}

TRANSMIT_MESSAGE_TYPE(CommonSubscribeMessage::transmitMessage)
{
	PrintDebug("[AZU] CommonSubscribeMessage::transmitMessage ");
	if (!presendMessage(&TxContext))
		return false;

	return TxContext.sendBuffer();
}


