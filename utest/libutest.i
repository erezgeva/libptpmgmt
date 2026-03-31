/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com> */

/* Utest Swig helper library file
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2026 Erez Geva
 *
 */

%module utest_help

%{
#include "libutest.h"
%}

/* Include standatd types and SWIG macros */
%include <stdint.i>
%include "libutest.h"
