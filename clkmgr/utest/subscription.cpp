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

using namespace clkmgr;

// Tests composite event mask
// ClockSyncSubscription()
// void enablePtpSubscription()
// void disablePtpSubscription()
// bool isPTPSubscriptionEnable() const
// void setPtpSubscription(const PTPClockSubscription &newPtpSub)
// const PTPClockSubscription &getPtpSubscription() const
// void PTPClockSubscription::setCompositeEventMask(uint32_t composite_event_mask)
// uint32_t PTPClockSubscription::getCompositeEventMask() const
TEST(SubscriptionTest, compositeEventMask)
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
    copy.setCompositeEventMask(0x1234);
    subscription.setPtpSubscription(copy);
    EXPECT_EQ(subscription.getPtpSubscription().getCompositeEventMask(), 0x1234);
    EXPECT_EQ(org.getCompositeEventMask(), 0x1234);
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
