/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file clkmgr_client_api_c.cpp
 * @brief Client utilities to setup and cleanup the library.
 *
 * @author Song Yoong Siang <yoong.siang.song@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#include "c/clkmgr_client_api_c.h"
#include "clkmgr_init.hpp"

clkmgr_c_client_ptr clkmgr_c_client_create()
{
    return new clkmgr::ClkmgrClientApi();
}

void clkmgr_c_client_destroy(clkmgr_c_client_ptr client_ptr)
{
    delete static_cast<clkmgr::ClkmgrClientApi *>(client_ptr);
}

bool clkmgr_c_connect(clkmgr_c_client_ptr client_ptr)
{
    return static_cast<clkmgr::ClkmgrClientApi *>
        (client_ptr)->clkmgr_connect();
}

bool clkmgr_c_disconnect(clkmgr_c_client_ptr client_ptr)
{
    return static_cast<clkmgr::ClkmgrClientApi *>
        (client_ptr)->clkmgr_disconnect();
}

bool clkmgr_c_subscribe(clkmgr_c_client_ptr client_ptr,
    struct clkmgr_c_subscription sub,
    struct clkmgr_c_state *current_state)
{
    clkmgr::clkmgr_subscription newsub = {};
    clkmgr::clkmgr_state state = {};
    bool ret;
    newsub.get_event().writeEvent(sub.event, sizeof(sub.event));
    newsub.get_value().setValue(gm_offset, sub.value[gm_offset].upper,
        sub.value[gm_offset].lower);
    newsub.get_composite_event().writeEvent(sub.composite_event,
        sizeof(sub.composite_event));
    ret = static_cast<clkmgr::ClkmgrClientApi *>(client_ptr)->clkmgr_subscribe(
            newsub, state);
    if(ret == false)
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
    return ret;
}

int clkmgr_c_status_wait(clkmgr_c_client_ptr client_ptr, int timeout,
    struct clkmgr_c_state *current_state,
    struct clkmgr_c_event_count *event_count)
{
    clkmgr::clkmgr_state_event_count eventCount = {};
    clkmgr::clkmgr_state state = {};
    int ret;
    ret = static_cast<clkmgr::ClkmgrClientApi *>
        (client_ptr)->clkmgr_status_wait(timeout, state, eventCount);
    if(ret <= 0)
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
    event_count->as_capable_event_count = eventCount.as_capable_event_count;
    event_count->composite_event_count = eventCount.composite_event_count;
    event_count->gm_changed_event_count = eventCount.gm_changed_event_count;
    event_count->offset_in_range_event_count =
        eventCount.offset_in_range_event_count;
    event_count->synced_to_primary_clock_event_count =
        eventCount.synced_to_primary_clock_event_count;
    return ret;
}
