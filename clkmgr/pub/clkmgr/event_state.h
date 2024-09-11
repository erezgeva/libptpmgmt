/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file event_state.h
 * @brief Structure for event state and event count
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLKMGR_EVENT_STATE_H
#define CLKMGR_EVENT_STATE_H

#include <cstdint>

#include <clkmgr/utility.h>

__CLKMGR_NAMESPACE_BEGIN

/**
 * Current State for the events
 */
struct clkmgr_state {
    uint8_t  gm_identity[8]; /**< Primary clock UUID */
    bool     as_capable; /**< IEEE 802.1AS capable */
    bool     offset_in_range; /**< Clock offset in range */
    bool     synced_to_primary_clock; /**< Synced to primary clock */
    bool     gm_changed; /**< Primary clock UUID changed */
    bool     composite_event; /**< Composite event */
    int64_t  clock_offset; /**< Clock offset */
    uint64_t notification_timestamp; /**< Timestamp for last notification */
};

/**
 * Event count for the events
 */
struct clkmgr_state_event_count {
    uint64_t offset_in_range_event_count; /**< Clk offset in range */
    uint64_t gm_changed_event_count; /**< Primary clk ID changed */
    uint64_t as_capable_event_count; /**< IEEE 802.1AS capable */
    uint64_t synced_to_primary_clock_event_count; /**< Synced to primary clk */
    uint64_t composite_event_count; /**< Composite event */
};

__CLKMGR_NAMESPACE_END

#endif /* CLKMGR_EVENT_STATE_H */
