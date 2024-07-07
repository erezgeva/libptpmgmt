dnl SPDX-License-Identifier: LGPL-3.0-or-later
dnl SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */
dnl
dnl @file
dnl @brief Management IDs enumerator
dnl
dnl @author Erez Geva <ErezGeva2@@gmail.com>
dnl @copyright © 2022 Erez Geva
dnl
dnl Create mngIds.h
dnl
include(lang().m4)dnl
/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Management IDs enumerator
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 * This header is generated automatically.
 *
 */

ics(MNG_IDS)

cpp_st()dnl
#include "name.h"

ns_s()

cpp_cod(`#undef DOMAIN /* Old math.h header uses this macro */')dnl
cpp_cod(`')dnl
/**
 * @brief Management TLVs ID
 * @details
 *  Available Management TLVs for use with PTP managment messages.
 */
enm(mng_vals_e) {
define(A, `    NM($1),')dnl
include(ids_base.m4)dnl
    /**< @cond internal */
    NM(LAST_MNG_ID), /**< Last management ID excluded */
    NM(SMPTE_MNG_ID), /**< SMPTE Organization Extension TLV */
cpp_cod(`    FIRST_MNG_ID = NULL_PTP_MANAGEMENT, /**< First management ID */')dnl
    /**< @endcond */
cpp_cod(`    NULL_MANAGEMENT = NULL_PTP_MANAGEMENT /**< old name in IEEE 1588-2008 */')dnl
};

ns_e()
cpp_en(mngIds)dnl

ice(MNG_IDS)
