/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file
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
 * @struct clkmgr_event_state
 * @brief Structure to represent the current state of events.
 */
struct clkmgr_event_state {
    uint64_t notification_timestamp; /**< Timestamp for last notification */
    int64_t clock_offset; /**< Clock offset */
    uint8_t gm_identity[8]; /**< Primary clock UUID */
    bool offset_in_range; /**< Clock offset in range */
    bool synced_to_primary_clock; /**< Synced to primary clock */
    bool as_capable; /**< IEEE 802.1AS capable */
    bool gm_changed; /**< Primary clock UUID changed */
    bool composite_event; /**< Composite event */
    bool reserved[27]; /**< Reserved for future */
};

/**
 * @struct clkmgr_event_count
 * @brief Structure to represent the event counts.
 */
struct clkmgr_event_count {
    uint32_t offset_in_range_event_count; /**< Clock offset in range */
    uint32_t synced_to_gm_event_count; /**< Synced to primary clock */
    uint32_t as_capable_event_count; /**< IEEE 802.1AS capable */
    uint32_t gm_changed_event_count; /**< Primary clock UUID changed */
    uint32_t composite_event_count; /**< Composite event */
    uint32_t reserved[27]; /**< Reserved for future */
};

__CLKMGR_NAMESPACE_END

#endif /* CLKMGR_EVENT_STATE_H */
