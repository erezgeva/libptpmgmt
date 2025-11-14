/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief test internal disconnect message
 *
 * @author Goh Wei Sheng <wei.sheng.goh@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include "client/disconnect_msg.hpp"
#include "client/client_state.hpp"
#include "proxy/disconnect_msg.hpp"
#include "proxy/client.hpp"

using namespace clkmgr;

// Used to define static members in ClientState class
DECLARE_STATIC(ClientState::m_connected, false);

static sessionId_t removed_session_id = 0;

void Client::removeClient(sessionId_t sessionId)
{
    removed_session_id = sessionId;
}

TEST(DisconnectMessage, toProxy)
{
    // Use the listener buffer
    Buffer &buf = Listener::getSingleListenerInstance().getBuff();
    // Set disconnect message for transmission
    ClientDisconnectMessage cmsg;
    EXPECT_EQ(cmsg.get_msgId(), DISCONNECT_MSG);
    cmsg.set_sessionId(42);
    EXPECT_EQ(cmsg.get_sessionId(), 42);
    EXPECT_EQ(cmsg.get_msgAck(), ACK_NONE);
    EXPECT_STREQ(cmsg.toString().c_str(),
        "get_msgId(): 3\n"
        "m_msgAck: 0\n");
    // Build message for transmission
    EXPECT_TRUE(cmsg.makeBuffer(buf));
    // Register the proxy side
    reg_message_type<ProxyDisconnectMessage>();
    // Perpare buffer and send to proxy
    buf.setLen(buf.getOffset());
    Message *msg = Message::parseBuffer(buf);
    ASSERT_NE(msg, nullptr);
    std::unique_ptr<Message> send_msg(msg);
    ProxyDisconnectMessage *ppmsg = dynamic_cast<ProxyDisconnectMessage *>(msg);
    ASSERT_NE(ppmsg, nullptr);
    // Check received disconnect message
    EXPECT_EQ(ppmsg->get_msgId(), DISCONNECT_MSG);
    EXPECT_EQ(ppmsg->get_sessionId(), 42);
    EXPECT_EQ(ppmsg->get_msgAck(), ACK_NONE);
    EXPECT_STREQ(ppmsg->toString().c_str(),
        "get_msgId(): 3\n"
        "m_msgAck: 0\n");
    // Verify that the client was removed with the correct session ID
    EXPECT_EQ(removed_session_id, 42);
}
