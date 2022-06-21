/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief convert time constant
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#ifndef __PTPMGMT_COMPILATION_H
#define __PTPMGMT_COMPILATION_H

#define caseItem(a) a: return #a
#define caseItemOff(a) a: return #a + off

#define stringify(s) #s
/* Need 2 levels to stringify macros value instead of macro name */
#define stringifyVal(a) stringify(a)

#define DO_PRAGMA(x) _Pragma (#x)

#ifdef __GNUC__
/* See:
 * GNU GCC
 * gcc.gnu.org/onlinedocs/gcc-4.0.0/gcc/Type-Attributes.html
 * Keil GNU mode
 * www.keil.com/support/man/docs/armcc/armcc_chr1359125007083.htm
 * www.keil.com/support/man/docs/armclang_ref/armclang_ref_chr1393328521340.htm
 */
#define PACK(__definition__) __definition__ __attribute__((packed))
#define PURE __attribute__((pure))
#define MAYBE_UNUSED __attribute__((unused))
#define PRINT_FORMAT(a, b) __attribute__((format(printf,a,b)))
#define DIAG_START  DO_PRAGMA(GCC diagnostic push)
#define DIAG_IGNORE(warn) DO_PRAGMA(GCC diagnostic ignored warn)
#define DIAG_END   DO_PRAGMA(GCC diagnostic pop)
#if __GNUC__ > 6
#define FALLTHROUGH __attribute__((fallthrough))
#endif
#define HAVE_FUNC_COMPARE
#elif defined _MSC_VER
/* See:
 * http://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
 * For MSVC:
 * http://docs.microsoft.com/en-us/cpp/preprocessor/pack
 */
#define PACK(__definition__) __pragma(pack(push, 1)) \
    __definition__ __pragma(pack(pop))
#define PURE
#define MAYBE_UNUSED
#define PRINT_FORMAT(a, b)
#define DIAG_START
#define DIAG_IGNORE(warn)
#define DIAG_END
#else
#error Unknown compiler
#endif

#ifndef FALLTHROUGH
#define FALLTHROUGH
#endif

#endif /* __PTPMGMT_COMPILATION_H */
