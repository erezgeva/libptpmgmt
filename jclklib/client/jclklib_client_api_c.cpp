/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file jclklib_client_api_c.cpp
 * @brief Client utilities to setup and cleanup the library.
 *
 * @author Song Yoong Siang <yoong.siang.song@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#include "c/jclklib_client_api_c.h"
#include "jclk_init.hpp"

jcl_c_client_ptr jcl_c_client_create()
{
    return new JClkLibClient::JClkLibClientApi();
}

void jcl_c_client_destroy(jcl_c_client_ptr client_ptr)
{
    delete static_cast<JClkLibClient::JClkLibClientApi *>(client_ptr);
}

bool jcl_c_connect(jcl_c_client_ptr client_ptr)
{
    return static_cast<JClkLibClient::JClkLibClientApi *>
        (client_ptr)->jcl_connect();
}

bool jcl_c_disconnect(jcl_c_client_ptr client_ptr)
{
    return static_cast<JClkLibClient::JClkLibClientApi *>
        (client_ptr)->jcl_disconnect();
}

bool jcl_c_subscribe(jcl_c_client_ptr client_ptr,
    struct jcl_c_subscription sub,
    struct jcl_c_state *current_state)
{
    JClkLibCommon::jcl_subscription newsub = {};
    JClkLibClient::jcl_state state = {};
    bool ret;
    newsub.get_event().writeEvent(sub.event, sizeof(sub.event));
    newsub.get_value().setValue(gm_offset, sub.value[gm_offset].upper,
        sub.value[gm_offset].lower);
    newsub.get_composite_event().writeEvent(sub.composite_event,
        sizeof(sub.composite_event));
    ret = static_cast<JClkLibClient::JClkLibClientApi *>(client_ptr)->jcl_subscribe(
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

int jcl_c_status_wait(jcl_c_client_ptr client_ptr, int timeout,
    struct jcl_c_state *current_state,
    struct jcl_c_event_count *event_count)
{
    JClkLibClient::jcl_state_event_count eventCount = {};
    JClkLibClient::jcl_state state = {};
    int ret;
    ret = static_cast<JClkLibClient::JClkLibClientApi *>
        (client_ptr)->jcl_status_wait(timeout, state, eventCount);
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
