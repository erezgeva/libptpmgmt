/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Common clock event class
 *
 * @author Lai Peter Jun Ann <peter.jun.ann.lai@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include "pub/clkmgr/event.h"
#include "common/clock_event_handler.hpp"

__CLKMGR_NAMESPACE_USE;

ClockEventBase::ClockEventBase()
{
    clockOffset = 0;
    offsetInRange = false;
    gmClockUUID = {};
    gmChanged = false;
    syncInterval = 0;
    notificationTimestamp = 0;
    offsetInRangeCount = 0;
    gmChangedCount = 0;
}

uint64_t ClockEventBase::getNotificationTimestamp() const
{
    return notificationTimestamp;
}

int64_t ClockEventBase::getClockOffset() const
{
    return clockOffset;
}

uint64_t ClockEventBase::getGmIdentity() const
{
    return gmClockUUID;
}

bool ClockEventBase::isOffsetInRange() const
{
    return offsetInRange;
}

bool ClockEventBase::isGmChanged() const
{
    return gmChanged;
}

int64_t ClockEventBase::getSyncInterval() const
{
    return syncInterval;
}

uint32_t ClockEventBase::getOffsetInRangeEventCount() const
{
    return offsetInRangeCount;
}

uint32_t ClockEventBase::getGmChangedEventCount() const
{
    return gmChangedCount;
}

PTPClockEvent::PTPClockEvent()
{
    syncedWithGm = false;
    asCapable = false;
    compositeEvent = false;
    syncedWithGmCount = 0;
    asCapableCount = 0;
    compositeEventCount = 0;
}

bool PTPClockEvent::isSyncedWithGm() const
{
    return syncedWithGm;
}

bool PTPClockEvent::isAsCapable() const
{
    return asCapable;
}

bool PTPClockEvent::isCompositeEventMet() const
{
    return compositeEvent;
}

uint32_t PTPClockEvent::getSyncedWithGmEventCount() const
{
    return syncedWithGmCount;
}

uint32_t PTPClockEvent::getAsCapableEventCount() const
{
    return asCapableCount;
}

uint32_t PTPClockEvent::getCompositeEventCount() const
{
    return compositeEventCount;
}

SysClockEvent::SysClockEvent() {}

ClockSyncData::ClockSyncData()
{
    ptpAvailable = false;
    sysAvailable = false;
}

bool ClockSyncData::havePTP() const
{
    return ptpAvailable;
}

PTPClockEvent &ClockSyncData::getPtp()
{
    return ptpClockSync;
}

bool ClockSyncData::haveSys() const
{
    return sysAvailable;
}

SysClockEvent &ClockSyncData::getSysClock()
{
    return sysClockSync;
}

void ClockSyncBaseHandler::setPTPAvailability(bool available)
{
    clockSyncData.ptpAvailable = available;
}

void ClockSyncBaseHandler::setSysAvailability(bool available)
{
    clockSyncData.ptpAvailable = available;
}

void ClockSyncBaseHandler::updatePTPClock(const PTPClockEvent &newPTPClock)
{
    clockSyncData.ptpClockSync = newPTPClock;
}

void ClockSyncBaseHandler::updateSysClock(const SysClockEvent &newSysClock)
{
    clockSyncData.sysClockSync = newSysClock;
}
