/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Create and parse PTP management messages
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 * Created following @"IEEE Std 1588-2008@", PTP version 2
 * with some updates from @"IEEE Std 1588-2019@"
 */

#ifndef __PTPMGMT_C_MSG_H
#define __PTPMGMT_C_MSG_H

#include "c/cfg.h"
#include "c/proc.h"
#include "c/sig.h"

/** pointer to ptpmgmt message structure */
typedef struct ptpmgmt_msg_t *ptpmgmt_msg;

/** pointer to constant ptpmgmt message structure */
typedef const struct ptpmgmt_msg_t *const_ptpmgmt_msg;

/**
 * pointer to callback traverse sig TLVs
 * @param[in] cookie pointer to a user cookie
 * @param[in] m msg object
 * @param[in] tlvType Signalling TLV type
 * @param[in] tlv Signalling TLV
 * @return true to stop traverse
 * @note user should cast the tlv to the proper Signalling TLV type structure
 */
typedef bool (*ptpmgmt_msg_sig_callback)(void *cookie, const_ptpmgmt_msg m,
    enum ptpmgmt_tlvType_e tlvType, const void *tlv);

/**
 * The ptpmgmt message structure hold the message object
 *  and call backs to call C++ methods
 */
struct ptpmgmt_msg_t {
    /**< @cond internal */
    void *_this; /**< pointer to actual C++ initialization object */
    struct ptpmgmt_PortIdentity_t _peer;
    struct ptpmgmt_PortIdentity_t _target;
    struct ptpmgmt_MsgParams _prms;
    /** The last user dataSend ysed in calling setAction */
    const void *dataSend;
    void *sendTlv; /**< The BaseMngTlv object used to send on C++ */
    void *data; /**< Received managment TLV converted to C */
    void *dataTbl; /**< Received managment TLV table converted to C */
    void *dataSig1; /**< Last signalling converted to C part 1 */
    void *dataSig2; /**< Last signalling converted to C part 2 */
    void *dataSig3; /**< Last signalling converted to C part 3 */
    /**< @endcond */

    /**
     * Free msg object
     * @param[in, out] m msg object
     */
    void (*free)(ptpmgmt_msg m);
    /**
     * Get the current msgparams parameters
     * @param[in, out] m msg object
     * @return msgparams parameters
     */
    ptpmgmt_pMsgParams(*getParams)(ptpmgmt_msg m);
    /**
     * Set and use a user MsgParams parameters
     * @param[in, out] m msg object
     * @param[in] prms MsgParams parameters
     * @return true if parameters are valid and updated
     */
    bool (*updateParams)(ptpmgmt_msg m, ptpmgmt_cpMsgParams prms);
    /**
     * Get the current parsed TLV id
     * @param[in] m msg object
     * @return current parsed TLV id
     */
    enum ptpmgmt_mng_vals_e(*getTlvId)(const_ptpmgmt_msg m);
    /**
     * Get the current build TLV id
     * @param[in] m msg object
     * @return current TLV id
     */
    enum ptpmgmt_mng_vals_e(*getBuildTlvId)(const_ptpmgmt_msg m);
    /**
     * Set target clock ID to use all clocks.
     * @param[in] m msg object
     */
    void (*setAllClocks)(const_ptpmgmt_msg m);
    /**
     * Query if target clock ID is using all clocks.
     * @param[in] m msg object
     * @return true if target use all clocks
     */
    bool (*isAllClocks)(const_ptpmgmt_msg m);
    /**
     * Fetch MsgParams parameters from configuration file
     * @param[in, out] m msg object
     * @param[in] cfg reference to configuration file object
     * @param[in] section in configuration file
     * @return true on success
     * @note calling with null section will fetch value from @"global@" section
     */
    bool (*useConfig)(ptpmgmt_msg m, const_ptpmgmt_cfg cfg, const char *section);
    /**
     * Convert parse error code to string
     * @param[in] err parse code
     * @return string with the error message
     */
    const char *(*err2str)(enum ptpmgmt_MNG_PARSE_ERROR_e err);
    /**
     * Convert message type to string
     * @param[in] type
     * @return string with the TLV type
     */
    const char *(*type2str)(enum ptpmgmt_msgType_e type);
    /**
     * Convert TLV type to string
     * @param[in] type
     * @return string with the TLV type
     */
    const char *(*tlv2str)(enum ptpmgmt_tlvType_e type);
    /**
     * Convert action to string
     * @param[in] action
     * @return string with the action in a string
     */
    const char *(*act2str)(enum ptpmgmt_actionField_e action);
    /**
     * Convert management id to string
     * @param[in] id parse code
     * @return string with ID name
     */
    const char *(*mng2str)(enum ptpmgmt_mng_vals_e id);
    /**
     * Convert string to management id
     * @param[in] str string to search
     * @param[out] id parse code
     * @param[in] exact perform an exact match
     * @return true if found
     */
    bool (*findMngID)(const char *str, enum ptpmgmt_mng_vals_e *id, bool exact);
    /**
     * Convert management error to string
     * @param[in] err ID
     * @return string with the error message
     */
    const char *(*errId2str)(enum ptpmgmt_managementErrorId_e err);
    /**
     * Convert clock type to string
     * @param[in] type clock type
     * @return string with the clock type
     */
    const char *(*clkType2str)(enum ptpmgmt_clockType_e type);
    /**
     * Convert network protocol to string
     * @param[in] protocol network protocol
     * @return string with the network protocol
     */
    const char *(*netProt2str)(enum ptpmgmt_networkProtocol_e protocol);
    /**
     * Convert clock accuracy to string
     * @param[in] value clock accuracy
     * @return string with the clock accuracy
     */
    const char *(*clockAcc2str)(enum ptpmgmt_clockAccuracy_e value);
    /**
     * Convert fault record severity code to string
     * @param[in] code severity code
     * @return string with the severity code
     */
    const char *(*faultRec2str)(enum ptpmgmt_faultRecord_e code);
    /**
     * Convert time source to string
     * @param[in] type time source
     * @return string with the time source
     */
    const char *(*timeSrc2str)(enum ptpmgmt_timeSource_e type);
    /**
     * Convert string to time source type
     * @param[in] str string to search
     * @param[out] type time source
     * @param[in] exact perform an exact match
     * @return true if found
     */
    bool (*findTimeSrc)(const char *str, enum ptpmgmt_timeSource_e *type,
        bool exact);
    /**
     * Convert port state to string
     * @param[in] state port state
     * @return string with the port state
     */
    const char *(*portState2str)(enum ptpmgmt_portState_e state);
    /**
     * Convert string to port state
     * @param[in] str string to search
     * @param[out] state port state
     * @param[in] caseSens perform case sensetive match
     * @return true if found
     */
    bool (*findPortState)(const char *str, enum ptpmgmt_portState_e *state,
        bool caseSens);
    /**
     * Convert delay mechanism to string
     * @param[in] type delay mechanism
     * @return string with a delay mechanism type
     */
    const char *(*delayMech2str)(enum ptpmgmt_delayMechanism_e type);
    /**
     * Convert string to delay mechanism
     * @param[in] str string to search
     * @param[out] type delay mechanism
     * @param[in] exact perform an exact match
     * @return true if found
     */
    bool (*findDelayMech)(const char *str, enum ptpmgmt_delayMechanism_e *type,
        bool exact);
    /**
     * Convert SMPTE clock locking state to string
     * @param[in] state SMPTE clock locking state
     * @return string with the SMPTE clock locking state
     */
    const char *(*smpteLck2str)(enum ptpmgmt_SMPTEmasterLockingStatus_e state);
    /**
     * Convert linuxptp time stamp type to string
     * @param[in] type time stamp type
     * @return string with the Linux time stamp type
     */
    const char *(*ts2str)(enum ptpmgmt_linuxptpTimeStamp_e type);
    /**
     * Convert linuxptp power profile version to string
     * @param[in] ver version
     * @return string with the Linux power profile version
     */
    const char *(*pwr2str)(enum ptpmgmt_linuxptpPowerProfileVersion_e ver);
    /**
     * Convert linuxptp master unicasy state to string
     * @param[in] state
     * @return string with the master state in the unicast master table
     */
    const char *(*us2str)(enum ptpmgmt_linuxptpUnicastState_e state);
    /**
     * Check if leap 61 seconds flag is enabled
     * @param[in] flags
     * @return true if flag on
     */
    bool (*is_LI_61)(uint8_t flags);
    /**
     * Check if leap 59 seconds flag is enabled
     * @param[in] flags
     * @return true if flag on
     */
    bool (*is_LI_59)(uint8_t flags);
    /**
     * Check if UTC offset is valid flag is enabled
     * @param[in] flags
     * @return true if flag on
     */
    bool (*is_UTCV)(uint8_t flags);
    /**
     * Check if is PTP instance flag is enabled
     * @param[in] flags
     * @return true if flag on
     */
    bool (*is_PTP)(uint8_t flags);
    /**
     * Check if timescale is traceable flag is enabled
     * @param[in] flags
     * @return true if flag on
     */
    bool (*is_TTRA)(uint8_t flags);
    /**
     * Check if frequency is traceable flag is enabled
     * @param[in] flags
     * @return true if flag on
     */
    bool (*is_FTRA)(uint8_t flags);
    /**
     * Check management TLV id uses empty dataField
     * @param[in] id management TLV id
     * @return true if dataField is empty
     */
    bool (*isEmpty)(enum ptpmgmt_mng_vals_e id);
    /**
     * Check if management TLV is valid for use
     * @param[in] m msg object
     * @param[in] id management TLV id
     * @return true if management TLV is valid
     * @note function also check implement specific TLVs!
     */
    bool (*isValidId)(const_ptpmgmt_msg m, enum ptpmgmt_mng_vals_e id);
    /**
     * Set message object management TLV id, action and data for dataField
     * @param[in, out] m msg object
     * @param[in] actionField for sending
     * @param[in] tlv_id management TLV id
     * @param[in] dataSend pointer to TLV object
     * @return true if setting is correct
     * @note the setting is valid for send only
     * @attention
     *  The caller must use the proper structure with the TLV id!
     *  Mismatch will probably cause a crash to your application.
     *  The library does @b NOT perform any error catchig of any kind!
     */
    bool (*setAction)(ptpmgmt_msg m, enum ptpmgmt_actionField_e actionField,
        enum ptpmgmt_mng_vals_e tlv_id, const void *dataSend);
    /**
     * Clear data for send, prevent accidentally use, in case it is freed
     * @param[in] m msg object
     */
    void (*clearData)(const_ptpmgmt_msg m);
    /**
     * Build a raw message for send using setAction setting
     * @param[in] m msg object
     * @param[in, out] buf memory buffer to fill with raw PTP Message
     * @param[in] bufSize buffer size
     * @param[in] sequence message sequence
     * @return parse error state
     * @note build before setting with setAction will create a GET raw message
     *  with NULL_PTP_MANAGEMENT management TLV
     * @note usually the user increases the sequence so it can be compared
     *  with replied message
     * @note if raw message is larger than buffer size the function
     *   return MNG_PARSE_ERROR_TOO_SMALL
     */
    enum ptpmgmt_MNG_PARSE_ERROR_e(*build)(const_ptpmgmt_msg m, void *buf,
        size_t bufSize, uint16_t sequence);
    /**
     * Get build management action
     * @param[in] m msg object
     * @return build management action
     */
    enum ptpmgmt_actionField_e(*getSendAction)(const_ptpmgmt_msg m);
    /**
     * Get last build message size
     * @param[in] m msg object
     * @return message size
     */
    size_t (*getMsgLen)(const_ptpmgmt_msg m);
    /**
     * Get planned message to build size
     * @param[in] m msg object
     * @return planned message size or negative for error
     * @note the planned message size is based on the management TLV id,
     *  action and the dataSend set by the user.
     * You can use the size to allocate proper buffer for sending
     */
    ssize_t (*getMsgPlanedLen)(const_ptpmgmt_msg m);
    /* Parsed message functions */
    /**
     * Parse a received raw socket
     * @param[in] m msg object
     * @param[in] buf memory buffer containing the raw PTP Message
     * @param[in] msgSize received size of PTP Message
     * @return parse error state
     */
    enum ptpmgmt_MNG_PARSE_ERROR_e(*parse)(const_ptpmgmt_msg m, const void *buf,
        ssize_t msgSize);
    /**
     * Get last reply management action
     * @param[in] m msg object
     * @return reply management action
     * @note set on parse
     */
    enum ptpmgmt_actionField_e(*getReplyAction)(const_ptpmgmt_msg m);
    /**
     * Is last parsed message a unicast or not
     * @param[in] m msg object
     * @return true if parsed message is unicast
     */
    bool (*isUnicast)(const_ptpmgmt_msg m);
    /**
     * Get last reply PTP Profile Specific
     * @param[in] m msg object
     * @return reply management action
     * @note set on parse
     */
    uint8_t (*getPTPProfileSpecific)(const_ptpmgmt_msg m);
    /**
     * Get last parsed message sequence number
     * @param[in] m msg object
     * @return parsed sequence number
     */
    uint16_t (*getSequence)(const_ptpmgmt_msg m);
    /**
     * Get last parsed message peer port ID
     * @param[in, out] m msg object
     * @return parsed message peer port ID
     */
    const struct ptpmgmt_PortIdentity_t *(*getPeer)(ptpmgmt_msg m);
    /**
     * Get last parsed message target port ID
     * @param[in, out] m msg object
     * @return parsed message target port ID
     */
    const struct ptpmgmt_PortIdentity_t *(*getTarget)(ptpmgmt_msg m);
    /**
     * Get last parsed message sdoId
     * @param[in] m msg object
     * @return parsed message sdoId
     * @note upper byte is was transportSpecific
     */
    uint32_t (*getSdoId)(const_ptpmgmt_msg m);
    /**
     * Get last parsed message domainNumber
     * @param[in] m msg object
     * @return parsed message domainNumber
     */
    uint8_t (*getDomainNumber)(const_ptpmgmt_msg m);
    /**
     * Get last parsed message PTP version
     * @param[in] m msg object
     * @return parsed message versionPTP
     */
    uint8_t (*getVersionPTP)(const_ptpmgmt_msg m);
    /**
     * Get last parsed message minor PTP version
     * @param[in] m msg object
     * @return parsed message versionPTP
     */
    uint8_t (*getMinorVersionPTP)(const_ptpmgmt_msg m);
    /**
     * Get last parsed message dataField
     * @param[in, out] m msg object
     * @return pointer to last parsed message dataField or null
     * @note You need to cast to proper structure depends on
     *  management TLV ID, get with.
     * @note You @b should not try to free or change this TLV object
     */
    const void *(*getData)(ptpmgmt_msg m);
    /**
     * Get send message dataField
     * @param[in] m msg object
     * @return pointer to send message dataField or null
     *         a pointer to the last setAction dataSend parameter.
     * @note You need to cast to proper structure depends on
     *  management TLV ID.
     * @note In case you release this memory,
     *  you should call @code clearData() @endcode
     */
    const void *(*getSendData)(const_ptpmgmt_msg m);
    /**
     * Get management error code ID
     * @param[in] m msg object
     * Relevant only when parsed message return MNG_PARSE_ERROR_MSG
     * @return error code
     */
    enum ptpmgmt_managementErrorId_e(*getErrId)(const_ptpmgmt_msg m);
    /**
     * Get management error message
     * @param[in] m msg object
     * Relevant only when parsed message return MNG_PARSE_ERROR_MSG
     * @return error message
     */
    const char *(*getErrDisplay)(const_ptpmgmt_msg m);
    /**
     * query if last message is a signalling message
     * @param[in] m msg object
     * @return true if last message is a signalling message
     */
    bool (*isLastMsgSig)(const_ptpmgmt_msg m);
    /**
     * Get message type
     * @param[in] m msg object
     * @return message type
     */
    enum ptpmgmt_msgType_e(*getType)(const_ptpmgmt_msg m);
    /**
     * Get management message type
     * @param[in] m msg object
     * @return management message type
     * @note return MANAGEMENT or MANAGEMENT_ERROR_STATUS
     */
    enum ptpmgmt_tlvType_e(*getMngType)(const_ptpmgmt_msg m);
    /**
     * Traverse all last signalling message TLVs
     * @param[in, out] m msg object
     * @param[in] cookie pointer to a user cookie
     * @param[in] callback function to call with each TLV
     * @return true if any of the calling to call-back return true
     * @note stop once a call-back return true
     */
    bool (*traversSigTlvs)(ptpmgmt_msg m, void *cookie,
        ptpmgmt_msg_sig_callback callback);
    /**
     * Get number of the last signalling message TLVs
     * @param[in] m msg object
     * @return number of TLVs or zero
     */
    size_t (*getSigTlvsCount)(const_ptpmgmt_msg m);
    /**
     * Get a TLV from the last signalling message TLVs by position
     * @param[in, out] m msg object
     * @param[in] position of TLV
     * @return TLV or null
     */
    const void *(*getSigTlv)(ptpmgmt_msg m, size_t position);
    /**
     * Get a type of TLV from the last signalling message TLVs by position
     * @param[in] m msg object
     * @param[in] position of TLV
     * @return type of TLV or unknown
     */
    enum ptpmgmt_tlvType_e(*getSigTlvType)(const_ptpmgmt_msg m, size_t position);
    /**
     * Get the management TLV ID of a management TLV
     * from the last signalling message TLVs by position
     * @param[in] m msg object
     * @param[in] position of TLV
     * @return management TLV ID or NULL_PTP_MANAGEMENT
     * @note return NULL_PTP_MANAGEMENT if TLV is not management
     */
    enum ptpmgmt_mng_vals_e(*getSigMngTlvType)(const_ptpmgmt_msg m,
        size_t position);
    /**
     * Get a management TLV from the last signalling message TLVs by position
     * @param[in, out] m msg object
     * @param[in] position of TLV
     * @return management TLV or null
     * @note return null if TLV is not management
     * @note You @b should not try to free this TLV object
     */
    const void *(*getSigMngTlv)(ptpmgmt_msg m, size_t position);
};

/**
 * Convert parse error code to string
 * @param[in] err parse code
 * @return string with the error message
 */
const char *ptpmgmt_msg_err2str(enum ptpmgmt_MNG_PARSE_ERROR_e err);
/**
 * Convert message type to string
 * @param[in] type
 * @return string with the TLV type
 */
const char *ptpmgmt_msg_type2str(enum ptpmgmt_msgType_e type);
/**
 * Convert TLV type to string
 * @param[in] type
 * @return string with the TLV type
 */
const char *ptpmgmt_msg_tlv2str(enum ptpmgmt_tlvType_e type);
/**
 * Convert action to string
 * @param[in] action
 * @return string with the action in a string
 */
const char *ptpmgmt_msg_act2str(enum ptpmgmt_actionField_e action);
/**
 * Convert management id to string
 * @param[in] id parse code
 * @return string with ID name
 */
const char *ptpmgmt_msg_mng2str(enum ptpmgmt_mng_vals_e id);
/**
 * Convert string to management id
 * @param[in] str string to search
 * @param[out] id parse code
 * @param[in] exact perform an exact match
 * @return true if found
 */
bool ptpmgmt_msg_findMngID(const char *str, enum ptpmgmt_mng_vals_e *id,
    bool exact);
/**
 * Convert management error to string
 * @param[in] err ID
 * @return string with the error message
 */
const char *ptpmgmt_msg_errId2str(enum ptpmgmt_managementErrorId_e err);
/**
 * Convert clock type to string
 * @param[in] type clock type
 * @return string with the clock type
 */
const char *ptpmgmt_msg_clkType2str(enum ptpmgmt_clockType_e type);
/**
 * Convert network protocol to string
 * @param[in] protocol network protocol
 * @return string with the network protocol
 */
const char *ptpmgmt_msg_netProt2str(enum ptpmgmt_networkProtocol_e protocol);
/**
 * Convert clock accuracy to string
 * @param[in] value clock accuracy
 * @return string with the clock accuracy
 */
const char *ptpmgmt_msg_clockAcc2str(enum ptpmgmt_clockAccuracy_e value);
/**
 * Convert fault record severity code to string
 * @param[in] code severity code
 * @return string with the severity code
 */
const char *ptpmgmt_msg_faultRec2str(enum ptpmgmt_faultRecord_e code);
/**
 * Convert time source to string
 * @param[in] type time source
 * @return string with the time source
 */
const char *ptpmgmt_msg_timeSrc2str(enum ptpmgmt_timeSource_e type);
/**
 * Convert string to time source type
 * @param[in] str string to search
 * @param[out] type time source
 * @param[in] exact perform an exact match
 * @return true if found
 */
bool ptpmgmt_msg_findTimeSrc(const char *str, enum ptpmgmt_timeSource_e *type,
    bool exact);
/**
 * Convert port state to string
 * @param[in] state port state
 * @return string with the port state
 */
const char *ptpmgmt_msg_portState2str(enum ptpmgmt_portState_e state);
/**
 * Convert string to port state
 * @param[in] str string to search
 * @param[out] state port state
 * @param[in] caseSens perform case sensetive match
 * @return true if found
 */
bool ptpmgmt_msg_findPortState(const char *str, enum ptpmgmt_portState_e *state,
    bool caseSens);
/**
 * Convert delay mechanism to string
 * @param[in] type delay mechanism
 * @return string with a delay mechanism type
 */
const char *ptpmgmt_msg_delayMech2str(enum ptpmgmt_delayMechanism_e type);
/**
 * Convert string to delay mechanism
 * @param[in] str string to search
 * @param[out] type delay mechanism
 * @param[in] exact perform an exact match
 * @return true if found
 */
bool ptpmgmt_msg_findDelayMech(const char *str,
    enum ptpmgmt_delayMechanism_e *type, bool exact);
/**
 * Convert SMPTE clock locking state to string
 * @param[in] state SMPTE clock locking state
 * @return string with the SMPTE clock locking state
 */
const char *ptpmgmt_msg_smpteLck2str(enum ptpmgmt_SMPTEmasterLockingStatus_e
    state);
/**
 * Convert linuxptp time stamp type to string
 * @param[in] type time stamp type
 * @return string with the Linux time stamp type
 */
const char *ptpmgmt_msg_ts2str(enum ptpmgmt_linuxptpTimeStamp_e type);
/**
 * Convert linuxptp power profile version to string
 * @param[in] ver version
 * @return string with the Linux power profile version
 */
const char *ptpmgmt_msg_pwr2str(enum ptpmgmt_linuxptpPowerProfileVersion_e ver);
/**
 * Convert linuxptp master unicasy state to string
 * @param[in] state
 * @return string with the master state in the unicast master table
 */
const char *ptpmgmt_msg_us2str(enum ptpmgmt_linuxptpUnicastState_e state);
/**
 * Check if leap 61 seconds flag is enabled
 * @param[in] flags
 * @return true if flag on
 */
bool ptpmgmt_msg_is_LI_61(uint8_t flags);
/**
 * Check if leap 59 seconds flag is enabled
 * @param[in] flags
 * @return true if flag on
 */
bool ptpmgmt_msg_is_LI_59(uint8_t flags);
/**
 * Check if UTC offset is valid flag is enabled
 * @param[in] flags
 * @return true if flag on
 */
bool ptpmgmt_msg_is_UTCV(uint8_t flags);
/**
 * Check if is PTP instance flag is enabled
 * @param[in] flags
 * @return true if flag on
 */
bool ptpmgmt_msg_is_PTP(uint8_t flags);
/**
 * Check if is PTP instance flag is enabled
 * @param[in] flags
 * @return true if flag on
 */
bool ptpmgmt_msg_is_TTRA(uint8_t flags);
/**
 * Check if frequency is traceable flag is enabled
 * @param[in] flags
 * @return true if flag on
 */
bool ptpmgmt_msg_is_FTRA(uint8_t flags);
/**
 * Check management TLV id uses empty dataField
 * @param[in] id management TLV id
 * @return true if dataField is empty
 */
bool ptpmgmt_msg_isEmpty(enum ptpmgmt_mng_vals_e id);

/**
 * Alocate new message structure
 * @return new message structure or null in case of error
 */
ptpmgmt_msg ptpmgmt_msg_alloc();

/**
 * Alocate new message structure with parameters
 * @param[in] prms ptpmgmt_MsgParams
 * @return new message structure or null in case of error
 */
ptpmgmt_msg ptpmgmt_msg_alloc_prms(ptpmgmt_cpMsgParams prms);

#endif /* __PTPMGMT_C_MSG_H */
