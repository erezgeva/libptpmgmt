/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief ClientState class unit tests
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "pub/clockmanager.h"

using namespace clkmgr;

// Tests copy constructor
// ClientState(const ClientState &newState)
TEST(ClientStateTest, copy_constructor)
{
    ClientState cstate1;
    cstate1.set_connected(true);
    ClientState cstate2(cstate1);
    EXPECT_TRUE(cstate2.get_connected());
}

// Tests copy setting from another object
// void set_clientState(const ClientState &newState)
TEST(ClientStateTest, set_clientState)
{
    ClientState cstate1;
    cstate1.set_connected(true);
    ClientState cstate2;
    EXPECT_FALSE(cstate2.get_connected());
    cstate2.set_clientState(cstate1);
    EXPECT_TRUE(cstate2.get_connected());
}

// Tests connected status flag
// bool get_connected() const
// void set_connected(bool state)
TEST(ClientStateTest, connected)
{
    ClientState cstate;
    cstate.set_connected(true);
    EXPECT_TRUE(cstate.get_connected());
}

// Tests subscribed status flag
// bool get_subscribed() const
// void set_subscribed(bool subscriptionState)
TEST(ClientStateTest, subscribed)
{
    ClientState cstate;
    cstate.set_subscribed(true);
    EXPECT_TRUE(cstate.get_subscribed());
}

// Tests Transport client ID
// TransportClientId get_clientID() const
// void set_clientID(const TransportClientId &cID)
TEST(ClientStateTest, clientID)
{
    ClientState cstate;
    TransportClientId refClientID = { 0x41, 0x42, 0x43 };
    cstate.set_clientID(refClientID);
    //EXPECT_EQ(cstate.get_clientID(), { 0x41, 0x42, 0x43} );  // TODO
}

// Tests event counters
// const Event_count &get_eventStateCount()
// void set_eventStateCount(const Event_count &eCount)
TEST(ClientStateTest, eventStateCount)
{
    ClientState cstate;
    Event_count eCount = {};
    eCount.as_capable_event_count = 0x2;
    eCount.offset_in_range_event_count = 0x4;
    cstate.set_eventStateCount(eCount);
    const Event_count &e = cstate.get_eventStateCount();
    EXPECT_EQ(e.as_capable_event_count, 0x2);
    EXPECT_EQ(e.offset_in_range_event_count, 0x4);
}

// Tests event state
// Event_state &get_eventState()
// void set_eventState(const Event_state &eState)
TEST(ClientStateTest, eventState)
{
    ClientState cstate;
    Event_state eState = {};
    eState.notification_timestamp = 141000268448;
    eState.as_capable = true;
    eState.offset_in_range = true;
    eState.synced_to_primary_clock = true;
    eState.composite_event = false;
    eState.gm_changed = false;
    eState.clock_offset = 0x1234;
    cstate.set_eventState(eState);
    Event_state &e = cstate.get_eventState();
    EXPECT_EQ(e.notification_timestamp, 141000268448);
    EXPECT_EQ(e.clock_offset, 0x1234);
    EXPECT_TRUE(e.offset_in_range);
    EXPECT_TRUE(e.synced_to_primary_clock);
    EXPECT_TRUE(e.as_capable);
    EXPECT_FALSE(e.gm_changed);
    EXPECT_FALSE(e.composite_event);
}

// Tests last notification timestamp
// void set_last_notification_time(const struct timespec &last_notification_time)
// struct timespec get_last_notification_time() const
TEST(ClientStateTest, notification_time)
{
    ClientState cstate;
    cstate.set_last_notification_time({14, 5019});
    struct timespec c = cstate.get_last_notification_time();
    EXPECT_EQ(c.tv_sec, 14);
    EXPECT_EQ(c.tv_nsec, 5019);
}

// Tests stringify
// std::string toString() const
TEST(ClientStateTest, toString)
{
    ClientState cstate;
    Event_state &es = cstate.get_eventState();
    es.as_capable = true;
    es.gm_changed = true;
    es.offset_in_range = true;
    es.synced_to_primary_clock = true;
    EXPECT_STREQ(cstate.toString().c_str(),
        "[ClientState::eventState] as_capable = 1 gm_changed = 1 "
        "offset_in_range = 1 synced_to_primary_clock = 1\n");
}

// Tests event subscription masks
// TODO
// const ClkMgrSubscription &get_eventSub()
// void set_eventSub(const ClkMgrSubscription &eSub)

// Tests session ID
// sessionId_t get_sessionId() const
// void set_sessionId(sessionId_t sessionId)
TEST(ClientStateTest, sessionId)
{
    ClientState cstate;
    cstate.set_sessionId(0x3f);
    EXPECT_EQ(cstate.get_sessionId(), 0x3f);
}
