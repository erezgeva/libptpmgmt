/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Clock events subscription.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "pub/clkmgr/subscription.h"

__CLKMGR_NAMESPACE_USE;

#define GET_SET(cls, type, func, var)\
    void cls::set##func(type _v) { var = _v; } \
    type cls::get##func() const { return var; }

GET_SET(ClockSubscriptionBase, uint32_t,
    ClockOffsetThreshold, clockOffsetThreshold)
GET_SET(ClockSubscriptionBase, uint32_t, EventMask, eventMask)
GET_SET(PTPClockSubscription, uint32_t,
    CompositeEventMask, m_composite_event_mask)

PTPClockSubscription::PTPClockSubscription() noexcept
    : m_composite_event_mask(0)
{
}

SysClockSubscription::SysClockSubscription() noexcept
    : ClockSubscriptionBase()
{
}

ClockSyncSubscription::ClockSyncSubscription()
{
    ptpSubscribed = false;
    sysSubscribed = false;
}

#define OBJ_FUNC(Nm, NM, nm)\
    void ClockSyncSubscription::enable##Nm##Subscription() {\
        nm##Subscribed = true; }\
    void ClockSyncSubscription::disable##Nm##Subscription() {\
        nm##Subscribed = false; }\
    bool ClockSyncSubscription::is##NM##SubscriptionEnable() const {\
        return nm##Subscribed; }\
    void ClockSyncSubscription::set##Nm##Subscription(\
        const NM##ClockSubscription &_o) {\
        nm##Subscription = _o; }\
    const NM##ClockSubscription &ClockSyncSubscription::\
    get##Nm##Subscription() const {\
        return nm##Subscription; }

OBJ_FUNC(Ptp, PTP, ptp)
OBJ_FUNC(Sys, Sys, sys)
