/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file message.hpp
 * @brief Common message base class.
 * Extended for specific messages such as connect and subscribe.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#ifndef COMMON_MESSAGE_HPP
#define COMMON_MESSAGE_HPP

#include <functional>
#include <map>
#include <memory>

#include <common/clkmgrtypes.hpp>
#include <common/transport.hpp>

__CLKMGR_NAMESPACE_BEGIN

class Message;
class TransportListenerContext;
class TransportTransmitterContext;

#define MAKE_RXBUFFER_TYPE(name) bool name (Message *&msg, \
    TransportListenerContext &LxContext)
typedef std::function<MAKE_RXBUFFER_TYPE()> BuildMessage_t;

typedef std::pair<msgId_t, BuildMessage_t> parseMsgMapElement_t;

class Message
{
  private:
    static std::map<decltype(parseMsgMapElement_t::first),
           decltype(parseMsgMapElement_t::second)> parseMsgMap;
    msgId_t msgId;
    msgAck_t msgAck;
    sessionId_t sessionId;

  protected:
    Message(decltype(msgId) msgId);
    static bool addMessageType(parseMsgMapElement_t);
    static std::string ExtractClassName(std::string prettyFunction,
        std::string function);
  public:
#define COMMON_PRESEND_MESSAGE_TYPE(name) \
    bool name (TransportTransmitterContext *ctx)
    COMMON_PRESEND_MESSAGE_TYPE(presendMessage);

#define BUILD_TXBUFFER_TYPE(name) \
    bool name (TransportTransmitterContext &TxContext)
    static bool registerBuild(BUILD_TXBUFFER_TYPE(buildMessage));

#define PROCESS_MESSAGE_TYPE(name) \
    bool name(TransportListenerContext &LxContext, \
        TransportTransmitterContext *&TxContext)
    virtual PROCESS_MESSAGE_TYPE(processMessage) = 0;

#define TRANSMIT_MESSAGE_TYPE(name) \
    bool name(TransportTransmitterContext &TxContext)
    virtual TRANSMIT_MESSAGE_TYPE(transmitMessage) = 0;

    static MAKE_RXBUFFER_TYPE(buildMessage);

#define PARSE_RXBUFFER_TYPE(name)                   \
    bool name (TransportListenerContext &LxContext)
    virtual PARSE_RXBUFFER_TYPE(parseBuffer);
    virtual BUILD_TXBUFFER_TYPE(makeBuffer) const;

#define PRIMITIVE_TOSTRING(p) std::string(#p) + ": " + std::to_string(p) + "\n"
    virtual std::string toString();

    virtual ~Message() = default;
    DECLARE_ACCESSOR(msgId);
    DECLARE_ACCESSOR(msgAck);
    DECLARE_ACCESSOR(sessionId);

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
