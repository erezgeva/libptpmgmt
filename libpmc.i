/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* libpmc.i Library Swig header file
 *
 * @author Erez Geva <ErezGeva2@gmail.com>
 * @copyright 2021 Erez Geva
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
%include "mngIds.h"
%include "cpointer.i"
#define caseUF(n) %pointer_cast(baseData*, n##_t*, conv_##n);
#define A(n, v, sc, a, sz, f) case##f(n)
%include "ids.h"
