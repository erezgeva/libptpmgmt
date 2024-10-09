/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief ClientState class unit tests
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#define NSEC_PER_SEC    (1000000000)

#include "clockmanager.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace clkmgr;

TEST(ClientStateTest, ApiTest)
{
    ClientState cstate = {};
    struct timespec current_time = {};
    TransportClientId refClientID = { 0x41, 0x42, 0x43 };
    clkmgr_event_state eState = {};
    clkmgr_event_count eCount = {};
    cstate.set_connected(true);
    EXPECT_EQ(cstate.get_connected(), true);
    cstate.set_subscribed(true);
    EXPECT_EQ(cstate.get_subscribed(), true);
    cstate.set_sessionId(0x3f);
    EXPECT_EQ(cstate.get_sessionId(), 0x3f);
    cstate.set_ptp4l_id(0x55);
    EXPECT_EQ(cstate.get_ptp4l_id(), 0x55);
    cstate.set_clientID(refClientID);
    if(clock_gettime(CLOCK_REALTIME, &current_time) == -1)
        printf("clock_gettime function failure\n");
    else {
        cstate.set_last_notification_time(current_time);
        EXPECT_EQ(cstate.get_last_notification_time().tv_sec, \
            current_time.tv_sec);
        EXPECT_EQ(cstate.get_last_notification_time().tv_nsec, \
            current_time.tv_nsec);
    }
    eState.notification_timestamp = current_time.tv_sec;
    eState.notification_timestamp *= NSEC_PER_SEC;
    eState.notification_timestamp += current_time.tv_nsec;
    eState.as_capable = true;
    eState.offset_in_range = true;
    eState.synced_to_primary_clock = true;
    eState.composite_event = false;
    eState.gm_changed = false;
    eState.clock_offset = 0x1234;
    eCount.as_capable_event_count = 0x2;
    eCount.offset_in_range_event_count = 0x4;
    cstate.set_eventState(eState);
    cstate.set_eventStateCount(eCount);
    EXPECT_EQ(cstate.get_eventStateCount().as_capable_event_count, 0x2);
    EXPECT_EQ(cstate.get_eventStateCount().offset_in_range_event_count, 0x4);
    EXPECT_EQ(cstate.get_eventState().as_capable, true);
    EXPECT_EQ(cstate.get_eventState().composite_event, false);
}
