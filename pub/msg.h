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

#ifndef __PTPMGMT_MSG_H
#define __PTPMGMT_MSG_H

#ifdef __cplusplus
#include <functional>
#include "cfg.h"
#include "buf.h"
#include "proc.h"
#include "sig.h"

__PTPMGMT_NAMESPACE_BEGIN

struct MsgProc;
class Message;
struct HMAC_Key;

/**
 * @brief Abstract class used for callback for Signalling TLVs traverse
 * @note For PHP, Perl, Python and Ruby use
 */
class MessageSigTlvCallback
{
  public:
    MessageSigTlvCallback() = default;
    virtual ~MessageSigTlvCallback() = default;
    /**
     * Callback for handling a single signalling TLV
     * @param[in] msg reference to the Message object calling this callback
     * @param[in] tlvType signalling TLV type
     * @param[in] tlv signalling TLV
     * @return true to stop traverse
     * @note Available for PHP, Perl, Python and Ruby use
     */
    virtual bool callback(const Message &msg, tlvType_e tlvType,
        const BaseSigTlv *tlv) = 0;
};

/**
 * @brief Handle PTP management message
 * @details
 *  Handle parse and build of a PTP management massage.
 *  Handle TLV specific dataField by calling a specific call-back per TLV id
 */
class Message
{
    /** @cond internal
     * Doxygen does not know how to process.
     * This is a private section any way.
     */
  private:

    /**< @endcond */

    /* build parameters */
    actionField_e     m_sendAction = GET;
    size_t            m_msgLen = 0;
    const BaseMngTlv *m_dataSend = nullptr;
    mng_vals_e        m_tlv_id = NULL_PTP_MANAGEMENT; /* managementId */

    /* parsing parameters */
    uint16_t          m_sequence = 0;
    bool              m_isUnicast = true;
    uint8_t           m_PTPProfileSpecific = 0;
    actionField_e     m_replyAction = RESPONSE;
    mng_vals_e        m_replayTlv_id = NULL_PTP_MANAGEMENT; /* managementId */
    uint32_t          m_sdoId = 0; /* parsed message sdoId (transportSpecific) */
    msgType_e         m_type = Management; /* parsed message type */
    tlvType_e         m_mngType = MANAGEMENT; /* parsed management message type */
    uint8_t           m_domainNumber = 0; /* parsed message domainNumber*/
    uint8_t           m_versionPTP = 2; /* parsed message ptp version */
    uint8_t           m_minorVersionPTP = 1; /* parsed message ptp version */
    /* hold signalling TLVs */
    std::vector<std::unique_ptr<BaseSigTlv>> m_sigTlvs;
    /* hold signalling TLVs type */
    std::vector<tlvType_e> m_sigTlvsType;
    /* parsed managment TLV */
    std::unique_ptr<BaseMngTlv> m_dataGet;

    /* Generic */
    MsgParams         m_prms;

    /* Authentication TLV */
    uint32_t          m_keyID = 0; /**< Key id used for sending */
    uint8_t           m_sppID = -1; /**< authentication security parameters ID */
    SaFile            m_sa; /**< authentication security association pool */
    bool              m_haveAuth = false;  /**< Have Authentication */
    HMAC_Key         *m_hmac = nullptr; /**< sending key HMAC library instance */

    /* parsing parameters */
    PortIdentity_t    m_peer; /* parsed message peer port id */
    PortIdentity_t    m_target; /* parsed message target port id */

    /* For error messages */
    managementErrorId_e m_errorId = (managementErrorId_e)0;
    PTPText_t m_errorDisplay;

    /* Map to all management IDs */
    static const ManagementId_t mng_all_vals[];

    bool allowedAction(mng_vals_e id, actionField_e action);
    /* val in network order */
    static bool findTlvId(uint16_t val, mng_vals_e &rid, implementSpecific_e spec);
    bool checkReplyAction(uint8_t actionField);
    /* parse signalling message */
    MNG_PARSE_ERROR_e parseSig(const void *buf, MsgProc *);
    /* parse authentication message */
    MNG_PARSE_ERROR_e parseAuth(const void *buf, const void *auth, ssize_t left,
        bool check = false);
    /*
     * dataFieldSize() for sending SET/COMMAND
     * Get dataField of current build managment ID
     * For id with non fixed size
     * The size is determined by the m_dataSend content
     */
    ssize_t dataFieldSize(const BaseMngTlv *data) const;
    /**
     * Verift TLV is of the proper type, match to the TLV ID
     * Set and use a user MsgParams parameters
     * @param[in] tlv_id TLV ID
     * @param[in] tlv TLV to verify
     * @return true if TLV is not null and is of TLV ID type
     */
    static bool verifyTlv(mng_vals_e tlv_id, const BaseMngTlv *tlv);

  public:
    Message();
    ~Message();
    /**
     * Construct a new object using the user MsgParams parameters
     * @param[in] prms MsgParams parameters
     * @note you may use the parameters from a different message object
     */
    Message(const MsgParams &prms);
    /**
     * Get the current msgparams parameters
     * @return msgparams parameters
     */
    const MsgParams &getParams() const { return m_prms; }
    /**
     * Set and use a user MsgParams parameters
     * @param[in] prms MsgParams parameters
     * @return true if parameters are valid and updated
     */
    bool updateParams(const MsgParams &prms);
    /**
     * Use the AUTHENTICATION TLV on send and receive
     *  read parameters from configuration file.
     * @param[in] cfg reference to configuration file object
     * @param[in] section in configuration file
     * @return true if find valid authentication parameters and hmac library
     */
    bool useAuth(const ConfigFile &cfg, const std::string &section = "");
    /**
     * Use the AUTHENTICATION TLV on send and receive, read SA file.
     * @param[in] sa reference to SA file object
     * @param[in] spp ID to use to send
     * @param[in] key ID to use to send
     * @return true if find valid authentication parameters and hmac library
     */
    bool useAuth(const SaFile &sa, uint8_t spp, uint32_t key);
    /**
     * Change the spp and send key used in the AUTHENTICATION TLVs
     * @param[in] spp ID to use to send
     * @param[in] key ID to use to send
     * @return true if change success
     */
    bool changeAuth(uint8_t spp, uint32_t key);
    /**
     * Change the send key used in the AUTHENTICATION TLVs
     * @param[in] key ID to use to send
     * @return true if change success
     */
    bool changeAuth(uint32_t key);
    /**
     * Disable the use of AUTHENTICATION TLV
     * @return true if disabled
     */
    bool disableAuth() { m_haveAuth = false; return true; }
    /**
     * Get used AUTHENTICATION TLV Spp ID used for send
     * @return Spp ID or -1
     */
    int usedAuthSppID() const { return m_haveAuth ? m_sppID : -1; }
    /**
     * Get used AUTHENTICATION TLV key ID used for send
     * @return key ID or 0
     */
    uint32_t usedAuthKeyID() const { return m_haveAuth ? m_keyID : 0; }
    /**
     * Get authentication security association pool
     * @return authentication parameters
     */
    const SaFile &getSa() const { return m_sa; }
    /**
     * Get if AUTHENTICATION TLV with authentication parameters are used
     * @return true if AUTHENTICATION TLV is used
     */
    bool haveAuth() const { return m_haveAuth; }
    /**
     * Get the current parsed TLV id
     * @return current parsed TLV id
     */
    mng_vals_e getTlvId() const { return m_replayTlv_id; }
    /**
     * Get the current build TLV id
     * @return current TLV id
     */
    mng_vals_e getBuildTlvId() const { return m_tlv_id; }
    /**
     * Set target clock ID to use all clocks.
     */
    void setAllClocks();
    /**
     * Query if target clock ID is using all clocks.
     * @return true if target use all clocks
     */
    bool isAllClocks() const;
    /**
     * Fetch MsgParams parameters from configuration file
     * @param[in] cfg reference to configuration file object
     * @param[in] section in configuration file
     * @return true on success
     * @note calling without section will fetch value from @"global@" section
     */
    bool useConfig(const ConfigFile &cfg, const std::string &section = "");
    /**
     * Convert parse error code to string
     * @param[in] err parse code
     * @return string with the error message
     */
    static const char *err2str_c(MNG_PARSE_ERROR_e err);

    /**
     * Convert message type to string
     * @param[in] type
     * @return string with the TLV type
     */
    static const char *type2str_c(msgType_e type);
    /**
     * Convert TLV type to string
     * @param[in] type
     * @return string with the TLV type
     */
    static const char *tlv2str_c(tlvType_e type);
    /**
     * Convert action to string
     * @param[in] action
     * @return string with the action in a string
     */
    static const char *act2str_c(actionField_e action);
    /**
     * Convert management id to string
     * @param[in] id parse code
     * @return string with ID name
     */
    static const char *mng2str_c(mng_vals_e id);
    /**
     * Convert string to management id
     * @param[in] str string to search
     * @param[out] id parse code
     * @param[in] exact perform an exact match
     * @return true if found
     */
    static const bool findMngID(const std::string &str, mng_vals_e &id,
        bool exact = true);
    /**
     * Convert management error to string
     * @param[in] err ID
     * @return string with the error message
     */
    static const char *errId2str_c(managementErrorId_e err);
    /**
     * Convert clock type to string
     * @param[in] type clock type
     * @return string with the clock type
     */
    static const char *clkType2str_c(clockType_e type);
    /**
     * Convert network protocol to string
     * @param[in] protocol network protocol
     * @return string with the network protocol
     */
    static const char *netProt2str_c(networkProtocol_e protocol);
    /**
     * Convert clock accuracy to string
     * @param[in] value clock accuracy
     * @return string with the clock accuracy
     */
    static const char *clockAcc2str_c(clockAccuracy_e value);
    /**
     * Convert fault record severity code to string
     * @param[in] code severity code
     * @return string with the severity code
     */
    static const char *faultRec2str_c(faultRecord_e code);
    /**
     * Convert time source to string
     * @param[in] type time source
     * @return string with the time source
     */
    static const char *timeSrc2str_c(timeSource_e type);
    /**
     * Convert string to time source type
     * @param[in] str string to search
     * @param[out] type time source
     * @param[in] exact perform an exact match
     * @return true if found
     */
    static const bool findTimeSrc(const std::string &str, timeSource_e &type,
        bool exact = true);
    /**
     * Convert port state to string
     * @param[in] state port state
     * @return string with the port state
     */
    static const char *portState2str_c(portState_e state);
    /**
     * Convert string to port state
     * @param[in] str string to search
     * @param[out] state port state
     * @param[in] caseSens perform case sensetive match
     * @return true if found
     */
    static const bool findPortState(const std::string &str, portState_e &state,
        bool caseSens = true);
    /**
     * Convert delay mechanism to string
     * @param[in] type delay mechanism
     * @return string with a delay mechanism type
     */
    static const char *delayMech2str_c(delayMechanism_e type);
    /**
     * Convert string to delay mechanism
     * @param[in] str string to search
     * @param[out] type delay mechanism
     * @param[in] exact perform an exact match
     * @return true if found
     */
    static const bool findDelayMech(const std::string &str, delayMechanism_e &type,
        bool exact = true);
    /**
     * Convert SMPTE clock locking state to string
     * @param[in] state SMPTE clock locking state
     * @return string with the SMPTE clock locking state
     */
    static const char *smpteLck2str_c(SMPTEmasterLockingStatus_e state);
    /**
     * Convert linuxptp time stamp type to string
     * @param[in] type time stamp type
     * @return string with the Linux time stamp type
     */
    static const char *ts2str_c(linuxptpTimeStamp_e type);
    /**
     * Convert linuxptp power profile version to string
     * @param[in] ver version
     * @return string with the Linux power profile version
     */
    static const char *pwr2str_c(linuxptpPowerProfileVersion_e ver);
    /**
     * Convert linuxptp master unicasy state to string
     * @param[in] state
     * @return string with the master state in the unicast master table
     */
    static const char *us2str_c(linuxptpUnicastState_e state);
    /**
     * Check if leap 61 seconds flag is enabled
     * @param[in] flags
     * @return true if flag on
     */
    static bool is_LI_61(uint8_t flags) { return (flags & F_LI_61) != 0; }
    /**
     * Check if leap 59 seconds flag is enabled
     * @param[in] flags
     * @return true if flag on
     */
    static bool is_LI_59(uint8_t flags) { return (flags & F_LI_59) != 0; }
    /**
     * Check if UTC offset is valid flag is enabled
     * @param[in] flags
     * @return true if flag on
     */
    static bool is_UTCV(uint8_t flags) { return (flags & F_UTCV) != 0; }
    /**
     * Check if is PTP instance flag is enabled
     * @param[in] flags
     * @return true if flag on
     */
    static bool is_PTP(uint8_t flags) { return (flags & F_PTP) != 0; }
    /**
     * Check if timescale is traceable flag is enabled
     * @param[in] flags
     * @return true if flag on
     */
    static bool is_TTRA(uint8_t flags) { return (flags & F_TTRA) != 0; }
    /**
     * Check if frequency is traceable flag is enabled
     * @param[in] flags
     * @return true if flag on
     */
    static bool is_FTRA(uint8_t flags) { return (flags & F_FTRA) != 0; }
    /**
     * Check management TLV id uses empty dataField
     * @param[in] id management TLV id
     * @return true if dataField is empty
     */
    static bool isEmpty(mng_vals_e id);
    /**
     * Check if management TLV is valid for use
     * @param[in] id management TLV id
     * @return true if management TLV is valid
     * @note function also check implement specific TLVs!
     */
    bool isValidId(mng_vals_e id);
    /**
     * Set message object management TLV id, action and data for dataField
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
    bool setAction(actionField_e actionField, mng_vals_e tlv_id,
        const BaseMngTlv *dataSend = nullptr);
    /**
     * Clear data for send, prevent accidentally use, in case it is freed
     */
    void clearData();
    /**
     * Build a raw message for send using setAction setting
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
    MNG_PARSE_ERROR_e build(void *buf, size_t bufSize, uint16_t sequence);
    /**
     * Build a raw message for send based on last setAction call
     * @param[in, out] buf object with memory buffer to fill with raw PTP Message
     * @param[in] sequence message sequence
     * @return parse error state
     * @note build before setting with setAction will create a GET raw message
     *  with NULL_PTP_MANAGEMENT management TLV
     * @note usually the user increases the sequence so it can be compared
     *  with replied message
     * @note if raw message is larger than buffer size the function
     *   return MNG_PARSE_ERROR_TOO_SMALL
     */
    MNG_PARSE_ERROR_e build(Buf &buf, uint16_t sequence)
    { return build(buf.get(), buf.size(), sequence); }
    /**
     * Get build management action
     * @return build management action
     */
    actionField_e getSendAction() const { return m_sendAction; }
    /**
     * Get last build message size
     * @return message size
     */
    size_t getMsgLen() const { return m_msgLen; }
    /**
     * Get planned message to build size
     * @return planned message size or negative for error
     * @note the planned message size is based on the management TLV id,
     *  action and the dataSend set by the user.
     * You can use the size to allocate proper buffer for sending.
     * Add Authentication TLV size if used.
     */
    ssize_t getMsgPlanedLen() const;
    /* Parsed message functions */
    /**
     * Parse a received raw socket
     * @param[in] buf memory buffer containing the raw PTP Message
     * @param[in] msgSize received size of PTP Message
     * @return parse error state
     */
    MNG_PARSE_ERROR_e parse(const void *buf, ssize_t msgSize);
    /**
     * Parse a received raw socket
     * @param[in] buf object with memory buffer containing the raw PTP Message
     * @param[in] msgSize received size of PTP Message
     * @return parse error state
     */
    MNG_PARSE_ERROR_e parse(const Buf &buf, ssize_t msgSize);
    /**
     * Get last reply management action
     * @return reply management action
     * @note set on parse
     */
    actionField_e getReplyAction() const { return m_replyAction; }
    /**
     * Is last parsed message a unicast or not
     * @return true if parsed message is unicast
     */
    bool isUnicast() const { return m_isUnicast; }
    /**
     * Get last reply PTP Profile Specific
     * @return reply management action
     * @note set on parse
     */
    uint8_t getPTPProfileSpecific() const { return m_PTPProfileSpecific; }
    /**
     * Get last parsed message sequence number
     * @return parsed sequence number
     */
    uint16_t getSequence() const { return m_sequence; }
    /**
     * Get last parsed message peer port ID
     * @return parsed message peer port ID
     */
    const PortIdentity_t &getPeer() const { return m_peer; }
    /**
     * Get last parsed message target port ID
     * @return parsed message target port ID
     */
    const PortIdentity_t &getTarget() const { return m_target; }
    /**
     * Get last parsed message sdoId
     * @return parsed message sdoId
     * @note upper byte is was transportSpecific
     */
    uint32_t getSdoId() const { return m_sdoId; }
    /**
     * Get last parsed message domainNumber
     * @return parsed message domainNumber
     */
    uint8_t getDomainNumber() const { return m_domainNumber; }
    /**
     * Get last parsed message PTP version
     * @return parsed message versionPTP
     */
    uint8_t getVersionPTP() const { return m_versionPTP; }
    /**
     * Get last parsed message minor PTP version
     * @return parsed message versionPTP
     */
    uint8_t getMinorVersionPTP() const { return m_minorVersionPTP; }
    /**
     * Get last parsed message dataField
     * @return pointer to last parsed message dataField or null
     * @note You need to cast to proper structure depends on
     *  management TLV ID, get with.
     * @note You @b should not try to free or change this TLV object
     */
    const BaseMngTlv *getData() const { return m_dataGet.get(); }
    /**
     * Get send message dataField
     * @return pointer to send message dataField or null
     * @note You need to cast to proper structure depends on
     *  management TLV ID.
     * @note In case you release this memory,
     *  you should call @code clearData() @endcode
     */
    const BaseMngTlv *getSendData() const { return m_dataSend; }
    /**
     * Get management error code ID
     * Relevant only when parsed message return MNG_PARSE_ERROR_MSG
     * @return error code
     */
    managementErrorId_e getErrId() const { return m_errorId; }
    /**
     * Get management error message
     * Relevant only when parsed message return MNG_PARSE_ERROR_MSG
     * @return error message
     */
    const std::string &getErrDisplay() const { return m_errorDisplay.textField; }
    /**
     * Get management error message
     * Relevant only when parsed message return MNG_PARSE_ERROR_MSG
     * @return error message
     */
    const char *getErrDisplay_c() const { return m_errorDisplay.string(); }
    /**
     * query if last message is a signalling message
     * @return true if last message is a signalling message
     */
    bool isLastMsgSig() const { return m_type == Signaling; }
    /**
     * query if last message is a SMPTE message
     * @return true if last message is a SMPTE message
     */
    bool isLastMsgSMPTE() const;
    /**
     * Get message type
     * @return message type
     */
    msgType_e getType() const { return m_type; }
    /**
     * Get management message type
     * @return management message type
     * @note return MANAGEMENT or MANAGEMENT_ERROR_STATUS
     */
    tlvType_e getMngType() const { return m_mngType; }
    /**
     * Traverse all last signalling message TLVs
     * @param[in] callback function to call with each TLV
     * @return true if any of the calling to call-back return true
     * @note stop if any of the calling to call-back return true
     */
    bool traversSigTlvs(const std::function<bool (const Message &msg,
            tlvType_e tlvType, const BaseSigTlv *tlv)> callback) const;
    /**
     * Traverse all last signalling message TLVs
     * @param[in] callback object with callback to be called with each TLV
     * @return true if any of the calling to call-back return true
     * @note stop if any of the calling to call-back return true
     * @note Available for PHP, Perl, Python and Ruby use
     */
    bool traversSigTlvsCl(MessageSigTlvCallback &callback);
    /**
     * Get number of the last signalling message TLVs
     * @return number of TLVs or zero
     */
    size_t getSigTlvsCount() const;
    /**
     * Get a TLV from the last signalling message TLVs by position
     * @param[in] position of TLV
     * @return TLV or null
     */
    const BaseSigTlv *getSigTlv(size_t position) const;
    /**
     * Get a type of TLV from the last signalling message TLVs by position
     * @param[in] position of TLV
     * @return type of TLV or unknown
     */
    tlvType_e getSigTlvType(size_t position) const;
    /**
     * Get the management TLV ID of a management TLV
     * from the last signalling message TLVs by position
     * @param[in] position of TLV
     * @return management TLV ID or NULL_PTP_MANAGEMENT
     * @note return NULL_PTP_MANAGEMENT if TLV is not management
     */
    mng_vals_e getSigMngTlvType(size_t position) const;
    /**
     * Get a management TLV from the last signalling message TLVs by position
     * @param[in] position of TLV
     * @return management TLV or null
     * @note return null if TLV is not management
     * @note You @b should not try to free this TLV object
     */
    const BaseMngTlv *getSigMngTlv(size_t position) const;
};

__PTPMGMT_NAMESPACE_END
#else /* __cplusplus */
#include "c/msg.h"
#endif /* __cplusplus */

#endif /* __PTPMGMT_MSG_H */
