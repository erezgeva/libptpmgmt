%@* SPDX-License-Identifier: LGPL-3.0-or-later
 %- SPDX-FileCopyrightText: Copyright 2021 Erez Geva *%@
%_
%@** @file
 * @brief management IDs enumerator
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 * This header is generated automatically.
 *
 *%@
%_
%#ifndef __PTPMGMT_MNG_IDS_H
%#define __PTPMGMT_MNG_IDS_H
%_
%#ifndef SWIG
namespace ptpmgmt
{
%#endif
%_
%#undef DOMAIN %@* Old math.h header uses this macro *%@
%_
%@**
 * @brief Management TLVs ID
 * @details
 *%- Available Management TLVs for use with PTP managment messages.
 *%@
enum mng_vals_e {
#define A(n, v, sc, a, sz, f) %- %-  n,
#include "ids.h"
    LAST_MNG_ID, %@**< Last management ID excluded *%@
    FIRST_MNG_ID = NULL_PTP_MANAGEMENT, %@**< First management ID *%@
    NULL_MANAGEMENT = NULL_PTP_MANAGEMENT %@**< old name in IEEE 1588-2008 *%@
};
%_
%#ifndef SWIG
}; %@* namespace ptpmgmt *%@
%#endif %@* SWIG *%@
%_
%#endif %@* __PTPMGMT_MNG_IDS_H *%@
