/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file thread.cpp
 * @brief Proxy message thread class.
 *
 * @author Lai Peter Jun Ann <peter.jun.ann.lai@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#include "thread.hpp"

static int start_work_thread( struct ptp4l_handle *handle )
{
	int ret;

	if(( ret = -pthread_create( &handle->work_thread, NULL, ptp4l_event_loop, (void *) handle)) < 0)
			return ret;

	return 0;
}

int connect_ptp4l( struct ptp4l_handle **phandle, struct epoll_event epd_event, state_update_t update_cb, void *ctx)
{
	struct ptp4l_handle *handle;
	int ret;

	handle = (typeof( handle)) malloc(( size_t) sizeof(*handle));
	if( handle == NULL) {
		ret = -errno;
		goto alloc_handle_fail;
	}

	if(( ret = start_work_thread( handle)) < 0)
		goto thread_failed;

	*phandle = handle;
	return 0;

 thread_failed:
 alloc_handle_fail:
	return ret;
}

int set_thread_signal( struct thread_signal *signal)
{
	int err, ret;

	if(( ret = pthread_mutex_lock( &signal->siglock)) != 0)
		goto lock_fail;
	signal->signaled = true;
	if(( ret = pthread_cond_signal( &signal->signal )) != 0)
		goto signal_fail;

 signal_fail:
	if(( err = pthread_mutex_unlock( &signal->siglock)) != 0)
		ret = ret == 0 ? err : ret; // Report first error only

 lock_fail:
	return -ret;
}

static bool state_update_callback( struct ptp4l_state *state, void * ctx)
{
	struct jcl_handle *handle = (typeof(handle)) ctx;
	int ret;

	if(( ret = -pthread_mutex_lock( &handle->state_lock)) < 0)
		goto lock_failed;

	if(( ret = -pthread_mutex_unlock( &handle->state_lock)) < 0)
		goto unlock_failed;

 unlock_failed:
 lock_failed:
	return ret >= 0 ? true : false;
}

/**
 * @brief Initializes and connects a handle for PTP (Precision Time Protocol) event handling.
 *
 * This function allocates and initializes a jcl_handle structure, sets up thread signaling,
 * mutexes, and subscribes to PTP events. It also initiates a connection to ptp4l and enters
 * a loop to wait for and handle status updates.
 *
 * @param phandle A double pointer to a jcl_handle structure where the initialized handle will
 *                be stored.
 * @param epd_event An epoll_event structure containing the event data used for the connection.
 * @return An integer indicating the status of the connection attempt.
 *         Returns 0 on success, or a negative error code on failure.
 *
 */
int handle_connect(struct epoll_event epd_event )
{
	struct jcl_handle *handle;
	int ret;

	handle = ( typeof( handle)) malloc(( size_t) sizeof( *handle));
	if( handle == NULL) {
		ret = -errno;
		goto alloc_fail;
	}

	event_subscription( &handle );

	ret = connect_ptp4l( &handle->ptp4l_handle,
				  epd_event, state_update_callback, handle);
	if (ret != 0) {
		free(handle); // Free the memory if connect_ptp4l fails
		goto alloc_fail;
	}

	free(handle);
	return 0;

 alloc_fail:
	return ret;
}

