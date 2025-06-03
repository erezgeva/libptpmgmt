/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Subscription class unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "pub/clockmanager.h"
#include "pub/clkmgr/types.h"

using namespace clkmgr;

// Tests event mask
// ClockSyncSubscription()
// void enablePtpSubscription()
// void disablePtpSubscription()
// bool isPTPSubscriptionEnable() const
// void setPtpSubscription(const PTPClockSubscription &newPtpSub)
// const PTPClockSubscription &getPtpSubscription() const
// bool PTPClockSubscription::setEventMask(uint32_t event_mask)
// uint32_t PTPClockSubscription::getEventMask() const
// bool PTPClockSubscription::setCompositeEventMask(uint32_t composite_event_mask)
// uint32_t PTPClockSubscription::getCompositeEventMask() const
TEST(SubscriptionTest, setAndGet)
{
    ClockSyncSubscription subscription;
    EXPECT_FALSE(subscription.isPTPSubscriptionEnable());
    subscription.enablePtpSubscription();
    EXPECT_TRUE(subscription.isPTPSubscriptionEnable());
    subscription.disablePtpSubscription();
    EXPECT_FALSE(subscription.isPTPSubscriptionEnable());
    const PTPClockSubscription &org = subscription.getPtpSubscription();
    PTPClockSubscription copy = org;
    EXPECT_EQ(org.getCompositeEventMask(), 0);
    EXPECT_FALSE(copy.setEventMask(eventLast));
    EXPECT_TRUE(copy.setEventMask(eventLast - 1));
    EXPECT_FALSE(copy.setCompositeEventMask(COMPOSITE_EVENT_ALL + 1));
    EXPECT_TRUE(copy.setCompositeEventMask(COMPOSITE_EVENT_ALL));
    subscription.setPtpSubscription(copy);
    EXPECT_EQ(subscription.getPtpSubscription().getEventMask(), eventLast - 1);
    EXPECT_EQ(subscription.getPtpSubscription().getCompositeEventMask(),
        COMPOSITE_EVENT_ALL);
    EXPECT_EQ(org.getEventMask(), eventLast - 1);
    EXPECT_EQ(org.getCompositeEventMask(), COMPOSITE_EVENT_ALL);
}

// Test system clock subscription
// void disableSysSubscription()
// void enableSysSubscription()
// bool isSysSubscriptionEnable() const
// void setSysSubscription(const SysClockSubscription &newSysSub)
// const SysClockSubscription &getSysSubscription() const
TEST(SubscriptionTest, systemClock)
{
    ClockSyncSubscription subscription;
    EXPECT_FALSE(subscription.isSysSubscriptionEnable());
    subscription.enableSysSubscription();
    EXPECT_TRUE(subscription.isSysSubscriptionEnable());
    subscription.disableSysSubscription();
    EXPECT_FALSE(subscription.isSysSubscriptionEnable());
    const SysClockSubscription &org = subscription.getSysSubscription();
    SysClockSubscription copy = org;
    subscription.setSysSubscription(copy);
}
