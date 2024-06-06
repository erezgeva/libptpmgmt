/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file thread.hpp
 * @brief Proxy message thread class.
 *
 * @author Lai Peter Jun Ann <peter.jun.ann.lai@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <unistd.h>

#include <common/jclklib_import.hpp>

typedef struct thread_signal *thread_signal_t;
typedef struct ptp4l_handle *ptp4l_handle_t;
typedef struct jcl_handle *jcl_handle_t;

typedef bool ( *state_update_t)( struct ptp4l_state *, void *);
typedef struct thread_signal *thread_signal_t;

struct thread_signal {
    pthread_mutex_t siglock;
    pthread_cond_t signal;
    bool signaled;
};

struct jcl_handle
{
    ptp4l_handle_t ptp4l_handle;
    thread_signal_t status_signal;
    pthread_mutex_t state_lock;
};

struct ptp4l_handle {
    struct epoll_event epd_event;
    state_update_t update_cb;
    void *ctx;
    pthread_t work_thread;
};

struct ptp4l_state {
    bool initialized;
    unsigned valid_mask;
    bool peer_present;
    unsigned offset;
    bool servo_locked;
};

int handle_connect(struct epoll_event epd_event );
int connect_ptp4l(ptp4l_handle_t *phandle, struct epoll_event epd_event,
    state_update_t update, void *ctx);

void *ptp4l_event_loop(void *arg);
bool event_subscription(struct jcl_handle **handle);
