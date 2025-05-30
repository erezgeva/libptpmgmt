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

static ClockEventHandler ptpClockEventHandler(ClockEventHandler::PTPClock);
static ClockEventHandler sysClockEventHandler(ClockEventHandler::SysClock);

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

const PTPClockEvent &TimeBaseState::get_ptp4lEventState() const
{
    return ptp4lEventState;
}

const SysClockEvent &TimeBaseState::get_chronyEventState() const
{
    return chronyEventState;
}

void TimeBaseState::set_ptpEventState(const PTPClockEvent &ptpState)
{
    ptp4lEventState = ptpState;
}

void TimeBaseState::set_chronyEventState(const SysClockEvent &chronyState)
{
    chronyEventState = chronyState;
}

string TimeBaseState::toString() const
{
    return string("[TimeBaseState::eventState]") +
        " as_capable = " + to_string(eventState.as_capable) +
        " gm_changed = " + to_string(eventState.gm_changed) +
        " offset_in_range = " + to_string(eventState.offset_in_range) +
        " synced_to_primary_clock = " +
        to_string(eventState.synced_to_primary_clock) + "\n";
}

const PTPClockSubscription &TimeBaseState::get_ptpEventSub() const
{
    return ptpEventSub;
}

const SysClockSubscription &TimeBaseState::get_sysEventSub() const
{
    return sysEventSub;
}

void TimeBaseState::set_ptpEventSub(const PTPClockSubscription &eSub)
{
    ptpEventSub.setEventMask(eSub.getEventMask());
    ptpEventSub.setClockOffsetThreshold(eSub.getClockOffsetThreshold());
    ptpEventSub.setCompositeEventMask(eSub.getCompositeEventMask());
}

void TimeBaseState::set_sysEventSub(const SysClockSubscription &eSub)
{
    sysEventSub.setEventMask(eSub.getEventMask());
    sysEventSub.setClockOffsetThreshold(eSub.getClockOffsetThreshold());
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
        PTPClockEvent ptp4lEventState = it->second.get_ptp4lEventState();
        SysClockEvent chronyEventState = it->second.get_chronyEventState();
        // Reset the Event Count
        ptpClockEventHandler.setOffsetInRangeEventCount(ptp4lEventState, 0);
        ptpClockEventHandler.setSyncedWithGmEventCount(ptp4lEventState, 0);
        ptpClockEventHandler.setGmChangedEventCount(ptp4lEventState, 0);
        ptpClockEventHandler.setAsCapableEventCount(ptp4lEventState, 0);
        ptpClockEventHandler.setCompositeEventCount(ptp4lEventState, 0);
        ptpClockEventHandler.setGmChanged(ptp4lEventState, false);
        it->second.set_ptpEventState(ptp4lEventState);
        sysClockEventHandler.setOffsetInRangeEventCount(chronyEventState, 0);
        it->second.set_chronyEventState(chronyEventState);
        it->second.set_event_changed(false);
        return true;
    }
    // If timeBaseIndex is not found, return false
    return false;
}

void TimeBaseStates::setTimeBaseState(size_t timeBaseIndex,
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
    SysClockSubscription sysSub = state.get_sysEventSub();
    uint32_t ptpEventSub = ptpSub.getEventMask();
    uint32_t ptpCompositeEventSub = ptpSub.getCompositeEventMask();
    int32_t ptpThreshold = static_cast<uint32_t>(ptpSub.getClockOffsetThreshold());
    int32_t sysThreshold = static_cast<uint32_t>(sysSub.getClockOffsetThreshold());
    //uint32_t sysEventSub = sysSub.getEventMask();
    // Get the current state of the timebase
    PTPClockEvent ptp4lEventState = state.get_ptp4lEventState();
    SysClockEvent chronyEventState = state.get_chronyEventState();
    // Update eventGMOffset
    if((ptpEventSub & eventGMOffset) &&
        (newEvent.master_offset != ptp4lEventState.getClockOffset())) {
        ptpClockEventHandler.setClockOffset(ptp4lEventState,
            newEvent.master_offset);
        bool ptpInRange = (ptp4lEventState.getClockOffset() >= -ptpThreshold)
            && (ptp4lEventState.getClockOffset() <= ptpThreshold);
        if(ptpInRange) {
            if(!(ptp4lEventState.isOffsetInRange())) {
                ptpClockEventHandler.setOffsetInRange(ptp4lEventState, true);
                ptpClockEventHandler.setOffsetInRangeEventCount(ptp4lEventState,
                    ptp4lEventState.getOffsetInRangeEventCount() + 1);
                state.set_event_changed(true);
            }
        } else {
            if((ptp4lEventState.isOffsetInRange())) {
                ptpClockEventHandler.setOffsetInRange(ptp4lEventState, false);
                ptpClockEventHandler.setOffsetInRangeEventCount(ptp4lEventState,
                    ptp4lEventState.getOffsetInRangeEventCount() + 1);
                state.set_event_changed(true);
            }
        }
    }
    // Update eventSyncedToGM
    if((ptpEventSub & eventSyncedToGM) &&
        (newEvent.synced_to_primary_clock !=
            ptp4lEventState.isSyncedWithGm())) {
        ptpClockEventHandler.setSyncedWithGm(ptp4lEventState,
            newEvent.synced_to_primary_clock);
        ptpClockEventHandler.setSyncedWithGmEventCount(ptp4lEventState,
            ptp4lEventState.getSyncedWithGmEventCount() + 1);
        state.set_event_changed(true);
    }
    // Update eventGMChanged
    uint64_t sourceClockUUID = ptp4lEventState.getGmIdentity();
    uint8_t sourceClockUUIDBytes[8];
    for(int i = 0; i < 8; ++i) {
        sourceClockUUIDBytes[i] =
            static_cast<uint8_t>(sourceClockUUID >>(8 * (7 - i)));
    }
    if((ptpEventSub & eventGMChanged) &&
        (memcmp(sourceClockUUIDBytes, newEvent.gm_identity,
                sizeof(newEvent.gm_identity)) != 0)) {
        uint64_t identity = 0;
        for(int i = 0; i < 8; ++i) {
            identity |=
                static_cast<uint64_t>(newEvent.gm_identity[i]) << (8 * (7 - i));
        }
        ptpClockEventHandler.setGmIdentity(ptp4lEventState, identity);
        ptpClockEventHandler.setGmChanged(ptp4lEventState, true);
        ptpClockEventHandler.setGmChangedEventCount(ptp4lEventState,
            ptp4lEventState.getGmChangedEventCount() + 1);
        state.set_event_changed(true);
    }
    // Update eventASCapable
    if((ptpEventSub & eventASCapable) &&
        (newEvent.as_capable != ptp4lEventState.isAsCapable())) {
        ptpClockEventHandler.setAsCapable(ptp4lEventState, newEvent.as_capable);
        ptpClockEventHandler.setAsCapableEventCount(ptp4lEventState,
            ptp4lEventState.getAsCapableEventCount() + 1);
        state.set_event_changed(true);
    }
    // Update composite event
    bool composite_event = true;
    if(ptpCompositeEventSub & eventGMOffset)
        composite_event &= ptp4lEventState.isOffsetInRange();
    if(ptpCompositeEventSub & eventSyncedToGM)
        composite_event &= ptp4lEventState.isSyncedWithGm();
    if(ptpCompositeEventSub & eventASCapable)
        composite_event &= ptp4lEventState.isAsCapable();
    if(ptpCompositeEventSub &&
        (composite_event != ptp4lEventState.isCompositeEventMet())) {
        ptpClockEventHandler.setCompositeEvent(ptp4lEventState,
            composite_event);
        ptpClockEventHandler.setCompositeEventCount(ptp4lEventState,
            ptp4lEventState.getCompositeEventCount() + 1);
        state.set_event_changed(true);
    }
    // Update notification timestamp
    uint64_t notification_timestamp = last_notification_time.tv_sec;
    notification_timestamp *= NSEC_PER_SEC;
    notification_timestamp += last_notification_time.tv_nsec;
    ptpClockEventHandler.setNotificationTimestamp(ptp4lEventState,
        notification_timestamp);
    // Update GM logSyncInterval
    ptpClockEventHandler.setSyncInterval(ptp4lEventState,
        newEvent.ptp4l_sync_interval);
    // Update Chrony clock offset
    if(newEvent.chrony_offset != chronyEventState.getClockOffset()) {
        sysClockEventHandler.setClockOffset(chronyEventState,
            newEvent.chrony_offset);
        bool sysInRange = (chronyEventState.getClockOffset() >= -sysThreshold) &&
            (chronyEventState.getClockOffset() <= sysThreshold);
        if(sysInRange) {
            if(!(chronyEventState.isOffsetInRange())) {
                sysClockEventHandler.setOffsetInRange(chronyEventState, true);
                sysClockEventHandler.setOffsetInRangeEventCount(chronyEventState,
                    chronyEventState.getOffsetInRangeEventCount() + 1);
                state.set_event_changed(true);
            }
        } else {
            if((chronyEventState.isOffsetInRange())) {
                sysClockEventHandler.setOffsetInRange(chronyEventState, false);
                sysClockEventHandler.setOffsetInRangeEventCount(chronyEventState,
                    chronyEventState.getOffsetInRangeEventCount() + 1);
                state.set_event_changed(true);
            }
        }
    }
    sysClockEventHandler.setGmIdentity(chronyEventState,
        newEvent.chrony_reference_id);
    sysClockEventHandler.setSyncInterval(chronyEventState,
        newEvent.polling_interval);
    state.set_chronyEventState(chronyEventState);
    state.set_ptpEventState(ptp4lEventState);
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
        setPtpEventSubscription(timeBaseIndex, newPtpSub);
    }
    // ToDo: Check whether system clock is available for subscription
    if(newSub.isSysSubscriptionEnable()) {
        const SysClockSubscription &newSysSub = newSub.getSysSubscription();
        setSysEventSubscription(timeBaseIndex, newSysSub);
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
    ClientState::sendMessage(cmsg);
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

bool TimeBaseStates::subscribeReply(size_t timeBaseIndex, const ptp_event &data)
{
    setTimeBaseState(timeBaseIndex, data);
    unique_lock<rtpi::mutex> lock(subscribe_mutex);
    setSubscribed(timeBaseIndex, true);
    subscribe_cv.notify_one(lock);
    return true;
}
