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

#ifndef __PTPMGMT_MNG_IDS_H
#define __PTPMGMT_MNG_IDS_H

#include "name.h"

__PTPMGMT_NAMESPACE_BEGIN

#undef DOMAIN /* Old math.h header uses this macro */

/**
 * @brief Management TLVs ID
 * @details
 *  Available Management TLVs for use with PTP managment messages.
 */
enum mng_vals_e {
define(A, `    $1,')dnl
include(ids_base.m4)dnl
    LAST_MNG_ID, /**< Last management ID excluded */
    SMPTE_MNG_ID, /**< SMPTE Organization Extension TLV */
    FIRST_MNG_ID = NULL_PTP_MANAGEMENT, /**< First management ID */
    NULL_MANAGEMENT = NULL_PTP_MANAGEMENT /**< old name in IEEE 1588-2008 */
};

__PTPMGMT_NAMESPACE_END

#endif /* __PTPMGMT_MNG_IDS_H */
