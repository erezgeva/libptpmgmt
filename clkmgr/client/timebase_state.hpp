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

#include "pub/clockmanager.h"
#include "pub/clkmgr/subscription.h"
#include "client/clock_event_handler.hpp"
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
    PTPClockEvent ptp4lEventState; /**< PTP4L Event state */
    SysClockEvent chronyEventState; /**< Chrony Event state */
    PTPClockSubscription ptpEventSub; /**< PTP Event subscription */
    SysClockSubscription sysEventSub; /**< Chrony Event subscription */
    timespec last_notification_time = {}; /**< Last notification time */
    bool havePtPData = false; /**< Flag to indicate if PTP data is available */
    bool haveSysData = false; /**< Flag to indicate if Chrony data is available */

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
    const PTPClockEvent &get_ptp4lEventState() const;

    /**
     * Get the chrony event state
     * @return Reference to the event state
     */
    const SysClockEvent &get_chronyEventState() const;

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
    const timespec &get_last_notification_time() const;

    /**
     * Get the PTP event subscription
     * @return Reference to the PTP event subscription
     */
    const PTPClockSubscription &get_ptpEventSub() const;

    /**
     * Get the system clock event subscription
     * @return Reference to the system clock event subscription
     */
    const SysClockSubscription &get_sysEventSub() const;

    /**
     * Set the PTP event subscription
     * @param[in] eSub PTP event subscription
     * @return True if the subscription is set successfully, false otherwise
     */
    bool set_ptpEventSub(const PTPClockSubscription &eSub);

    /**
     * Set the system clock event subscription
     * @param[in] eSub System clock event subscription
     * @return True if the subscription is set successfully, false otherwise
     */
    bool set_sysEventSub(const SysClockSubscription &eSub);

    /**
     * Check whether there is any PTP clock data available
     * @return true if available, false otherwise
     */
    bool is_havePtp() const;

    /**
     * Set havePtpData to indicate whether there is any PTP data available
     * @param[in] havePtp True if PTP data is available, false otherwise
     */
    void set_havePtp(bool havePtp);

    /**
     * Check whether there is any system clock data available
     * @return true if available, false otherwise
     */
    bool is_haveSys() const;

    /**
     * Set haveSysData to indicate whether there is any system clock data available
     * @param[in] haveSys True if system clock data is available, false otherwise
     */
    void set_haveSys(bool haveSys);
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

    // Method to set TimeBaseState for PTP clock by timeBaseIndex
    void setTimeBaseStatePtp(size_t timeBaseIndex, const ptp_event &event);

    // Method to set TimeBaseState for System clock by timeBaseIndex
    void setTimeBaseStateSys(size_t timeBaseIndex, const chrony_event &event);

    // Method to set PTPClockSubscription by timeBaseIndex
    bool setPtpEventSubscription(int timeBaseIndex,
        const PTPClockSubscription &sub) {
        std::lock_guard<rtpi::mutex> lock(mtx);
        return timeBaseStateMap[timeBaseIndex].set_ptpEventSub(sub);
    }

    // Method to set SysClockSubscription by timeBaseIndex
    bool setSysEventSubscription(int timeBaseIndex,
        const SysClockSubscription &sub) {
        std::lock_guard<rtpi::mutex> lock(mtx);
        return timeBaseStateMap[timeBaseIndex].set_sysEventSub(sub);
    }

    // Method to get the subscription status by timeBaseIndex
    bool getSubscribed(size_t timeBaseIndex) {
        std::lock_guard<rtpi::mutex> lock(mtx);
        return timeBaseStateMap.count(timeBaseIndex) > 0 ?
            timeBaseStateMap[timeBaseIndex].get_subscribed() : false;
    }

    // Method to set the subscription status by timeBaseIndex
    void setSubscribed(size_t timeBaseIndex, bool subscribed) {
        std::lock_guard<rtpi::mutex> lock(mtx);
        timeBaseStateMap[timeBaseIndex].set_subscribed(subscribed);
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
    bool subscribe(size_t timeBaseIndex, const ClockSyncSubscription &newSub);
    bool subscribeReply(size_t timeBaseIndex, const ptp_event &ptpData,
        const chrony_event &chronyData);
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_TIMEBASE_STATE_HPP */
