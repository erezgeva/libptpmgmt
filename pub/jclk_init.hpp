/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file jclk_init.hpp
 * @brief Client utilities to setup and cleanup the library.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef JCLK_INIT_HPP
#define JCLK_INIT_HPP

#include <map>
#include "jclk_client_state.hpp"
#include "jclk_subscription.hpp"

namespace JClkLibClient
{
/**
 * Clock Manager APIs
 */
class JClkLibClientApi
{
  private:
    /**
     * Map of client states
     */
    static std::map <JClkLibCommon::sessionId_t, ClientState> clientStateMap;
    /**
     * Application client state
     */
    ClientState appClientState;

  public:
    /**
     * Default constructor
     */
    JClkLibClientApi() {}

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
    bool jcl_connect();

    /**
     * Remove the connection between Client and Proxy
     * @return true on success, false on failure
     */
    bool jcl_disconnect();

    /**
     * Subscribe to events
     * @param[in] newSub Reference to the new subscription
     * @param[out] currentState Reference to the current state
     * @return true on success, false on failure
     */
    bool jcl_subscribe(JClkLibCommon::jcl_subscription &newSub,
        jcl_state &currentState);

    /**
     * Wait for client status
     * @param[in] timeout Timeout value in seconds
     * @param[out] jcl_state Reference to the current state
     * @param[out] eventCount Reference to the event count
     * @return Returns true if there is event changes within the timeout period,
     *         and false otherwise.
     */
    int jcl_status_wait(int timeout, jcl_state &jcl_state,
        jcl_state_event_count &eventCount);
};
}
#endif /* JCLK_INIT_HPP */
