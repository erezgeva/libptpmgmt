/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief class for clock events handler.
 *
 * @author Lai Peter Jun Ann <peter.jun.ann.lai@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#ifndef CLOCK_EVENT_HANDLER_HPP
#define CLOCK_EVENT_HANDLER_HPP

#include "pub/clkmgr/event.h"

__CLKMGR_NAMESPACE_BEGIN

class TimeBaseState;

/**
 * Modify the protected attributes of ClockEventBase.
 * Contain static methods to set various attributes of ClockEventBase.
 */
class ClockEventHandler
{
  public:
    /**
     * Specify whether the clock event handler is for a PTP clock or a system
     * clock. It helps differentiate the operations and attributes specific to
     * each clock type.
     */
    enum ClockType {
        PTPClock,   /**< Represents a PTP clock event. */
        SysClock    /**< Represents a system clock event. */
    };

    /**
     * Initialize the handler to operate on either PTP or system clock events
     * based on the provided clock type. This constructor sets the internal
     * state to ensure that the handler performs the correct operations for
     * the specified clock type.
     *
     * @param[in] type The type of clock event to handle (PTPClock or SysClock).
     */
    ClockEventHandler(ClockType type) : clockType(type) {}

    /**
     * Set the notification timestamp of a ClockEventBase object.
     * @param[in] event The ClockEventBase object to modify.
     * @param[in] timestamp The new notification timestamp.
     */
    static void setNotificationTimestamp(ClockEventBase &event,
        uint64_t timestamp) {
        event.notificationTimestamp = timestamp;
    }

    /**
     * Set the clock offset of a ClockEventBase object.
     * @param[in] event The ClockEventBase object to modify.
     * @param[in] offset The new clock offset.
     */
    static void setClockOffset(ClockEventBase &event, int64_t offset) {
        event.clockOffset = offset;
    }

    /**
     * Set the grandmaster clock identity of a ClockEventBase object.
     * @param[in] event The ClockEventBase object to modify.
     * @param[in] identity The new grandmaster clock identity.
     */
    static void setGmIdentity(ClockEventBase &event, uint64_t identity) {
        event.gmClockUUID = identity;
    }

    /**
     * Set whether the offset is in range for a ClockEventBase object.
     * @param[in] event The ClockEventBase object to modify.
     * @param[in] in_range True if the offset is in range, false otherwise.
     */
    static void setOffsetInRange(ClockEventBase &event, bool in_range) {
        event.offsetInRange = in_range;
    }

    /**
     * Set whether the grandmaster has changed for a ClockEventBase object.
     * @param[in] event The ClockEventBase object to modify.
     * @param[in] changed True if the grandmaster has changed, false otherwise.
     */
    static void setGmChanged(ClockEventBase &event, bool changed) {
        event.gmChanged = changed;
    }

    /**
     * Set the synchronization interval of a ClockEventBase object.
     * @param[in] event The ClockEventBase object to modify.
     * @param[in] interval The new synchronization interval.
     */
    static void setSyncInterval(ClockEventBase &event, int64_t interval) {
        event.syncInterval = interval;
    }

    /**
     * Set whether all the conditions in the subscribed composite event are met
     * for a ClockEventBase object.
     * @param[in] event The ClockEventBase object to modify.
     * @param[in] composite True if all the conditions are met, false otherwise
     */
    static void setCompositeEvent(PTPClockEvent &event, bool composite) {
        event.compositeEvent = composite;
    }

    /**
     * Set the count of offset in range events for a ClockEventBase object.
     * @param[in] event The ClockEventBase object to modify.
     * @param[in] count The new count of offset in range events.
     */
    static void setOffsetInRangeEventCount(ClockEventBase &event, uint32_t count) {
        event.offsetInRangeCount = count;
    }

    /**
     * Set the count of source clock UUID changed events for a
     * ClockEventBase object.
     * @param[in] event The ClockEventBase object to modify.
     * @param[in] count The new count of source clock UUID changed events.
     */
    static void setGmChangedEventCount(ClockEventBase &event, uint32_t count) {
        event.gmChangedCount = count;
    }

    /**
     * Set the count of composite events for a ClockEventBase object.
     * @param[in] event The ClockEventBase object to modify.
     * @param[in] count The new count of composite events.
     */
    static void setCompositeEventCount(PTPClockEvent &event, uint32_t count) {
        event.compositeEventCount = count;
    }

    /**
     * Set whether the clock is synced with the GM clock.
     * @param[in] event The ClockEventBase object to modify.
     * @param[in] synced True if the clock is synced with the GM clock, false
     * otherwise.
     */
    void setSyncedWithGm(PTPClockEvent &event, bool synced) {
        if(clockType == PTPClock)
            event.syncedWithGm = synced;
    }

    /**
     * Set whether the clock is IEEE 802.1AS capable.
     * @param[in] event The ClockEventBase object to modify.
     * @param[in] capable True if the clock is IEEE 802.1AS capable,
     * false otherwise.
     */
    void setAsCapable(PTPClockEvent &event, bool capable) {
        if(clockType == PTPClock)
            event.asCapable = capable;
    }

    /**
     * Set the count of events where the clock is synced to the GM
     * clock.
     * @param[in] event The ClockEventBase object to modify.
     * @param[in] count The new count of synced to GM clock events.
     */
    static void setSyncedWithGmEventCount(PTPClockEvent &event, uint32_t count) {
        event.syncedWithGmCount = count;
    }

    /**
     * Set the count of events where the clock is IEEE 802.1AS capable.
     * @param[in] event The ClockEventBase object to modify.
     * @param[in] count The new count of IEEE 802.1AS capable events.
     */
    static void setAsCapableEventCount(PTPClockEvent &event, uint32_t count) {
        event.asCapableCount = count;
    }

  private:
    ClockType clockType;
};

/**
 * Modify the protected attributes of ClockSyncData.
 * Contain static methods to set various attributes of ClockSyncData.
 */
class ClockSyncBaseHandler
{
  private:
    ClockSyncData &clockSyncData;

  public:
    ClockSyncBaseHandler(ClockSyncData &clData) : clockSyncData(clData) {}
    void updateAll(const TimeBaseState &state);
};

__CLKMGR_NAMESPACE_END

#endif /* CLOCK_EVENT_HANDLER_HPP */
