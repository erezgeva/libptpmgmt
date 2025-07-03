/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief test internal notification message
 *
 * @author Lai Peter Jun Ann <peter.jun.ann.lai@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include "client/notification_msg.hpp"
#include "client/timebase_state.hpp"
#include "proxy/notification_msg.hpp"
#include "proxy/client.hpp"

using namespace clkmgr;
ClockEventHandler::ClockType Client::clockType;

static ptp_event ptp_data;
void TimeBaseStates::setTimeBaseStatePtp(size_t timeBaseIndex,
    const ptp_event &newEvent)
{
    ptp_data = newEvent;
}

static chrony_event chrony_data;
void TimeBaseStates::setTimeBaseStateSys(size_t timeBaseIndex,
    const chrony_event &newchronyEvent)
{
    chrony_data = newchronyEvent;
}

TEST(NotificationMessage, toProxy)
{
    // Set clock type to PTPClock to trigger PTP data flow
    Client::setClockType(ClockEventHandler::PTPClock);
    // We use the listener buffer
    Buffer &buf = Listener::getSingleListenerInstance().getBuff();
    // Set notification message for transmission
    ClientNotificationMessage cmsg;
    EXPECT_EQ(cmsg.get_msgId(), NOTIFY_MESSAGE);
    cmsg.set_sessionId(65535);
    EXPECT_EQ(cmsg.get_sessionId(), 65535);
    EXPECT_EQ(cmsg.get_msgAck(), ACK_NONE);
    EXPECT_STREQ(cmsg.toString().c_str(),
        "get_msgId(): 2\n"
        "m_msgAck: 0\n");
    // Build message for transmission
    EXPECT_TRUE(cmsg.makeBuffer(buf));
    // Register the proxy side
    reg_message_type<ProxyNotificationMessage>();
    // Perpare buffer for parsing
    buf.setLen(buf.getOffset());
    // Parse send message
    Message *msg = Message::parseBuffer(buf);
    ASSERT_NE(msg, nullptr);
    std::unique_ptr<Message> send_msg(msg);
    ProxyNotificationMessage *ppmsg = dynamic_cast<ProxyNotificationMessage *>(msg);
    ASSERT_NE(ppmsg, nullptr);
    // Check received notification message
    EXPECT_EQ(ppmsg->get_msgId(), NOTIFY_MESSAGE);
    EXPECT_EQ(ppmsg->get_sessionId(), 65535);
    EXPECT_EQ(ppmsg->get_msgAck(), ACK_NONE);
    EXPECT_STREQ(ppmsg->toString().c_str(),
        "get_msgId(): 2\n"
        "m_msgAck: 0\n");
    // Build reply message
    EXPECT_TRUE(ppmsg->makeBuffer(buf));
    // Register the client side
    reg_message_type<ClientNotificationMessage>();
    // Perpare buffer for parsing
    buf.setLen(buf.getOffset());
    // Parse reply
    msg = Message::parseBuffer(buf);
    ASSERT_NE(msg, nullptr);
    send_msg.reset(msg);
    ClientNotificationMessage *pcmsg = dynamic_cast<ClientNotificationMessage *>
        (msg);
    // Check received reply message
    ASSERT_NE(pcmsg, nullptr);
    EXPECT_EQ(pcmsg->get_msgId(), NOTIFY_MESSAGE);
    EXPECT_EQ(pcmsg->get_sessionId(), 65535);
    EXPECT_EQ(pcmsg->get_msgAck(), ACK_NONE);
    EXPECT_STREQ(pcmsg->toString().c_str(),
        "get_msgId(): 2\n"
        "m_msgAck: 0\n");
    EXPECT_TRUE(ptp_data.as_capable);
    EXPECT_EQ(ptp_data.gm_identity[0], 1);
    EXPECT_EQ(ptp_data.master_offset, 12);
    EXPECT_EQ(ptp_data.ptp4l_sync_interval, 10000);
    EXPECT_FALSE(ptp_data.synced_to_primary_clock);
    // No chrony data is received.
    EXPECT_EQ(chrony_data.chrony_offset, 0);
    EXPECT_EQ(chrony_data.chrony_reference_id, 0);
    EXPECT_EQ(chrony_data.polling_interval, 0);
}
