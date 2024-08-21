/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file thread.cpp
 * @brief Proxy message thread class.
 *
 * @author Lai Peter Jun Ann <peter.jun.ann.lai@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#include "thread.hpp"

static int start_work_thread(struct ptp4l_handle *handle)
{
    int ret;
    if((ret = -pthread_create(&handle->work_thread, nullptr, ptp4l_event_loop,
                    (void *) handle)) < 0)
        return ret;
    return 0;
}

int connect_ptp4l(struct ptp4l_handle **phandle)
{
    struct ptp4l_handle *handle;
    int ret;
    handle = (typeof(handle)) malloc((size_t) sizeof(*handle));
    if(handle == nullptr) {
        ret = -errno;
        goto alloc_handle_fail;
    }
    if((ret = start_work_thread(handle)) < 0)
        goto thread_failed;
    *phandle = handle;
    return 0;
thread_failed:
alloc_handle_fail:
    return ret;
}

/**
 * @brief Initializes and connects a handle for PTP (Precision Time Protocol)
 * event handling.
 *
 * This function allocates and initializes a jcl_handle structure,
 * sets up thread signaling, mutexes, and subscribes to PTP events.
 * It also initiates a connection to ptp4l and enters
 * a loop to wait for and handle status updates.
 *
 * @param phandle A double pointer to a jcl_handle structure
 *                where the initialized handle will be stored.
 * @param epd_event An epoll_event structure containing the
 *                  event data used for the connection.
 * @return An integer indicating the status of the connection attempt.
 *         Returns 0 on success, or a negative error code on failure.
 *
 */
int handle_connect()
{
    struct jcl_handle *handle;
    int ret;
    handle = (typeof(handle)) malloc((size_t) sizeof(*handle));
    if(handle == nullptr) {
        ret = -errno;
        goto alloc_fail;
    }
    event_subscription(&handle);
    ret = connect_ptp4l(&handle->ptp4l_handle);
    if(ret != 0) {
        free(handle);
        goto alloc_fail;
    }
    free(handle);
    return 0;
alloc_fail:
    return ret;
}
