/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file jclklib_client_api_c.h
 * @brief Client utilities to setup and cleanup the library.
 *
 * @author Song Yoong Siang <yoong.siang.song@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

enum event_type {
    gmOffsetEvent,
    syncedToPrimaryClockEvent,
    asCapableEvent,
    gmChangedEvent,
    eventLast
};

enum value_type {
    gm_offset,
    value_last
};

struct value {
    int32_t upper;
    int32_t lower;
};

struct jcl_c_subscription {
    uint32_t event[1];
    struct value value[value_last];
    uint32_t composite_event[1];
};

struct jcl_c_state {
    uint8_t gm_identity[8];
    bool    as_capable;
    bool    offset_in_range;
    bool    synced_to_primary_clock;
    bool    gm_changed;
    bool    composite_event;
};

struct jcl_c_event_count {
    uint64_t offset_in_range_event_count;
    uint64_t gm_changed_event_count;
    uint64_t as_capable_event_count;
    uint64_t synced_to_primary_clock_event_count;
    uint64_t composite_event_count;
};

typedef void *jcl_c_client_ptr;

jcl_c_client_ptr jcl_c_client_create();
void jcl_c_client_destroy(jcl_c_client_ptr client_ptr);

bool jcl_c_connect(jcl_c_client_ptr client_ptr);
bool jcl_c_disconnect(jcl_c_client_ptr client_ptr);
bool jcl_c_subscribe(jcl_c_client_ptr client_ptr,
    struct jcl_c_subscription sub, struct jcl_c_state *current_state);
int jcl_c_status_wait(jcl_c_client_ptr client_ptr, int timeout,
    struct jcl_c_state *current_state, struct jcl_c_event_count *event_count);

#ifdef __cplusplus
}
#endif
