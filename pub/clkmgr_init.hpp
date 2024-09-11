/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file clkmgr_init.hpp
 * @brief Client utilities to setup and cleanup the library.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLKMGR_INIT_HPP
#define CLKMGR_INIT_HPP

#include <map>
#include "clkmgr_client_state.hpp"
#include "clkmgr_subscription.hpp"

__CLKMGR_NAMESPACE_BEGIN

/**
 * Clock Manager APIs
 */
class ClkmgrClientApi
{
  private:
    /**
     * Map of client states
     */
    static std::map <sessionId_t, ClientState> clientStateMap;
    /**
     * Application client state
     */
    ClientState appClientState;

  public:
    /**
     * Default constructor
     */
    ClkmgrClientApi() {}

    /**
     * Initialize the Clock Manager library
     * @return 0 on success
     */
    static int init() { return 0; }

    /**
     * Get the application client state
     * @return Reference to the application client state
     */
    ClientState &getClientState() { return appClientState; }

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
     * Subscribe to events
     * @param[in] newSub Reference to the new subscription
     * @param[out] currentState Reference to the current state
     * @return true on success, false on failure
     */
    bool clkmgr_subscribe(clkmgr_subscription &newSub,
        clkmgr_state &currentState);

    /**
     * Wait for client status
     * @param[in] timeout Timeout value in seconds
     * @param[out] clkmgr_state Reference to the current state
     * @param[out] eventCount Reference to the event count
     * @return Returns true if there is event changes within the timeout period,
     *         and false otherwise.
     */
    int clkmgr_status_wait(int timeout, clkmgr_state &clkmgr_state,
        clkmgr_state_event_count &eventCount);
};

__CLKMGR_NAMESPACE_END

#endif /* CLKMGR_INIT_HPP */
