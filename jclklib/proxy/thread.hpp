/*! \file thread.hpp
    \brief Proxy message thread class.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Lai Peter Jun Ann <peter.jun.ann.lai@intel.com>
*/
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/epoll.h>

typedef struct thread_signal *thread_signal_t;
typedef struct ptp4l_handle *ptp4l_handle_t;
typedef struct jcl_handle *jcl_handle_t;

struct thread_signal {
	pthread_mutex_t siglock;
	pthread_cond_t signal;
	bool signaled;
};

struct jcl_state
{
	bool     peer_present;
	bool     gm_present;
	unsigned offset;
	bool servo_locked;
};

struct jcl_threshold
{
	unsigned offset; /* ns */
};

struct jcl_subscription
{
	unsigned	     event;
	struct jcl_threshold threshold;
};

struct jcl_handle
{
	clockid_t clk_id;

	ptp4l_handle_t ptp4l_handle;
	bool ptp4l_state_initialized;

	thread_signal_t status_signal;
	pthread_mutex_t state_lock;
	unsigned event_count[4]; //jcl_event_count
	struct jcl_subscription subscription;
	struct jcl_state state;
};

typedef enum { jcl_peer_present, jcl_gm_present, jcl_offset_threshold, jcl_servo_locked,
	       jcl_event_count,}
	jcl_event_t;

#define jcl_servo_locked_flag			  ( 0x1 << jcl_gm_present)
#define jcl_gm_present_flag			  ( 0x1 << jcl_gm_present)
#define jcl_peer_present_flag			( 0x1 << jcl_peer_present)
#define jcl_offset_threshold_flag	    ( 0x1 << jcl_offset_threshold)

typedef bool ( *state_update_t)( struct ptp4l_state *, void *);
typedef struct thread_signal *thread_signal_t;

int handle_connect( jcl_handle_t *handle, struct epoll_event epd_event );
int connect_ptp4l( ptp4l_handle_t *phandle, struct epoll_event epd_event, state_update_t update, void *ctx);
int disconnect_ptp4l( ptp4l_handle_t handle);

void *ptp4l_event_loop(void *arg);
bool event_subscription(struct jcl_handle **handle);

struct ptp4l_handle {
	int ptp4l_sock;
	int epd;
	struct epoll_event epd_event;
	int error;

	pid_t pid;
	struct sockaddr_un server_addr;
	struct sockaddr_un client_addr;

	state_update_t update_cb;
	void *ctx;

	bool do_exit;
	thread_signal_t exit_signal;
	thread_signal_t init_signal;

	pthread_t work_thread;
};

enum ptp4l_status_update_type
{
	ptp4l_peer_present,
	ptp4l_offset,
	ptp4l_gmpresent,
	ptp4l_servolocked,
};

#define ptp4l_peer_present_valid	(0x1 << ptp4l_peer_present)
#define ptp4l_offset_valid		(0x1 << ptp4l_offset)
#define ptp4l_gmpresent_valid		(0x1 << ptp4l_gmpresent)
#define ptp4l_servolocked_valid		(0x1 << ptp4l_servolocked)

struct ptp4l_state {
	bool			 initialized;
	unsigned		  valid_mask;
	bool			peer_present;
	unsigned		      offset;
	bool			  gm_present;
	bool			servo_locked;
};
