dnl SPDX-License-Identifier: LGPL-3.0-or-later
dnl SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com> */
dnl
dnl @file
dnl @brief macros for wrapper c
dnl
dnl @author Erez Geva <ErezGeva2@@gmail.com>
dnl @copyright © 2023 Erez Geva
dnl
define(ics, `#ifndef __PTPMGMT_C_$1_H
#define __PTPMGMT_C_$1_H')dnl
define(ice, `#endif /* __PTPMGMT_C_$1_H */')dnl
define(incpp, `')dnl
define(inc, `#include $1
')dnl
define(incb, `#include "c/$1.h"')dnl
define(inc_c, `#include <$1.h>')dnl
define(enm, `enum ptpmgmt_$1')dnl
define(enmc, `enum ptpmgmt_$1')dnl
define(strc, `struct ptpmgmt_$1')dnl
define(strcc, `struct ptpmgmt_$1')dnl
define(nm, `ptpmgmt_$1')dnl
define(NM, `PTPMGMT_$1')dnl
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
define(vec, `struct ptpmgmt_$1 *')dnl
define(pmng, `const void *')dnl
define(arr, `$1 $2[LIBPTPMGMT_$3]')dnl
define(cnst, `#define LIBPTPMGMT_$2 ($3)')dnl
define(cpp_st, `')dnl
define(cpp_en, `')dnl
define(cnst_st, `static const')dnl
define(idf, `')dnl
define(ref_s, `pointer')dnl
