dnl SPDX-License-Identifier: BSD-3-Clause
dnl SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */
dnl
dnl @file
dnl @brief Generate structures and enums types header for library and C wrapper
dnl
dnl @author Erez Geva <ErezGeva2@@gmail.com>
dnl @copyright © 2024 Intel Corporation.
dnl
include(lang().m4)dnl
/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief structures and enums types
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

ics(TYPE)

incpp(<cstddef>)dnl
inc_c(`stdint')
c_cod(`#include <stdbool.h>')
ns_s()

/**
 * Bitmask of events available for subscription. Each bit represents one event.
 * @note The eventLast is reserved for future use.
 */
enum Nm(EventIndex) sz(`: uint32_t '){
    /** Offset between primary and secondary clock */
    Nm(eventGMOffset) = 1 << 0,
    /** Secondary clock is synced to primary clock */
    Nm(eventSyncedToGM) = 1 << 1,
    Nm(eventASCapable) = 1 << 2, /**< Link Partner is IEEE 802.1AS capable */
    Nm(eventGMChanged) = 1 << 3, /**< UUID of primary clock is changed */
    Nm(eventLast) = 1 << 4 /**< Last event */
};

/**
 * Maximum number of events that can have user predefined threshold
 * (upper and lower limit) as indicator on whether the event is true or false.
 */
cnst(uint8_t,THRESHOLD_MAX,8)

/**
 * Structure to represent the current state of events.
 */
struct Nm(Event_state) {
    /** Timestamp for last notification in nanoseconds */
    uint64_t notification_timestamp;
    int64_t clock_offset; /**< Clock offset in nanoseconds */
    uint8_t gm_identity[8]; /**< Primary clock UUID */
    int64_t ptp4l_sync_interval; /**< Clock Sync Interval in microseconds */
    bool offset_in_range; /**< Clock offset in range */
    bool synced_to_primary_clock; /**< Synced to primary clock */
    bool as_capable; /**< IEEE 802.1AS capable */
    bool gm_changed; /**< Primary clock UUID changed */
    bool composite_event; /**< Composite event */
    int64_t chrony_clock_offset; /**< Chrony clock offset in nanoseconds */
    bool chrony_offset_in_range; /**< Chrony_clock offset in range */
    uint32_t chrony_reference_id; /**< Chrony reference ID */
    uint32_t polling_interval; /**< Chrony polling interval in microseconds */
};

/**
 * Structure to represent the event counts.
 */
struct Nm(Event_count) {
    uint32_t offset_in_range_event_count; /**< Clock offset in range */
    uint32_t synced_to_gm_event_count; /**< Synced to primary clock */
    uint32_t as_capable_event_count; /**< IEEE 802.1AS capable */
    uint32_t gm_changed_event_count; /**< Primary clock UUID changed */
    uint32_t composite_event_count; /**< Composite event */
    uint32_t chrony_offset_in_range_event_count; /**< Chrony clock offset */
};

ns_e()

ice(TYPE)
