/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* libpmc.i Library Swig header file
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#ifdef SWIGPERL
%module PmcLib
#else
%module pmc
#endif
%{
    #include "cfg.h"
    #include "msg.h"
    #include "ptp.h"
    #include "sock.h"
    #include "bin.h"
%}

%include "stdint.i"
%include "std_string.i"
%apply long { ssize_t };
%include "cfg.h"
%include "msg.h"
%include "ptp.h"
%include "sock.h"
%include "bin.h"
%include "proc.h"
%include "mngIds.h"
%include "cpointer.i"
/* Allow script to convert a string to a buffer
 * The script need to ensure the string length is proper */
%pointer_cast(char*, void*, conv_buf);
#define caseUF(n) %pointer_cast(baseData*, n##_t*, conv_##n);
#define A(n, v, sc, a, sz, f) case##f(n)
%include "ids.h"
