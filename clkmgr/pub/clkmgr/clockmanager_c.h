/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file clockmanager_c.h
 * @brief C wrapper for the Clock Manager APIs to set up client-runtime.
 *
 * @author Song Yoong Siang <yoong.siang.song@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLKMGR_CLOCKMANAGER_C_H
#define CLKMGR_CLOCKMANAGER_C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

/** Types of PTP events subscription */
enum event_type {
    gmOffsetEvent,                /**< Primary-secondary clock offset event */
    syncedToPrimaryClockEvent,    /**< Synced to primary clock event */
    asCapableEvent,               /**< IEEE 802.1AS capable event */
    gmChangedEvent,               /**< Primary clock UUID changed event */
    eventLast                     /**< Last event type */
};

/** Types for struct value */
enum value_type {
    gm_offset,    /**< Primary-secondary clock offset */
    value_last    /**< Last value type */
};

/** Structure to hold upper and lower limits */
struct value {
    int32_t upper;    /**< Upper limit */
    int32_t lower;    /**< Lower limit */
};

/** Subscription structure for the events */
struct clkmgr_c_subscription {
    uint32_t event[1];                    /**< Events subscription */
    struct value value[value_last];       /**< Limits */
    uint32_t composite_event[1];          /**< Composite events subscription */
};

/** Current State for the events */
struct clkmgr_c_state {
    uint8_t gm_identity[8];               /**< Primary clock UUID */
    bool as_capable;                      /**< IEEE 802.1AS capable */
    bool offset_in_range;                 /**< Clock offset in range */
    bool synced_to_primary_clock;         /**< Synced to primary clock */
    bool gm_changed;                      /**< Primary clock UUID changed */
    bool composite_event;                 /**< Composite event */
    int64_t  clock_offset;                /**< Clock offset */
    uint64_t notification_timestamp;      /**< Timestamp for last notification */
};

/** Event count for the events */
struct clkmgr_c_event_count {
    uint64_t offset_in_range_event_count;         /**< Clk offset in range */
    uint64_t gm_changed_event_count;              /**< Primary clk ID changed */
    uint64_t as_capable_event_count;              /**< IEEE 802.1AS capable */
    uint64_t synced_to_primary_clock_event_count; /**< Synced to primary clk */
    uint64_t composite_event_count;               /**< Composite event */
};

/** Pointer to the client structure */
typedef void *clkmgr_c_client_ptr;

/**
 * Create a new client instance
 * @return Pointer to the new client instance
 */
clkmgr_c_client_ptr clkmgr_c_client_create();

/**
 * Destroy a client instance
 * @param[in, out] client_ptr Pointer to the client instance
 */
void clkmgr_c_client_destroy(clkmgr_c_client_ptr client_ptr);

/**
 * Connect the client
 * @param[in, out] client_ptr Pointer to the client instance
 * @return true on success, false on failure
 */
bool clkmgr_c_connect(clkmgr_c_client_ptr client_ptr);

/**
 * Disconnect the client
 * @param[in, out] client_ptr Pointer to the client instance
 * @return true on success, false on failure
 */
bool clkmgr_c_disconnect(clkmgr_c_client_ptr client_ptr);

/**
 * Subscribe to client events
 * @param[in, out] client_ptr Pointer to the client instance
 * @param[in] sub Subscription structure
 * @param[out] current_state Pointer to the current state structure
 * @return true on success, false on failure
 */
bool clkmgr_c_subscribe(clkmgr_c_client_ptr client_ptr,
    struct clkmgr_c_subscription sub, struct clkmgr_c_state *current_state);

/**
 * Wait for client status
 * @param[in, out] client_ptr Pointer to the client instance
 * @param[in] timeout Timeout value in seconds
 * @param[out] current_state Pointer to the current state structure
 * @param[out] event_count Pointer to the event count structure
 * @return true if there is event changes within the timeout period,
 *         and false otherwise.
 */
int clkmgr_c_status_wait(clkmgr_c_client_ptr client_ptr, int timeout,
    struct clkmgr_c_state *current_state, struct clkmgr_c_event_count *event_count);

#ifdef __cplusplus
}
#endif

#endif /* CLKMGR_CLOCKMANAGER_C_H */
