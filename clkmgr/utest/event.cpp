/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Clock event and data class unit tests
 *
 * @author Song Yoong Siang <yoong.siang.song@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include "pub/clkmgr/event.h"
#include "client/clock_event_handler.hpp"

using namespace clkmgr;

class ClockEventBaseTestable : public ClockEventBase
{
  public:
    ClockEventBaseTestable() : ClockEventBase() {}
};

class PTPClockEventTestable : public PTPClockEvent
{
  public:
    PTPClockEventTestable() : PTPClockEvent() {}
};

class SysClockEventTestable : public SysClockEvent
{
  public:
    SysClockEventTestable() : SysClockEvent() {}
};

// ClockEventBaseTestable::ClockEventBaseTestable()
// int64_t getClockOffset() const
// bool isOffsetInRange() const
// uint32_t getOffsetInRangeEventCount() const
// int64_t getSyncInterval() const
// uint64_t getGmIdentity() const
// bool isGmChanged() const
// uint32_t getGmChangedEventCount() const
// uint64_t getNotificationTimestamp() const
TEST(ClockEventBaseTest, defaultValue)
{
    ClockEventBaseTestable event;
    EXPECT_EQ(event.getClockOffset(), 0);
    EXPECT_TRUE(event.isOffsetInRange());
    EXPECT_EQ(event.getOffsetInRangeEventCount(), 0u);
    EXPECT_EQ(event.getSyncInterval(), 0);
    EXPECT_EQ(event.getGmIdentity(), 0u);
    EXPECT_FALSE(event.isGmChanged());
    EXPECT_EQ(event.getGmChangedEventCount(), 0u);
    EXPECT_EQ(event.getNotificationTimestamp(), 0u);
}

// ClockEventBaseTestable::ClockEventBaseTestable()
// ClockEventHandler::setClockOffset()
// ClockEventHandler::setOffsetInRange()
// ClockEventHandler::setOffsetInRangeEventCount()
// ClockEventHandler::setSyncInterval()
// ClockEventHandler::setGmIdentity()
// ClockEventHandler::setGmChanged()
// ClockEventHandler::setGmChangedEventCount()
// ClockEventHandler::setNotificationTimestamp()
// int64_t getClockOffset() const
// bool isOffsetInRange() const
// uint32_t getOffsetInRangeEventCount() const
// int64_t getSyncInterval() const
// uint64_t getGmIdentity() const
// bool isGmChanged() const
// uint32_t getGmChangedEventCount() const
// uint64_t getNotificationTimestamp() const
TEST(ClockEventBaseTest, setAndGet)
{
    ClockEventBaseTestable event;
    ClockEventHandler handler(SysClock);
    ClockEventHandler::setClockOffset(event, 123456789);
    ClockEventHandler::setOffsetInRange(event, true);
    ClockEventHandler::setOffsetInRangeEventCount(event, 5);
    ClockEventHandler::setSyncInterval(event, 1000000);
    ClockEventHandler::setGmIdentity(event, 0xABCDEF);
    ClockEventHandler::setGmChanged(event, true);
    ClockEventHandler::setGmChangedEventCount(event, 3);
    ClockEventHandler::setNotificationTimestamp(event, 987654321);
    EXPECT_EQ(event.getClockOffset(), 123456789);
    EXPECT_TRUE(event.isOffsetInRange());
    EXPECT_EQ(event.getOffsetInRangeEventCount(), 5u);
    EXPECT_EQ(event.getSyncInterval(), 1000000);
    EXPECT_EQ(event.getGmIdentity(), 0xABCDEFu);
    EXPECT_TRUE(event.isGmChanged());
    EXPECT_EQ(event.getGmChangedEventCount(), 3u);
    EXPECT_EQ(event.getNotificationTimestamp(), 987654321u);
}

// ClockEventBaseTestable::ClockEventBaseTestable()
// ClockEventHandler::setClockOffset()
// ClockEventHandler::setOffsetInRangeEventCount()
// ClockEventHandler::setGmChangedEventCount()
// ClockEventHandler::setSyncInterval()
// ClockEventHandler::setGmIdentity()
// ClockEventHandler::setNotificationTimestamp()
// int64_t getClockOffset() const
// uint32_t getOffsetInRangeEventCount() const
// uint32_t getGmChangedEventCount() const
// int64_t getSyncInterval() const
// uint64_t getGmIdentity() const
// uint64_t getNotificationTimestamp() const
TEST(ClockEventBaseTest, setBoundaryValue)
{
    ClockEventBaseTestable event;
    ClockEventHandler::setClockOffset(event, INT64_MIN);
    EXPECT_EQ(event.getClockOffset(), INT64_MIN);
    ClockEventHandler::setClockOffset(event, INT64_MAX);
    EXPECT_EQ(event.getClockOffset(), INT64_MAX);
    ClockEventHandler::setOffsetInRangeEventCount(event, UINT32_MAX);
    EXPECT_EQ(event.getOffsetInRangeEventCount(), UINT32_MAX);
    ClockEventHandler::setGmChangedEventCount(event, UINT32_MAX);
    EXPECT_EQ(event.getGmChangedEventCount(), UINT32_MAX);
    ClockEventHandler::setSyncInterval(event, INT64_MAX);
    EXPECT_EQ(event.getSyncInterval(), INT64_MAX);
    ClockEventHandler::setGmIdentity(event, UINT64_MAX);
    EXPECT_EQ(event.getGmIdentity(), UINT64_MAX);
    ClockEventHandler::setNotificationTimestamp(event, UINT64_MAX);
    EXPECT_EQ(event.getNotificationTimestamp(), UINT64_MAX);
}

// ClockEventBaseTestable::ClockEventBaseTestable()
// ClockEventHandler::setOffsetInRange()
// ClockEventHandler::setGmChanged()
// bool isOffsetInRange() const
// bool isGmChanged() const
TEST(ClockEventBaseTest, toggleBooleanFlag)
{
    ClockEventBaseTestable event;
    ClockEventHandler::setOffsetInRange(event, true);
    EXPECT_TRUE(event.isOffsetInRange());
    ClockEventHandler::setOffsetInRange(event, false);
    EXPECT_FALSE(event.isOffsetInRange());
    ClockEventHandler::setGmChanged(event, true);
    EXPECT_TRUE(event.isGmChanged());
    ClockEventHandler::setGmChanged(event, false);
    EXPECT_FALSE(event.isGmChanged());
}

// PTPClockEventTestable::PTPClockEventTestable()
// bool isSyncedWithGm() const
// uint32_t getSyncedWithGmEventCount() const
// bool isAsCapable() const
// uint32_t getAsCapableEventCount() const
// bool isCompositeEventMet() const
// uint32_t getCompositeEventCount() const
TEST(PTPClockEventTest, defaultValue)
{
    PTPClockEventTestable event;
    EXPECT_FALSE(event.isSyncedWithGm());
    EXPECT_EQ(event.getSyncedWithGmEventCount(), 0u);
    EXPECT_FALSE(event.isAsCapable());
    EXPECT_EQ(event.getAsCapableEventCount(), 0u);
    EXPECT_FALSE(event.isCompositeEventMet());
    EXPECT_EQ(event.getCompositeEventCount(), 0u);
}

// PTPClockEventTestable::PTPClockEventTestable()
// ClockEventHandler::setSyncedWithGm()
// ClockEventHandler::setAsCapable()
// ClockEventHandler::setSyncedWithGmEventCount()
// ClockEventHandler::setAsCapableEventCount()
// ClockEventHandler::setCompositeEvent()
// ClockEventHandler::setCompositeEventCount()
// bool isSyncedWithGm() const
// uint32_t getSyncedWithGmEventCount() const
// bool isAsCapable() const
// uint32_t getAsCapableEventCount() const
// bool isCompositeEventMet() const
// uint32_t getCompositeEventCount() const
TEST(PTPClockEventTest, setAndGet)
{
    PTPClockEventTestable event;
    ClockEventHandler handler(PTPClock);
    handler.setSyncedWithGm(event, true);
    handler.setAsCapable(event, true);
    ClockEventHandler::setSyncedWithGmEventCount(event, 2);
    ClockEventHandler::setAsCapableEventCount(event, 4);
    ClockEventHandler::setCompositeEvent(event, true);
    ClockEventHandler::setCompositeEventCount(event, 7);
    EXPECT_TRUE(event.isSyncedWithGm());
    EXPECT_EQ(event.getSyncedWithGmEventCount(), 2u);
    EXPECT_TRUE(event.isAsCapable());
    EXPECT_EQ(event.getAsCapableEventCount(), 4u);
    EXPECT_TRUE(event.isCompositeEventMet());
    EXPECT_EQ(event.getCompositeEventCount(), 7u);
}

// PTPClockEventTestable::PTPClockEventTestable()
// ClockEventHandler::setSyncedWithGmEventCount()
// ClockEventHandler::setAsCapableEventCount()
// ClockEventHandler::setCompositeEventCount()
// uint32_t getSyncedWithGmEventCount() const
// uint32_t getAsCapableEventCount() const
// uint32_t getCompositeEventCount() const
TEST(PTPClockEventTest, setBoundaryValue)
{
    PTPClockEventTestable event;
    ClockEventHandler handler(PTPClock);
    ClockEventHandler::setSyncedWithGmEventCount(event, UINT32_MAX);
    EXPECT_EQ(event.getSyncedWithGmEventCount(), UINT32_MAX);
    ClockEventHandler::setAsCapableEventCount(event, UINT32_MAX);
    EXPECT_EQ(event.getAsCapableEventCount(), UINT32_MAX);
    ClockEventHandler::setCompositeEventCount(event, UINT32_MAX);
    EXPECT_EQ(event.getCompositeEventCount(), UINT32_MAX);
}

// PTPClockEventTestable::PTPClockEventTestable()
// ClockEventHandler::setSyncedWithGm()
// ClockEventHandler::setAsCapable()
// ClockEventHandler::setCompositeEvent()
// bool isSyncedWithGm() const
// bool isAsCapable() const
// bool isCompositeEventMet() const
TEST(PTPClockEventTest, toggleBooleanFlag)
{
    PTPClockEventTestable event;
    ClockEventHandler handler(PTPClock);
    handler.setSyncedWithGm(event, true);
    EXPECT_TRUE(event.isSyncedWithGm());
    handler.setSyncedWithGm(event, false);
    EXPECT_FALSE(event.isSyncedWithGm());
    handler.setAsCapable(event, true);
    EXPECT_TRUE(event.isAsCapable());
    handler.setAsCapable(event, false);
    EXPECT_FALSE(event.isAsCapable());
    ClockEventHandler::setCompositeEvent(event, true);
    EXPECT_TRUE(event.isCompositeEventMet());
    ClockEventHandler::setCompositeEvent(event, false);
    EXPECT_FALSE(event.isCompositeEventMet());
}

// SysClockEventTestable::SysClockEventTestable()
// int64_t getClockOffset() const
// bool isOffsetInRange() const
// uint32_t getOffsetInRangeEventCount() const
TEST(SysClockEventTest, defaultValue)
{
    SysClockEventTestable event;
    EXPECT_EQ(event.getClockOffset(), 0);
    EXPECT_TRUE(event.isOffsetInRange());
    EXPECT_EQ(event.getOffsetInRangeEventCount(), 0u);
}

// ClockSyncData::ClockSyncData()
// bool havePTP() const
// bool haveSys() const
TEST(ClockSyncDataTest, defaultValue)
{
    ClockSyncData clData;
    EXPECT_FALSE(clData.havePTP());
    EXPECT_FALSE(clData.haveSys());
}

// ClockSyncData::ClockSyncData()
// ClockSyncData::getPtp()
// ClockSyncData::getSysClock()
// ClockEventHandler::setClockOffset()
// int64_t getClockOffset() const
TEST(ClockSyncDataTest, setAndGet)
{
    ClockSyncData clData;
    // Should be able to access and modify via reference
    ClockEventHandler::setClockOffset(clData.getPtp(), 42);
    ClockEventHandler::setClockOffset(clData.getSysClock(), 99);
    EXPECT_EQ(clData.getPtp().getClockOffset(), 42);
    EXPECT_EQ(clData.getSysClock().getClockOffset(), 99);
}
