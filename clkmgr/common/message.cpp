/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Common message base implementation.
 * Extended for specific messages such as connect and subscribe.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "common/message.hpp"
#include "common/serialize.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

DECLARE_STATIC(Message::parseMsgMap);

Message::Message(msgId_t msgId)
{
    this->msgId = msgId;
    this->msgAck = ACK_NONE;
    this->sessionId = InvalidSessionId;
}

string Message::ExtractClassName(string prettyFunction, string function)
{
    const auto &fpos = prettyFunction.find(function);
    if(fpos == string::npos)
        return prettyFunction;
    auto spos = prettyFunction.rfind(" ", fpos);
    ++spos;
    if(spos == string::npos || spos >= fpos)
        return prettyFunction;
    string ret = prettyFunction.substr(spos, fpos - spos);
    while(!ret.empty() && ret.back() == ':')
        ret.pop_back();
    return ret.empty() ? prettyFunction : ret;
}

#define PRIMITIVE_TOSTRING(p) #p ": " + to_string(p) + "\n"
string Message::toString()
{
    string ret;
    ret += PRIMITIVE_TOSTRING(msgId);
    ret += PRIMITIVE_TOSTRING(msgAck);
    return ret;
}

bool Message::makeBuffer(TransportTransmitterContext &TxContext) const
{
    PrintDebug("[Message]::makeBuffer");
    if(!WRITE_TX(FIELD, msgId, TxContext))
        return false;
    if(!WRITE_TX(FIELD, msgAck, TxContext))
        return false;
    return true;
}

bool Message::presendMessage(TransportTransmitterContext *ctx)
{
    PrintDebug("[Message]::presendMessage starts");
    ctx->resetOffset();
    if(!makeBuffer(*ctx)) {
        PrintError("Failed to make buffer from message object");
        return false;
    }
    DumpOctetArray("Sending message (length = " + to_string(ctx->getc_offset()) +
        "): ", ctx->getc_buffer().data(), ctx->getc_offset());
    PrintDebug("[Message]::presendMessage successful");
    return true;
}

bool Message::addMessageType(parseMsgMapElement_t mapping)
{
    auto size = parseMsgMap.size();
    parseMsgMap.insert(mapping);
    if(parseMsgMap.size() == size)
        return false;
    PrintDebug("Added message type: " + to_string(mapping.first));
    return true;
}

bool Message::parseBuffer(TransportListenerContext &LxContext)
{
    PrintDebug("[Message]::parseBuffer ");
    if(!PARSE_RX(FIELD, msgId, LxContext))
        return false;
    if(!PARSE_RX(FIELD, msgAck, LxContext))
        return false;
    return true;
}

bool Message::buildMessage(Message *&msg, TransportListenerContext &LxContext)
{
    msgId_t msgId;
    if(!PARSE_RX(FIELD, msgId, LxContext))
        return false;
    const auto &it = parseMsgMap.find(msgId);
    if(it == parseMsgMap.cend()) {
        PrintError("Unknown message type " + to_string(msgId));
        return false;
    }
    if(!it->second(msg, LxContext)) {
        PrintError("Error parsing message");
        return false;
    }
    LxContext.resetOffset();
    if(!msg->parseBuffer(LxContext))
        return false;
    return true;
}
