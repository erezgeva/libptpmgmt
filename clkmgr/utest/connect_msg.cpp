/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief test internal connect message
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include "pub/clkmgr/timebase_configs.h"
#include "client/connect_msg.hpp"
#include "client/client_state.hpp"
#include "client/timebase_state.hpp"
#include "proxy/connect_msg.hpp"
#include "proxy/client.hpp"
#include "proxy/config_parser.hpp"

using namespace clkmgr;
using namespace std;

// Used on ProxyConnectMessage::parseBufferTail()
sessionId_t Client::connect(sessionId_t sessionId, const string &id)
{
    return sessionId + 2;
}
// ProxyConnectMessage::makeBufferTail
bool JsonConfigParser::process_json(const string &file)
{
    timeBaseCfgs.push_back({{
            .timeBaseIndex = 1,
            .timeBaseName = { 'm', 'e', 0 },
            .interfaceName = { 'e', 't', 'h', '0', 0 },
            .transportSpecific = 7,
            .domainNumber = 9
        }});
    return true;
}
JsonConfigParser &JsonConfigParser::getInstance()
{
    static JsonConfigParser me;
    me.process_json("");
    return me;
}

// Used in ClientConnectMessage::parseBufferTail()
static TimeBaseCfg lastCfg;
void TimeBaseConfigurations::addTimeBaseCfg(const struct TimeBaseCfg &cfg)
{
    lastCfg = cfg;
}
bool ClientState::connectReply(sessionId_t sessionId) { return sessionId == 14; }

// For linking
Transmitter *Transmitter::getTransmitterInstance(sessionId_t sessionId)
{
    static Transmitter me;
    return &me;
}

TEST(ConnectMessage, toProxy)
{
    // We use the listener buffer
    Buffer &buf = Listener::getSingleListenerInstance().getBuff();
    // Set connect message for transmission
    ClientConnectMessage cmsg;
    EXPECT_EQ(cmsg.get_msgId(), CONNECT_MSG);
    cmsg.setClientId("test");
    cmsg.set_sessionId(12);
    EXPECT_EQ(cmsg.get_sessionId(), 12);
    EXPECT_EQ(cmsg.get_msgAck(), ACK_NONE);
    EXPECT_STREQ(cmsg.toString().c_str(),
        "clkmgr::ConnectMessage\n"
        "get_msgId(): 0\n"
        "m_msgAck: 0\n"
        "Client ID: test\n");
    // Build message for transmission
    EXPECT_TRUE(cmsg.makeBuffer(buf));
    // Register the proxy side
    reg_message_type<ProxyConnectMessage>();
    // Perpare buffer for parsing
    buf.setLen(buf.getOffset());
    // Parse send message
    Message *msg = Message::parseBuffer(buf);
    ASSERT_NE(msg, nullptr);
    unique_ptr<Message> send_msg(msg);
    ProxyConnectMessage *ppmsg = dynamic_cast<ProxyConnectMessage *>(msg);
    ASSERT_NE(ppmsg, nullptr);
    // Check received connect message
    EXPECT_EQ(ppmsg->get_msgId(), CONNECT_MSG);
    const string &id = ppmsg->getClientId();
    EXPECT_EQ(id.size(), CLIENTID_LENGTH);
    EXPECT_STREQ(id.c_str(), "test");
    // Client::connect() add 2 to what was send
    EXPECT_EQ(ppmsg->get_sessionId(), 12 + 2);
    EXPECT_EQ(ppmsg->get_msgAck(), ACK_SUCCESS);
    EXPECT_STREQ(ppmsg->toString().c_str(),
        "clkmgr::ConnectMessage\n"
        "get_msgId(): 0\n"
        "m_msgAck: 1\n"
        "Client ID: test");
    // Build reply message
    EXPECT_TRUE(ppmsg->makeBuffer(buf));
    // Register the client side
    reg_message_type<ClientConnectMessage>();
    // Perpare buffer for parsing
    buf.setLen(buf.getOffset());
    // Parse reply
    msg = Message::parseBuffer(buf);
    ASSERT_NE(msg, nullptr);
    send_msg.reset(msg);
    ClientConnectMessage *pcmsg = dynamic_cast<ClientConnectMessage *>(msg);
    // Check received reply message
    ASSERT_NE(pcmsg, nullptr);
    EXPECT_EQ(pcmsg->get_msgId(), CONNECT_MSG);
    EXPECT_EQ(pcmsg->get_sessionId(), 14);
    EXPECT_EQ(pcmsg->get_msgAck(), ACK_NONE);
    EXPECT_STREQ(pcmsg->toString().c_str(),
        "clkmgr::ConnectMessage\n"
        "get_msgId(): 0\n"
        "m_msgAck: 0\n"
        "Client ID: test");
    EXPECT_EQ(lastCfg.timeBaseIndex, 1);
    EXPECT_STREQ(lastCfg.timeBaseName, "me");
    EXPECT_STREQ(lastCfg.interfaceName, "eth0");
    EXPECT_EQ(lastCfg.transportSpecific, 7);
    EXPECT_EQ(lastCfg.domainNumber, 9);
}
