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
    int64_t master_offset;
    uint8_t gm_identity[8]; /* Grandmaster clock ID */
    bool as_capable; /* 802@.1AS Capable */
    int64_t ptp4l_sync_interval; /* in microseconds */
    bool synced_to_primary_clock;
};

struct chrony_event {
    int64_t chrony_offset;
    uint32_t chrony_reference_id;
    int64_t polling_interval;
};

__CLKMGR_NAMESPACE_END

#endif /* PTP_EVENT_HPP */
