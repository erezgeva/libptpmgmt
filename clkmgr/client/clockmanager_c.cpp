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
#include <cstring>

static inline clkmgr::ClockManager &c_fetch()
{
    return clkmgr::ClockManager::FetchSingle();
}
bool clkmgr_c_connect()
{
    return c_fetch().clkmgr_connect();
}
bool clkmgr_c_disconnect()
{
    return c_fetch().clkmgr_disconnect();
}
bool clkmgr_c_get_timebase_cfgs(int time_base_index,
    struct Clkmgr_TimeBaseCfg *cfg)
{
    if(cfg == nullptr)
        return false;
    std::vector<clkmgr::TimeBaseCfg> cfgs = c_fetch().clkmgr_get_timebase_cfgs();
    if(static_cast<size_t>(time_base_index) > cfgs.size() || !time_base_index)
        return false;
    cfg->timeBaseIndex = cfgs[time_base_index - 1].timeBaseIndex;
    strncpy(cfg->timeBaseName, cfgs[time_base_index - 1].timeBaseName,
        CLKMGR_STRING_SIZE_MAX - 1);
    strncpy(cfg->interfaceName, cfgs[time_base_index - 1].interfaceName,
        CLKMGR_STRING_SIZE_MAX - 1);
    cfg->transportSpecific = cfgs[time_base_index - 1].transportSpecific;
    cfg->domainNumber = cfgs[time_base_index - 1].domainNumber;
    return true;
}

size_t clkmgr_c_get_timebase_cfgs_size()
{
    return c_fetch().clkmgr_get_timebase_cfgs().size();
}

bool clkmgr_c_subscribe_by_name(const clkmgr_c_subscription sub,
    const char *timeBaseName,
    Clkmgr_Event_state *current_state)
{
    if(timeBaseName == nullptr || current_state == nullptr)
        return false;
    int timeBaseIndex = -1;
    std::vector<clkmgr::TimeBaseCfg> cfgs = c_fetch().clkmgr_get_timebase_cfgs();
    for(const auto &cfg : cfgs) {
        if(strncmp(cfg.timeBaseName, timeBaseName, CLKMGR_STRING_SIZE_MAX) == 0) {
            timeBaseIndex = cfg.timeBaseIndex;
            break;
        }
    }
    if(timeBaseIndex == -1)
        return false;
    return clkmgr_c_subscribe(sub, timeBaseIndex, current_state);
}

bool clkmgr_c_subscribe(const clkmgr_c_subscription sub, int time_base_index,
    Clkmgr_Event_state *current_state)
{
    if(current_state == nullptr)
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
    ret = c_fetch().clkmgr_subscribe(newsub, time_base_index, state);
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
        current_state->polling_interval = state.polling_interval;
    }
    return ret;
}

int clkmgr_c_status_wait_by_name(int timeout,
    const char *timeBaseName, Clkmgr_Event_state *current_state,
    Clkmgr_Event_count *current_count)
{
    if(timeBaseName == nullptr || current_state == nullptr)
        return -1;
    int timeBaseIndex = -1;
    std::vector<clkmgr::TimeBaseCfg> cfgs = c_fetch().clkmgr_get_timebase_cfgs();
    for(const auto &cfg : cfgs) {
        if(strncmp(cfg.timeBaseName, timeBaseName, CLKMGR_STRING_SIZE_MAX) == 0) {
            timeBaseIndex = cfg.timeBaseIndex;
            break;
        }
    }
    if(timeBaseIndex == -1)
        return -1;
    return clkmgr_c_status_wait(timeout, timeBaseIndex, current_state,
            current_count);
}

int clkmgr_c_status_wait(int timeout, int time_base_index,
    Clkmgr_Event_state *current_state, Clkmgr_Event_count *current_count)
{
    if(current_state == nullptr || current_count == nullptr)
        return -1;
    clkmgr::Event_count eventCount = {};
    clkmgr::Event_state state = {};
    int ret;
    ret = c_fetch().clkmgr_status_wait(timeout, time_base_index, state, eventCount);
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
    current_state->polling_interval = state.polling_interval;
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

bool clkmgr_c_gettime(struct timespec *ts)
{
    return ts != nullptr && clock_gettime(CLOCK_REALTIME, ts) == 0;
}
