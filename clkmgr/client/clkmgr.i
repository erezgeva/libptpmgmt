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
    #include "clockmanager.h"
    using namespace clkmgr;
%}

/* prevent C++ namespace in swig */
#define __CLKMGR_NAMESPACE_USE
#define __CLKMGR_NAMESPACE_BEGIN
#define __CLKMGR_NAMESPACE_END
/* Handle obsolete functions */
#ifdef SWIGGO
/* go is used for development only, no need for obsolete functions */
/*
#define __PTPMGMT_DEPRECATED(_func, _body)
#define __PTPMGMT_DEPRECATED_DEC(_func)
#else
#define __PTPMGMT_DEPRECATED(_func, _body) _func { _body; }
#define __PTPMGMT_DEPRECATED_DEC(_func) _func
*/
#endif
/* Handle multithreads support. */
#ifdef SWIG_USE_MULTITHREADS
%nothread;
/* Mark functions,
 * that are allowed to block when using multithreads.
 * The other threads would not be blocked when using these functions.
 */
#define __PTPMGMT_SWIG_THREAD_START %thread
#define __PTPMGMT_SWIG_THREAD_END %nothread
#endif

/* Include standatd types and SWIG macroes
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
%include "clockmanager.h"
%include "clkmgr/client_state.h"
%include "clkmgr/event_state.h"
%include "clkmgr/subscription.h"
%include "clkmgr/utility.h"
