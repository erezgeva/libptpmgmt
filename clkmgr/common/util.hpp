/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Helper functions, templates, and macros
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef UTIL_HPP
#define UTIL_HPP

#include "pub/clkmgr/utility.h"

#include <cstdint>
#include <array>

__CLKMGR_NAMESPACE_BEGIN

/** Maximum number of character for transport client ID */
const int TRANSPORT_CLIENTID_LENGTH = 512;

/** Array to store transport client ID. */
typedef std::array<uint8_t, TRANSPORT_CLIENTID_LENGTH> TransportClientId;

/** Type definition for session ID. */
typedef uint16_t sessionId_t;

/** Invalid session ID (default session ID) */
const sessionId_t InvalidSessionId = UINT16_MAX;

/** Number of nanoseconds in a millisecond */
const int32_t NSEC_PER_MSEC = 1000000;
/** Number of nanoseconds in a second */
const int32_t NSEC_PER_SEC  = 1000000000;
/** Number of microsecond in a second */
const int32_t USEC_PER_SEC = 1000000;

#define DECLARE_STATIC(x,...) decltype(x) x __VA_OPT__({) __VA_ARGS__ __VA_OPT__(})

__CLKMGR_NAMESPACE_END

#endif /* UTIL_HPP */
