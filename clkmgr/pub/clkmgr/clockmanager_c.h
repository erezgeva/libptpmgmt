/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief C wrapper for the Clock Manager APIs to set up client-runtime.
 *
 * @author Song Yoong Siang <yoong.siang.song@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLKMGR_CLOCKMANAGER_C_H
#define CLKMGR_CLOCKMANAGER_C_H

#include <stdbool.h>
#include <stdint.h>

#include "pub/clkmgr/types_c.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * structure to hold the event mask, composite event mask, and thresholds
 * for events that require user-defined threshold (upper and lower limits).
 */
struct clkmgr_c_subscription {
    uint32_t event_mask; /**< Events subscription mask */
    uint32_t composite_event_mask; /**< Composite events mask */
    struct Clkmgr_Threshold threshold[CLKMGR_THRESHOLD_MAX]; /**< Limits */
};

/** Pointer to the client structure */
typedef void *clkmgr_c_client_ptr;

/**
 * @brief Create a new client instance
 * @return Pointer to the new client instance
 */
clkmgr_c_client_ptr clkmgr_c_client_create();

/**
 * @brief Destroy a client instance
 * @param[in, out] client_ptr Pointer to the client instance
 */
void clkmgr_c_client_destroy(clkmgr_c_client_ptr client_ptr);

/**
 * @brief Connect the client
 * @param[in, out] client_ptr Pointer to the client instance
 * @return true on success, false on failure
 */
bool clkmgr_c_connect(clkmgr_c_client_ptr client_ptr);

/**
 * @brief Disconnect the client
 * @param[in, out] client_ptr Pointer to the client instance
 * @return true on success, false on failure
 */
bool clkmgr_c_disconnect(clkmgr_c_client_ptr client_ptr);

/**
 * @brief Subscribe to client events
 * @param[in, out] client_ptr Pointer to the client instance
 * @param[in] sub Subscription structure
 * @param[out] current_state Pointer to the current state structure
 * @return true on success, false on failure
 */
bool clkmgr_c_subscribe(clkmgr_c_client_ptr client_ptr,
    struct clkmgr_c_subscription sub,
    struct Clkmgr_Event_state *current_state);

/**
 * @brief Waits for a specified timeout period for any event changes
 * @param[in, out] client_ptr Pointer to the client instance
 * @param[in] timeout TThe timeout in seconds. If timeout is 0, the function
 * will check event changes once. If timeout is -1, the function will wait
 * until there is event changes occurs
 * @param[out] current_state Pointer to the current state structure
 * @param[out] current_count Pointer to the event count structure
 * @return true if there is event changes within the timeout period,
 *         and false otherwise
 */
int clkmgr_c_status_wait(clkmgr_c_client_ptr client_ptr, int timeout,
    struct Clkmgr_Event_state *current_state,
    struct Clkmgr_Event_count *current_count);

#ifdef __cplusplus
}
#endif

#endif /* CLKMGR_CLOCKMANAGER_C_H */
