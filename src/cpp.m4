dnl SPDX-License-Identifier: LGPL-3.0-or-later
dnl SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com> */
dnl
dnl @file
dnl @brief macros for wrapper c
dnl
dnl @author Erez Geva <ErezGeva2@@gmail.com>
dnl @copyright © 2023 Erez Geva
dnl
define(ics, `#ifndef __PTPMGMT_$1_H
#define __PTPMGMT_$1_H')dnl
define(ice, `#endif /* __PTPMGMT_$1_H */')dnl
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
define(NM, `$1')dnl
define(ns_s, `__PTPMGMT_NAMESPACE_BEGIN

')dnl
define(ns_e, `
__PTPMGMT_NAMESPACE_END
')dnl
define(sz, `$1')dnl
define(cpp_cod, `$1
')dnl
define(c_cod, `')dnl
define(bintyp, `Binary ')dnl
define(strtyp, `std::string ')dnl
define(vec, `std::vector<$1> ')dnl
define(pmng, `std::unique_ptr<BaseMngTlv> ')dnl
define(arr, `$1 $2[$3]')dnl
define(cnst, `const $1 $2 = $3;')dnl
define(cpp_st, `#ifdef __cplusplus
')dnl
define(cpp_en, `#else /* __cplusplus */
#include "c/$1.h"
#endif /* __cplusplus */
')dnl
define(cnst_st, `const')dnl
define(idf, `    ')dnl
define(ref_s, `referance')dnl
