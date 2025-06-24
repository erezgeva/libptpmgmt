/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief C wrapper to get the configuration of time base
 *
 * @author Song Yoong Siang <yoong.siang.song@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#ifndef CLIENT_TIMEBASE_CONFIGS_C_H
#define CLIENT_TIMEBASE_CONFIGS_C_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Check if a given timeBaseIndex exist
 * @param[in] timeBaseIndex The time base index to check.
 * @return true if found, false otherwise.
 */
bool clkmgr_isTimeBaseIndexPresent(size_t timeBaseIndex);
/**
 * Get Network interface name
 * @param[in] timeBaseIndex the time base index
 * @return Network interface name of the PTP hardware clock
 */
const char *clkmgr_ifName(size_t timeBaseIndex);
/**
 * Get Network interface index
 * @return Network interface index
 */
int clkmgr_ifIndex(size_t timeBaseIndex);
/**
 * Get PTP device index
 * @return PTP device index
 */
int clkmgr_ptpIndex(size_t timeBaseIndex);
/**
 * Get PTP Transport specific
 * @return PTP Transport specific
 */
uint8_t clkmgr_transportSpecific(size_t timeBaseIndex);
/**
 * Get PTP Domain number
 * @return PTP Domain number
 */
uint8_t clkmgr_domainNumber(size_t timeBaseIndex);
/**
 * Get name of the time base
 * @return name of the time base
 */
const char *clkmgr_timeBaseName(size_t timeBaseIndex);
/**
 * Query if we use PTP
 * @return true if this time base have PTP
 */
bool clkmgr_havePtp(size_t timeBaseIndex);
/**
 * Query if synchronize the system clock
 * @return true if this time base does synchronize
 */
bool clkmgr_haveSysClock(size_t timeBaseIndex);
/**
 * find index of the time base using its name.
 * @param[in] timeBaseName The time base name.
 * @param[out] timeBaseIndex pointer to the time base index
 * @return true if found
 */
bool clkmgr_BaseNameToBaseIndex(const char *timeBaseName,
    size_t *timeBaseIndex);

#ifdef __cplusplus
}
#endif

#endif /* CLIENT_TIMEBASE_CONFIGS_C_H */
