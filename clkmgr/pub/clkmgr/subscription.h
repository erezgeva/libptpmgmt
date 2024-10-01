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
#include "pub/clkmgr/types.h"

__CLKMGR_NAMESPACE_BEGIN

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
