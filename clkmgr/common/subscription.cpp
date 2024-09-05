/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file subscription.cpp
 * @brief PTP event subscription.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#include <clkmgr/subscription.h>

__CLKMGR_NAMESPACE_USE

bool ClkMgrSubscription::define_threshold(std::uint8_t index,
    std::int32_t upper, std::int32_t lower)
{
    if(index < static_cast<std::uint8_t>(thresholdLast)) {
        threshold[index].upper_limit = upper;
        threshold[index].lower_limit = lower;
        return true;
    }
    return false;
}

bool ClkMgrSubscription::in_range(std::uint8_t index, std::int32_t value) const
{
    if(index < static_cast<std::uint8_t>(thresholdLast)) {
        if(value > threshold[index].lower_limit &&
            value < threshold[index].upper_limit)
            return true;
    }
    return false;
}
