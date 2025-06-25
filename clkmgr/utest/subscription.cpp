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

// Test ClockSubscriptionBase
// ClockSubscriptionBase()
// bool setEventMask(uint32_t newEventMask)
// uint32_t getEventMask() const
// void setClockOffsetThreshold(uint32_t threshold)
// uint32_t getClockOffsetThreshold() const
TEST(SubscriptionTest, clockSubscriptionBase)
{
    ClockSubscriptionBase sub;
    // Test default values
    EXPECT_EQ(sub.getEventMask(), 0u);
    EXPECT_EQ(sub.getClockOffsetThreshold(), 0u);
    // Test setting event mask and clock offset threshold
    EXPECT_TRUE(sub.setEventMask(0xA5A5A5A5));
    EXPECT_EQ(sub.getEventMask(), 0xA5A5A5A5u);
    sub.setClockOffsetThreshold(12345);
    EXPECT_EQ(sub.getClockOffsetThreshold(), 12345u);
}

// Test PTP clock subscription
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
TEST(SubscriptionTest, ptpClock)
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
    EXPECT_FALSE(copy.setEventMask(PTP_EVENT_ALL + 1));
    EXPECT_TRUE(copy.setEventMask(PTP_EVENT_ALL));
    EXPECT_FALSE(copy.setCompositeEventMask(PTP_COMPOSITE_EVENT_ALL + 1));
    EXPECT_TRUE(copy.setCompositeEventMask(PTP_COMPOSITE_EVENT_ALL));
    subscription.setPtpSubscription(copy);
    EXPECT_EQ(subscription.getPtpSubscription().getEventMask(), PTP_EVENT_ALL);
    EXPECT_EQ(subscription.getPtpSubscription().getCompositeEventMask(),
        PTP_COMPOSITE_EVENT_ALL);
    EXPECT_EQ(org.getEventMask(), PTP_EVENT_ALL);
    EXPECT_EQ(org.getCompositeEventMask(), PTP_COMPOSITE_EVENT_ALL);
}

// Test system clock subscription
// void enableSysSubscription()
// void disableSysSubscription()
// bool isSysSubscriptionEnable() const
// void setSysSubscription(const SysClockSubscription &newSysSub)
// const SysClockSubscription &getSysSubscription() const
// bool SysClockSubscription::setEventMask(uint32_t event_mask)
// uint32_t SysClockSubscription::getEventMask() const
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
    EXPECT_FALSE(copy.setEventMask(SYS_EVENT_ALL + 1));
    EXPECT_TRUE(copy.setEventMask(SYS_EVENT_ALL));
    subscription.setSysSubscription(copy);
    EXPECT_EQ(subscription.getSysSubscription().getEventMask(), SYS_EVENT_ALL);
    EXPECT_EQ(org.getEventMask(), SYS_EVENT_ALL);
}
