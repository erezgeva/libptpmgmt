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
    /** Event indicating whether clock offset is in-range */
    Nm(EventOffsetInRange) = 1 << 0,
    /** Event indicating whether PTP clock is synchronized with a grandmaster */
    Nm(EventSyncedWithGm) = 1 << 1,
    /** Event indicating whether clock is an IEEE 802.1AS capable */
    Nm(EventAsCapable) = 1 << 2,
    /** Event indicating whether grandmaster has changed */
    Nm(EventGmChanged) = 1 << 3,
};

/**
 * All the PTP clock events available for subscription.
 */
cnst(uint32_t,PTP_EVENT_ALL,Nm(EventOffsetInRange) | \
    Nm(EventSyncedWithGm) | Nm(EventAsCapable) | Nm(EventGmChanged))

/**
 * All the System clock events available for subscription.
 */
cnst(uint32_t,SYS_EVENT_ALL,Nm(EventOffsetInRange))

/**
 * All the events that can be used as conditions for satisfying the composite
 * event of PTP clock.
 */
cnst(uint32_t,PTP_COMPOSITE_EVENT_ALL,Nm(EventOffsetInRange) | \
    Nm(EventSyncedWithGm) | Nm(EventAsCapable))

/**
* Types of clock available for subscription.
*/
enum Nm(ClockType) sz(`: uint8_t '){
    Nm(PTPClock) = 1, /**< PTP Clock */
    Nm(SysClock) = 2, /**< System Clock */
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
