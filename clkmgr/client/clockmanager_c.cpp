/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client utilities to setup and cleanup the library.
 *
 * @author Song Yoong Siang <yoong.siang.song@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "pub/clockmanager.h"
#include "pub/clkmgr/clockmanager_c.h"
#include "pub/clkmgr/timebase_configs_c.h"
#include "client/opaque_struct_c.hpp"

#include <cstring>

__CLKMGR_NAMESPACE_USE;

bool clkmgr_connect()
{
    return ClockManager::connect();
}
bool clkmgr_disconnect()
{
    return ClockManager::disconnect();
}

size_t clkmgr_getTimebaseCfgsSize()
{
    return TimeBaseConfigurations::size();
}

bool clkmgr_subscribeByName(const Clkmgr_Subscription *sub_c,
    const char *timeBaseName, Clkmgr_ClockSyncData *data_c)
{
    if(!data_c)
        return false;
    size_t timeBaseIndex = 0;
    if(TimeBaseConfigurations::BaseNameToBaseIndex(timeBaseName, timeBaseIndex))
        return clkmgr_subscribe(sub_c, timeBaseIndex, data_c);
    return false;
}

bool clkmgr_subscribe(const Clkmgr_Subscription *sub_c,
    size_t timeBaseIndex, Clkmgr_ClockSyncData *data_c)
{
    if(!sub_c || timeBaseIndex == 0 || !data_c)
        return false;
    // Create a C++ subscription object and set its parameters from the C struct
    ClockSyncSubscription newSub;
    if(clkmgr_isSubscriptionEnabled(sub_c, Clkmgr_PTPClock)) {
        newSub.setPtpSubscription(*sub_c->ptp);
        newSub.enablePtpSubscription();
    }
    if(clkmgr_isSubscriptionEnabled(sub_c, Clkmgr_SysClock)) {
        newSub.setSysSubscription(*sub_c->sys);
        newSub.enableSysSubscription();
    }
    return ClockManager::subscribe(newSub, timeBaseIndex, *data_c->data);
}

int clkmgr_statusWaitByName(int timeout, const char *timeBaseName,
    Clkmgr_ClockSyncData *data_c)
{
    if(!data_c)
        return -1;
    size_t timeBaseIndex = 0;
    if(TimeBaseConfigurations::BaseNameToBaseIndex(timeBaseName, timeBaseIndex))
        return clkmgr_statusWait(timeout, timeBaseIndex, data_c);
    return -1;
}

int clkmgr_statusWait(int timeout, size_t timeBaseIndex,
    Clkmgr_ClockSyncData *data_c)
{
    if(timeBaseIndex == 0 || !data_c)
        return -1;
    return ClockManager::statusWait(timeout, timeBaseIndex, *data_c->data);
}

bool clkmgr_getTime(timespec *ts)
{
    return ts != nullptr && clock_gettime(CLOCK_REALTIME, ts) == 0;
}
