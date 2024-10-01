/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Subscription class unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "clockmanager.h"

using namespace clkmgr;

// Tests event_mask
// void set_event_mask(const uint32_t &varname)
// uint32_t get_event_mask() const
TEST(SubscriptionTest, MacroMajor)
{
    ClkMgrSubscription subscription = {};
    uint32_t events = eventGMOffset | eventSyncedToGM;
    subscription.set_event_mask(events);
    EXPECT_EQ(subscription.get_event_mask(), events);
    EXPECT_TRUE(subscription.get_event_mask() & eventSyncedToGM);
    EXPECT_FALSE(subscription.get_event_mask() & eventASCapable);
}
