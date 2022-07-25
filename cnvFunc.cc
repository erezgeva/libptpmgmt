%@* SPDX-License-Identifier: LGPL-3.0-or-later
 %- SPDX-FileCopyrightText: Copyright 2022 Erez Geva *%@
%_
%@** @file
 * @brief Conversions functions
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2022 Erez Geva
 *
 * This header is generated automatically, do @b NOT change,
 *  or use it in your application!
 *
 * This header is used by Doxygen only!
 *
 *%@
%_
%#ifndef __PTPMGMT_CONVERT_FUNCTIONS_H
%#define __PTPMGMT_CONVERT_FUNCTIONS_H
%_
%#include "proc.h"
%#include "sig.h"
%_
__PTPMGMT_NAMESPACE_BEGIN
%_
#define _ptpmCaseUF(n) \
%@**%^\
 * Convert a TLV from BaseMngTlv to n##_t structure%^\
 * @param[in] tlv pointer to the Message dataField%^\
 * @return pointer to n##_t%^\
 * @note This function is available in scripts only!%^\
 *%- C++ code can simply cast.%^\
 * @note Use Message:getData() to retrieve the tlv from the Message%^\
 *%@%^\
n##_t%^*conv_##n(const BaseMngTlv *tlv);
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
#include "ids.h"
%_
#define S(n) \
%@**%^\
 * Convert a signaling TLV from BaseSigTlv%^\
 *%- to n##_t structure%^\
 * @param[in] tlv pointer to a TLV from a signaling message%^\
 * @return pointer to n##_t%^\
 * @note This function is available in scripts only!%^\
 *%- C++ code can simply cast.%^\
 * @note Use Message:getSigMngTlv() to retrieve the tlv%^\
 *%@%^\
n##_t%^\
*conv_##n(const BaseSigTlv *tlv);
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
%_
__PTPMGMT_NAMESPACE_END
%_
%#endif %@* __PTPMGMT_CONVERT_FUNCTIONS_H *%@
