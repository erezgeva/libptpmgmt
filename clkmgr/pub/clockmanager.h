/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file clockmanager.hpp
 * @brief The Clock Manager APIs to set up client-runtime.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLOCKMANAGER_H
#define CLOCKMANAGER_H

#ifdef __cplusplus
#include <map>

#include <clkmgr/client_state.h>
#include <clkmgr/event_state.h>
#include <clkmgr/subscription.h>
#include <clkmgr/utility.h>

__CLKMGR_NAMESPACE_BEGIN

/**
 * @class ClockManager
 * @brief Class to provide APIs to set up and manage the client-runtime.
 */
class ClockManager
{
  private:
    /**
     * @brief Map of multiple client states.
     */
    static std::map <sessionId_t, ClientState> clientStateMap;

    /**
     * @brief State of current client-runtime.
     */
    ClientState appClientState;

  public:
    /**
     * @brief Default constructor
     */
    ClockManager() = default;

    /**
     * @brief Initialize the Clock Manager library
     * @return 0 on success
     */
    static int init() { return 0; }

    /**
     * @brief Get the client state
     * @return Reference to the client state
     */
    ClientState &getClientState() { return appClientState; }

    /**
     * @brief Establish connection between Client and Proxy
     * @return true on success, false on failure
     */
    bool clkmgr_connect();

    /**
     * @brief Remove the connection between Client and Proxy
     * @return true on success, false on failure
     */
    bool clkmgr_disconnect();

    /**
     * @brief Subscribe to events
     * @param[in] newSub Reference to the new subscription
     * @param[out] currentState Reference to the current state
     * @return true on success, false on failure
     */
    bool clkmgr_subscribe(ClkMgrSubscription &newSub,
        clkmgr_event_state &currentState);

    /**
     * @brief Waits for a specified timeout period for any event changes.
     * @param[in] timeout The timeout in seconds. If timeout is 0, the function
     * will check event changes once. If timeout is -1, the function will wait
     * until there is event changes occurs.
     * @param[out] currentState Reference to the current event state
     * @param[out] currentCount Reference to the current event count
     * @return Returns true if there is event changes within the timeout period,
     * and false otherwise.
     */
    int clkmgr_status_wait(int timeout, clkmgr_event_state &currentState,
        clkmgr_event_count &currentCount);
};

__CLKMGR_NAMESPACE_END
#else /* __cplusplus */
#include "clkmgr/clockmanager_c.h"
#endif /* __cplusplus */

#endif /* CLOCKMANAGER_H */
