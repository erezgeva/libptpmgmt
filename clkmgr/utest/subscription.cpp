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

// Tests event mask
// void set_event_mask(const uint32_t &varname)
// uint32_t get_event_mask() const
TEST(SubscriptionTest, event_mask)
{
    ClkMgrSubscription subscription;
    uint32_t events = eventGMOffset | eventSyncedToGM;
    subscription.set_event_mask(events);
    EXPECT_EQ(subscription.get_event_mask(), events);
    EXPECT_TRUE(subscription.get_event_mask() & eventSyncedToGM);
    EXPECT_FALSE(subscription.get_event_mask() & eventASCapable);
}

// Tests composite event mask
// void set_composite_event_mask(uint32_t composite_event_mask)
// uint32_t get_composite_event_mask() const
TEST(SubscriptionTest, composite_event_mask)
{
    ClkMgrSubscription subscription;
    subscription.set_composite_event_mask(0x7945);
    EXPECT_EQ(subscription.get_composite_event_mask(), 0x7945);
}

// Tests threshold
// const threshold_t &get_threshold() const
// void set_threshold(const threshold_t &threshold)
// bool define_threshold(ThresholdIndex index, int32_t upper, int32_t lower)
// bool in_range(ThresholdIndex index, int32_t value) const
TEST(SubscriptionTest, threshold)
{
    ClkMgrSubscription subscription;
    // Index out of range
    EXPECT_FALSE(subscription.define_threshold(thresholdLast, 10, 0));
    // Wrong range
    EXPECT_FALSE(subscription.define_threshold(thresholdGMOffset, 5, 10));
    // Proper
    EXPECT_TRUE(subscription.define_threshold(thresholdGMOffset, 10, 5));
    const threshold_t &t = subscription.get_threshold();
    ClkMgrSubscription subscription2;
    subscription2.set_threshold(t);
    // Index out of range
    EXPECT_FALSE(subscription2.in_range(thresholdLast, 0));
    // value out of range
    EXPECT_FALSE(subscription2.in_range(thresholdGMOffset, 20));
    // Proper
    EXPECT_TRUE(subscription2.in_range(thresholdGMOffset, 8));
}
