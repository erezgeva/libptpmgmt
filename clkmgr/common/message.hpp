/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Common message base class.
 * Extended for specific messages such as connect and subscribe.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef COMMON_MESSAGE_HPP
#define COMMON_MESSAGE_HPP

#include "common/clkmgrtypes.hpp"
#include "common/transport.hpp"

#include <functional>
#include <string>
#include <map>

__CLKMGR_NAMESPACE_BEGIN

class Message;
class TransportListenerContext;
class TransportTransmitterContext;

typedef std::function<bool (Message *&, TransportListenerContext &)>
BuildMessage_t;

typedef std::pair<msgId_t, BuildMessage_t> parseMsgMapElement_t;

class Message
{
  private:
    static std::map<msgId_t, BuildMessage_t> parseMsgMap;
    msgId_t msgId;
    msgAck_t msgAck;
    sessionId_t sessionId;

  protected:
    Message(msgId_t msgId);
    static bool addMessageType(parseMsgMapElement_t);
    static std::string ExtractClassName(std::string prettyFunction,
        std::string function);
  public:
    bool presendMessage(TransportTransmitterContext *ctx);

    virtual bool processMessage(TransportListenerContext &LxContext,
        TransportTransmitterContext *&TxContext) = 0;

    virtual bool transmitMessage(TransportTransmitterContext &TxContext) = 0;

    static bool buildMessage(Message *&msg, TransportListenerContext &LxContext);

    virtual bool parseBuffer(TransportListenerContext &LxContext);
    virtual bool makeBuffer(TransportTransmitterContext &TxContext) const;

    virtual std::string toString();

    virtual ~Message() = default;

    const msgId_t &getc_msgId() { return msgId; }
    msgId_t &get_msgId() { return msgId; }
    void set_msgId(const msgId_t &msgId) { this->msgId = msgId; }
    msgId_t c_get_val_msgId() const { return msgId; }

    const msgAck_t &getc_msgAck() { return msgAck; }
    msgAck_t &get_msgAck() { return msgAck; }
    void set_msgAck(const msgAck_t &msgAck) { this->msgAck = msgAck; }
    msgAck_t c_get_val_msgAck() const { return msgAck; }

    const sessionId_t &getc_sessionId() { return sessionId; }
    sessionId_t &get_sessionId() { return sessionId; }
    void set_sessionId(const sessionId_t &sessionId) {
        this->sessionId = sessionId;
    }
    sessionId_t c_get_val_sessionId() const { return sessionId; }

    static bool initMessage() { return false; };
    static bool init() { return false; }
};

template <typename T>
inline bool _initMessage()
{
    return T::initMessage();
}

template <typename T, typename... Types>
inline typename std::enable_if < sizeof...(Types) != 0,
       bool >::type _initMessage()
{
    return _initMessage<T>() && _initMessage<Types...>();
}

__CLKMGR_NAMESPACE_END

#endif /* COMMON_MESSAGE_HPP */
