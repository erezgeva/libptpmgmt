/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief PTP event subscription.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "pub/clkmgr/subscription.h"

__CLKMGR_NAMESPACE_USE;

ClockSubscriptionBase::ClockSubscriptionBase() noexcept : eventMask(0) {}

void ClockSubscriptionBase::setClockOffsetThreshold(uint32_t threshold)
{
    clockOffsetThreshold = threshold;
}

uint32_t ClockSubscriptionBase::getClockOffsetThreshold() const
{
    return clockOffsetThreshold;
}

void ClockSubscriptionBase::setEventMask(uint32_t newEventMask)
{
    eventMask = newEventMask;
}

uint32_t ClockSubscriptionBase::getEventMask() const
{
    return eventMask;
}

void PTPClockSubscription::setCompositeEventMask(uint32_t
    composite_event_mask)
{
    m_composite_event_mask = composite_event_mask;
}

uint32_t PTPClockSubscription::getCompositeEventMask() const
{
    return m_composite_event_mask;
}

PTPClockSubscription::PTPClockSubscription() noexcept
    : m_composite_event_mask(0) {}

SysClockSubscription::SysClockSubscription() noexcept
    : ClockSubscriptionBase() {}

ClockSyncSubscription::ClockSyncSubscription()
{
    ptpSubscribed = false;
    sysSubscribed = false;
}

void ClockSyncSubscription::enablePtpSubscription()
{
    ptpSubscribed = true;
}

void ClockSyncSubscription::disablePtpSubscription()
{
    ptpSubscribed = false;
}

bool ClockSyncSubscription::isPTPSubscriptionEnable() const
{
    return ptpSubscribed;
}

void ClockSyncSubscription::setPtpSubscription(
    const PTPClockSubscription &newPtpSub)
{
    ptpSubscription = newPtpSub;
}

const PTPClockSubscription &ClockSyncSubscription::getPtpSubscription() const
{
    return ptpSubscription;
}

void ClockSyncSubscription::enableSysSubscription()
{
    sysSubscribed = true;
}

void ClockSyncSubscription::disableSysSubscription()
{
    sysSubscribed = false;
}

bool ClockSyncSubscription::isSysSubscriptionEnable() const
{
    return sysSubscribed;
}

void ClockSyncSubscription::setSysSubscription(
    const SysClockSubscription &newSysSub)
{
    sysSubscription = newSysSub;
}

const SysClockSubscription &ClockSyncSubscription::getSysSubscription() const
{
    return sysSubscription;
}
