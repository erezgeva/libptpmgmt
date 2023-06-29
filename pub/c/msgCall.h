/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Messages dispatcher for C
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 */

#ifndef __PTPMGMT_C_MSG_CALL_H
#define __PTPMGMT_C_MSG_CALL_H

#include "c/callDef.h"

/**
 * Call handler based on Message last received message
 * @param[in] cookie user cookie passed to user callback
 * @param[in] d pointer to structure with the callbacks
 * @param[in] msg pointer to message object wrapper
 * @note Do not forget to null unused callbacks
 */
void ptpmgmt_callHadler(void *cookie, const_ptpmgmt_dispatcher d,
    ptpmgmt_msg msg);
/**
 * Call handler based on supplied TLV
 * @param[in] cookie user cookie passed to user callback
 * @param[in] d pointer to structure with the callbacks
 * @param[in] msg pointer to message object wrapper
 * @param[in] tlv_id TLV ID
 * @param[in] tlv pointer to a TLV of TLV ID
 * @note caller @b MUST @/b ensure the TLV ID match the actual TLV!
 * @note Do not forget to null unused callbacks
 */
void ptpmgmt_callHadler_tlv(void *cookie, const_ptpmgmt_dispatcher d,
    ptpmgmt_msg msg, enum ptpmgmt_mng_vals_e tlv_id, const void *tlv);

#endif /* __PTPMGMT_C_MSG_CALL_H */
