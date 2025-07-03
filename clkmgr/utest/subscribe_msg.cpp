/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief test internal subscribe message
 *
 * @author Lai Peter Jun Ann <peter.jun.ann.lai@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include "client/subscribe_msg.hpp"
#include "client/timebase_state.hpp"
#include "proxy/subscribe_msg.hpp"
#include "proxy/client.hpp"

using namespace clkmgr;

// Used on ProxySubscribeMessage::parseBufferTail()
bool Client::subscribe(size_t timeBaseIndex, sessionId_t sessionId)
{
    return true;
}

// ProxySubscribeMessage::makeBufferTail
void Client::getPTPEvent(size_t timeBaseIndex, ptp_event &event)
{
    event.asCapable = true;
    event.gmClockUUID = 123;
    event.clockOffset = 12;
    event.syncInterval = 10000;
    event.syncedWithGm = false;
}

void Client::getChronyEvent(size_t timeBaseIndex, chrony_event &chronyEvent)
{
    chronyEvent.clockOffset = 123;
    chronyEvent.gmClockUUID = 456;
    chronyEvent.syncInterval = 500000;
}

// For other messages
bool TimeBaseStates::subscribe(size_t timeBaseIndex,
    const ClockSyncSubscription &newSub)
{
    return true;
}

static ptp_event ptp_data;
static chrony_event chrony_data;
bool TimeBaseStates::subscribeReply(size_t timeBaseIndex,
    const ptp_event &ptpData, const chrony_event &chronyData)
{
    ptp_data = ptpData;
    chrony_data = chronyData;
    return true;
}

TEST(SubscribeMessage, toProxy)
{
    // We use the listener buffer
    Buffer &buf = Listener::getSingleListenerInstance().getBuff();
    // Set subscribe message for transmission
    ClientSubscribeMessage cmsg;
    EXPECT_EQ(cmsg.get_msgId(), SUBSCRIBE_MSG);
    cmsg.set_sessionId(12);
    EXPECT_EQ(cmsg.get_sessionId(), 12);
    EXPECT_EQ(cmsg.get_msgAck(), ACK_NONE);
    EXPECT_STREQ(cmsg.toString().c_str(),
        "clkmgr::SubscribeMessage\n"
        "get_msgId(): 1\n"
        "m_msgAck: 0\n");
    // Build message for transmission
    EXPECT_TRUE(cmsg.makeBuffer(buf));
    // Register the proxy side
    reg_message_type<ProxySubscribeMessage>();
    // Perpare buffer for parsing
    buf.setLen(buf.getOffset());
    // Parse send message
    Message *msg = Message::parseBuffer(buf);
    ASSERT_NE(msg, nullptr);
    std::unique_ptr<Message> send_msg(msg);
    ProxySubscribeMessage *ppmsg = dynamic_cast<ProxySubscribeMessage *>(msg);
    ASSERT_NE(ppmsg, nullptr);
    // Check received subscribe message
    EXPECT_EQ(ppmsg->get_msgId(), SUBSCRIBE_MSG);
    EXPECT_EQ(ppmsg->get_sessionId(), 12);
    EXPECT_EQ(ppmsg->get_msgAck(), ACK_SUCCESS);
    EXPECT_STREQ(ppmsg->toString().c_str(),
        "clkmgr::SubscribeMessage\n"
        "get_msgId(): 1\n"
        "m_msgAck: 1\n");
    // Build reply message
    EXPECT_TRUE(ppmsg->makeBuffer(buf));
    // Register the client side
    reg_message_type<ClientSubscribeMessage>();
    // Perpare buffer for parsing
    buf.setLen(buf.getOffset());
    // Parse reply
    msg = Message::parseBuffer(buf);
    ASSERT_NE(msg, nullptr);
    send_msg.reset(msg);
    ClientSubscribeMessage *pcmsg = dynamic_cast<ClientSubscribeMessage *>(msg);
    // Check received reply message
    ASSERT_NE(pcmsg, nullptr);
    EXPECT_EQ(pcmsg->get_msgId(), SUBSCRIBE_MSG);
    EXPECT_EQ(pcmsg->get_sessionId(), 12);
    EXPECT_EQ(pcmsg->get_msgAck(), ACK_NONE);
    EXPECT_STREQ(pcmsg->toString().c_str(),
        "clkmgr::SubscribeMessage\n"
        "get_msgId(): 1\n"
        "m_msgAck: 0\n");
    EXPECT_TRUE(ptp_data.asCapable);
    EXPECT_EQ(chrony_data.clockOffset, 123);
    EXPECT_EQ(chrony_data.gmClockUUID, 456);
    EXPECT_EQ(ptp_data.gmClockUUID, 123);
    EXPECT_EQ(ptp_data.clockOffset, 12);
    EXPECT_EQ(chrony_data.syncInterval, 500000);
    EXPECT_EQ(ptp_data.syncInterval, 10000);
    EXPECT_FALSE(ptp_data.syncedWithGm);
}
