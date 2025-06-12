/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief C++ shared_mutex class using pthread_rwlock
 *
 * @nore The class is identical to C++17 std::shared_mutex
 *       The class do NOT provide priority inheritance!
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#ifndef CLKMGR_SHARED_MUTEX_HPP
#define CLKMGR_SHARED_MUTEX_HPP

#if __cplusplus < 201703L // C++17
#include <pthread.h>

__CLKMGR_NAMESPACE_BEGIN

// Full implementation of C++17 std::shared_mutex
// Using POSIX read write lock
class shared_mutex
{
  private:
    pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

  public:
    shared_mutex() = default;
    ~shared_mutex() { pthread_rwlock_destroy(&rwlock); }

    shared_mutex(const shared_mutex &) = delete;
    shared_mutex &operator=(const shared_mutex &) = delete;

    // Exclusive ownership
    void lock() { pthread_rwlock_wrlock(&rwlock); }
    bool try_lock() { return pthread_rwlock_trywrlock(&rwlock) == 0; }
    void unlock() { pthread_rwlock_unlock(&rwlock); }

    // Shared ownership
    void lock_shared() { pthread_rwlock_rdlock(&rwlock); }
    bool try_lock_shared() { return pthread_rwlock_tryrdlock(&rwlock) == 0; }
    void unlock_shared() { unlock(); }

    typedef void *native_handle_type;
    native_handle_type native_handle() { return &rwlock; }
};

__CLKMGR_NAMESPACE_END

#endif /* __cplusplus C++17 */

#if __cplusplus >= 201402L // C++14
#include <shared_mutex>
#else // __cplusplus C++14

__CLKMGR_NAMESPACE_BEGIN

// Partial inmplementation of C++14 std::shared_lock
// Only the relevant used methods
template<typename mutexType> class shared_lock
{
  private:
    mutexType &mutexObj;
    bool isLock = true;
  public:
    shared_lock() = delete;
    shared_lock(mutexType &m) : mutexObj(m) { m.lock_shared(); }
    ~shared_lock() { if(isLock) mutexObj.unlock_shared(); }

    shared_lock(shared_lock const &) = delete;
    shared_lock &operator=(shared_lock const &) = delete;
    shared_lock &operator=(shared_lock &&) = delete;

    void lock() { mutexObj.lock_shared(); isLock = true; }
    bool try_lock() { return isLock = mutexObj.try_lock_shared(); }
    void unlock() { mutexObj.unlock_shared(); isLock = false; }
};

__CLKMGR_NAMESPACE_END

#endif // __cplusplus C++14

#endif /* CLKMGR_SHARED_MUTEX_HPP */
