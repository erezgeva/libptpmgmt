/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief C wrapper to the Clock Manager APIs to set up Client and monitor clock
 * synchronization events
 *
 * @author Song Yoong Siang <yoong.siang.song@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLKMGR_CLOCKMANAGER_C_H
#define CLKMGR_CLOCKMANAGER_C_H

#include "pub/clkmgr/types_c.h"
#include "pub/clkmgr/timebase_configs_c.h"
#include "pub/clkmgr/subscription_c.h"
#include "pub/clkmgr/event_c.h"
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Connect the client
 * @return true on success, false on failure
 */
bool clkmgr_connect();

/**
 * Disconnect the client
 * @return true on success, false on failure
 */
bool clkmgr_disconnect();

/**
 * Get the size of the time base configurations
 * @return The size of the time base configurations
 */
size_t clkmgr_getTimebaseCfgsSize();

/**
 * Subscribe to client events by name for the time base
 * @param[in] sub_c Pointer to the Clkmgr_Subscription
 * @param[in] timeBaseName Name of the time base to be subscribed
 * @param[out] data_c Pointer to the Clkmgr_ClockSyncData
 * @return true on success, false on failure
 */
bool clkmgr_subscribeByName(const Clkmgr_Subscription *sub_c,
    const char *timeBaseName, Clkmgr_ClockSyncData *data_c);

/**
 * Subscribe to client events
 * @param[in] sub_c Pointer to the Clkmgr_Subscription
 * @param[in] timeBaseIndex Index of the time base to be subscribed
 * @param[out] data_c Pointer to the Clkmgr_ClockSyncData
 * @return true on success, false on failure
 */
bool clkmgr_subscribe(const Clkmgr_Subscription *sub_c,
    size_t timeBaseIndex, Clkmgr_ClockSyncData *data_c);

/**
 * Waits for a specified timeout period for any event changes by name of the
 * time base
 * @param[in] timeout The timeout in seconds. If timeout is 0, the function
 * will check event changes once. If timeout is -1, the function will wait
 * until there is event changes occurs
 * @param[in] timeBaseName Name of the time base to be monitored
 * @param[out] data_c Pointer to the Clkmgr_ClockSyncData
 * @return Status of wait
 * @li Clkmgr_SWRLostConnection: Lost connection to Proxy
 * @li Clkmgr_SWRInvalidArgument: Invalid argument
 * @li Clkmgr_SWRNoEventDetected: No event changes detected
 * @li Clkmgr_SWREventDetected: At least an event change detected
 */
enum Clkmgr_StatusWaitResult clkmgr_statusWaitByName(int timeout,
    const char *timeBaseName, Clkmgr_ClockSyncData *data_c);

/**
 * Waits for a specified timeout period for any event changes
 * @param[in] timeout TThe timeout in seconds. If timeout is 0, the function
 * will check event changes once. If timeout is -1, the function will wait
 * until there is event changes occurs
 * @param[in] timeBaseIndex Index of the time base to be monitored
 * @param[out] data_c Pointer to the Clkmgr_ClockSyncData
 * @return Status of wait
 * @li Clkmgr_SWRLostConnection: Lost connection to Proxy
 * @li Clkmgr_SWRInvalidArgument: Invalid argument
 * @li Clkmgr_SWRNoEventDetected: No event changes detected
 * @li Clkmgr_SWREventDetected: At least an event change detected
 */
enum Clkmgr_StatusWaitResult clkmgr_statusWait(int timeout,
    size_t timeBaseIndex, Clkmgr_ClockSyncData *data_c);

/**
 * Retrieve the time of the CLOCK_REALTIME
 * @param[out] ts timestamp of the CLOCK_REALTIME
 * @return true on success
 */
bool clkmgr_getTime(struct timespec *ts);

#ifdef __cplusplus
}
#endif

#endif /* CLKMGR_CLOCKMANAGER_C_H */
