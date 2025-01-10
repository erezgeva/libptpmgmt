/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief data structure for PTP events.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef PTP_EVENT_HPP
#define PTP_EVENT_HPP

#include "common/util.hpp"

#include <atomic>
#include <cstdint>

__CLKMGR_NAMESPACE_BEGIN

struct ptp_event {
    int64_t master_offset;
    uint8_t gm_identity[8]; /* Grandmaster clock ID */
    bool as_capable; /* 802@.1AS Capable */
    bool synced_to_primary_clock;
    uint8_t ptp4l_id;
    int64_t chrony_offset;
    uint32_t chrony_reference_id;
    int64_t polling_interval;
};

struct client_ptp_event {
    int64_t master_offset;
    int64_t master_offset_low;
    int64_t master_offset_high;
    uint8_t gm_identity[8]; /* Grandmaster clock ID */
    uint8_t ptp4l_id;
    bool as_capable; /* 802@.1AS Capable */
    bool synced_to_primary_clock;
    bool master_offset_in_range;
    bool composite_event;
    std::atomic<int> offset_in_range_event_count{};
    std::atomic<int> as_capable_event_count{};
    std::atomic<int> synced_to_gm_event_count{};
    std::atomic<int> gm_changed_event_count{};
    std::atomic<int> composite_event_count{};
    int64_t chrony_offset;
    uint32_t chrony_reference_id;
    int64_t polling_interval;
    bool chrony_offset_in_range;
    std::atomic<int> chrony_offset_in_range_event_count{};
};

__CLKMGR_NAMESPACE_END

#endif /* PTP_EVENT_HPP */
