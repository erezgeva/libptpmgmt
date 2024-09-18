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

#include <clkmgr/utility.h>

__CLKMGR_NAMESPACE_BEGIN

/**
 * @enum EventIndex
 * @brief Bitmask of events available for subscription. Each bit represents one
 * event.
 * @note The eventLast is reserved for future use.
 */
typedef enum : std::uint32_t {
    eventGMOffset = 1 << 0, /**< Offset between primary and secondary clock */
    eventSyncedToGM = 1 << 1, /**< Secondary clock is synced to primary clock */
    eventASCapable = 1 << 2, /**< Link Partner is IEEE 802.1AS capable */
    eventGMChanged = 1 << 3, /**< UUID of primary clock is changed */
    eventLast = 1 << 4 /**< Last event */
} EventIndex;

/**
 * @brief Maximum number of events that can have user predefined threshold
 * (upper and lower limit) as indicator on whether the event is true or false.
 */
constexpr std::uint8_t THRESHOLD_MAX = 8;

/**
 * @enum ThresholdIndex
 * @brief Index of events which require user to provide predefined threshold.
 * @note The thresholdLast is reserved for future use. The maximum number of
 * events which can have threshold is THRESHOLD_MAX.
 */
typedef enum : std::uint8_t {
    thresholdGMOffset,  /**< threshold for primary-secondary clock offset */
    thresholdLast       /**< Last threshold */
} ThresholdIndex;

/**
 * @class ClkMgrSubscription
 * @brief Class to hold the event subscription mask, composite event mask, and
 * thresholds for events that require user-defined threshold (upper and lower
 * limits).
 */
class ClkMgrSubscription
{
  private:
    /**
     * @struct Threshold
     * @brief Structure to hold upper and lower limits
     */
    struct Threshold {
        std::int32_t upper_limit; /**< Upper limit */
        std::int32_t lower_limit; /**< Lower limit */
        Threshold() noexcept : upper_limit(0), lower_limit(0) {}
    };

    std::uint32_t event_mask; /**< Event subscription mask */
    std::uint32_t composite_event_mask; /**< Composite event mask */
    std::array<Threshold, THRESHOLD_MAX> threshold; /**< Upper & lower limits */

  public:
    ClkMgrSubscription() noexcept : event_mask(0), composite_event_mask(0) {}

    /**
    * @brief Get the constant reference to the event mask.
    * @return Constant reference to the event mask.
    */
    const decltype(event_mask) &getc_event_mask() { return event_mask; }

    /**
    * @brief Get the reference to the event mask.
    * @return Reference to the event mask.
    */
    decltype(event_mask) &get_event_mask() { return event_mask; }

    /**
    * @brief Set the event mask.
    * @param[in] event_mask The new event mask to set.
    */
    void set_event_mask(const decltype(event_mask) &event_mask) {
        this->event_mask = event_mask;
    }

    /**
    * @brief Get the value of the event mask.
    * @return The value of the event mask.
    */
    decltype(event_mask) c_get_val_event_mask() const { return event_mask; }

    /**
    * @brief Get the constant reference to the composite event mask.
    * @return Constant reference to the composite event mask.
    */
    const decltype(composite_event_mask) &getc_composite_event_mask() {
        return composite_event_mask;
    }

    /**
    * @brief Get the reference to the composite event mask.
    * @return Reference to the composite event mask.
    */
    decltype(composite_event_mask) &get_composite_event_mask() {
        return composite_event_mask;
    }

    /**
    * @brief Set the composite event mask.
    * @param[in] composite_event_mask The new composite event mask to set.
    */
    void set_composite_event_mask(const decltype(composite_event_mask)
        &composite_event_mask) {
        this->composite_event_mask = composite_event_mask;
    }

    /**
    * @brief Get the value of the composite event mask.
    * @return The value of the composite event mask.
    */
    decltype(composite_event_mask) c_get_val_composite_event_mask() const {
        return composite_event_mask;
    }

    /**
    * @brief Get the constant reference to the threshold.
    * @return Constant reference to the threshold.
    */
    const decltype(threshold) &getc_threshold() { return threshold; }

    /**
    * @brief Get the reference to the threshold.
    * @return Reference to the threshold.
    */
    decltype(threshold) &get_threshold() { return threshold; }

    /**
    * @brief Set the threshold.
    * @param[in] threshold The new threshold to set.
    */
    void set_threshold(const decltype(threshold) &threshold) {
        this->threshold = threshold;
    }

    /**
    * @brief Get the value of the threshold.
    * @return The value of the threshold.
    */
    decltype(threshold) c_get_val_threshold() const { return threshold; }

    /**
     * @brief Define the upper and lower limits of a specific event
     * @param[in] index Index of the event according to ThresholdIndex enum
     * @param[in] upper Upper limit
     * @param[in] lower Lower limit
     * @return true on success, false on failure
     */
    bool define_threshold(std::uint8_t index, std::int32_t upper,
        std::int32_t lower);

    /**
     * @brief Check whether a given value is within predefined threshold
     * @param[in] index Index of the event according to ThresholdIndex enum
     * @param[in] value Current value
     * @return Return true if value is within the threshold, and false otherwise
     */
    bool in_range(std::uint8_t index, std::int32_t value) const;
};

__CLKMGR_NAMESPACE_END

#endif /* CLKMGR_SUBSCRIPTION_H */
