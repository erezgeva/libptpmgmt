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

DECLARE_STATIC(Message::allocMessageMap);

string Message::ExtractClassName(const string &prettyFunction,
    const char *function)
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
    ret += PRIMITIVE_TOSTRING(get_msgId());
    ret += PRIMITIVE_TOSTRING(m_msgAck);
    return ret;
}

bool Message::makeBufferBase(Transmitter &txContext) const
{
    PrintDebug("[Message]::makeBufferBase");
    if(!WRITE_TX(FIELD, get_msgId(), txContext))
        return false;
    if(!WRITE_TX(FIELD, m_msgAck, txContext))
        return false;
    return true;
}

bool Message::presendMessage(Transmitter &ctx)
{
    PrintDebug("[Message]::presendMessage starts");
    ctx.resetOffset();
    if(!makeBuffer(ctx)) {
        PrintError("Failed to make buffer from message object");
        return false;
    }
    DumpOctetArray("Sending message (length = " + to_string(ctx.get_offset()) +
        "): ", ctx.data(), ctx.get_offset());
    PrintDebug("[Message]::presendMessage successful");
    return true;
}

bool Message::parseBuffer(Listener &rxContext)
{
    PrintDebug("[Message]::parseBuffer ");
    msgId_t msgId;
    if(!PARSE_RX(FIELD, msgId, rxContext))
        return false;
    if(msgId != get_msgId()) {
        PrintError("Wrong message type " + to_string(msgId));
        return false;
    }
    if(!PARSE_RX(FIELD, m_msgAck, rxContext))
        return false;
    return true;
}

Message *Message::buildMessage(Listener &rxContext)
{
    msgId_t msgId;
    if(!PARSE_RX(FIELD, msgId, rxContext))
        return nullptr;
    if(allocMessageMap.count(msgId) == 0) {
        PrintError("Unknown message type " + to_string(msgId));
        return nullptr;
    }
    Message *msg = allocMessageMap[msgId]();
    if(msg == nullptr) {
        PrintError("Error parsing message");
        return nullptr;
    }
    rxContext.resetOffset();
    if(!msg->parseBuffer(rxContext)) {
        delete msg;
        return nullptr;
    }
    return msg;
}
