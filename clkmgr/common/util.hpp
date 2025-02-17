/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Helper functions, templates, and macros
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef UTIL_HPP
#define UTIL_HPP

#include <chrono>
#include <future>

#include "pub/clkmgr/utility.h"

/** Maximum number of character for transport client ID */
const int TRANSPORT_CLIENTID_LENGTH = 512;

/** Array to store transport client ID. */
typedef std::array<uint8_t, TRANSPORT_CLIENTID_LENGTH> TransportClientId;

/** Type definition for session ID. */
typedef uint16_t sessionId_t;

/** Invalid session ID (default session ID) */
const sessionId_t InvalidSessionId = UINT16_MAX;

/* Some commonly used constants */
#define NSEC_PER_MSEC   (1000000)
#define NSEC_PER_SEC    (1000000000)

#define UNIQUE_TYPEOF(x) remove_reference<decltype(*(x).get())>::type
#define FUTURE_TYPEOF(x) decltype((x).get())
#define DECLARE_STATIC(x,...) decltype(x) x __VA_OPT__({) __VA_ARGS__ __VA_OPT__(})

template <typename type>
bool isFutureSet(std::future<type> &f)
{
    return f.valid() &&
        f.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
}

#define PTHREAD_CALL(func,err_msg,retval)               \
    {                               \
        int err;                        \
        if ((err = (func)) != 0) {              \
            PrintError(err_msg, err);           \
            return retval;                  \
        }                           \
    }

#endif /* UTIL_HPP */
