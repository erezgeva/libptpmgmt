/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief The Clock Manager APIs to set up client-runtime.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLOCKMANAGER_H
#define CLOCKMANAGER_H

#ifdef __cplusplus

#include "pub/clkmgr/subscription.h"
#include <memory>
#include <vector>

__CLKMGR_NAMESPACE_BEGIN

class ClientState; /* Forward declaration */

/**
 * Class to provide APIs to set up and manage the client-runtime.
 * @note the class is singelton
 */
class ClockManager
{
  private:
    /**
     * Implementation pointer to current state of client-runtime.
     */
    std::unique_ptr<ClientState> implClientState;

    /**
     * Constructor
     */
    ClockManager();

    /**
     * Destructor
     */
    ~ClockManager();

  public:

    /**
     * Fetch single class object
     * @return reference to single class object
     */
    static ClockManager &FetchSingle();

    /**
     * Initialize the Clock Manager library
     * @return true on success
     */
    bool init();

    /**
     * Establish connection between Client and Proxy
     * @return true on success, false on failure
     */
    bool clkmgr_connect();

    /**
     * Remove the connection between Client and Proxy
     * @return true on success, false on failure
     */
    bool clkmgr_disconnect();

    /**
     * Get the time base configurations
     * @return vector of TimeBaseCfg
     */
    std::vector<TimeBaseCfg> clkmgr_get_timebase_cfgs() const;

    /**
     * Subscribe to events by name of the time base
     * @param[in] newSub Reference to the new subscription
     * @param[in] timeBaseName Name of the time base to be subscribed
     * @param[out] currentState Reference to the current state
     * @return true on success, false on failure
     */
    bool clkmgr_subscribe_by_name(const ClkMgrSubscription &newSub,
        const std::string &timeBaseName, Event_state &currentState);

    /**
     * Subscribe to events
     * @param[in] newSub Reference to the new subscription
     * @param[in] timeBaseIndex Index of the time base to be subscribed
     * @param[out] currentState Reference to the current state
     * @return true on success, false on failure
     */
    bool clkmgr_subscribe(const ClkMgrSubscription &newSub, int timeBaseIndex,
        Event_state &currentState);

    /**
     * Waits for a specified timeout period for any event changes by
     * name of the time base.
     * @param[in] timeout in seconds
     * @li Use 0 to check without waiting
     * @li Use -1 to wait until there is event changes occurs.
     * @param[in] timeBaseName Name of the time base to be monitored
     * @param[out] currentState Reference to the current event state
     * @param[out] currentCount Reference to the current event count
     * @return result
     * @li 1 when an event changes within the timeout period
     * @li 0 No event changes
     * @li -1 lost connection to the Clock manager Proxy
     */
    int clkmgr_status_wait_by_name(int timeout, const std::string &timeBaseName,
        Event_state &currentState, Event_count &currentCount);

    /**
     * Waits for a specified timeout period for any event changes.
     * @param[in] timeout in seconds
     * @li Use 0 to check without waiting
     * @li Use -1 to wait until there is event changes occurs.
     * @param[in] timeBaseIndex Index of the time base to be monitored
     * @param[out] currentState Reference to the current event state
     * @param[out] currentCount Reference to the current event count
     * @return result
     * @li 1 when an event changes within the timeout period
     * @li 0 No event changes
     * @li -1 lost connection to the Clock manager Proxy
     */
    int clkmgr_status_wait(int timeout, int timeBaseIndex,
        Event_state &currentState, Event_count &currentCount);

    /**
     * Retrieve the time of the CLOCK_REALTIME
     * @param[out] ts timestamp of the CLOCK_REALTIME
     * @return true on success
     */
    bool clkmgr_gettime(timespec &ts);
};

__CLKMGR_NAMESPACE_END
#else /* __cplusplus */
#include "pub/clkmgr/clockmanager_c.h"
#endif /* __cplusplus */

#endif /* CLOCKMANAGER_H */
