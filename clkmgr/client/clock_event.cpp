/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Client clock event class
 *
 * @author Lai Peter Jun Ann <peter.jun.ann.lai@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include "pub/clkmgr/event.h"
#include "client/clock_event_handler.hpp"
#include "client/opaque_struct_c.hpp"
#include "client/timebase_state.hpp"

__CLKMGR_NAMESPACE_USE;

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

bool ClockSyncBaseHandler::updateAll(const TimeBaseState &state)
{
    // TODO: check ptp4l and chrony data is received
    clockSyncData.ptpClockSync = state.get_ptp4lEventState();
    clockSyncData.ptpAvailable = true;
    clockSyncData.sysClockSync = state.get_chronyEventState();
    clockSyncData.sysAvailable = true;
    return true;
}

extern "C" {

    Clkmgr_ClockSyncData *clkmgr_constructClockSyncDataInstance()
    {
        auto *data_c = new Clkmgr_ClockSyncData;
        data_c->data = new clkmgr::ClockSyncData();
        return data_c;
    }

    void clkmgr_destroyClockSyncDataInstance(Clkmgr_ClockSyncData *data_c)
    {
        if(!data_c)
            return;
        delete data_c->data;
        delete data_c;
    }

    bool clkmgr_havePtpData(const Clkmgr_ClockSyncData *data_c)
    {
        return data_c->data->havePTP();
    }

    bool clkmgr_haveSysData(const Clkmgr_ClockSyncData *data_c)
    {
        return data_c->data->haveSys();
    }

    int64_t clkmgr_getClockOffset(const Clkmgr_ClockSyncData *data_c,
        uint32_t clock_type)
    {
        if(!data_c)
            return 0;
        switch(clock_type) {
            case PTPClock:
                return data_c->data->getPtp().getClockOffset();
            case SysClock:
                return data_c->data->getSysClock().getClockOffset();
            default:
                return 0;
        }
    }

    bool clkmgr_isOffsetInRange(const Clkmgr_ClockSyncData *data_c,
        uint32_t clock_type)
    {
        if(!data_c)
            return false;
        switch(clock_type) {
            case PTPClock:
                return data_c->data->getPtp().isOffsetInRange();
            case SysClock:
                return data_c->data->getSysClock().isOffsetInRange();
            default:
                return false;
        }
    }

    uint32_t clkmgr_getOffsetInRangeEventCount(const Clkmgr_ClockSyncData *data_c,
        uint32_t clock_type)
    {
        if(!data_c)
            return 0;
        switch(clock_type) {
            case PTPClock:
                return data_c->data->getPtp().getOffsetInRangeEventCount();
            case SysClock:
                return data_c->data->getSysClock().getOffsetInRangeEventCount();
            default:
                return 0;
        }
    }

    int64_t clkmgr_getSyncInterval(const Clkmgr_ClockSyncData *data_c,
        uint32_t clock_type)
    {
        if(!data_c)
            return 0;
        switch(clock_type) {
            case PTPClock:
                return data_c->data->getPtp().getSyncInterval();
            case SysClock:
                return data_c->data->getSysClock().getSyncInterval();
            default:
                return 0;
        }
    }

    uint64_t clkmgr_getGmIdentity(const Clkmgr_ClockSyncData *data_c,
        uint32_t clock_type)
    {
        if(!data_c)
            return 0;
        switch(clock_type) {
            case PTPClock:
                return data_c->data->getPtp().getGmIdentity();
            case SysClock:
                return data_c->data->getSysClock().getGmIdentity();
            default:
                return 0;
        }
    }

    bool clkmgr_isGmChanged(const Clkmgr_ClockSyncData *data_c,
        uint32_t clock_type)
    {
        if(!data_c)
            return false;
        switch(clock_type) {
            case PTPClock:
                return data_c->data->getPtp().isGmChanged();
            case SysClock:
                return data_c->data->getSysClock().isGmChanged();
            default:
                return false;
        }
    }

    uint32_t clkmgr_getGmChangedEventCount(const Clkmgr_ClockSyncData *data_c,
        uint32_t clock_type)
    {
        if(!data_c)
            return 0;
        switch(clock_type) {
            case PTPClock:
                return data_c->data->getPtp().getGmChangedEventCount();
            case SysClock:
                return data_c->data->getSysClock().getGmChangedEventCount();
            default:
                return 0;
        }
    }

    uint64_t clkmgr_getNotificationTimestamp(const Clkmgr_ClockSyncData *data_c,
        uint32_t clock_type)
    {
        if(!data_c)
            return 0;
        switch(clock_type) {
            case PTPClock:
                return data_c->data->getPtp().getNotificationTimestamp();
            case SysClock:
                return data_c->data->getSysClock().getNotificationTimestamp();
            default:
                return 0;
        }
    }

    // PTPClockEvent
    bool clkmgr_isPtpSyncedWithGm(const Clkmgr_ClockSyncData *data_c)
    {
        return data_c->data->getPtp().isSyncedWithGm();
    }

    uint32_t clkmgr_getPtpSyncedWithGmEventCount(const Clkmgr_ClockSyncData *data_c)
    {
        return data_c->data->getPtp().getSyncedWithGmEventCount();
    }

    bool clkmgr_isPtpAsCapable(const Clkmgr_ClockSyncData *data_c)
    {
        return data_c->data->getPtp().isAsCapable();
    }

    uint32_t clkmgr_getPtpAsCapableEventCount(const Clkmgr_ClockSyncData *data_c)
    {
        return data_c->data->getPtp().getAsCapableEventCount();
    }

    bool clkmgr_isPtpCompositeEventMet(const Clkmgr_ClockSyncData *data_c)
    {
        return data_c->data->getPtp().isCompositeEventMet();
    }

    uint32_t clkmgr_getPtpCompositeEventCount(const Clkmgr_ClockSyncData *data_c)
    {
        return data_c->data->getPtp().getCompositeEventCount();
    }

} // extern "C"
