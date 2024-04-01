/*! \file util.hpp
    \brief Helper functions, templates, and macros

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <future>
#include <chrono>

#ifndef UTIL_HPP
#define UTIL_HPP

#include <common/mutex_signal.hpp>

#define UNIQUE_TYPEOF(x) remove_reference<decltype(*(x).get())>::type
#define FUTURE_TYPEOF(x) decltype((x).get())
#define DECLARE_STATIC(x,...) decltype(x) x __VA_OPT__({) __VA_ARGS__ __VA_OPT__(})

template <typename type>
bool isFutureSet(std::future<type> &f)
{
	return f.valid() && f.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
}

#define DECLARE_ACCESSOR(varname)					\
	const decltype(varname) &getc_##varname() { return varname; }		\
	decltype(varname) &get_##varname() { return varname; }		\
	void set_##varname (const decltype(varname) &varname) { this->varname = varname; }		\
	decltype(varname) c_get_val_##varname () const { return varname; }

#define PTHREAD_CALL(func,err_msg,retval)				\
	{								\
		int err;						\
		if ((err = (func)) != 0) {				\
			PrintError(err_msg, err);			\
			return retval;					\
		}							\
	}
	

#endif/*UTIL_HPP*/

