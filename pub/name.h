/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Define name space
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 * Define library name space
 */

#ifndef __PTPMGMT_NAME_H
#define __PTPMGMT_NAME_H

/** @cond internal */

#ifdef __cplusplus
/* system headers used by all modules */
#include <string>
#include <cstdint>
#include <cstring>
#include <cstdio>

#ifndef __PTPMGMT_NAMESPACE_BEGIN
/** Define start of namespace block */
#define __PTPMGMT_NAMESPACE_BEGIN namespace ptpmgmt {
/** Define end of namespace block */
#define __PTPMGMT_NAMESPACE_END };
#endif /* __PTPMGMT_NAMESPACE_BEGIN */
#if __cplusplus >= 201309L /* C++14 */
/** Mark obsolete functions */
#define __PTPMGMT_DEPRECATED(func) [[deprecated]] func
#endif

#else /* __cplusplus */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define __PTPMGMT_NAMESPACE_BEGIN
#define __PTPMGMT_NAMESPACE_END

#endif /* __cplusplus */

#ifndef __PTPMGMT_DEPRECATED
#ifdef __GNUC__
/** Mark obsolete functions */
#define __PTPMGMT_DEPRECATED(func) func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
/** Mark obsolete functions */
#define __PTPMGMT_DEPRECATED(func) __declspec(deprecated) func
#else
/** Mark obsolete functions */
#define __PTPMGMT_DEPRECATED(func) func
#endif
#endif /* __PTPMGMT_DEPRECATED */

/**< @endcond */

#endif /* __PTPMGMT_NAME_H */
