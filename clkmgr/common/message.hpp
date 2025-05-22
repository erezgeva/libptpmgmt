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

#include "common/msgq_tport.hpp"

#include <functional>
#include <string>
#include <map>

__CLKMGR_NAMESPACE_BEGIN

typedef uint8_t msgAck_t;
enum  : msgAck_t {ACK_FAIL = (msgAck_t) -1, ACK_NONE = 0, ACK_SUCCESS = 1, };

typedef uint8_t msgId_t;
enum : msgId_t {INVALID_MSG = (msgId_t) -1, NULL_MSG = 1, CONNECT_MSG,
    SUBSCRIBE_MSG, NOTIFY_MESSAGE, DISCONNECT_MSG
};

class Message;
typedef std::function<Message *()> AllocMessage_t;

#define MSG_EXTRACT_CLASS_NAME\
    Message::ExtractClassName(__PRETTY_FUNCTION__, __FUNCTION__)

class Message
{
  private:
    static std::map<msgId_t, AllocMessage_t> allocMessageMap;
    msgAck_t m_msgAck = ACK_NONE;
    sessionId_t m_sessionId = InvalidSessionId;

  protected:
    Message() = default;
    bool makeBufferBase(Transmitter &TxContext) const;

  public:
    static std::string ExtractClassName(const std::string &prettyFunction,
        const char *function);

    static Message *buildMessage(Listener &LxContext);
    static void registerMessageType(msgId_t id, AllocMessage_t allocFunc) {
        allocMessageMap[id] = allocFunc;
    }

    virtual ~Message() = default;
    virtual msgId_t get_msgId() const = 0;
    virtual bool processMessage(Listener &LxContext, Transmitter *&TxContext) = 0;
    virtual bool transmitMessage(Transmitter &TxContext) = 0;
    virtual bool makeBuffer(Transmitter &TxContext) const = 0;
    virtual bool parseBuffer(Listener &LxContext);
    virtual std::string toString();

    bool presendMessage(Transmitter &ctx);
    msgAck_t get_msgAck() const { return m_msgAck; }
    void set_msgAck(msgAck_t msgAck) { m_msgAck = msgAck; }
    sessionId_t get_sessionId() const { return m_sessionId; }
    void set_sessionId(sessionId_t sessionId) { m_sessionId = sessionId; }
};

template <typename T> inline void reg_message_type()
{
    Message::registerMessageType(T::get_ClassMsgId(), []() { return new T; });
}

__CLKMGR_NAMESPACE_END

#endif /* COMMON_MESSAGE_HPP */
