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

bool clkmgr_connect()
{
    return clkmgr::ClockManager::connect();
}
bool clkmgr_disconnect()
{
    return clkmgr::ClockManager::disconnect();
}
bool clkmgr_get_timebase_cfgs(size_t time_base_index, Clkmgr_TimeBaseCfg *cfg)
{
    if(cfg == nullptr || time_base_index == 0)
        return false;
    const auto &cfgs = clkmgr::ClockManager::clkmgr_get_timebase_cfgs();
    if(time_base_index > cfgs.size())
        return false;
    const auto &rcfg = cfgs[time_base_index - 1];
    strncpy(cfg->timeBaseName, rcfg.timeBaseName, CLKMGR_STRING_SIZE_MAX - 1);
    strncpy(cfg->interfaceName, rcfg.interfaceName, CLKMGR_STRING_SIZE_MAX - 1);
    cfg->timeBaseIndex = rcfg.timeBaseIndex;
    cfg->transportSpecific = rcfg.transportSpecific;
    cfg->domainNumber = rcfg.domainNumber;
    return true;
}

static inline int timeBaseName2Index(const char *timeBaseName)
{
    if(timeBaseName != nullptr)
        for(const auto &cfg : clkmgr::ClockManager::clkmgr_get_timebase_cfgs())
            if(strncmp(cfg.timeBaseName, timeBaseName, CLKMGR_STRING_SIZE_MAX) == 0)
                return cfg.timeBaseIndex;
    return -1;
}

size_t clkmgr_get_timebase_cfgs_size()
{
    return clkmgr::ClockManager::clkmgr_get_timebase_cfgs().size();
}

bool clkmgr_subscribe_by_name(const clkmgr_c_subscription sub,
    const char *timeBaseName, Clkmgr_Event_state *cur_stat)
{
    if(cur_stat == nullptr)
        return false;
    int timeBaseIndex = timeBaseName2Index(timeBaseName);
    if(timeBaseIndex < 0)
        return false;
    return clkmgr_subscribe(sub, timeBaseIndex, cur_stat);
}

bool clkmgr_subscribe(const clkmgr_c_subscription sub, size_t time_base_index,
    Clkmgr_Event_state *cur_stat)
{
    if(cur_stat == nullptr || time_base_index == 0)
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
    ret = clkmgr::ClockManager::subscribe(newsub, time_base_index, state);
    if(ret) {
        cur_stat->as_capable = state.as_capable;
        cur_stat->offset_in_range = state.offset_in_range;
        cur_stat->synced_to_primary_clock = state.synced_to_primary_clock;
        cur_stat->gm_changed = state.gm_changed;
        cur_stat->composite_event = state.composite_event;
        cur_stat->clock_offset = state.clock_offset;
        cur_stat->notification_timestamp = state.notification_timestamp;
        std::copy(std::begin(state.gm_identity), std::end(state.gm_identity),
            std::begin(cur_stat->gm_identity));
        cur_stat->chrony_clock_offset = state.chrony_clock_offset;
        cur_stat->chrony_reference_id = state.chrony_reference_id;
        cur_stat->chrony_offset_in_range = state.chrony_offset_in_range;
        cur_stat->polling_interval = state.polling_interval;
    }
    return ret;
}

int clkmgr_status_wait_by_name(int timeout, const char *timeBaseName,
    Clkmgr_Event_state *cur_stat, Clkmgr_Event_count *cur_cnt)
{
    if(cur_stat == nullptr)
        return -1;
    int timeBaseIndex = timeBaseName2Index(timeBaseName);
    if(timeBaseIndex < 0)
        return -1;
    return clkmgr_status_wait(timeout, timeBaseIndex, cur_stat, cur_cnt);
}

int clkmgr_status_wait(int timeout, size_t time_base_index,
    Clkmgr_Event_state *cur_stat, Clkmgr_Event_count *cur_cnt)
{
    if(cur_stat == nullptr || cur_cnt == nullptr || time_base_index == 0)
        return -1;
    clkmgr::Event_count eventCount = {};
    clkmgr::Event_state state = {};
    int ret = clkmgr::ClockManager::status_wait(timeout, time_base_index, state,
            eventCount);
    if(ret < 0)
        return ret;
    cur_stat->as_capable = state.as_capable;
    cur_stat->offset_in_range = state.offset_in_range;
    cur_stat->synced_to_primary_clock = state.synced_to_primary_clock;
    cur_stat->gm_changed = state.gm_changed;
    cur_stat->composite_event = state.composite_event;
    cur_stat->clock_offset = state.clock_offset;
    cur_stat->notification_timestamp = state.notification_timestamp;
    std::copy(std::begin(state.gm_identity), std::end(state.gm_identity),
        std::begin(cur_stat->gm_identity));
    cur_stat->chrony_clock_offset = state.chrony_clock_offset;
    cur_stat->chrony_reference_id = state.chrony_reference_id;
    cur_stat->chrony_offset_in_range = state.chrony_offset_in_range;
    cur_stat->polling_interval = state.polling_interval;
    if(ret > 0) {
        cur_cnt->as_capable_event_count = eventCount.as_capable_event_count;
        cur_cnt->composite_event_count = eventCount.composite_event_count;
        cur_cnt->gm_changed_event_count = eventCount.gm_changed_event_count;
        cur_cnt->offset_in_range_event_count =
            eventCount.offset_in_range_event_count;
        cur_cnt->synced_to_gm_event_count = eventCount.synced_to_gm_event_count;
        cur_cnt->chrony_offset_in_range_event_count =
            eventCount.chrony_offset_in_range_event_count;
    }
    return ret;
}

bool clkmgr_gettime(timespec *ts)
{
    return ts != nullptr && clock_gettime(CLOCK_REALTIME, ts) == 0;
}
