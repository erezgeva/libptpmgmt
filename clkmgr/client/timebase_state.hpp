/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Class to get and set the state of timebase
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#ifndef CLIENT_TIMEBASE_STATE_HPP
#define CLIENT_TIMEBASE_STATE_HPP

#include "pub/clkmgr/subscription.h"
#include "pub/clockmanager.h"
#include "common/clock_event_handler.hpp"
#include "common/ptp_event.hpp"

#include <map>
#include <string>
#include <rtpi/mutex.hpp>

__CLKMGR_NAMESPACE_BEGIN

/**
 * Class to keep the current state of specific timebase
 */
class TimeBaseState
{
  private:
    bool subscribed{false}; /**< Subscription status */
    bool event_changed{false}; /**< Subscription status */
    Event_state eventState = {}; /**< Event state */
    PTPClockEvent ptp4lEventState; /**< PTP4L Event state */
    SysClockEvent chronyEventState; /**< Chrony Event state */
    ClkMgrSubscription eventSub = {}; /**< Event subscription */
    timespec last_notification_time = {}; /**< Last notification time */

  public:
    /**
     * Get the subscription status
     * @return true if subscribed, false otherwise
     */
    bool get_subscribed() const;

    /**
     * Set the subscription status
     * @param[in] subscriptionState Subscription status
     */
    void set_subscribed(bool subscriptionState);

    /**
     * Check whether any event changes detected
     * @return true if detected, false otherwise
     */
    bool is_event_changed() const;

    /**
     * Set event_changed to indicate whether there is any event changes detected
     */
    void set_event_changed(bool state);

    /**
     * Get the ptp4l event state
     * @return Reference to the event state
     */
    //Event_state &get_eventState();
    PTPClockEvent &get_ptp4lEventState();

    /**
     * Get the chrony event state
     * @return Reference to the event state
     */
    SysClockEvent &get_chronyEventState();

    /**
     * Set the ptp4l event state
     * @param[in] eState Event state
     */
    void set_ptpEventState(const PTPClockEvent &ptpState);

    /**
     * Set the chrony event state
     * @param[in] eState Event state
     */
    void set_chronyEventState(const SysClockEvent &chronyState);

    /**
     * Set the last notification time
     * @param[in] last_notification_time Last notification time
     */
    void set_last_notification_time(const timespec &last_notification_time);

    /**
     * Get the last notification time
     * @return Last notification time
     */
    timespec get_last_notification_time() const;

    /**
     * Convert the client state to a string
     * @return String representation of the client state
     */
    std::string toString() const;

    /**
     * Get the event subscription
     * @return Reference to the event subscription
     */
    const ClkMgrSubscription &get_eventSub();

    /**
     * Set the event subscription
     * @param[in] eSub event subscription
     */
    void set_eventSub(const ClkMgrSubscription &eSub);
};

/**
 * Class to keep the current state of all the timebases
 */
class TimeBaseStates
{
  private:
    std::map<int, TimeBaseState> timeBaseStateMap;
    rtpi::mutex mtx;

    // Private constructor to prevent instantiation
    TimeBaseStates() = default;

  public:

    // Static method to get the singleton instance
    static TimeBaseStates &getInstance() {
        static TimeBaseStates instance;
        return instance;
    }

    // Method to get a copy of TimeBaseState by timeBaseIndex
    bool getTimeBaseState(size_t timeBaseIndex, TimeBaseState &state);

    // Method to set TimeBaseState by timeBaseIndex
    void setTimeBaseState(size_t timeBaseIndex, const ptp_event &event);

    // Method to set ClkMgrSubscription by timeBaseIndex
    void setEventSubscription(size_t timeBaseIndex, const ClkMgrSubscription &sub) {
        std::lock_guard<rtpi::mutex> lock(mtx);
        auto &state = timeBaseStateMap[timeBaseIndex];
        state.set_eventSub(sub);
    }

    // Method to get the subscription status by timeBaseIndex
    bool getSubscribed(size_t timeBaseIndex) {
        std::lock_guard<rtpi::mutex> lock(mtx);
        auto it = timeBaseStateMap.find(timeBaseIndex);
        if(it != timeBaseStateMap.end())
            return it->second.get_subscribed();
        // If timeBaseIndex is not found, return false
        return false;
    }

    // Method to set the subscription status by timeBaseIndex
    void setSubscribed(size_t timeBaseIndex, bool subscribed) {
        std::lock_guard<rtpi::mutex> lock(mtx);
        auto &state = timeBaseStateMap[timeBaseIndex];
        state.set_subscribed(subscribed);
    }

    // Method to get the last notification time by timeBaseIndex
    bool getLastNotificationTime(size_t timeBaseIndex,
        timespec &lastNotificationTime) {
        std::lock_guard<rtpi::mutex> lock(mtx);
        auto it = timeBaseStateMap.find(timeBaseIndex);
        if(it != timeBaseStateMap.end()) {
            lastNotificationTime = it->second.get_last_notification_time();
            return true;
        }
        // If timeBaseIndex is not found, return false
        return false;
    }

    // Send Client subscribe message
    bool subscribe(size_t timeBaseIndex, const ClkMgrSubscription &newSub);
    bool subscribeReply(size_t timeBaseIndex, const ptp_event &data);
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_TIMEBASE_STATE_HPP */
