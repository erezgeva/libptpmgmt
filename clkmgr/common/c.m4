dnl SPDX-License-Identifier: BSD-3-Clause
dnl SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */
dnl
dnl @file
dnl @brief macros for wrapper c
dnl
dnl @author Erez Geva <ErezGeva2@@gmail.com>
dnl @copyright © 2024 Intel Corporation.
dnl
define(ics, `#ifndef CLKMGR_C_$1_H
#define CLKMGR_C_$1_H')dnl
define(ice, `#endif /* CLKMGR_C_$1_H */')dnl
define(incpp, `')dnl
define(inc, `#include $1
')dnl
define(incb, `#include "c/$1.h"')dnl
define(inc_c, `#include <$1.h>')dnl
define(enm, `enum clkmgr_$1')dnl
define(enmc, `enum clkmgr_$1')dnl
define(strc, `struct clkmgr_$1')dnl
define(strcc, `struct clkmgr_$1')dnl
define(nm, `clkmgr_$1')dnl
define(Nm, `Clkmgr_$1')dnl
define(NM, `CLKMGR_$1')dnl
define(ns_s, `#ifdef __cplusplus
extern "C" {
#endif')dnl
define(ns_e, `#ifdef __cplusplus
}
#endif')dnl
define(sz, `')dnl
define(cpp_cod, `')dnl
define(c_cod, `$1
')dnl
define(bintyp, `uint8_t *')dnl
define(strtyp, `const char *')dnl
define(vec, `struct clkmgr_$1 *')dnl
define(arr, `$1 $2[CLKMGR_$3]')dnl
define(cnst, `#define CLKMGR_$2 ($3)')dnl
define(cpp_st, `')dnl
define(cpp_en, `')dnl
define(cnst_st, `static const')dnl
define(idf, `')dnl
define(ref_s, `pointer')dnl
