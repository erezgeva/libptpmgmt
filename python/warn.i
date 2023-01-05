/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2022 Erez Geva */

/* Swig warning suppress for Python
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2022 Erez Geva
 *
 */

/* internal structure */
%warnfilter(SWIGWARN_PARSE_BUILTIN_NAME) Message::sigTlv::set;

/* See more warnings in libptpmgmt.i for these SWIG_OPERS_xxxx */
#define SWIG_OPERS_IGNORE_OPERATOR
