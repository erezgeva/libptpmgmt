/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Set and get the timebase subscribe event state
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include "client/timebase_state.hpp"
#include "client/msgq_tport.hpp"
#include "common/clkmgrtypes.hpp"
#include "common/message.hpp"
#include "common/print.hpp"
#include "common/transport.hpp"

#include <cstring>
#include <string>

__CLKMGR_NAMESPACE_USE;

using namespace std;

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

const Event_count &TimeBaseState::get_eventStateCount()
{
    return eventStateCount;
}

Event_state &TimeBaseState::get_eventState()
{
    return eventState;
}

void TimeBaseState::set_eventStateCount(const Event_count &newCount)
{
    eventStateCount = newCount;
}

void TimeBaseState::set_eventState(const Event_state &newState)
{
    eventState = newState;
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

const ClkMgrSubscription &TimeBaseState::get_eventSub()
{
    return eventSub;
}

void TimeBaseState::set_eventSub(const ClkMgrSubscription &eSub)
{
    eventSub.set_ClkMgrSubscription(eSub);
}

void TimeBaseState::set_last_notification_time(const timespec &newTime)
{
    last_notification_time = newTime;
}

timespec TimeBaseState::get_last_notification_time() const
{
    return last_notification_time;
}

bool TimeBaseStates::getTimeBaseState(int timeBaseIndex, TimeBaseState &state)
{
    std::lock_guard<rtpi::mutex> lock(mtx);
    auto it = timeBaseStateMap.find(timeBaseIndex);
    if(it != timeBaseStateMap.end()) {
        state = it->second; // Copy the TimeBaseState object
        it->second.set_eventStateCount({}); // reset eventStateCount
        it->second.set_event_changed(false); // reset event_changed
        it->second.get_eventState().gm_changed = false; // reset gm_changed
        return true;
    }
    // If timeBaseIndex is not found, return false
    return false;
}

void TimeBaseStates::setTimeBaseState(int timeBaseIndex,
    const ptp_event &newEvent)
{
    std::lock_guard<rtpi::mutex> lock(mtx);
    auto &state = timeBaseStateMap[timeBaseIndex];
    // Update the notification timestamp
    timespec last_notification_time = {};
    if(clock_gettime(CLOCK_REALTIME, &last_notification_time) == -1)
        PrintDebug("Failed to update notification time.");
    else
        state.set_last_notification_time(last_notification_time);
    // Get a copy of subscription mask
    ClkMgrSubscription sub = state.get_eventSub();
    uint32_t eventSub = sub.get_event_mask();
    uint32_t composite_eventSub = sub.get_composite_event_mask();
    // Get the current state of the timebase
    Event_state &eventState = state.get_eventState();
    Event_count eventCount = state.get_eventStateCount();
    // Update eventGMOffset
    if((eventSub & eventGMOffset) &&
        (newEvent.master_offset != eventState.clock_offset)) {
        eventState.clock_offset = newEvent.master_offset;
        if(sub.in_range(thresholdGMOffset, eventState.clock_offset)) {
            if(!(eventState.offset_in_range)) {
                eventState.offset_in_range = true;
                eventCount.offset_in_range_event_count++;
                state.set_event_changed(true);
            }
        } else {
            if((eventState.offset_in_range)) {
                eventState.offset_in_range = false;
                eventCount.offset_in_range_event_count++;
                state.set_event_changed(true);
            }
        }
    }
    // Update eventSyncedToGM
    if((eventSub & eventSyncedToGM) &&
        (newEvent.synced_to_primary_clock !=
            eventState.synced_to_primary_clock)) {
        eventState.synced_to_primary_clock =
            newEvent.synced_to_primary_clock;
        eventCount.synced_to_gm_event_count++;
        state.set_event_changed(true);
    }
    // Update eventGMChanged
    if((eventSub & eventGMChanged) &&
        (memcmp(eventState.gm_identity, newEvent.gm_identity,
                sizeof(newEvent.gm_identity)) != 0)) {
        memcpy(eventState.gm_identity, newEvent.gm_identity,
            sizeof(newEvent.gm_identity));
        eventState.gm_changed = true;
        eventCount.gm_changed_event_count++;
        state.set_event_changed(true);
    }
    // Update eventASCapable
    if((eventSub & eventASCapable) &&
        (newEvent.as_capable != eventState.as_capable)) {
        eventState.as_capable = newEvent.as_capable;
        eventCount.as_capable_event_count++;
        state.set_event_changed(true);
    }
    // Update composite event
    bool composite_event = true;
    if(composite_eventSub & eventGMOffset)
        composite_event &= eventState.offset_in_range;
    if(composite_eventSub & eventSyncedToGM)
        composite_event &= eventState.synced_to_primary_clock;
    if(composite_eventSub & eventASCapable)
        composite_event &= eventState.as_capable;
    if(composite_eventSub &&
        (composite_event != eventState.composite_event)) {
        eventState.composite_event = composite_event;
        eventCount.composite_event_count++;
        state.set_event_changed(true);
    }
    // Update notification timestamp
    eventState.notification_timestamp = last_notification_time.tv_sec;
    eventState.notification_timestamp *= NSEC_PER_SEC;
    eventState.notification_timestamp += last_notification_time.tv_nsec;
    // Update GM logSyncInterval
    eventState.ptp4l_sync_interval = newEvent.ptp4l_sync_interval;
    // Update Chrony clock offset
    if(newEvent.chrony_offset != eventState.chrony_clock_offset) {
        eventState.chrony_clock_offset = newEvent.chrony_offset;
        if(sub.in_range(thresholdChronyOffset,
                eventState.chrony_clock_offset)) {
            if(!(eventState.chrony_offset_in_range)) {
                eventState.chrony_offset_in_range = true;
                eventCount.chrony_offset_in_range_event_count++;
                state.set_event_changed(true);
            }
        } else {
            if((eventState.chrony_offset_in_range)) {
                eventState.chrony_offset_in_range = false;
                eventCount.chrony_offset_in_range_event_count++;
                state.set_event_changed(true);
            }
        }
    }
    eventState.chrony_reference_id = newEvent.chrony_reference_id;
    eventState.polling_interval = newEvent.polling_interval;
    state.set_eventStateCount(eventCount);
}
