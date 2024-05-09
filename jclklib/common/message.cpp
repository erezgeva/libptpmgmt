/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file message.cpp
 * @brief Common message base implementation. Extended for specific messages such as connect and subscribe.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#include <common/serialize.hpp>
#include <common/null_msg.hpp>
#include <common/connect_msg.hpp>
#include <common/serialize.hpp>
#include <common/print.hpp>
#include <common/subscribe_msg.hpp>

using namespace JClkLibCommon;
using namespace std;

DECLARE_STATIC(Message::parseMsgMap);

Message::Message(decltype(msgId) msgId)
 {
	 this->msgId = msgId;
	 this->msgAck = ACK_NONE;
	 this->sessionId = InvalidSessionId;
 }

string Message::ExtractClassName(string prettyFunction, string function)
{
	auto fpos = prettyFunction.find(function);
	auto spos = fpos;
	if (fpos == string::npos)
		return prettyFunction;
	spos = prettyFunction.rfind(" ",fpos);
	++spos;
	if (spos == string::npos || spos >= fpos)
		return prettyFunction;
	auto ret = prettyFunction.substr(spos,fpos-spos);
	while(ret.length() > 0 && ret.back() == ':')
		ret.pop_back();
	return ret.length() != 0 ? ret : prettyFunction;
}

string Message::toString()
{
	string ret;

	ret += PRIMITIVE_TOSTRING(msgId);
	ret += PRIMITIVE_TOSTRING(msgAck);

	return ret;
}

BUILD_TXBUFFER_TYPE(Message::makeBuffer) const
{
	PrintDebug("[Message]::makeBuffer");
	if (!WRITE_TX(FIELD,msgId,TxContext))
		return false;
	if (!WRITE_TX(FIELD,msgAck,TxContext))
		return false;

	return true;
}

COMMON_PRESEND_MESSAGE_TYPE(Message::presendMessage)
{
	PrintDebug("[Message]::presendMessage starts");
	ctx->resetOffset();
	if (!makeBuffer(*ctx)) {
		PrintError("Failed to make buffer from message object");
		return false;
	}
	DumpOctetArray("Sending message (length = " + to_string(ctx->getc_offset()) + "): ", ctx->getc_buffer().data(),
	       ctx->getc_offset());
	PrintDebug("[Message]::presendMessage successful");

	return true;
}


bool Message::addMessageType(parseMsgMapElement_t mapping)
{
	decltype(parseMsgMap)::size_type size = parseMsgMap.size();
	parseMsgMap.insert(mapping);
	if (parseMsgMap.size() == size)
		return false;

	PrintDebug("Added message type: " + to_string(mapping.first));
	
	return true;
}

PARSE_RXBUFFER_TYPE(Message::parseBuffer)
{
	PrintDebug("[Message]::parseBuffer ");
	if (!PARSE_RX(FIELD,msgId, LxContext))
		return false;
	if (!PARSE_RX(FIELD,msgAck, LxContext))
		return false;

	return true;
}

MAKE_RXBUFFER_TYPE(Message::buildMessage)
{
	msgId_t msgId;
	decltype(parseMsgMap)::iterator it;

	if (!PARSE_RX(FIELD,msgId,LxContext))
		return false;
	
	it = parseMsgMap.find(msgId);
	if (it == parseMsgMap.cend()) {
		PrintError("Unknown message type " + to_string(msgId));
		return false;
	}
	if (!it->second(msg, LxContext)) {
		PrintError("Error parsing message");
		return false;
	}
	LxContext.resetOffset();
	if (!msg->parseBuffer(LxContext))
		return false;

	return true;
}
