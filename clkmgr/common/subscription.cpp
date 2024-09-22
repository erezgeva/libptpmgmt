/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief PTP event subscription.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "pub/clkmgr/subscription.h"

__CLKMGR_NAMESPACE_USE

bool ClkMgrSubscription::define_threshold(std::uint8_t index,
    std::int32_t upper, std::int32_t lower)
{
    if(index < static_cast<std::uint8_t>(thresholdLast)) {
        m_threshold[index].upper_limit = upper;
        m_threshold[index].lower_limit = lower;
        return true;
    }
    return false;
}

bool ClkMgrSubscription::in_range(std::uint8_t index, std::int32_t value) const
{
    if(index < static_cast<std::uint8_t>(thresholdLast)) {
        if(value > m_threshold[index].lower_limit &&
            value < m_threshold[index].upper_limit)
            return true;
    }
    return false;
}
