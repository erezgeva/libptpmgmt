/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file mutex_signal.cpp
 * @brief Common pthread mutex implementation.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#include <common/mutex_signal.hpp>

#include <cstring>
#include <iostream>

using namespace std;
using namespace mutex_signal_test;

#define NSEC_PER_SEC (1000000000)

#define PTHREAD_CALL(func,err_msg,retval,...)				\
	({								\
		int err;						\
		if ((err = (func)) != 0 __VA_OPT__(&& err !=) __VA_ARGS__) { \
			cerr << string(err_msg) + " " + strerror(err) + "\n"; \
			return retval;					\
		}							\
		(err == 0);						\
        })

mutex_signal::mutex_signal()
{
	update_available = false;
	consumer_locked = false;
	producer_locked = false;
}

bool mutex_signal::init()
{
        pthread_mutexattr_t attr;

        PTHREAD_CALL(pthread_mutexattr_init(&attr), "Failed to initialize mutex attribute", false);
        PTHREAD_CALL(pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT), "Failed to set priority inheritance", false);
	PTHREAD_CALL(pthread_mutex_init(&p_lock, &attr), "Failed to initialize producer mutex", false);
        PTHREAD_CALL(pthread_mutex_init(&c_lock, &attr), "Failed to initialize consumer mutex", false);
        PTHREAD_CALL(pthread_mutex_init(&d_lock, NULL), "Failed to initialize data mutex", false);
        PTHREAD_CALL(pthread_mutexattr_destroy(&attr), "Failed to cleanup mutex attribute", false);

        return true;
}

mutex_signal::~mutex_signal()
{
	PTHREAD_CALL(pthread_mutex_destroy(&d_lock), "Failed to cleanup data mutex",);
}

bool mutex_signal::consumerTryLock(bool &success)
{
	success = PTHREAD_CALL(pthread_mutex_trylock(&c_lock), "Failed to lock consumer lock", false, EBUSY);
	return true;
}
bool mutex_signal::consumerLock()
{
	PTHREAD_CALL(pthread_mutex_lock(&c_lock), "Failed to lock consumer lock", false);
	return true;
}
bool mutex_signal::consumerUnlock()
{
	PTHREAD_CALL(pthread_mutex_unlock(&c_lock), "Failed to unlock consumer lock", false);
	return true;
}

bool mutex_signal::producerTryLock(bool &success)
{
	success = PTHREAD_CALL(pthread_mutex_trylock(&p_lock), "Failed to lock producer lock", false, EBUSY);
	return true;
}
bool mutex_signal::producerTimedLock(bool &success, chrono::time_point<chrono::system_clock> wait_until)
{
	struct timespec wait_until_ts;
	wait_until_ts.tv_sec = chrono::duration_cast<chrono::nanoseconds>(wait_until.time_since_epoch()).count() / NSEC_PER_SEC;
	wait_until_ts.tv_nsec = chrono::duration_cast<chrono::nanoseconds>(wait_until.time_since_epoch()).count() % NSEC_PER_SEC;

	success = PTHREAD_CALL(pthread_mutex_timedlock(&p_lock, &wait_until_ts),"Failed to lock producer lock", false, ETIMEDOUT);

	return true;
}
bool mutex_signal::producerLock()
{
	PTHREAD_CALL(pthread_mutex_lock(&p_lock), "Failed to lock producer lock", false);
	return true;
}
bool mutex_signal::producerUnlock()
{
	PTHREAD_CALL(pthread_mutex_unlock(&p_lock), "Failed to unlock producer lock", false);
	return true;
}

bool mutex_signal::dataTryLock(bool &success)
{
	success = PTHREAD_CALL(pthread_mutex_trylock(&d_lock), "Failed to lock data lock", false, EBUSY);
	return true;
}
bool mutex_signal::dataTimedLock(bool &success, chrono::time_point<chrono::system_clock> wait_until)
{
	struct timespec wait_until_ts;
	wait_until_ts.tv_sec = chrono::duration_cast<chrono::nanoseconds>(wait_until.time_since_epoch()).count() / NSEC_PER_SEC;
	wait_until_ts.tv_nsec = chrono::duration_cast<chrono::nanoseconds>(wait_until.time_since_epoch()).count() % NSEC_PER_SEC;
	success = PTHREAD_CALL(pthread_mutex_timedlock(&d_lock, &wait_until_ts),"Failed to lock data lock", false, ETIMEDOUT);

	return true;
}
bool mutex_signal::dataLock()
{
	PTHREAD_CALL(pthread_mutex_lock(&d_lock), "Failed to lock data lock", false);
	return true;
}
bool mutex_signal::dataUnlock()
{
	PTHREAD_CALL(pthread_mutex_unlock(&d_lock), "Failed to unlock data lock", false);
	return true;
}

bool mutex_signal::preProduce()
{
	if (!dataLock())
		return false;

	producer_locked = true;
	return true;
}
bool mutex_signal::postProduce()
{
	bool producer_c_lock;
	if (!producer_locked)
		return false;
	update_available = true;
	if (!dataUnlock() || !consumerTryLock(producer_c_lock))
		return false;
	if (producer_c_lock && (!producerUnlock() || !consumerUnlock() || !producerLock()))
		return false;

	producer_locked = false;
	return true;
}

bool mutex_signal::preConsume(bool &success, chrono::duration<unsigned, nano> wait_for)
{
	auto wait_until = chrono::system_clock::now() + wait_for;
	bool consumer_p_lock;

	if (!dataTimedLock(success, wait_until))
		return false;
	consumer_locked = success && update_available;
	if (!success || consumer_locked)
		return true;

	success = false;
	if (!consumerUnlock() || !dataUnlock() || !producerTimedLock(consumer_p_lock, wait_until) || !consumerLock())
		return false;
	if (!consumer_p_lock)
		return true;
	if (!dataTimedLock(success, wait_until))
		return false;
	if (consumer_p_lock && !producerUnlock())
		return false;

	if (success && !update_available && !dataUnlock())
		return false;
	success = consumer_locked = success && update_available;

	return true;
}

bool mutex_signal::postConsume()
{
	if (!consumer_locked)
		return false;
	update_available = false;
	if(!dataUnlock())
		return false;

	consumer_locked = false;
	return true;
}

bool mutex_signal::producerInit()
{
	return producerLock();
}

bool mutex_signal::producerCleanup()
{
	if (producer_locked && !postProduce())
		return false;

	return producerUnlock();
}

bool mutex_signal::consumerInit()
{
	return consumerLock();
}

bool mutex_signal::consumerCleanup()
{
	if (consumer_locked && !postConsume())
		return false;

	return consumerUnlock();
}
