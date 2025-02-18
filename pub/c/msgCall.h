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

#ifdef __cplusplus
extern "C" {
#endif

/** pointer to a ptpmgmt dispatcher structure callback */
typedef void (*ptpmgmt_dispatcher_callback)(void *cookie, ptpmgmt_msg msg,
    const void *tlv, const char *idStr);
/** pointer to ptpmgmt dispatcher structure noTlv callback */
typedef void (*ptpmgmt_dispatcher_noTlv_callback)(void *cookie,
    ptpmgmt_msg msg);
/** pointer to ptpmgmt dispatcher structure noTlvCallBack callback */
typedef void (*ptpmgmt_dispatcher_noTlvCallBack_callback)(void *cookie,
    ptpmgmt_msg msg, const char *idStr);

/**
 * Allocate new empty dispatcher object
 * @return new dispatcher object or null on error
 */
ptpmgmt_dispatcher ptpmgmt_dispatcher_alloc();
/**
 * Assign tlv ID with callback function
 * @param[in] d pointer to dispatcher object
 * @param[in] tlv_id TLV ID
 * @param[in] callback to use
 * @return true on success
 * @note the callback parameters must match the callback in the structure
 */
bool ptpmgmt_dispatcher_assign(ptpmgmt_dispatcher d,
    enum ptpmgmt_mng_vals_e tlv_id, ptpmgmt_dispatcher_callback callback);
/**
 * Assign noTlv callback function
 * @param[in] d pointer to dispatcher object
 * @param[in] callback to use
 * @return true on success
 * @note We call the noTlv callback in case the TLV ID is unknown
 */
bool ptpmgmt_dispatcher_assign_noTlv(ptpmgmt_dispatcher d,
    ptpmgmt_dispatcher_noTlv_callback callback);
/**
 * Assign noTlvCallBack callback function
 * @param[in] d pointer to dispatcher object
 * @param[in] callback to use
 * @return true on success
 * @note We call the noTlvCallBack callback in case
 *       we do not have a callback to a TLV ID
 */
bool ptpmgmt_dispatcher_assign_noTlvCallBack(ptpmgmt_dispatcher d,
    ptpmgmt_dispatcher_noTlvCallBack_callback callback);
/**
 * Call handler based on Message last received message
 * @param[in] cookie user cookie passed to user callback
 * @param[in] d pointer to dispatcher object
 * @param[in] msg pointer to message object wrapper
 * @note Do not forget to null unused callbacks
 */
void ptpmgmt_callHadler(void *cookie, const_ptpmgmt_dispatcher d,
    ptpmgmt_msg msg);
/**
 * Call handler based on supplied TLV
 * @param[in] cookie user cookie passed to user callback
 * @param[in] d pointer to dispatcher object
 * @param[in] msg pointer to message object wrapper
 * @param[in] tlv_id TLV ID
 * @param[in] tlv pointer to a TLV of TLV ID
 * @note caller @b MUST @/b ensure the TLV ID match the actual TLV!
 * @note Do not forget to null unused callbacks
 */
void ptpmgmt_callHadler_tlv(void *cookie, const_ptpmgmt_dispatcher d,
    ptpmgmt_msg msg, enum ptpmgmt_mng_vals_e tlv_id, const void *tlv);

#ifdef __cplusplus
}
#endif

#endif /* __PTPMGMT_C_MSG_CALL_H */
