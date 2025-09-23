/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Set and get the timebase subscribe event state
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include "client/subscribe_msg.hpp"
#include "client/client_state.hpp"
#include "client/timebase_state.hpp"
#include "common/print.hpp"

#include <cstring>
#include <chrono>
#include <rtpi/condition_variable.hpp>

__CLKMGR_NAMESPACE_USE;

using namespace std;
using namespace std::chrono;

const uint32_t DEFAULT_SUBSCRIBE_TIME_OUT = 5;  //5 sec

static rtpi::mutex subscribe_mutex;
static rtpi::condition_variable subscribe_cv;

static ClockEventHandler ptpClockEventHandler(PTPClock);
static ClockEventHandler sysClockEventHandler(SysClock);

static bool isPTPDataEmpty(const ptp_event &ptpData)
{
    return (ptpData.clockOffset == 0 &&
            ptpData.gmClockUUID == 0 &&
            ptpData.syncInterval == 0 &&
            ptpData.asCapable == false &&
            ptpData.syncedWithGm == false);
}

static bool isChronyDataEmpty(const chrony_event &chronyData)
{
    return (chronyData.clockOffset == 0 &&
            chronyData.gmClockUUID == 0 &&
            chronyData.syncInterval == 0);
}

bool TimeBaseState::get_subscribed() const
{
    return subscribed;
}

void TimeBaseState::set_subscribed(bool subscriptionState)
{
    subscribed = subscriptionState;
}

bool TimeBaseState::is_event_changed() const
{
    return event_changed;
}

void TimeBaseState::set_event_changed(bool state)
{
    event_changed = state;
}

bool TimeBaseState::havePtp() const
{
    return havePtPData;
}

void TimeBaseState::set_ptpAvailability(bool havePtp)
{
    havePtPData = havePtp;
}

bool TimeBaseState::haveSys() const
{
    return haveSysData;
}

void TimeBaseState::set_sysAvailability(bool haveSys)
{
    haveSysData = haveSys;
}

const PTPClockEvent &TimeBaseState::get_ptpEventState() const
{
    return ptpEventState;
}

const SysClockEvent &TimeBaseState::get_sysEventState() const
{
    return sysEventState;
}

void TimeBaseState::set_ptpEventState(const PTPClockEvent &ptpState)
{
    ptpEventState = ptpState;
}

void TimeBaseState::set_sysEventState(const SysClockEvent &sysState)
{
    sysEventState = sysState;
}

const PTPClockSubscription &TimeBaseState::get_ptpEventSub() const
{
    return ptpEventSub;
}

const SysClockSubscription &TimeBaseState::get_sysEventSub() const
{
    return sysEventSub;
}

bool TimeBaseState::set_ptpEventSub(const PTPClockSubscription &eSub)
{
    ptpEventSub.setClockOffsetThreshold(eSub.getClockOffsetThreshold());
    return (ptpEventSub.setEventMask(eSub.getEventMask()) &&
            ptpEventSub.setCompositeEventMask(eSub.getCompositeEventMask()));
}

bool TimeBaseState::set_sysEventSub(const SysClockSubscription &eSub)
{
    sysEventSub.setClockOffsetThreshold(eSub.getClockOffsetThreshold());
    return sysEventSub.setEventMask(eSub.getEventMask());
}

void TimeBaseState::set_last_notification_time(const timespec &newTime)
{
    last_notification_time = newTime;
}

const timespec &TimeBaseState::get_last_notification_time() const
{
    return last_notification_time;
}

bool TimeBaseStates::getTimeBaseState(size_t timeBaseIndex,
    TimeBaseState &state)
{
    lock_guard<rtpi::mutex> lock(mtx);
    auto it = timeBaseStateMap.find(timeBaseIndex);
    if(it != timeBaseStateMap.end()) {
        state = it->second; // Copy the TimeBaseState object
        // Get the current state of the timebase
        PTPClockEvent ptpEventState = it->second.get_ptpEventState();
        SysClockEvent sysEventState = it->second.get_sysEventState();
        // Reset the Event Count
        ptpClockEventHandler.setOffsetInRangeEventCount(ptpEventState, 0);
        ptpClockEventHandler.setSyncedWithGmEventCount(ptpEventState, 0);
        ptpClockEventHandler.setGmChangedEventCount(ptpEventState, 0);
        ptpClockEventHandler.setAsCapableEventCount(ptpEventState, 0);
        ptpClockEventHandler.setCompositeEventCount(ptpEventState, 0);
        ptpClockEventHandler.setGmChanged(ptpEventState, false);
        it->second.set_ptpEventState(ptpEventState);
        sysClockEventHandler.setOffsetInRangeEventCount(sysEventState, 0);
        it->second.set_sysEventState(sysEventState);
        it->second.set_event_changed(false);
        return true;
    }
    // If timeBaseIndex is not found, return false
    return false;
}

void TimeBaseStates::setTimeBaseStatePtp(size_t timeBaseIndex,
    const ptp_event &newEvent)
{
    lock_guard<rtpi::mutex> lock(mtx);
    auto &state = timeBaseStateMap[timeBaseIndex];
    // Update the notification timestamp
    timespec last_notification_time = {};
    if(clock_gettime(CLOCK_REALTIME, &last_notification_time) == -1)
        PrintDebug("Failed to update notification time.");
    else
        state.set_last_notification_time(last_notification_time);
    // Get a copy of subscription mask
    PTPClockSubscription ptpSub = state.get_ptpEventSub();
    uint32_t ptpEventSub = ptpSub.getEventMask();
    uint32_t ptpCompositeEventSub = ptpSub.getCompositeEventMask();
    int32_t ptpThreshold = static_cast<uint32_t>(ptpSub.getClockOffsetThreshold());
    // Get the current state of the timebase
    PTPClockEvent ptpEventState = state.get_ptpEventState();
    // Update EventOffsetInRange
    if((ptpEventSub & EventOffsetInRange) &&
        (newEvent.clockOffset != ptpEventState.getClockOffset())) {
        ptpClockEventHandler.setClockOffset(ptpEventState,
            newEvent.clockOffset);
        bool ptpInRange = (ptpEventState.getClockOffset() >= -ptpThreshold)
            && (ptpEventState.getClockOffset() <= ptpThreshold);
        if(ptpInRange) {
            if(!(ptpEventState.isOffsetInRange())) {
                ptpClockEventHandler.setOffsetInRange(ptpEventState, true);
                ptpClockEventHandler.setOffsetInRangeEventCount(ptpEventState,
                    ptpEventState.getOffsetInRangeEventCount() + 1);
                state.set_event_changed(true);
            }
        } else {
            if((ptpEventState.isOffsetInRange())) {
                ptpClockEventHandler.setOffsetInRange(ptpEventState, false);
                ptpClockEventHandler.setOffsetInRangeEventCount(ptpEventState,
                    ptpEventState.getOffsetInRangeEventCount() + 1);
                state.set_event_changed(true);
            }
        }
    }
    // Update EventSyncedWithGm
    if((ptpEventSub & EventSyncedWithGm) &&
        (newEvent.syncedWithGm !=
            ptpEventState.isSyncedWithGm())) {
        ptpClockEventHandler.setSyncedWithGm(ptpEventState,
            newEvent.syncedWithGm);
        ptpClockEventHandler.setSyncedWithGmEventCount(ptpEventState,
            ptpEventState.getSyncedWithGmEventCount() + 1);
        state.set_event_changed(true);
    }
    // Update EventGmChanged
    uint64_t sourceClockUUID = ptpEventState.getGmIdentity();
    if((ptpEventSub & EventGmChanged) &&
        (sourceClockUUID != newEvent.gmClockUUID)) {
        ptpClockEventHandler.setGmIdentity(ptpEventState, newEvent.gmClockUUID);
        ptpClockEventHandler.setGmChanged(ptpEventState, true);
        ptpClockEventHandler.setGmChangedEventCount(ptpEventState,
            ptpEventState.getGmChangedEventCount() + 1);
        state.set_event_changed(true);
    }
    // Update EventAsCapable
    if((ptpEventSub & EventAsCapable) &&
        (newEvent.asCapable != ptpEventState.isAsCapable())) {
        ptpClockEventHandler.setAsCapable(ptpEventState, newEvent.asCapable);
        ptpClockEventHandler.setAsCapableEventCount(ptpEventState,
            ptpEventState.getAsCapableEventCount() + 1);
        state.set_event_changed(true);
    }
    // Update composite event
    bool composite_event = true;
    if(ptpCompositeEventSub & EventOffsetInRange)
        composite_event &= ptpEventState.isOffsetInRange();
    if(ptpCompositeEventSub & EventSyncedWithGm)
        composite_event &= ptpEventState.isSyncedWithGm();
    if(ptpCompositeEventSub & EventAsCapable)
        composite_event &= ptpEventState.isAsCapable();
    if(ptpCompositeEventSub &&
        (composite_event != ptpEventState.isCompositeEventMet())) {
        ptpClockEventHandler.setCompositeEvent(ptpEventState,
            composite_event);
        ptpClockEventHandler.setCompositeEventCount(ptpEventState,
            ptpEventState.getCompositeEventCount() + 1);
        state.set_event_changed(true);
    }
    // Update notification timestamp
    uint64_t notification_timestamp = last_notification_time.tv_sec;
    notification_timestamp *= NSEC_PER_SEC;
    notification_timestamp += last_notification_time.tv_nsec;
    ptpClockEventHandler.setNotificationTimestamp(ptpEventState,
        notification_timestamp);
    // Update GM logSyncInterval
    ptpClockEventHandler.setSyncInterval(ptpEventState,
        newEvent.syncInterval);
    state.set_ptpEventState(ptpEventState);
    state.set_ptpAvailability(true);
}

void TimeBaseStates::setTimeBaseStateSys(size_t timeBaseIndex,
    const chrony_event &newEvent)
{
    lock_guard<rtpi::mutex> lock(mtx);
    auto &state = timeBaseStateMap[timeBaseIndex];
    // Update the notification timestamp
    timespec last_notification_time = {};
    if(clock_gettime(CLOCK_REALTIME, &last_notification_time) == -1)
        PrintDebug("Failed to update notification time.");
    else
        state.set_last_notification_time(last_notification_time);
    // Get a copy of subscription mask
    SysClockSubscription sysSub = state.get_sysEventSub();
    uint32_t sysEventSub = sysSub.getEventMask();
    int32_t sysThreshold = static_cast<uint32_t>(sysSub.getClockOffsetThreshold());
    SysClockEvent sysEventState = state.get_sysEventState();
    // Update Chrony clock offset
    if((sysEventSub & EventOffsetInRange) &&
        (newEvent.clockOffset != sysEventState.getClockOffset())) {
        sysClockEventHandler.setClockOffset(sysEventState,
            newEvent.clockOffset);
        bool sysInRange = (sysEventState.getClockOffset() >= -sysThreshold) &&
            (sysEventState.getClockOffset() <= sysThreshold);
        if(sysInRange) {
            if(!(sysEventState.isOffsetInRange())) {
                sysClockEventHandler.setOffsetInRange(sysEventState, true);
                sysClockEventHandler.setOffsetInRangeEventCount(sysEventState,
                    sysEventState.getOffsetInRangeEventCount() + 1);
                state.set_event_changed(true);
            }
        } else {
            if((sysEventState.isOffsetInRange())) {
                sysClockEventHandler.setOffsetInRange(sysEventState, false);
                sysClockEventHandler.setOffsetInRangeEventCount(sysEventState,
                    sysEventState.getOffsetInRangeEventCount() + 1);
                state.set_event_changed(true);
            }
        }
    }
    sysClockEventHandler.setGmIdentity(sysEventState,
        newEvent.gmClockUUID);
    sysClockEventHandler.setSyncInterval(sysEventState,
        newEvent.syncInterval);
    // Update notification timestamp
    uint64_t notification_timestamp = last_notification_time.tv_sec;
    notification_timestamp *= NSEC_PER_SEC;
    notification_timestamp += last_notification_time.tv_nsec;
    sysClockEventHandler.setNotificationTimestamp(sysEventState,
        notification_timestamp);
    state.set_sysEventState(sysEventState);
    state.set_sysAvailability(true);
}

bool TimeBaseStates::subscribe(size_t timeBaseIndex,
    const ClockSyncSubscription &newSub)
{
    // Check whether connection between Proxy and Client is established or not
    if(!ClientState::get_connected()) {
        PrintDebug("[SUBSCRIBE] Client is not connected to Proxy.");
        return false;
    }
    // Check whether requested timeBaseIndex is valid or not
    if(!TimeBaseConfigurations::isTimeBaseIndexPresent(timeBaseIndex)) {
        PrintDebug("[SUBSCRIBE] Invalid timeBaseIndex.");
        return false;
    }
    if(newSub.isPTPSubscriptionEnable()) {
        const PTPClockSubscription &newPtpSub = newSub.getPtpSubscription();
        if(!setPtpEventSubscription(timeBaseIndex, newPtpSub))
            return false;
    }
    // ToDo: Check whether system clock is available for subscription
    if(newSub.isSysSubscriptionEnable()) {
        const SysClockSubscription &newSysSub = newSub.getSysSubscription();
        if(!setSysEventSubscription(timeBaseIndex, newSysSub))
            return false;
    }
    // Send a subscribe message to Proxy Daemon
    ClientSubscribeMessage *cmsg = new ClientSubscribeMessage();
    if(cmsg == nullptr) {
        PrintDebug("[SUBSCRIBE] Failed to create subscribe message.");
        return false;
    }
    unique_ptr<Message> subscribeMsg(cmsg);
    cmsg->set_timeBaseIndex(timeBaseIndex);
    cmsg->set_sessionId(ClientState::get_sessionId());
    ClientState::sendMessage(*cmsg);
    // Wait DEFAULT_SUBSCRIBE_TIME_OUT seconds for response from Proxy Daemon
    auto endTime = system_clock::now() + seconds(DEFAULT_SUBSCRIBE_TIME_OUT);
    unique_lock<rtpi::mutex> lock(subscribe_mutex);
    while(!getSubscribed(timeBaseIndex)) {
        auto res = subscribe_cv.wait_until(lock, endTime);
        if(res == cv_status::timeout) {
            if(!getSubscribed(timeBaseIndex)) {
                PrintDebug("[SUBSCRIBE] Timeout waiting reply from Proxy.");
                return false;
            }
        } else
            PrintDebug("[SUBSCRIBE] Received reply from Proxy.");
    }
    return true;
}

bool TimeBaseStates::subscribeReply(size_t timeBaseIndex,
    const ptp_event &ptpData, const chrony_event &chronyData)
{
    // Check if ptpData is not empty before setting PTP state
    if(!isPTPDataEmpty(ptpData))
        setTimeBaseStatePtp(timeBaseIndex, ptpData);
    // Check if chronyData is not empty before setting system clock state
    if(!isChronyDataEmpty(chronyData))
        setTimeBaseStateSys(timeBaseIndex, chronyData);
    unique_lock<rtpi::mutex> lock(subscribe_mutex);
    setSubscribed(timeBaseIndex, true);
    subscribe_cv.notify_one(lock);
    return true;
}
