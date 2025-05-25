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

Message::Message() : rxContext(Listener::getSingleListenerInstance())
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

bool Message::makeBuffer(Transmitter &txContext) const
{
    PrintDebug("[Message]::makeBuffer");
    txContext.resetOffset();
    if(!WRITE_TX(FIELD, get_msgId(), txContext) ||
        !WRITE_TX(FIELD, m_msgAck, txContext) ||
        !makeBufferComm(txContext) || !makeBufferTail(txContext)) {
        PrintError("Failed to make buffer from message object");
        return false;
    }
    DumpOctetArray("Sending message (length = " +
        to_string(txContext.getOffset()) + "): ",
        txContext.data(), txContext.getOffset());
    PrintDebug("[Message]::makeBuffer successful");
    return true;
}

bool Message::transmitMessage()
{
    Transmitter *ptxContext = Transmitter::getTransmitterInstance(m_sessionId);
    return ptxContext != nullptr && makeBuffer(*ptxContext) &&
        ptxContext->sendBuffer();
}

bool Message::parseBuffer()
{
    return PARSE_RX(FIELD, m_msgAck, rxContext) &&
        parseBufferComm() && parseBufferTail();
}

Message *Message::parseBuffer(Listener &rxContext)
{
    msgId_t msgId;
    if(!PARSE_RX(FIELD, msgId, rxContext))
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
