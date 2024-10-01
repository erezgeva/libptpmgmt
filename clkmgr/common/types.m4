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
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

ics(TYPE)

inc_c(`stdint')

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
 * Index of events which require user to provide predefined threshold.
 *
 * @note The thresholdLast is reserved for future use. The maximum number of
 * events which can have threshold is NM(THRESHOLD_MAX).
 */
enum Nm(ThresholdIndex) sz(`: uint8_t '){
    Nm(thresholdGMOffset),  /**< threshold for primary-secondary clock offset */
    Nm(thresholdLast)       /**< Last threshold */
};

/**
 * Structure to hold upper and lower limits
 */
struct Nm(Threshold) {
    int32_t upper_limit; /**< Upper limit */
    int32_t lower_limit; /**< Lower limit */
cpp_cod(`    Threshold() noexcept : upper_limit(0), lower_limit(0) {}')dnl
};

ns_e()

ice(TYPE)
