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
 * @brief Enumerator types
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
 */
enum Nm(EventIndex) sz(`: uint32_t '){
    /** Offset between primary and secondary clock */
    Nm(EventGMOffset) = 1 << 0,
    /** Secondary clock is synced to primary clock */
    Nm(EventSyncedToGM) = 1 << 1,
    Nm(EventASCapable) = 1 << 2, /**< Link Partner is IEEE 802.1AS capable */
    Nm(EventGMChanged) = 1 << 3, /**< UUID of primary clock is changed */
};

/**
 * All the PTP clock events available for subscription.
 */
cnst(uint32_t,PTP_EVENT_ALL,Nm(EventGMOffset) | \
    Nm(EventSyncedToGM) | Nm(EventASCapable) | Nm(EventGMChanged))

/**
 * All the System clock events available for subscription.
 */
cnst(uint32_t,SYS_EVENT_ALL,Nm(EventGMOffset))

/**
 * All the events that can be used as conditions for satisfying the composite
 * event of PTP clock.
 */
cnst(uint32_t,PTP_COMPOSITE_EVENT_ALL,Nm(EventGMOffset) | \
    Nm(EventSyncedToGM) | Nm(EventASCapable))

/**
* Types of clock available for subscription.
* @note The Nm(ClockLast) is reserved for future use.
*/
enum Nm(ClockType) sz(`: uint32_t '){
    Nm(PTPClock) = 1, /**< PTP Clock */
    Nm(SysClock) = 2, /**< System Clock */
    Nm(ClockLast) = 3 /**< Last Clock */
};

/**
* Return value of statusWait API.
*/
enum Nm(StatusWaitResult) sz(`: int8_t '){
    Nm(SWRLostConnection) = -2, /**< Lost connection to Proxy */
    Nm(SWRInvalidArgument) = -1, /**< Invalid argument */
    Nm(SWRNoEventDetected) = 0, /**< No event changes detected */
    Nm(SWREventDetected) = 1 /**< At least an event change detected */
};

ns_e()

ice(TYPE)
