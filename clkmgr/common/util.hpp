/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file util.hpp
 * @brief Helper functions, templates, and macros
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#include <chrono>
#include <future>

#ifndef UTIL_HPP
#define UTIL_HPP

/** Using clkmgr namespace */
#define __CLKMGR_NAMESPACE_USE using namespace clkmgr;
/** Define start of clkmgr namespace block */
#define __CLKMGR_NAMESPACE_BEGIN namespace clkmgr {
/** Define end of clkmgr namespace block */
#define __CLKMGR_NAMESPACE_END }

/* Some commonly used constants */
#define NSEC_PER_MSEC   (1000000)
#define NSEC_PER_SEC    (1000000000)

#define UNIQUE_TYPEOF(x) remove_reference<decltype(*(x).get())>::type
#define FUTURE_TYPEOF(x) decltype((x).get())
#define DECLARE_STATIC(x,...) decltype(x) x __VA_OPT__({) __VA_ARGS__ __VA_OPT__(})

template <typename type>
bool isFutureSet(std::future<type> &f)
{
    return f.valid() &&
        f.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
}

#define DECLARE_ACCESSOR(varname)                   \
    const decltype(varname) &getc_##varname() { return varname; }       \
    decltype(varname) &get_##varname() { return varname; }      \
    void set_##varname (const decltype(varname) &varname) \
    { this->varname = varname; }      \
    decltype(varname) c_get_val_##varname () const { return varname; }

#define PTHREAD_CALL(func,err_msg,retval)               \
    {                               \
        int err;                        \
        if ((err = (func)) != 0) {              \
            PrintError(err_msg, err);           \
            return retval;                  \
        }                           \
    }

#endif /* UTIL_HPP */
