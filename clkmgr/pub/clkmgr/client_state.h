/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file client_state.h
 * @brief Class to get and set the state of client-runtime
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLKMGR_CLIENT_STATE_H
#define CLKMGR_CLIENT_STATE_H

#include <atomic>
#include <string>

#include <clkmgr/event_state.h>
#include <clkmgr/subscription.h>
#include <clkmgr/utility.h>

__CLKMGR_NAMESPACE_BEGIN

/**
 * @class ClientState
 * @brief Class to keep the current state of client-runtime
 */
class ClientState
{
  private:
    std::atomic_bool connected = false; /**< Connection status */
    std::atomic_bool subscribed = false; /**< Subscription status */
    sessionId_t sessionId = InvalidSessionId; /**< Session ID */
    TransportClientId clientID = {}; /**< Client ID */
    std::uint8_t ptp4l_id = 0; /**< PTP4L ID */
    clkmgr_event_state eventState = {}; /**< Event state */
    clkmgr_event_count eventStateCount = {}; /**< Event count */
    ClkMgrSubscription eventSub = {}; /**< Event subscription */
    struct timespec last_notification_time; /**< Last notification time */

  public:
    /**
     * @brief Default constructor
     */
    ClientState();

    /**
     * @brief Copy constructor
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
    TransportClientId get_clientID();

    /**
     * @brief Set the client ID
     * @param cID Reference to the client ID
     */
    void set_clientID(TransportClientId &cID);

    /**
     * @brief Get the event state counts
     * @return Reference to the event state counts
     */
    clkmgr_event_count &get_eventStateCount();

    /**
     * @brief Get the event state
     * @return Reference to the event state
     */
    clkmgr_event_state &get_eventState();

    /**
     * @brief Set the event state counts
     * @param eCount Event state counts
     */
    void set_eventStateCount(clkmgr_event_count eCount);

    /**
     * @brief Set the event state
     * @param eState Event state
     */
    void set_eventState(clkmgr_event_state eState);

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
    ClkMgrSubscription &get_eventSub();

    DECLARE_ACCESSOR(sessionId); /**< Declare accessor for sessionId */
    DECLARE_ACCESSOR(ptp4l_id); /**< Declare accessor for ptp4l_id */
};

__CLKMGR_NAMESPACE_END

#endif /* CLKMGR_CLIENT_STATE_H */
