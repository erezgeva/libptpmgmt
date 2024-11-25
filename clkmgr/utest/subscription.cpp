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
// void set_ClkMgrSubscription(const ClkMgrSubscription &newSubscription)
TEST(SubscriptionTest, set_ClkMgrSubscription)
{
    ClkMgrSubscription subscription;
    subscription.set_event_mask(eventGMOffset | eventSyncedToGM);
    subscription.set_composite_event_mask(0x7945);
    EXPECT_TRUE(subscription.define_threshold(thresholdGMOffset, 10, 5));
    ClkMgrSubscription subscription2;
    subscription2.set_ClkMgrSubscription(subscription);
    EXPECT_EQ(subscription2.get_event_mask(), eventGMOffset | eventSyncedToGM);
    EXPECT_EQ(subscription2.get_composite_event_mask(), 0x7945);
    EXPECT_TRUE(subscription2.in_range(thresholdGMOffset, 8));
}

// Tests event mask
// void set_event_mask(const uint32_t &varname)
// uint32_t get_event_mask() const
TEST(SubscriptionTest, event_mask)
{
    ClkMgrSubscription subscription;
    subscription.set_event_mask(eventGMOffset | eventSyncedToGM);
    EXPECT_EQ(subscription.get_event_mask(), eventGMOffset | eventSyncedToGM);
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
// bool define_threshold(ThresholdIndex index, int32_t upper, int32_t lower)
// bool get_threshold(ThresholdIndex index, int32_t &upper, int32_t &lower)
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
    // Index out of range
    EXPECT_FALSE(subscription.in_range(thresholdLast, 0));
    // value out of range
    EXPECT_FALSE(subscription.in_range(thresholdGMOffset, 20));
    // Proper
    EXPECT_TRUE(subscription.in_range(thresholdGMOffset, 8));
    int32_t upper = 0, lower = 0;
    EXPECT_FALSE(subscription.get_threshold(thresholdLast, upper, lower));
    EXPECT_TRUE(subscription.get_threshold(thresholdGMOffset, upper, lower));
    EXPECT_EQ(upper, 10);
    EXPECT_EQ(lower, 5);
}
