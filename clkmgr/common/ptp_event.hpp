/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief data structure for PTP events.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef PTP_EVENT_HPP
#define PTP_EVENT_HPP

#include "common/util.hpp"

__CLKMGR_NAMESPACE_BEGIN

struct ptp_event {
    int64_t clockOffset;
    uint64_t gmClockUUID;
    uint64_t syncInterval;
    bool asCapable;
    bool syncedWithGm;
};

struct chrony_event {
    int64_t clockOffset;
    uint64_t gmClockUUID;
    uint64_t syncInterval;
};

__CLKMGR_NAMESPACE_END

#endif /* PTP_EVENT_HPP */
