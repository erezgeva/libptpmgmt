/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief Convert time constant
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#ifndef __PTPMGMT_TIME_COMVERT_H
#define __PTPMGMT_TIME_COMVERT_H

#include "name.h"

__PTPMGMT_NAMESPACE_BEGIN

/** Number of nanoseconds in a microsecond */
const int32_t NSEC_PER_USEC = 1000;
/** Number of nanoseconds in a millisecond */
const int32_t NSEC_PER_MSEC = 1000000;
/** Number of nanoseconds in a second */
const int32_t NSEC_PER_SEC = 1000000000;
/** Number of microsecond in a millisecond */
const int32_t USEC_PER_MSEC = 1000;
/** Number of microsecond in a second */
const int32_t USEC_PER_SEC = 1000000;
/** Number of millisecond in a second */
const int32_t MSEC_PER_SEC = 1000;

__PTPMGMT_NAMESPACE_END

#endif /* __PTPMGMT_TIME_COMVERT_H */
