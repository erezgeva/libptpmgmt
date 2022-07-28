dnl SPDX-License-Identifier: LGPL-3.0-or-later
dnl SPDX-FileCopyrightText: Copyright 2022 Erez Geva */
dnl
dnl @file
dnl @brief Version definitions for compilation
dnl
dnl @author Erez Geva <ErezGeva2@@gmail.com>
dnl @copyright 2022 Erez Geva
dnl
dnl Create verDef.h for the compilation version of the library
dnl
/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief Version definitions for compilation
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 * This header is generated automatically.
 * @note These macroes are provided for compilation only
 *       scripts should use version functions.
 *
 */

#ifndef __PTPMGMT_VER_DEFS_H
#define __PTPMGMT_VER_DEFS_H

define(V,`#define LIBPTPMGMT_VER$1 $2 /**< Library version $3 */')dnl
define(V2, `V(`_$1', (VER_$1), `$2')')dnl
V2(MAJ, major)
V2(MIN, minor)
V2(VAL, value)
V(`', "VER_STR", `string')

#endif /* __PTPMGMT_VER_DEFS_H */
