/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief class, structures and enums used for events subsciption
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLKMGR_SUBSCRIPTION_H
#define CLKMGR_SUBSCRIPTION_H

#include <array>
#include <cstdint>
#include <string>

#include "pub/clkmgr/utility.h"

__CLKMGR_NAMESPACE_BEGIN

/**
 * @enum EventIndex
 * @brief Bitmask of events available for subscription. Each bit represents one
 * event.
 * @note The eventLast is reserved for future use.
 */
enum EventIndex : uint32_t {
    eventGMOffset = 1 << 0, /**< Offset between primary and secondary clock */
    eventSyncedToGM = 1 << 1, /**< Secondary clock is synced to primary clock */
    eventASCapable = 1 << 2, /**< Link Partner is IEEE 802.1AS capable */
    eventGMChanged = 1 << 3, /**< UUID of primary clock is changed */
    eventLast = 1 << 4 /**< Last event */
};

/**
 * @brief Maximum number of events that can have user predefined threshold
 * (upper and lower limit) as indicator on whether the event is true or false.
 */
constexpr uint8_t THRESHOLD_MAX = 8;

/**
 * @enum ThresholdIndex
 * @brief Index of events which require user to provide predefined threshold.
 * @note The thresholdLast is reserved for future use. The maximum number of
 * events which can have threshold is THRESHOLD_MAX.
 */
enum ThresholdIndex : uint8_t {
    thresholdGMOffset,  /**< threshold for primary-secondary clock offset */
    thresholdLast       /**< Last threshold */
} ;

/**
 * @struct Threshold
 * @brief Structure to hold upper and lower limits
 */
struct Threshold {
    int32_t upper_limit; /**< Upper limit */
    int32_t lower_limit; /**< Lower limit */
    Threshold() noexcept : upper_limit(0), lower_limit(0) {}
};

/** Upper & lower limits type */
typedef std::array<Threshold, THRESHOLD_MAX> threshold_t;

/**
 * @class ClkMgrSubscription
 * @brief Class to hold the event subscription mask, composite event mask, and
 * thresholds for events that require user-defined threshold (upper and lower
 * limits).
 */

class ClkMgrSubscription
{
  private:
    uint32_t m_event_mask; /**< Event subscription mask */
    uint32_t m_composite_event_mask; /**< Composite event mask */
    threshold_t m_threshold; /**< Upper & lower limits */

  public:
    ClkMgrSubscription() noexcept : m_event_mask(0), m_composite_event_mask(0) {}

    /**
    * @brief Set the event mask.
    * @param[in] event_mask The new event mask to set.
    */
    void set_event_mask(uint32_t event_mask) {
        m_event_mask = event_mask;
    }

    /**
    * @brief Get the value of the event mask.
    * @return The value of the event mask.
    */
    uint32_t get_event_mask() const { return m_event_mask; }

    /**
    * @brief Set the composite event mask.
    * @param[in] composite_event_mask The new composite event mask to set.
    */
    void set_composite_event_mask(const uint32_t composite_event_mask) {
        m_composite_event_mask = composite_event_mask;
    }

    /**
    * @brief Get the value of the composite event mask.
    * @return the composite event mask.
    */
    uint32_t get_composite_event_mask() const {
        return m_composite_event_mask;
    }

    /**
    * @brief Get the constant reference to the threshold.
    * @return Constant reference to the threshold.
    */
    const threshold_t &get_threshold() const { return m_threshold; }

    /**
    * @brief Set the threshold.
    * @param[in] threshold The new threshold to set.
    */
    void set_threshold(const threshold_t &threshold) {
        m_threshold = threshold;
    }

    /**
     * @brief Define the upper and lower limits of a specific event
     * @param[in] index Index of the event according to ThresholdIndex enum
     * @param[in] upper Upper limit
     * @param[in] lower Lower limit
     * @return true on success, false on failure
     */
    bool define_threshold(uint8_t index, int32_t upper, int32_t lower);

    /**
     * @brief Check whether a given value is within predefined threshold
     * @param[in] index Index of the event according to ThresholdIndex enum
     * @param[in] value Current value
     * @return Return true if value is within the threshold, and false otherwise
     */
    bool in_range(uint8_t index, int32_t value) const;
};

__CLKMGR_NAMESPACE_END

#endif /* CLKMGR_SUBSCRIPTION_H */
