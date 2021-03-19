/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* libpmc.i Library Swig header file
 *
 * Authors: Erez Geva <ErezGeva2@gmail.com>
 *
 */

%module PmcLib
%{
    #include "cfg.h"
    #include "msg.h"
    #include "ptp.h"
    #include "sock.h"
    #include "buf.h"
%}

%include "stdint.i"
%include "std_string.i"
%include "cfg.h"
%include "msg.h"
%include "ptp.h"
%include "sock.h"
%include "buf.h"
%include "proc.h"
#define IDS_START enum mng_vals_e {
#define A(n, v, sc, a, sz, f) n,
#define IDS_END };
%include "ids.h"
%include "cpointer.i"
#define caseUF(n) %pointer_cast(baseData*, n##_t*, conv_##n);
#define A(n, v, sc, a, sz, f) case##f(n)
/* We need to same file again, so use link */
%include "ids2.h"
