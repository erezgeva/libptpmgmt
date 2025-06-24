/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Get clock synchronization events data
 *
 * This file defines the model for managing clock synchronization events.
 * The primary class, ClockSyncData represents a complete event model
 * that encapsulates events for different types of clocks. Each clock event
 * is derived from the ClockEventBase class, providing a unified interface
 * for handling clock-specific attributes and operations.
 *
 * Two types of clock events is supported:
 * - PTPClockEvent represents events specific to Precision Time Protocol
 *   (PTP) clocks.
 * - SysClockEvent represents events specific to system clocks, such as
 *   those managed by Chrony.
 *
 * @author Lai Peter Jun Ann <peter.jun.ann.lai@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#ifndef EVENT_H
#define EVENT_H

#include "pub/clkmgr/utility.h"
#include "pub/clkmgr/types.h"

#include <string>

__CLKMGR_NAMESPACE_BEGIN

class ClockEventHandler;

/**
 * Provide a set of common attributes and methods for clock synchronization
 * events, as listed below:
 * @li clock offset
 * @li clock offset in-range event and its event count
 * @li synchronization interval
 * @li grandmaster clock identity
 * @li grandmaster changed event and its event count
 * @li notification timestamp
 */
class ClockEventBase
{
  public:
    /**
     * Get the clock offset in nanosecond
     * @return Clock offset in nanosecond
     */
    int64_t getClockOffset() const;

    /**
     * Check if the clock offset is in-range
     * @return True if the clock offset is in-range, false otherwise
     * @note The range of clock offset is defined by user during subscription
     */
    bool isOffsetInRange() const;

    /**
     * Get the count of clock offset in-range event
     * @return The count of clock offset in-range event
     * @note This count indicates the number of times the clock offset has
     *  transitioned from in-range to out-of-range or vice versa since the last
     *  call to statusWait() or statusWaitByName()
     */
    uint32_t getOffsetInRangeEventCount() const;

    /**
     * Get the synchronization interval in microsecond
     * @return Synchronization interval in microsecond
     * @note This interval indicates the time between clock update messages send
     *  to the local service that synchronizing clock. This interval specify the
     *  time the synchronization protocol may need to react for a change in the
     *  grandmaster.
     */
    uint64_t getSyncInterval() const;

    /**
     * Get the grandmaster clock identity
     * @return Grandmaster clock identity
     * @note The grandmaster is the source clock for the PTP domain (network),
     *  whom all the clocks will synchronize to.
     */
    uint64_t getGmIdentity() const;

    /**
     * Get the grandmaster clock identity in string format
     * @return Grandmaster clock identity
     * @note The grandmaster is the source clock for the PTP domain (network),
     *  whom all the clocks will synchronize to.
     */
    std::string getGmIdentityStr() const;

    /**
     * Check if the grandmaster has changed
     * @return True if the grandmaster has changed, false otherwise
     * @note This boolean variable indicates if the grandmaster has changed
     *  since the last call of statusWait() or statusWaitByName()
     */
    bool isGmChanged() const;

    /**
     * Get the count of grandmaster changed event
     * @return The count of grandmaster changed event
     * @note This count indicates how many times the grandmaster has changed
     *  since the last call to statusWait() or statusWaitByName()
     */
    uint32_t getGmChangedEventCount() const;

    /**
     * Get the notification timestamp in nanosecond
     * @return Notification timestamp in nanosecond
     * @note This timestamp indicates to the most recent time in CLOCK_REALTIME
     *  when the Client received a clock sync data notification from the Proxy
     */
    uint64_t getNotificationTimestamp() const;

  protected:
    /**< @cond internal
     * set by the clockeventhandler class
     */
    ClockEventBase() = default; /**< Constructs a ClockEventBase object. */
    friend class ClockEventHandler; /**< Grants access to ClockEventHandler. */
    int64_t clockOffset = 0; /**< Offset of the clock in nanoseconds. */
    uint64_t syncInterval = 0; /**< Synchronization interval in nanoseconds. */
    uint64_t gmClockUUID = 0; /**< UUID of the grandmaster clock. */
    /** Timestamp of last notification event. */
    uint64_t notificationTimestamp = 0;
    uint32_t offsetInRangeCount = 0; /**< Count of offsets within defined range. */
    uint32_t gmChangedCount = 0; /**< Count of grandmaster clock changes. */
    bool offsetInRange = false; /**< Indicates if the offset is within range. */
    bool gmChanged = false; /**< Indicates if the grandmaster clock has changed. */
    /**< @endcond */
};

/**
 * Extends the ClockEventBase class to include attributes specific to
 * Precision Time Protocol (PTP) synchronization, such as:
 * @li clock synced with grandmaster event
 * @li IEEE 802.1AS capable event
 * @li composite event
 */
class PTPClockEvent : public ClockEventBase
{
  public:
    /**
     * Check if the PTP clock is synchronized with a grandmaster
     * @return True if the clock is synchronized with a grandmaster, false if
     *  otherwise
     */
    bool isSyncedWithGm() const;

    /**
     * Get the count of clock synced with grandmaster event
     * @return The count of clock synced with grandmaster event
     * @note This count indicates the number of times the clock has transitioned
     *  from synced with grandmaster to out-of-sync or vice versa since the last
     *  call to statusWait() or statusWaitByName()
     */
    uint32_t getSyncedWithGmEventCount() const;

    /**
     * Check if the clock is an IEEE 802.1AS capable
     * @return True if the clock is an IEEE 802.1AS capable, false otherwise
     */
    bool isAsCapable() const;

    /**
     * Get the count of IEEE 802.1AS capable event
     * @return The count of IEEE 802.1AS capable event
     * @note This count indicates the number of times the clock has transitioned
     *  from non capable to capable or vice versa since the last call to
     *  statusWait() or statusWaitByName()
     */
    uint32_t getAsCapableEventCount() const;

    /**
     * Check if all the conditions in the subscribed composite event are met
     * @return True if all the conditions are met, false otherwise
     * @note Condition of composite event is defined by the user during
     *  subsciption
     */
    bool isCompositeEventMet() const;

    /**
     * Get the count of composite event
     * @return The count of composite event
     * @note This count indicates the number of times the condition of composite
     *  event has transitioned from not met to met or vice versa since the last
     *  call to statusWait() or statusWaitByName()
     */
    uint32_t getCompositeEventCount() const;

  protected:
    PTPClockEvent() = default;

  private:
    friend class ClockEventHandler;
    friend class ClockSyncData;
    friend class TimeBaseState;
    uint32_t syncedWithGmCount = 0;
    uint32_t asCapableCount = 0;
    uint32_t compositeEventCount = 0;
    bool syncedWithGm = false;
    bool asCapable = false;
    bool compositeEvent = false;
};

/**
 * Extend the ClockEventBase class to represent synchronization attributes
 * specific to Chrony, a versatile implementation of the Network Time Protocol
 * (NTP).
 */
class SysClockEvent : public ClockEventBase
{
  protected:
    SysClockEvent() = default;

  private:
    friend class ClockEventHandler;
    friend class ClockSyncData;
    friend class TimeBaseState;
};

/**
 * Provides functionality to manage and update the state of PTP and system clocks.
 * It includes methods to check availability, retrieve clock objects, and
 * update clock states.
 */
class ClockSyncData
{
  public:
    /**
     * Construct a ClockSyncData object, setting the PTP and system clock
     * availability flags to false
     */
    ClockSyncData() = default;

    /**
     * Check if the PTP clock is available
     * @return True if the PTP clock is available, false otherwise
     */
    bool havePTP() const;

    /**
     * Retrieve the PTP clock object
     * @return Reference to the PTPClockEvent object
     */
    PTPClockEvent &getPtp();

    /**
     * Check if the system clock is available
     * @return True if the system clock is available, false otherwise
     */
    bool haveSys() const;

    /**
     * Retrieve the system clock object
     * @return Reference to the SysClockEvent object
     */
    SysClockEvent &getSysClock();

  private:
    friend class ClockSyncBaseHandler;
    PTPClockEvent ptpClockSync;
    SysClockEvent sysClockSync;
    bool ptpAvailable = false;
    bool sysAvailable = false;
};

__CLKMGR_NAMESPACE_END

#endif /* EVENT_H */
