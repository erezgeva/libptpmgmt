dnl SPDX-License-Identifier: LGPL-3.0-or-later
dnl SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com>
dnl
dnl @file
dnl @brief Allocation and free TLV functions for builder class
dnl
dnl @author Erez Geva <ErezGeva2@@gmail.com>
dnl @copyright © 2024 Erez Geva
dnl
dnl Create allocTlv.i
dnl
/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Allocation and free TLV functions for builder class
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 */

%{
BaseMngTlv *allocTlv(mng_vals_e id) {
  switch(id) {
define(D,`case $1: return new $1_t;')dnl
define(A, `ifelse(regexp($6, `^UF', `0'),`0',`D($1)',`dnl')')dnl
include(ids_base.m4)dnl
    default:
      break;
  }
  return nullptr;
}
void freeTlv(BaseMngTlv *tlv) {
  if (tlv != nullptr)
    delete tlv;
}
%}

BaseMngTlv *allocTlv(mng_vals_e);
void freeTlv(BaseMngTlv *tlv);
