/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Opaque struct for C APIs
 *
 * @author Goh Wei Sheng <wei.sheng.goh@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#ifndef OPAQUE_STRUCT_C_HPP
#define OPAQUE_STRUCT_C_HPP

#include "pub/clkmgr/subscription.h"
#include "pub/clkmgr/event.h"

/**
 * Opaque struct for clock event data
 */
struct Clkmgr_ClockSyncData { clkmgr::ClockSyncData *data; };

/**
 * Opaque struct for clock subscription
 */
struct Clkmgr_Subscription {
    clkmgr::PTPClockSubscription *ptp;
    clkmgr::SysClockSubscription *sys;
    clkmgr::ClockSyncSubscription *sub;
};

#endif /* OPAQUE_STRUCT_C_HPP */
