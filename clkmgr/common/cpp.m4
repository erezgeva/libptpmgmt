dnl SPDX-License-Identifier: BSD-3-Clause
dnl SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */
dnl
dnl @file
dnl @brief macros for wrapper c
dnl
dnl @author Erez Geva <ErezGeva2@@gmail.com>
dnl @copyright © 2024 Intel Corporation.
dnl
define(ics, `#ifndef CLKMGR_$1_H
#define CLKMGR_$1_H')dnl
define(ice, `#endif /* CLKMGR_$1_H */')dnl
define(incpp, `#include $1
')dnl
define(inc, `')dnl
define(incb, `#include "$1.h"')dnl
define(inc_c, `#include <c$1>')dnl
define(enm, `enum $1')dnl
define(enmc, `$1')dnl
define(strc, `struct $1')dnl
define(strcc, `$1')dnl
define(nm, `$1')dnl
define(Nm, `$1')dnl
define(NM, `$1')dnl
define(ns_s, `__CLKMGR_NAMESPACE_BEGIN')dnl
define(ns_e, `__CLKMGR_NAMESPACE_END')dnl
define(sz, `$1')dnl
define(cpp_cod, `$1
')dnl
define(c_cod, `')dnl
define(bintyp, `Binary ')dnl
define(strtyp, `std::string ')dnl
define(vec, `std::vector<$1> ')dnl
define(arr, `$1 $2[$3]')dnl
define(cnst, `constexpr $1 $2 = $3;')dnl
define(cpp_st, `#ifdef __cplusplus
')dnl
define(cpp_en, `#else /* __cplusplus */
#include "c/$1.h"
#endif /* __cplusplus */
')dnl
define(cnst_st, `const')dnl
define(idf, `    ')dnl
define(ref_s, `referance')dnl
