/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief The Clock Manager APIs to set up and manage the Client.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLOCKMANAGER_H
#define CLOCKMANAGER_H

#ifdef __cplusplus

#include "pub/clkmgr/event.h"
#include "pub/clkmgr/subscription.h"
#include "pub/clkmgr/timebase_configs.h"
#include <memory>

__CLKMGR_NAMESPACE_BEGIN

/**
 * Provide APIs to set up and manage the Client.
 * @note This is a singleton class
 */
class ClockManager
{
  private:
    ClockManager() = default;

  public:

    /**
     * Fetch the single instance of object
     * @return Reference to the single instance
     */
    static ClockManager &fetchSingleInstance();

    /**
     * Establish connection between Client and Proxy
     * @return True on success, false on failure
     */
    static bool connect();

    /**
     * Remove the connection between Client and Proxy
     * @return True on success, false on failure
     */
    static bool disconnect();

    /**
     * Get the configurations of available time-bases
     * @return Reference to the TimeBaseConfigurations object
     */
    static const TimeBaseConfigurations &getTimebaseCfgs();

    /**
     * Subscribe to specific time-base by providing timeBaseName and interested
     * events.
     * @param[in] newSub Reference to the subscription details
     * @param[in] timeBaseName Name of the time-base to be subscribed
     * @param[out] clockSyncData Reference to the current telemetry data and
     * synchronization errors
     * @return True on successful subscription, false on failure
     * @note Event counting will begin once this API is called
     */
    static bool subscribeByName(const ClkMgrSubscription &newSub,
        const std::string &timeBaseName,
        ClockSyncData &clockSyncData);

    /**
     * Subscribe to specific time-base by providing timeBaseIndex and interested
     * events.
     * @param[in] newSub Reference to the subscription details
     * @param[in] timeBaseIndex Index of the time-base to be subscribed
     * @param[out] clockSyncData Reference to the current telemetry data and
     * synchronization errors
     * @return True on successful subscription, false on failure
     * @note Event counting will begin once this API is called
     */
    static bool subscribe(const ClkMgrSubscription &newSub, size_t timeBaseIndex,
        ClockSyncData &clockSyncData);

    /**
     * Wait for status changes in the specified time-base by providing the
     * timeBaseName and a timeout duration
     * @param[in] timeout Timeout duration in seconds
     * @li -1: wait indefinitely until at least an event change occurs
     * @li 0: retrieve the latest clock sync data immediately
     * @param[in] timeBaseName Name of the time-base to be monitored
     * @param[out] clockSyncData Reference to the current telemetry data and
     * synchronization errors
     * @return Status of wait
     * @li -1: Client lost connection to Proxy
     * @li 0: no event changes occur within the timeout duration
     * @li 1: at least an event change occurs within the timeout duration
     * @note Calling this API will reset all event counts
     */
    static int statusWaitByName(int timeout, const std::string &timeBaseName,
        ClockSyncData &clockSyncData);

    /**
     * Wait for status changes in the specified time-base by providing the
     * timeBaseIndex and a timeout duration
     * @param[in] timeout Timeout duration in seconds
     * @li -1: wait indefinitely until at least an event change occurs
     * @li 0: retrieve the latest clock sync data immediately
     * @param[in] timeBaseIndex Index of the time-base to be monitored
     * @param[out] clockSyncData Reference to the current telemetry data and
     * synchronization errors
     * @return Status of wait
     * @li -1: Client lost connection to Proxy
     * @li 0: no event changes occur within the timeout duration
     * @li 1: at least an event change occurs within the timeout duration
     * @note Calling this API will reset all event counts
     */
    static int statusWait(int timeout, size_t timeBaseIndex,
        ClockSyncData &clockSyncData);

    /**
     * Retrieve the time of the CLOCK_REALTIME
     * @param[out] ts timestamp of the CLOCK_REALTIME
     * @return True on success, false on failure
     */
    static bool getTime(timespec &ts);
};

__CLKMGR_NAMESPACE_END
#else /* __cplusplus */
#include "pub/clkmgr/clockmanager_c.h"
#endif /* __cplusplus */

#endif /* CLOCKMANAGER_H */
