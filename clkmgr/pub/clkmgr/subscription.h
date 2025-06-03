/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Set clock event subsciption.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLKMGR_SUBSCRIPTION_H
#define CLKMGR_SUBSCRIPTION_H

#include "pub/clkmgr/utility.h"
#include "pub/clkmgr/types.h"
#include <array>

__CLKMGR_NAMESPACE_BEGIN

/**
 * Manage event masks and clock offset thresholds for clock synchronization
 * event subscriptions.
 */
class ClockSubscriptionBase
{
  public:
    ClockSubscriptionBase() = default;

    /**
     * Set the event mask
     * @param[in] newEventMask The new event mask to set
     * @return True if the event mask is set successfully, false otherwise
     * @note The event mask is a bitmask where each bit represents an event,
     * as defined by enum EventIndex
     */
    bool setEventMask(uint32_t newEventMask);

    /**
     * Get the value of the event mask
     * @return The value of the event mask
     */
    uint32_t getEventMask() const;

    /**
     * Set the threshold of clock offset
     * @param[in] threshold Threshold of clock offset
     * @note The threshold sets a symmetric range of clock offset
     */
    void setClockOffsetThreshold(uint32_t threshold);

    /**
     * Get the threshold of clock offset
     * @return Threshold of clock offset
     */
    uint32_t getClockOffsetThreshold() const;

  private:
    uint32_t clockOffsetThreshold = 0;
    uint32_t eventMask = 0;
};

/**
 * Manage event masks and clock offset thresholds for PTP clock synchronization
 * event subscriptions.
 */
class PTPClockSubscription : public ClockSubscriptionBase
{
  public:
    PTPClockSubscription() noexcept;

    /**
     * Set the composite event mask.
     * @param[in] composite_event_mask The new composite event mask to set.
     * @return True if the composite event mask is set successfully, false
     * otherwise.
     * @note The compositie event mask is a bitmask where each bit represents an
     * event, as defined by enum EventIndex and COMPOSITE_EVENT_ALL.
     */
    bool setCompositeEventMask(uint32_t composite_event_mask);

    /**
     * Get the value of the composite event mask.
     * @return The composite event mask.
     */
    uint32_t getCompositeEventMask() const;

  private:
    uint32_t m_composite_event_mask = 0;
};

/**
 * Manage event masks and clock offset thresholds for system clock
 * synchronization event subscriptions.
 */
class SysClockSubscription : public ClockSubscriptionBase
{
  public:
    SysClockSubscription() noexcept;
};

/**
 * Provide overview of clock event subscriptions to Clock Manager.
 */
class ClockSyncSubscription
{
  public:
    ClockSyncSubscription();

    /**
     * Enable the subscription of the PTP clock.
     */
    void enablePtpSubscription();

    /**
     * Disable the subscription of the PTP clock.
     */
    void disablePtpSubscription();

    /**
     * Check if the PTP clock subscription is enabled
     * @return True if the PTP clock subscription is enabled, false otherwise
     */
    bool isPTPSubscriptionEnable() const;

    /**
     * Set the PTP clock subscription with a new PTPClockSubscription object
     * @param[in] newPtpSub The new PTPClockSubscription object to update
     */
    void setPtpSubscription(const PTPClockSubscription &newPtpSub);

    /**
     * Retrieve the PTP clock manager subscription.
     * @return A constant reference to the PTP clock manager subscription.
     */
    const PTPClockSubscription &getPtpSubscription() const;

    /**
     * Enable the subscription of the system clock.
     */
    void enableSysSubscription();

    /**
     * Disable the subscription of the system clock.
     */
    void disableSysSubscription();

    /**
     * Check if the system clock subscription is enabled
     * @return True if the system clock subscription is enabled, false otherwise
     */
    bool isSysSubscriptionEnable() const;

    /**
     * Set the system clock subscription with a new SysClockSubscription object
     * @param[in] newSysSub The new SysClockSubscription object to update
     */
    void setSysSubscription(const SysClockSubscription &newSysSub);

    /**
     * Retrieve the system clock manager subscription.
     * @return A constant reference to the system clock manager subscription.
     */
    const SysClockSubscription &getSysSubscription() const;

  private:
    PTPClockSubscription ptpSubscription;
    SysClockSubscription sysSubscription;
    bool ptpSubscribed;
    bool sysSubscribed;
};

__CLKMGR_NAMESPACE_END

#endif /* CLKMGR_SUBSCRIPTION_H */
