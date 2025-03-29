/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief time base structure
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#ifndef CLKMGR_TIMEBASE_H
#define CLKMGR_TIMEBASE_H

#include <cstddef>
#include <cstdint>

__CLKMGR_NAMESPACE_BEGIN

/**
 * Maximum size for strings used in the time base configuration.
 */
const size_t STRING_SIZE_MAX = 64;

/**
 * Structure to represent the configuration of a time base.
 */
struct TimeBaseCfg {
    int timeBaseIndex; /**< Index of the time base */
    char timeBaseName[STRING_SIZE_MAX]; /**< Name of the time base */
    char interfaceName[STRING_SIZE_MAX]; /**< Network interface name */
    uint8_t transportSpecific; /**< Transport specific for ptp4l */
    uint8_t domainNumber; /**< Domain number for ptp4l */
};

__CLKMGR_NAMESPACE_END

#endif /* CLKMGR_TIMEBASE_H */
