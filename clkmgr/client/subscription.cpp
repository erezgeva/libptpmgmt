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
#include "client/opaque_struct_c.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

#define GET_SET(cls, type, func, var)\
    void cls::set##func(type _v) { var = _v; } \
    type cls::get##func() const { return var; }

#define GET(cls, type, func, var)\
    type cls::get##func() const { return var; }

GET_SET(ClockSubscriptionBase, uint32_t,
    ClockOffsetThreshold, clockOffsetThreshold)
GET(ClockSubscriptionBase, uint32_t, EventMask, eventMask)
GET(PTPClockSubscription, uint32_t,
    CompositeEventMask, m_composite_event_mask)

bool ClockSubscriptionBase::setEventMask(uint32_t newEventMask)
{
    modifyEventMask(newEventMask);
    return true;
}

void ClockSubscriptionBase::modifyEventMask(uint32_t newEventMask)
{
    eventMask = newEventMask;
}

ClockSubscriptionBase::~ClockSubscriptionBase() {}

bool PTPClockSubscription::setEventMask(uint32_t newEventMask)
{
    if(newEventMask > PTP_EVENT_ALL) {
        PrintDebug("Event mask contains invalid bits.");
        return false;
    }
    modifyEventMask(newEventMask);
    return true;
}

bool PTPClockSubscription::setCompositeEventMask(uint32_t composite_event_mask)
{
    if(composite_event_mask & ~PTP_COMPOSITE_EVENT_ALL) {
        PrintDebug("Composite event mask contains invalid bits.");
        return false;
    }
    m_composite_event_mask = composite_event_mask;
    return true;
}

PTPClockSubscription::PTPClockSubscription() noexcept
    : m_composite_event_mask(0)
{
}

PTPClockSubscription::~PTPClockSubscription() {}

SysClockSubscription::SysClockSubscription() noexcept
    : ClockSubscriptionBase()
{
}

SysClockSubscription::~SysClockSubscription() {}

bool SysClockSubscription::setEventMask(uint32_t newEventMask)
{
    if(newEventMask > SYS_EVENT_ALL) {
        PrintDebug("Event mask contains invalid bits.");
        return false;
    }
    modifyEventMask(newEventMask);
    return true;
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
        nm##Subscription = _o;\
        nm##Subscribed = true;}\
    const NM##ClockSubscription &ClockSyncSubscription::\
    get##Nm##Subscription() const {\
        return nm##Subscription; }

OBJ_FUNC(Ptp, PTP, ptp)
OBJ_FUNC(Sys, Sys, sys)

extern "C" {

    Clkmgr_Subscription *clkmgr_constructSubscriptionInstance(void)
    {
        auto *sub_c = new Clkmgr_Subscription;
        sub_c->ptp = new PTPClockSubscription();
        sub_c->sys = new SysClockSubscription();
        sub_c->sub = new ClockSyncSubscription();
        return sub_c;
    }

    void clkmgr_destroySubscriptionInstance(Clkmgr_Subscription *sub_c)
    {
        if(!sub_c)
            return;
        delete sub_c->ptp;
        delete sub_c->sys;
        delete sub_c->sub;
        delete sub_c;
    }

    bool clkmgr_setEventMask(Clkmgr_Subscription *sub_c, uint32_t clock_type,
        uint32_t mask)
    {
        if(!sub_c)
            return false;
        switch(clock_type) {
            case PTPClock:
                if(sub_c->ptp->setEventMask(mask)) {
                    sub_c->sub->setPtpSubscription(*sub_c->ptp);
                    return true;
                }
                return false;
            case SysClock:
                if(sub_c->sys->setEventMask(mask)) {
                    sub_c->sub->setSysSubscription(*sub_c->sys);
                    return true;
                }
                return false;
            default:
                return false;
        }
    }

    uint32_t clkmgr_getEventMask(const Clkmgr_Subscription *sub_c,
        uint32_t clock_type)
    {
        if(!sub_c)
            return 0;
        switch(clock_type) {
            case PTPClock:
                return sub_c->sub->getPtpSubscription().getEventMask();
            case SysClock:
                return sub_c->sub->getSysSubscription().getEventMask();
            default:
                return 0;
        }
    }

    bool clkmgr_setPtpCompositeEventMask(Clkmgr_Subscription *sub_c,
        uint32_t mask)
    {
        if(sub_c && sub_c->ptp->setCompositeEventMask(mask)) {
            sub_c->sub->setPtpSubscription(*sub_c->ptp);
            return true;
        }
        return false;
    }

    uint32_t clkmgr_getPtpCompositeEventMask(const Clkmgr_Subscription *sub_c)
    {
        if(sub_c)
            return sub_c->sub->getPtpSubscription().getCompositeEventMask();
        return 0;
    }

    bool clkmgr_setClockOffsetThreshold(Clkmgr_Subscription *sub_c,
        uint32_t clock_type, uint32_t threshold)
    {
        if(!sub_c)
            return false;
        switch(clock_type) {
            case PTPClock:
                sub_c->ptp->setClockOffsetThreshold(threshold);
                sub_c->sub->setPtpSubscription(*sub_c->ptp);
                return true;
            case SysClock:
                sub_c->sys->setClockOffsetThreshold(threshold);
                sub_c->sub->setSysSubscription(*sub_c->sys);
                return true;
            default:
                return false;
        }
    }

    uint32_t clkmgr_getClockOffsetThreshold(const Clkmgr_Subscription
        *sub_c, uint32_t clock_type)
    {
        if(!sub_c)
            return 0;
        switch(clock_type) {
            case PTPClock:
                return sub_c->sub->getPtpSubscription().getClockOffsetThreshold();
            case SysClock:
                return sub_c->sub->getSysSubscription().getClockOffsetThreshold();
            default:
                return 0;
        }
    }

    bool clkmgr_enableSubscription(Clkmgr_Subscription *sub_c, uint32_t clock_type)
    {
        if(!sub_c)
            return false;
        switch(clock_type) {
            case PTPClock:
                sub_c->sub->enablePtpSubscription();
                return true;
            case SysClock:
                sub_c->sub->enableSysSubscription();
                return true;
            default:
                return false;
        }
    }

    bool clkmgr_disableSubscription(Clkmgr_Subscription *sub_c, uint32_t clock_type)
    {
        if(!sub_c)
            return false;
        switch(clock_type) {
            case PTPClock:
                sub_c->sub->disablePtpSubscription();
                return true;
            case SysClock:
                sub_c->sub->disableSysSubscription();
                return true;
            default:
                return false;
        }
    }

    bool clkmgr_isSubscriptionEnabled(const Clkmgr_Subscription *sub_c,
        uint32_t clock_type)
    {
        if(!sub_c)
            return false;
        switch(clock_type) {
            case PTPClock:
                return sub_c->sub->isPTPSubscriptionEnable();
            case SysClock:
                return sub_c->sub->isSysSubscriptionEnable();
            default:
                return false;
        }
    }

} // extern "C"
