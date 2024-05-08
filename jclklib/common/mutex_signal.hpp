/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file mutex_signal.hpp
 * @brief Common pthread mutex implementation.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#include <chrono>
#include <pthread.h>

#ifndef MUTEX_SIGNAL_HPP
#define MUTEX_SIGNAL_HPP

namespace mutex_signal_test {
	class mutex_signal
	{
	private:
		pthread_mutex_t p_lock;
		pthread_mutex_t c_lock;
		pthread_mutex_t d_lock;
		bool update_available;

		bool consumer_locked;
		bool producer_locked;

		bool consumerTryLock(bool &success);
		bool consumerLock();
		bool consumerUnlock();

		bool producerTryLock(bool &success);
		bool producerTimedLock(bool &success, std::chrono::time_point<std::chrono::system_clock> wait_until);
		bool producerLock();
		bool producerUnlock();

		bool dataTryLock(bool &success);
		bool dataTimedLock(bool &success, std::chrono::time_point<std::chrono::system_clock> wait_until);
		bool dataLock();
		bool dataUnlock();
	public:
		mutex_signal();
		~mutex_signal();
		bool init();

		bool producerInit();
		bool producerCleanup();
		bool preProduce();
		bool postProduce();

		bool consumerInit();
		bool consumerCleanup();
		bool preConsume(bool &success, std::chrono::duration<unsigned, std::nano> wait_until);
		bool postConsume();
	};
}

#endif/*MUTEX_SIGNAL_HPP*/
