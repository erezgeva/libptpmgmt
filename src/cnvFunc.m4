dnl SPDX-License-Identifier: LGPL-3.0-or-later
dnl SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */
dnl
dnl @file
dnl @brief Conversions functions
dnl
dnl @author Erez Geva <ErezGeva2@@gmail.com>
dnl @copyright © 2022 Erez Geva
dnl
dnl Create cnvFunc.h used by Doxygen
dnl Contain conversions functions per TLV created by swig
dnl
/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Conversions functions
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 * This header is generated automatically, do @b NOT change,
 * or use it in your application!
 *
 * This header is used by Doxygen only!
 *
 */

#ifndef __PTPMGMT_CONVERT_FUNCTIONS_H
#define __PTPMGMT_CONVERT_FUNCTIONS_H

#include "proc.h"
#include "sig.h"

__PTPMGMT_NAMESPACE_BEGIN

define(D,`/**
 * Convert a TLV from BaseMngTlv to $1_t structure
 * @param[in] tlv pointer to the Message dataField
 * @return pointer to $1_t
 * @note This function is available in scripts only!
 *  C++ code can simply cast.
 * @note Use Message:getData() to retrieve the tlv from the Message
 */
$1_t
*conv_$1(const BaseMngTlv *tlv);')dnl
define(A, `ifelse(regexp($6, `^UF', `0'),`0',`D($1)',`dnl')')dnl
include(ids_base.m4)dnl

define(S,`/**
 * Convert a signaling TLV from BaseSigTlv
 *  to $1_t structure
 * @param[in] tlv pointer to a TLV from a signaling message
 * @return pointer to $1_t
 * @note This function is available in scripts only!
 *  C++ code can simply cast.
 * @note Use Message:getSigMngTlv() to retrieve the tlv
 */
$1_t
*conv_$1(const BaseSigTlv *tlv);')dnl
S(ORGANIZATION_EXTENSION)
S(PATH_TRACE)
S(ALTERNATE_TIME_OFFSET_INDICATOR)
S(ENHANCED_ACCURACY_METRICS)
S(L1_SYNC)
S(PORT_COMMUNICATION_AVAILABILITY)
S(PROTOCOL_ADDRESS)
S(SLAVE_RX_SYNC_TIMING_DATA)
S(SLAVE_RX_SYNC_COMPUTED_DATA)
S(SLAVE_TX_EVENT_TIMESTAMPS)
S(CUMULATIVE_RATE_RATIO)
S(SLAVE_DELAY_TIMING_DATA_NP)

__PTPMGMT_NAMESPACE_END

#endif /* __PTPMGMT_CONVERT_FUNCTIONS_H */
