/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client utilities to setup and cleanup the library.
 *
 * @author Song Yoong Siang <yoong.siang.song@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "pub/clkmgr/clockmanager_c.h"
#include "pub/clockmanager.h"

clkmgr_c_client_ptr clkmgr_c_client_fetch()
{
    return &(clkmgr::ClockManager::FetchSingle());
}

bool clkmgr_c_connect(clkmgr_c_client_ptr client_ptr)
{
    if(client_ptr == nullptr)
        return false;
    return static_cast<clkmgr::ClockManager *>
        (client_ptr)->clkmgr_connect();
}

bool clkmgr_c_disconnect(clkmgr_c_client_ptr client_ptr)
{
    if(client_ptr == nullptr)
        return false;
    return static_cast<clkmgr::ClockManager *>
        (client_ptr)->clkmgr_disconnect();
}

bool clkmgr_c_subscribe(clkmgr_c_client_ptr client_ptr,
    const struct clkmgr_c_subscription sub,
    struct Clkmgr_Event_state *current_state)
{
    if(client_ptr == nullptr || current_state == nullptr)
        return false;
    clkmgr::ClkMgrSubscription newsub = {};
    clkmgr::Event_state state = {};
    bool ret;
    newsub.set_event_mask(sub.event_mask);
    newsub.define_threshold(clkmgr::thresholdGMOffset,
        sub.threshold[Clkmgr_thresholdGMOffset].upper_limit,
        sub.threshold[Clkmgr_thresholdGMOffset].lower_limit);
    newsub.define_threshold(clkmgr::thresholdChronyOffset,
        sub.threshold[Clkmgr_thresholdChronyOffset].upper_limit,
        sub.threshold[Clkmgr_thresholdChronyOffset].lower_limit);
    newsub.set_composite_event_mask(sub.composite_event_mask);
    ret = static_cast<clkmgr::ClockManager *>(client_ptr)->clkmgr_subscribe(newsub,
            state);
    if(ret) {
        current_state->as_capable = state.as_capable;
        current_state->offset_in_range = state.offset_in_range;
        current_state->synced_to_primary_clock = state.synced_to_primary_clock;
        current_state->gm_changed = state.gm_changed;
        current_state->composite_event = state.composite_event;
        current_state->clock_offset = state.clock_offset;
        current_state->notification_timestamp = state.notification_timestamp;
        std::copy(std::begin(state.gm_identity), std::end(state.gm_identity),
            std::begin(current_state->gm_identity));
        current_state->chrony_clock_offset = state.chrony_clock_offset;
        current_state->chrony_reference_id = state.chrony_reference_id;
        current_state->chrony_offset_in_range = state.chrony_offset_in_range;
    }
    return ret;
}

int clkmgr_c_status_wait(clkmgr_c_client_ptr client_ptr, int timeout,
    struct Clkmgr_Event_state *current_state,
    struct Clkmgr_Event_count *current_count)
{
    if(client_ptr == nullptr || current_state == nullptr ||
        current_count == nullptr)
        return -1;
    clkmgr::Event_count eventCount = {};
    clkmgr::Event_state state = {};
    int ret;
    ret = static_cast<clkmgr::ClockManager *>
        (client_ptr)->clkmgr_status_wait(timeout, state, eventCount);
    if(ret < 0)
        return ret;
    current_state->as_capable = state.as_capable;
    current_state->offset_in_range = state.offset_in_range;
    current_state->synced_to_primary_clock = state.synced_to_primary_clock;
    current_state->gm_changed = state.gm_changed;
    current_state->composite_event = state.composite_event;
    current_state->clock_offset = state.clock_offset;
    current_state->notification_timestamp = state.notification_timestamp;
    std::copy(std::begin(state.gm_identity), std::end(state.gm_identity),
        std::begin(current_state->gm_identity));
    current_state->chrony_clock_offset = state.chrony_clock_offset;
    current_state->chrony_reference_id = state.chrony_reference_id;
    current_state->chrony_offset_in_range = state.chrony_offset_in_range;
    if(ret > 0) {
        current_count->as_capable_event_count = eventCount.as_capable_event_count;
        current_count->composite_event_count = eventCount.composite_event_count;
        current_count->gm_changed_event_count = eventCount.gm_changed_event_count;
        current_count->offset_in_range_event_count =
            eventCount.offset_in_range_event_count;
        current_count->synced_to_gm_event_count =
            eventCount.synced_to_gm_event_count;
        current_count->chrony_offset_in_range_event_count =
            eventCount.chrony_offset_in_range_event_count;
    }
    return ret;
}
