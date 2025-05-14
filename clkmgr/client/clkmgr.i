/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/* Clock manager library Swig header file
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

%module clkmgr

/* Headers and namespace for moudle source code */
%{
    #include "pub/clockmanager.h"
    using namespace clkmgr;
%}

/* prevent C++ namespace in swig */
#define __CLKMGR_NAMESPACE_USE
#define __CLKMGR_NAMESPACE_BEGIN
#define __CLKMGR_NAMESPACE_END
/* Handle obsolete functions */
#ifdef SWIGGO
/* go is used for development only, no need for obsolete functions */
#define __CLKMGR_DEPRECATED(_func, _body)
#define __CLKMGR_DEPRECATED_DEC(_func)
#else
#define __CLKMGR_DEPRECATED(_func, _body) _func { _body; }
#define __CLKMGR_DEPRECATED_DEC(_func) _func
#endif
/* Handle multithreads support. */
#ifdef SWIG_USE_MULTITHREADS
%nothread;
/* Mark functions,
 * that are allowed to block when using multithreads.
 * The other threads would not be blocked when using these functions.
 */
#define __CLKMGR_SWIG_THREAD_START %thread
#define __CLKMGR_SWIG_THREAD_END %nothread
#endif

/* Include standatd types and SWIG macros
   From /usr/share/swig./ */
%include <stdint.i>
%include <cpointer.i>
/* From /usr/share/swig././ */
%include <std_string.i>
%include <std_vector.i>
%apply uint16_t { sessionId_t };

/* warning definitions per language */
%include "warn.i"

/* library code */
%include "pub/clockmanager.h"
%include "pub/clkmgr/subscription.h"
%include "pub/clkmgr/utility.h"
