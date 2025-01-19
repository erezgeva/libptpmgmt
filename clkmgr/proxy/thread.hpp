/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy message thread class.
 *
 * @author Lai Peter Jun Ann <peter.jun.ann.lai@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "common/ptp_event.hpp"

#include <stdlib.h>
#include <string.h>

typedef struct ptp4l_handle *ptp4l_handle_t;
typedef struct clkmgr_handle *clkmgr_handle_t;

typedef bool (*state_update_t)(struct ptp4l_state *, void *);

struct clkmgr_handle {
    ptp4l_handle_t ptp4l_handle;
};

struct ptp4l_handle {
    state_update_t update_cb;
    void *ctx;
    pthread_t work_thread;
};

struct ptp4l_state {
    bool initialized;
    unsigned valid_mask;
    bool peer_present;
    unsigned offset;
    bool synced_to_primary_clock;
};

int handle_connect();
int connect_ptp4l(ptp4l_handle_t *phandle);

void *ptp4l_event_loop(void *arg);
bool event_subscription(struct clkmgr_handle **handle);
