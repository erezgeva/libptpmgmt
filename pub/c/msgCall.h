/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Messages dispatcher and builder for C
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 */

#ifndef __PTPMGMT_C_MSG_CALL_H
#define __PTPMGMT_C_MSG_CALL_H

#include "c/msg.h"

#ifdef __cplusplus
extern "C" {
#endif

/** pointer to constant ptpmgmt dispatcher structure */
typedef const struct ptpmgmt_dispatcher_t *const_ptpmgmt_dispatcher;
/** pointer to ptpmgmt dispatcher structure */
typedef struct ptpmgmt_dispatcher_t *ptpmgmt_dispatcher;
/** pointer to a ptpmgmt dispatcher structure callback
 * The real callback TLV type should:
 * @code{.cpp} const struct ptpmgmt_<TLV ID name>_t *tlv @endcode
 */
typedef void (*ptpmgmt_dispatcher_callback)(void *cookie, ptpmgmt_msg msg,
    const void *tlv, const char *idStr);
/** pointer to ptpmgmt dispatcher structure noTlv callback */
typedef void (*ptpmgmt_dispatcher_noTlv_callback)(void *cookie,
    ptpmgmt_msg msg);
/** pointer to ptpmgmt dispatcher structure noTlvCallBack callback */
typedef void (*ptpmgmt_dispatcher_noTlvCallBack_callback)(void *cookie,
    ptpmgmt_msg msg, const char *idStr);

/**< @cond internal */
struct ptpmgmt_dispatcher_cb_t; /**< hold the callbacks to the tlvs */
/**< @endcond */

/**
 * @brief Dispacher for management TLV
 * @details
 *  Call-backs used by the callHadler and callHadler_tlv.
 * @note the actual callbacks are assing in the library
 *       Only free() should be called directly
 */
struct ptpmgmt_dispatcher_t {
    /**
     * Free this dispacher object
     * @param[in, out] dispacher object to free
     */
    void (*free)(ptpmgmt_dispatcher dispacher);
    /**
     * Assign tlv ID with callback function
     * @param[in] dispacher pointer to dispatcher object
     * @param[in] tlv_id TLV ID
     * @param[in] callback to use
     * @return true on success
     * @note the callback parameters must match the callback in the structure
     */
    bool (*assign)(ptpmgmt_dispatcher dispacher, enum ptpmgmt_mng_vals_e tlv_id,
        ptpmgmt_dispatcher_callback callback);
    /**
     * Assign noTlv callback function
     * @param[in] dispacher pointer to dispatcher object
     * @param[in] callback to use
     * @return true on success
     * @note We call the noTlv callback in case the TLV ID is unknown
     */
    bool (*assign_noTlv)(ptpmgmt_dispatcher dispacher,
        ptpmgmt_dispatcher_noTlv_callback callback);
    /**
     * Assign noTlvCallBack callback function
     * @param[in] dispacher pointer to dispatcher object
     * @param[in] callback to use
     * @return true on success
     * @note We call the noTlvCallBack callback in case
     *       we do not have a callback to a TLV ID
     */
    bool (*assign_noTlvCallBack)(ptpmgmt_dispatcher dispacher,
        ptpmgmt_dispatcher_noTlvCallBack_callback callback);
    /**
     * Call handler based on Message last received message
     * @param[in] dispacher pointer to dispatcher object
     * @param[in] cookie user cookie passed to user callback
     * @param[in] msg pointer to message object wrapper
     * @note Do not forget to null unused callbacks
     */
    void (*callHadler)(const_ptpmgmt_dispatcher dispacher, void *cookie,
        ptpmgmt_msg msg);
    /**
     * Call handler based on supplied TLV
     * @param[in] dispacher pointer to dispatcher object
     * @param[in] cookie user cookie passed to user callback
     * @param[in] msg pointer to message object wrapper
     * @param[in] tlv_id TLV ID
     * @param[in] tlv pointer to a TLV of TLV ID
     * @note caller @b MUST @/b ensure the TLV ID match the actual TLV!
     * @note Do not forget to null unused callbacks
     */
    void (*callHadler_tlv)(const_ptpmgmt_dispatcher dispacher, void *cookie,
        ptpmgmt_msg msg, enum ptpmgmt_mng_vals_e tlv_id, const void *tlv);
    /**
     * Handler called if there is no TLV data
     * It could be an empty TLV or unkown
     * @param[in, out] cookie pointer to a user cookie
     * @param[in, out] msg pointer to the Message object
     */
    void (*noTlv)(void *cookie, ptpmgmt_msg msg);
    /**
     * Handler called if TLV does not have a callback.
     * @param[in, out] cookie pointer to a user cookie
     * @param[in, out] msg pointer to the Message object
     * @param[in] idStr string of the tlv_id
     */
    void (*noTlvCallBack)(void *cookie, ptpmgmt_msg msg, const char *idStr);
    /**< @cond internal */
    struct ptpmgmt_dispatcher_cb_t *cbs; /**< hold the callbacks to the tlvs */
    /**< @endcond */
};

/**
 * Allocate new empty dispatcher object
 * @return new dispatcher object or null on error
 */
ptpmgmt_dispatcher ptpmgmt_dispatcher_alloc();

/** pointer to constant ptpmgmt builder structure */
typedef const struct ptpmgmt_builder_t *const_ptpmgmt_builder;
/** pointer to ptpmgmt builder structure */
typedef struct ptpmgmt_builder_t *ptpmgmt_builder;
/** pointer to a ptpmgmt builder structure callback
 * The real callback TLV type should:
 * @code{.cpp} struct ptpmgmt_<TLV ID name>_t *tlv @endcode
 * The tlv is allocated with the tlv memory object.
 * callback can use the tlv memory object for further memory allocation
 * All memory allocation done with the tlv memory object, will be free
 * when releasing the tlv memory object.
 */
typedef bool (*ptpmgmt_builder_callback)(void *cookie, ptpmgmt_msg msg,
    void *tlv, ptpmgmt_tlv_mem tlv_mem);

/**< @cond internal */
struct ptpmgmt_builder_cb_t; /**< hold the callbacks to the tlvs */
/**< @endcond */

/**
 * @brief Builder for a management TLV
 * @details
 *  Call-backs used by the callHadler and callHadler_tlv.
 * @note the actual callbacks are assing in the library
 *       Only free() should be called directly
 */
struct ptpmgmt_builder_t {
    ptpmgmt_msg msg; /**< The Message wrapper onject */
    ptpmgmt_tlv_mem tlv_mem; /**< The TLV memory object */
    /**
     * Free this builder object
     * @param[in, out] builder object to free
     */
    void (*free)(ptpmgmt_builder builder);
    /**
     * Get pointer to the Message object
     * @param[in] builder object
     * @return pointer the Message object
     */
    ptpmgmt_msg(*getMsg)(const_ptpmgmt_builder builder);
    /**
     * Get pointer to the TLV memory object
     * @param[in] builder object
     * @return pointer the TLV memory object
     */
    ptpmgmt_tlv_mem(*getTlvMem)(const_ptpmgmt_builder builder);
    /**
     * Clear the tlv in the Message object
     * @param[in, out] builder object to clean
     */
    void (*clear)(ptpmgmt_builder builder);
    /**
     * Assign tlv ID with callback function
     * @param[in] builder pointer to builder object
     * @param[in] tlv_id TLV ID
     * @param[in] callback to use
     * @return true on success
     * @note the callback parameters must match the callback in the structure
     */
    bool (*assign)(ptpmgmt_builder builder, enum ptpmgmt_mng_vals_e tlv_id,
        ptpmgmt_builder_callback callback);
    /**
     * Allocate a management TLV, use a call-back to set its values and
     *  assign it with the Message object ready for sending
     * @param[in, out] builder object
     * @param[in] cookie from user, that pass to callback
     * @param[in] actionField action type
     * @param[in] tlv_id TLV ID to send
     * @return true if setAction() succes
     * @note This function calls Message.setAction
     * @note For empty TLV or GET action,
     *       the function will call setAction without TLV
     */
    bool (*buildTlv)(ptpmgmt_builder builder, void *cookie,
        enum ptpmgmt_actionField_e actionField, enum ptpmgmt_mng_vals_e tlv_id);
    /**< @cond internal */
    struct ptpmgmt_builder_cb_t *cbs; /**< hold the callbacks to the tlvs */
    /**< @endcond */
};

/**
 * Allocate new empty builder object
 * @param[in] msg pointer to the Message object
 * @return new builder object or null on error
 */
ptpmgmt_builder ptpmgmt_builder_alloc(ptpmgmt_msg msg);

#ifdef __cplusplus
}
#endif

#endif /* __PTPMGMT_C_MSG_CALL_H */
