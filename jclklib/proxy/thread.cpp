/*! \file thread.cpp
    \brief Proxy message thread class.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Lai Peter Jun Ann <peter.jun.ann.lai@intel.com>
*/

#include <cstdio>
#include <ptpmgmt/init.h>
#include <poll.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <poll.h>
#include <libgen.h>
#include <string>
#include <map>

#include "thread.hpp"
//#include "ptp4l_connect.hpp"

using namespace ptpmgmt;

#define MSEC_PER_SEC	(1000)
#define USEC_PER_SEC	(1000000)
#define NSEC_PER_SEC	(1000000000)
#define NSEC_PER_USEC	(NSEC_PER_SEC/USEC_PER_SEC)
#define USEC_PER_MSEC	(USEC_PER_SEC/MSEC_PER_SEC)

int initialize_thread_signal( struct thread_signal **psignal)
{
	struct thread_signal *signal;
	pthread_condattr_t signal_attr;
	pthread_mutexattr_t siglock_attr;
	int err, ret;

	signal = ( typeof( signal)) malloc(( size_t) sizeof( *signal));
	if( signal == NULL) {
		ret = errno;
		goto allocate_fail;
	}

	if(( ret = pthread_mutexattr_init( &siglock_attr)) != 0)
		goto mutexattr_fail;
	if(( ret = pthread_condattr_init( &signal_attr)) != 0)
		goto condattr_fail;

	if(( ret = pthread_mutex_init
	     ( &signal->siglock, &siglock_attr)) != 0 ) {
		goto fail;
	}
	if(( ret = pthread_cond_init
	     ( &signal->signal, &signal_attr)) != 0 ) {
		goto fail;
	}

 fail:
	if(( err = pthread_condattr_destroy( &signal_attr)) != 0)
		ret = ret == 0 ? err : ret; // Report first error only
 condattr_fail:
	if(( err = pthread_mutexattr_destroy( &siglock_attr)) != 0)
		ret = ret == 0 ? err : ret; // Report first error only
 mutexattr_fail:
 allocate_fail:

	if( ret == 0) {
		signal->signaled = false;
		*psignal = signal;
	} else {
		free(signal);
	}

	return -ret;
}

int wait_thread_signal( struct thread_signal *signal, unsigned timeout )
{
	struct timespec abs_timeout;
	int err, ret;

	clock_gettime( CLOCK_REALTIME, &abs_timeout);
	abs_timeout.tv_nsec += (timeout % USEC_PER_SEC) * NSEC_PER_USEC;
	abs_timeout.tv_sec  += abs_timeout.tv_nsec / NSEC_PER_SEC;
	abs_timeout.tv_nsec %= NSEC_PER_SEC;
	abs_timeout.tv_sec  += timeout / USEC_PER_SEC;

	if(( ret = pthread_mutex_lock( &signal->siglock)) != 0)
		goto lock_fail;
	while( !signal->signaled) {
		if(( ret = pthread_cond_timedwait
		     ( &signal->signal, &signal->siglock, &abs_timeout )) != 0) {
			break;
		}
	}
	if( ret == 0)
		signal->signaled = false;

	if(( err = pthread_mutex_unlock( &signal->siglock)) != 0)
		ret = ret == 0 ? err : ret; // Report first error only

 lock_fail:
	return -ret;
}

static bool check_for_events( struct jcl_handle *handle)
{
	int i;

	for( i = 0; i < jcl_event_count; ++i) {
		if( handle->event_count[i] != 0)
			return true;
	}

	return false;
}

static int check_status( struct jcl_handle *handle,
			 unsigned *event_count, struct jcl_state *state, bool *done)
{
	int ret;

	*done = false;
	if(( ret = -pthread_mutex_lock( &handle->state_lock)) < 0)
		goto lock_failed;
	if( check_for_events( handle)) {
		*state = handle->state;
		memcpy( event_count, handle->event_count, sizeof( handle->event_count));
		*done = true;
	}
	ret = -pthread_mutex_unlock( &handle->state_lock);

 lock_failed:
	return ret;
}

int handle_status_wait( struct jcl_handle *handle, unsigned timeout,
			    unsigned *event_count, struct jcl_state *state )
{
	bool done;
	int ret;

	if(( ret = check_status( handle, event_count, state, &done)) < 0 || done || timeout == 0)
		goto do_exit;

	if(( ret = wait_thread_signal( handle->status_signal, timeout)) < 0)
		goto do_exit;

	ret = check_status( handle, event_count, state, &done);

 do_exit:
	return ret;
}

static int start_work_thread( struct ptp4l_handle *handle )
{
	int ret;

	if(( ret = initialize_thread_signal( &handle->init_signal)) < 0)
		goto init_signal_failed;
	if(( ret = initialize_thread_signal( &handle->exit_signal)) < 0)
		goto exit_signal_failed;

	if(( ret = -pthread_create( &handle->work_thread, NULL, ptp4l_event_loop, (void *) handle)) < 0)
			return ret;

//	if(( ret = wait_thread_signal( handle->init_signal, 30000)) < 0) {
//				       PTP4L_CONNECT_TIMEOUT*USEC_PER_MSEC)) < 0)
//		goto start_wait_failed;
//	}
	return 0;

// start_wait_failed:
//	stop_work_thread( handle);
//	free_thread_signal( handle->exit_signal);
 exit_signal_failed:
//	free_thread_signal( handle->init_signal);
 init_signal_failed:
	return ret;
}

int connect_ptp4l( struct ptp4l_handle **phandle, struct epoll_event epd_event, state_update_t update_cb, void *ctx)
{
	struct ptp4l_handle *handle;
	int ret;
	char *path;

	handle = (typeof( handle)) malloc(( size_t) sizeof(*handle));
	if( handle == NULL) {
		ret = -errno;
		goto alloc_handle_fail;
	}

	memset( &handle->server_addr, 0, sizeof( handle->server_addr));
	handle->server_addr.sun_family = AF_LOCAL;

	//TODO: hard-coded for now
	path = "/var/run/ptp4l";
	strncpy( handle->server_addr.sun_path, path,
		 sizeof( handle->server_addr.sun_path ) - 1);

	//subscribe event
	handle->epd_event = epd_event;
	handle->ptp4l_sock = epd_event.data.fd;

	handle->update_cb = update_cb;
	handle->ctx = ctx;
	handle->pid = getpid();

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
	bool generate_signal = false;
	int ret;

	if(( ret = -pthread_mutex_lock( &handle->state_lock)) < 0)
		goto lock_failed;

	// TODO: update the state based on the event notification
	//generate_signal = update_offset( handle, state) || generate_signal ? true : false;
	//generate_signal = update_peer_present( handle, state) || generate_signal ? true : false;
	//generate_signal = update_gm_present( handle, state) || generate_signal ? true : false;
	//generate_signal = update_servo_locked( handle, state) || generate_signal ? true : false;

	generate_signal = handle->ptp4l_state_initialized ? generate_signal : false;
	handle->ptp4l_state_initialized = state->initialized;

	if(( ret = -pthread_mutex_unlock( &handle->state_lock)) < 0)
		goto unlock_failed;

	if( generate_signal)
		set_thread_signal( handle->status_signal);

 unlock_failed:
 lock_failed:
	return ret >= 0 ? true : false;
}

int init_thread_signal( struct thread_signal **psignal)
{
	struct thread_signal *signal;
	pthread_condattr_t signal_attr;
	pthread_mutexattr_t siglock_attr;
	int err, ret;

	signal = ( typeof( signal)) malloc(( size_t) sizeof( *signal));
	if( signal == NULL) {
		ret = errno;
		goto allocate_fail;
	}

	if(( ret = pthread_mutexattr_init( &siglock_attr)) != 0)
		goto mutexattr_fail;
	if(( ret = pthread_condattr_init( &signal_attr)) != 0)
		goto condattr_fail;

	if(( ret = pthread_mutex_init
	     ( &signal->siglock, &siglock_attr)) != 0 ) {
		goto fail;
	}
	if(( ret = pthread_cond_init
	     ( &signal->signal, &signal_attr)) != 0 ) {
		goto fail;
	}

 fail:
	if(( err = pthread_condattr_destroy( &signal_attr)) != 0)
		ret = ret == 0 ? err : ret; // Report first error only
 condattr_fail:
	if(( err = pthread_mutexattr_destroy( &siglock_attr)) != 0)
		ret = ret == 0 ? err : ret; // Report first error only
 mutexattr_fail:
 allocate_fail:
	if( ret == 0) {
		signal->signaled = false;
		*psignal = signal;
	} else {
		free(signal);
	}

	return -ret;
}

int free_thread_signal( struct thread_signal *signal)
{
	int err, ret;

	ret = pthread_mutex_destroy( &signal->siglock);
	if(( err = pthread_cond_destroy( &signal->signal)) != 0 )
		ret = ret == 0 ? err : ret; // Report first error only

	free(signal);

	return -ret;
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
int handle_connect( struct jcl_handle **phandle, struct epoll_event epd_event )
{
	struct jcl_handle *handle;
	int ret;

	handle = ( typeof( handle)) malloc(( size_t) sizeof( *handle));
	if( handle == NULL) {
		ret = -errno;
		goto alloc_fail;
	}

	if(( ret = init_thread_signal( &handle->status_signal)) < 0)
		goto status_signal_fail;

	if(( ret = -pthread_mutex_init( &handle->state_lock, NULL)) < 0)
		goto state_lock_fail;

    //handle_init start
	handle->state.peer_present = false;
	handle->state.offset = UINT_MAX;
	handle->subscription.event = 0;
	memset( handle->event_count, 0, sizeof( handle->event_count));

	event_subscription( &handle );

	connect_ptp4l( &handle->ptp4l_handle,
				  epd_event, state_update_callback, handle);

	//TODO: wait operation
	unsigned event_count[1000];
	int iter;
	iter = 100;
	while (iter > 0) {
		handle_status_wait( handle, 1000, handle->event_count, &handle->state );
		--iter;
	}

	return 0;

 state_lock_fail:
	free_thread_signal( handle->status_signal);
 status_signal_fail:
 alloc_fail:
	return ret;
}

