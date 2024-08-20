/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file jclk_client_state.hpp
 * @brief Set and get the client subscribe event state
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef PROXY_CLIENT_STATE
#define PROXY_CLIENT_STATE

#include <atomic>
#include <string>

#include <common/jcltypes.hpp>
#include <common/transport.hpp>
#include <common/util.hpp>
#include "jclk_subscription.hpp"

namespace JClkLibClient
{
/**
 * Current State for the events
 */
struct jcl_state {
    uint8_t  gm_identity[8]; /**< Primary clock UUID */
    bool     as_capable; /**< IEEE 802.1AS capable */
    bool     offset_in_range; /**< Clock offset in range */
    bool     synced_to_primary_clock; /**< Synced to primary clock */
    bool     gm_changed; /**< Primary clock UUID changed */
    bool     composite_event; /**< Composite event */
};

/**
 * Event count for the events
 */
struct jcl_state_event_count {
    uint64_t offset_in_range_event_count; /**< Clk offset in range */
    uint64_t gm_changed_event_count; /**< Primary clk ID changed */
    uint64_t as_capable_event_count; /**< IEEE 802.1AS capable */
    uint64_t synced_to_primary_clock_event_count; /**< Synced to primary clk */
    uint64_t composite_event_count; /**< Composite event */
};

/**
 * Class to manage the client state
 */
class ClientState
{
  private:
    std::atomic_bool connected = false; /**< Connection status */
    std::atomic_bool subscribed = false; /**< Subscription status */
    /**
     * Session ID
     */
    JClkLibCommon::sessionId_t sessionId = JClkLibCommon::InvalidSessionId;
    JClkLibCommon::TransportClientId clientID = {}; /**< Client ID */
    uint8_t ptp4l_id = 0; /**< PTP4L ID */
    jcl_state eventState = {}; /**< Event state */
    jcl_state_event_count eventStateCount = {}; /**< Event count */
    JClkLibCommon::jcl_subscription eventSub = {}; /**< Event subscription */
    struct timespec last_notification_time; /**< Last notification time */

  public:
    /**
     * Default constructor
     */
    ClientState();

    /**
     * Copy constructor
     * @param newState Reference to the new state
     */
    ClientState(ClientState &newState);

    /**
     * @brief Set the client state
     * @param newState Reference to the new state
     */
    void set_clientState(ClientState &newState);

    /**
     * @brief Get the connection status
     * @return true if connected, false otherwise
     */
    bool get_connected();

    /**
     * @brief Set the connection status
     * @param state Connection status
     */
    void set_connected(bool state);

    /**
     * @brief Get the subscription status
     * @return true if subscribed, false otherwise
     */
    bool get_subscribed();

    /**
     * @brief Set the subscription status
     * @param subscriptionState Subscription status
     */
    void set_subscribed(bool subscriptionState);

    /**
     * @brief Get the client ID
     * @return Client ID
     */
    JClkLibCommon::TransportClientId get_clientID();

    /**
     * @brief Set the client ID
     * @param cID Reference to the client ID
     */
    void set_clientID(JClkLibCommon::TransportClientId &cID);

    /**
     * @brief Get the event state counts
     * @return Reference to the event state counts
     */
    jcl_state_event_count &get_eventStateCount();

    /**
     * @brief Get the event state
     * @return Reference to the event state
     */
    jcl_state &get_eventState();

    /**
     * @brief Set the event state counts
     * @param eCount Event state counts
     */
    void set_eventStateCount(jcl_state_event_count eCount);

    /**
     * @brief Set the event state
     * @param eState Event state
     */
    void set_eventState(jcl_state eState);

    /**
     * @brief Set the last notification time
     * @param last_notification_time Last notification time
     */
    void set_last_notification_time(struct timespec last_notification_time);

    /**
     * @brief Get the last notification time
     * @return Last notification time
     */
    struct timespec get_last_notification_time();

    /**
     * @brief Convert the client state to a string
     * @return String representation of the client state
     */
    std::string toString();

    /**
     * @brief Get the event subscription
     * @return Reference to the event subscription
     */
    JClkLibCommon::jcl_subscription &get_eventSub();

    DECLARE_ACCESSOR(sessionId); /**< Declare accessor for sessionId */
    DECLARE_ACCESSOR(ptp4l_id); /**< Declare accessor for ptp4l_id */
};
}
#endif /* PROXY_CLIENT_STATE */
