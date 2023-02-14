dnl SPDX-License-Identifier: LGPL-3.0-or-later
dnl SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */
dnl
dnl @file
dnl @brief List of management IDs
dnl
dnl @author Erez Geva <ErezGeva2@@gmail.com>
dnl @copyright © 2022 Erez Geva
dnl
dnl Create ids.h for internall use by library
dnl
/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief List of management IDs
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 * This header is generated automatically.
 * It is design to be multiple included.
 *
 */

/*
 * Use:
 *  #define A(n, v, sc, a, sz, f) <macro text>
 *  #include "ids.h"
 */
#undef DOMAIN /* Old math.h header uses this macro */
#ifndef _ptpmCaseNA
#define _ptpmCaseNA(n)
#endif
#ifndef _ptpmCaseUF
#define _ptpmCaseUF(n)
#endif
#ifndef _ptpmCaseUFS
#define _ptpmCaseUFS(n) _ptpmCaseUF(n)
#endif
#ifndef _ptpmCaseUFB
#define _ptpmCaseUFB(n) _ptpmCaseUF(n)
#endif
#ifndef _ptpmCaseUFBS
#define _ptpmCaseUFBS(n) _ptpmCaseUFB(n)
#endif
/*
 * For functions use
 * #define _ptpmCaseXX(n)    <macro text>
 * NA   - no functions needed
 * UF   - function for parsing
 * UFS  - function for parsing having variable size
 * UFB  - function for parsing and build
 * UFBS - function for parsing and build having variable size
 */
/*
 * size: > 0  fixed size dataField
 *         0  No dataField (with NA)
 *        -2  Variable length dataField, need calculation
 */
/*Name                                 value scope  allow       size   func*/
include(ids_base.m4)dnl
#undef A
#undef _ptpmCaseNA
#undef _ptpmCaseUF
#undef _ptpmCaseUFS
#undef _ptpmCaseUFB
#undef _ptpmCaseUFBS
