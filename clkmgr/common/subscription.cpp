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

__CLKMGR_NAMESPACE_USE;

ClkMgrSubscription::ClkMgrSubscription() noexcept : m_event_mask(0),
    m_composite_event_mask(0) {}

void ClkMgrSubscription::set_ClkMgrSubscription(const ClkMgrSubscription &n)
{
    m_event_mask = n.m_event_mask;
    m_composite_event_mask = n.m_composite_event_mask;
    m_threshold = n.m_threshold;
}

void ClkMgrSubscription::set_event_mask(uint32_t event_mask)
{
    m_event_mask = event_mask;
}

uint32_t ClkMgrSubscription::get_event_mask() const
{
    return m_event_mask;
}

void ClkMgrSubscription::set_composite_event_mask(uint32_t composite_event_mask)
{
    m_composite_event_mask = composite_event_mask;
}

uint32_t ClkMgrSubscription::get_composite_event_mask() const
{
    return m_composite_event_mask;
}

bool ClkMgrSubscription::define_threshold(ThresholdIndex index, int32_t upper,
    int32_t lower)
{
    if(index >= thresholdLast || upper <= lower)
        return false;
    m_threshold[index].upper_limit = upper;
    m_threshold[index].lower_limit = lower;
    return true;
}

bool ClkMgrSubscription::get_threshold(ThresholdIndex index, int32_t &upper,
    int32_t &lower)
{
    if(index >= thresholdLast)
        return false;
    upper = m_threshold[index].upper_limit;
    lower = m_threshold[index].lower_limit;
    return true;
}

bool ClkMgrSubscription::in_range(ThresholdIndex index, int32_t value) const
{
    return index < thresholdLast &&
        value > m_threshold[index].lower_limit &&
        value < m_threshold[index].upper_limit;
}
