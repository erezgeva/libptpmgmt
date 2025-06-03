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

Message::Message() : rxBuf(Listener::getSingleListenerInstance().getBuff())
{
}

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
string Message::toString() const
{
    string ret;
    ret += PRIMITIVE_TOSTRING(get_msgId());
    ret += PRIMITIVE_TOSTRING(m_msgAck);
    return ret;
}

bool Message::makeBuffer(Buffer &buff) const
{
    PrintDebug("[Message]::makeBuffer");
    buff.resetOffset();
    if(!WRITE_TX(get_msgId(), buff) ||
        !WRITE_TX(m_msgAck, buff) ||
        !makeBufferComm(buff) || !makeBufferTail(buff)) {
        PrintError("Failed to make buffer from message object");
        return false;
    }
    DumpOctetArray("Sending message (length = " +
        to_string(buff.getOffset()) + "): ", buff.data(), buff.getOffset());
    PrintDebug("[Message]::makeBuffer successful");
    return true;
}

bool Message::parseBuffer()
{
    return PARSE_RX(m_msgAck, rxBuf) &&
        parseBufferComm() && parseBufferTail();
}

Message *Message::parseBuffer(Buffer &rxBuf)
{
    msgId_t msgId;
    if(!PARSE_RX(msgId, rxBuf))
        return nullptr;
    if(allocMessageMap.count(msgId) == 0) {
        PrintError("Unknown message type " + to_string(msgId));
        return nullptr;
    }
    Message *msg = allocMessageMap[msgId]();
    if(msg != nullptr) {
        if(msg->parseBuffer())
            return msg;
        PrintError("Error parsing message");
        delete msg;
    } else
        PrintError("Failing allocating message");
    return nullptr;
}
