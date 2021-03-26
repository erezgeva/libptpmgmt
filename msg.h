/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief Create and parse PTP managment messages
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 * Created following @"IEEE Std 1588-2008@", PTP version 2
 * with some updates from @"IEEE Std 1588-2019@"
 */

#ifndef __MSG_H
#define __MSG_H

#include <memory>
#include <string>
#include <vector>
#include <stdint.h>
#include "cfg.h"
#include "bin.h"

#ifndef INT48_MIN
/** Minimum value for signed integer 48 bits */
#define INT48_MIN (-INT64_C(0x7fffffffffff) - 1)
#endif
#ifndef INT48_MAX
/** Maximum value for signed integer 48 bits */
#define INT48_MAX (INT64_C(0x7fffffffffff))
#endif
#ifndef UINT48_MAX
/** Maximum value for unsigned integer 48 bits */
#define UINT48_MAX (UINT64_C(0xffffffffffff))
#endif

/** IEEE 1588 Protocol unsigned 8 bits integer */
typedef uint8_t UInteger8_t;
/** IEEE 1588 Protocol unsigned 16 bits integer */
typedef uint16_t UInteger16_t;
/** IEEE 1588 Protocol unsigned 32 bits integer */
typedef uint32_t UInteger32_t;
/** IEEE 1588 Protocol unsigned 48 bits integer */
typedef uint64_t UInteger48_t;
/** IEEE 1588 Protocol unsigned 64 bits integer */
typedef uint64_t UInteger64_t;
/** IEEE 1588 Protocol signed 8 bits integer */
typedef int8_t   Integer8_t;
/** IEEE 1588 Protocol signed 16 bits integer */
typedef int16_t  Integer16_t;
/** IEEE 1588 Protocol signed 32 bits integer */
typedef int32_t  Integer32_t;
/** IEEE 1588 Protocol signed 48 bits integer */
typedef int64_t Integer48_t;
/** IEEE 1588 Protocol signed 64 bits integer */
typedef int64_t  Integer64_t;
/** IEEE 1588 Protocol Octet */
typedef uint8_t  Octet_t;

class message;

/** Parsing and building errors */
enum MNG_PARSE_ERROR_e {
    MNG_PARSE_ERROR_OK,          /**< no error */
    MNG_PARSE_ERROR_MSG,         /**< Error message */
    MNG_PARSE_ERROR_INVALID_ID,  /**< Invalid TLV mng id or action for TLV */
    MNG_PARSE_ERROR_INVALID_TLV, /**< Wrong TLV header */
    MNG_PARSE_ERROR_SIZE_MISS,   /**< size missmatch of field with length */
    MNG_PARSE_ERROR_TOO_SMALL,   /**< buffer is too small */
    MNG_PARSE_ERROR_SIZE,        /**< size is even */
    MNG_PARSE_ERROR_VAL,         /**< Value is out of range or invalid */
    MNG_PARSE_ERROR_HEADER,      /**< Wrong value in header */
    MNG_PARSE_ERROR_ACTION,      /**< Wrong action value */
    MNG_PARSE_ERROR_UNSUPPORT,   /**< Do not know how to parse the TLV data */
    MNG_PARSE_ERROR_MEM,         /**< fail to allocate TLV data */
};
/** PTP managment action */
enum actionField_e : uint8_t {
    GET = 0,            /**< Send a get message */
    SET = 1,            /**< Send a set message */
    RESPONSE = 2,       /**< Recieve responce for get or set */
    COMMAND = 3,        /**< Send command message */
    ACKNOWLEDGE = 4,    /**< Recieve acknowlage on command */
};
/** @cond internal
 * Doxygen and SWIG do not know how to proccess.
 * Both proccess this enum from mngIds.h
 */
#ifndef SWIG
enum mng_vals_e {
#define A(n, v, sc, a, sz, f) n,
#include "ids.h"
};
#endif
/**< @endcond */
/** PTP Managment Error IDs */
enum managementErrorId_e : uint16_t {
    RESPONSE_TOO_BIG = 0x0001, /**< Response is too big */
    NO_SUCH_ID       = 0x0002, /**< No such id */
    WRONG_LENGTH     = 0x0003, /**< Wrong length */
    WRONG_VALUE      = 0x0004, /**< Wrong value */
    NOT_SETABLE      = 0x0005, /**< Not setable */
    NOT_SUPPORTED    = 0x0006, /**< Not supported */
    GENERAL_ERROR    = 0xfffe, /**< General error */
};
/** PTP clock type bit mask
 * @details A PTP clock could act as more then a single type
 */
enum clockType_e : uint16_t {
    ordinaryClock       = 0x8000, /**< ordinary clock */
    boundaryClock       = 0x4000, /**< boundary clock */
    p2pTransparentClock = 0x2000, /**< peer-to-peer transparent clock */
    e2eTransparentClock = 0x1000, /**< end-to-end transparent clock */
    management          = 0x0800, /**< management node (deprecated) */
};
/** PTP using a network layer */
enum networkProtocol_e : uint16_t {
    UDP_IPv4   = 1, /**< UDP over IP version 4 */
    UDP_IPv6   = 2, /**< UDP over IP version 6 */
    IEEE_802_3 = 3, /**< Raw Ethernet using IEEE 802@.3 */
    DeviceNet  = 4, /**< DeviceNet */
    ControlNet = 5, /**< ControlNet */
    PROFINET   = 6, /**< PROFINET */
};
/** PTP clock accuracy */
enum clockAccuracy_e : uint8_t {
    Accurate_within_1ps   = 0x17, /**< higher then 1 picosecond */
    Accurate_within_2_5ps = 0x18, /**< higher then 2@.5 picoseconds */
    Accurate_within_10ps  = 0x19, /**< higher then 10 picoseconds */
    Accurate_within_25ps  = 0x1a, /**< higher then 25 picoseconds */
    Accurate_within_100ps = 0x1b, /**< higher then 100 picoseconds */
    Accurate_within_250ps = 0x1c, /**< higher then 250 picoseconds */
    Accurate_within_1ns   = 0x1d, /**< higher then 1 nanosecond */
    Accurate_within_2_5ns = 0x1e, /**< higher then 2@.5 nanoseconds */
    Accurate_within_10ns  = 0x1f, /**< higher then 10 nanoseconds */
    Accurate_within_25ns  = 0x20, /**< higher then 25 nanoseconds */
    Accurate_within_100ns = 0x21, /**< higher then 100 nanoseconds */
    Accurate_within_250ns = 0x22, /**< higher then 250 nanoseconds */
    Accurate_within_1us   = 0x23, /**< higher then 1 microsecond */
    Accurate_within_2_5us = 0x24, /**< higher then 2@.5 microseconds */
    Accurate_within_10us  = 0x25, /**< higher then 10 microseconds */
    Accurate_within_25us  = 0x26, /**< higher then 25 microseconds */
    Accurate_within_100us = 0x27, /**< higher then 100 microseconds */
    Accurate_within_250us = 0x28, /**< higher then 250 microseconds */
    Accurate_within_1ms   = 0x29, /**< higher then 1 millisecond */
    Accurate_within_2_5ms = 0x2a, /**< higher then 2@.5 milliseconds */
    Accurate_within_10ms  = 0x2b, /**< higher then 10 milliseconds */
    Accurate_within_25ms  = 0x2c, /**< higher then 25 milliseconds */
    Accurate_within_100ms = 0x2d, /**< higher then 100 milliseconds */
    Accurate_within_250ms = 0x2e, /**< higher then 250 milliseconds */
    Accurate_within_1s    = 0x2f, /**< higher then 1 seconds */
    Accurate_within_10s   = 0x30, /**< higher then 10 seconds */
    Accurate_more_10s     = 0x31, /**< lower then 10 seconds */
    Accurate_Unknown      = 0xef, /**< Unknown */
};
/** Fault record severity code */
enum faultRecord_e : uint8_t {
    Emergency     = 0x00, /**< system is unusable */
    Alert         = 0x01, /**< immediate action needed */
    Critical      = 0x02, /**< critical conditions */
    Error         = 0x03, /**< error conditions */
    Warning       = 0x04, /**< warning conditions */
    Notice        = 0x05, /**< normal but significant condition */
    Informational = 0x06, /**< informational messages */
    Debug         = 0x07, /**< debug-level messages */
};
/** Time source type */
enum timeSource_e : uint8_t {
    ATOMIC_CLOCK        = 0x10, /**< Atomic clock */
    GNSS                = 0x20, /**< Satellite navigation */
    GPS                 = 0x20, /**< Satellite navigation */
    TERRESTRIAL_RADIO   = 0x30, /**< Terrestrial radio */
    SERIAL_TIME_CODE    = 0x39, /**< Serial time code */
    PTP                 = 0x40, /**< PTP of a different domain */
    NTP                 = 0x50, /**< IP Network time protocol */
    HAND_SET            = 0x60, /**< Hand set */
    OTHER               = 0x90, /**< Other */
    INTERNAL_OSCILLATOR = 0xA0, /**< Internal oscillator */
};
/** Port state */
enum portState_e : uint8_t {
    INITIALIZING = 1, /**< Initializing */
    FAULTY       = 2, /**< Faulty */
    DISABLED     = 3, /**< Disabled */
    LISTENING    = 4, /**< Listening */
    PRE_MASTER   = 5, /**< Pre source */
    MASTER       = 6, /**< Source */
    SOURCE       = 6, /**< Source */
    PASSIVE      = 7, /**< Passive */
    UNCALIBRATED = 8, /**< Uncalibrated */
    SLAVE        = 9, /**< Client */
    CLIENT       = 9, /**< Client */
};
/** linuxptp timestamp
 *  Specify the underlaying Linux time stamps type that the daemon recieve
 */
enum linuxptpTimeStamp_e : uint8_t {
    /** Using system clock based with an offset that is maintain by driver */
    TS_SOFTWARE,
    /** network interface have a PTP Hardware clock, and the
        driver read the value */
    TS_HARDWARE,
    /** Old hardware */
    TS_LEGACY_HW,
    /** One step PTP, the driver write the time stamp into the frame */
    TS_ONESTEP,
    /** One step PTP plus insert time to PDelay_Resp */
    TS_P2P1STEP,
};
/** Clock time properties bit mask */
enum : uint8_t {
    /** The last minute of the current UTC day contains 61 seconds */
    f_LI_61 = (1 << 0),
    /** the last minute of the current UTC day contains 59 seconds */
    f_LI_59 = (1 << 1),
    f_UTCV  = (1 << 2), /**< Current UTC offset is valid */
    /** The timescale of the grand source PTP Instance is PTP */
    f_PTP   = (1 << 3),
    f_TTRA  = (1 << 4), /**< timescale is traceable to a primary reference */
    /** The frequency determining the timescale is
        traceable to a primary reference */
    f_FTRA  = (1 << 5),
};
/** PTP Time interval value */
struct TimeInterval_t {
    Integer64_t scaledNanoseconds; /**< nanoseconds */
    /**
     * Get object size
     * @return object size
     */
    static size_t size() { return 8; }
    /**
     * Get interval from time interval
     * @return scaled time interval
     */
    double getInterval() const { return (double)scaledNanoseconds / 0x10000; }
};
/** PTP Time stamp */
struct Timestamp_t {
    UInteger48_t secondsField; /**< seconds */
    UInteger32_t nanosecondsField; /**< nanoseconds */
    /**
     * Get object size
     * @return object size
     */
    static size_t size() { return 10; }
    /**
     * Convert to string
     * @return string
     */
    std::string str() const;
};
/** PTP clock ID */
struct ClockIdentity_t {
    Octet_t v[8]; /**< value */
    /**
     * Get object size
     * @return object size
     */
    static size_t size() { return 8; }
    /**
     * Convert to string
     * @return string
     */
    std::string str() const;
};
/** PTP port ID */
struct PortIdentity_t {
    ClockIdentity_t clockIdentity; /**< clock ID */
    UInteger16_t portNumber; /**< port number */
    /**
     * Get object size
     * @return object size
     */
    static size_t size() { return 10; }
    /**
     * Convert to string
     * @return string
     */
    std::string str() const;
};
/** PTP port address */
struct PortAddress_t {
    networkProtocol_e networkProtocol; /**< network protocol */
    UInteger16_t addressLength; /**< address length */
    binary addressField; /**< binary from address */
    /**
     * Get object size
     * @return object size
     */
    size_t size() const { return 4 + addressField.length(); }
    /**
     * Convert to string
     * @return string
     */
    std::string str() const;
};
/** PTP clock quality */
struct ClockQuality_t {
    UInteger8_t clockClass; /**< clock class */
    clockAccuracy_e clockAccuracy; /**< clock accuracy */
    uint16_t offsetScaledLogVariance; /**< variance of the clock's phase */
};
/** PTP text value */
struct PTPText_t {
    uint8_t lengthField; /**< string length */
    std::string textField; /**< string value */
    /**
     * Get object size
     * @return object size
     */
    size_t size() const { return 1 + textField.length(); }
    /**
     * Get string
     * @return pointer to string
     */
    const char *str() const { return textField.c_str(); }
};
/** PTP fault record */
struct FaultRecord_t {
    uint16_t faultRecordLength; /**< record length */
    Timestamp_t faultTime; /**< time stamp of fault */
    faultRecord_e severityCode; /**< severity code */
    PTPText_t faultName; /**< name */
    PTPText_t faultValue; /**< value */
    PTPText_t faultDescription; /**< description */
    /**
     * Get object size
     * @return object size
     */
    size_t size() {
        return 3 + sizeof(faultTime) + faultName.size() + faultValue.size() +
            faultDescription.size();
    }
};
/** PTP Acceptable source */
struct AcceptableMaster_t {
    PortIdentity_t acceptablePortIdentity; /**< acceptable port ID */
    uint8_t alternatePriority1; /**< alternate priority 1 */
    /**
     * Get object size
     * @return object size
     */
    static size_t size() { return 11; }
};
/** Properties of a PTP managment TLV */
struct managementId_t {
    uint16_t value;   /**< managementId value */
    uint8_t scope;   /**< Applies port or clock using scope_e */
    uint8_t allowed; /**< Allowed actions, bits from allowAction_t */
    /**
     * TLV dataField size
     * @li positive size of TLV dataField when it is fixed length
     * @li 0 for TLV without dataField
     * @li -1 TLV is not supported
     * @li -2 The dataField length need calculation based on values
     * @note the size might be even. Just pad another byte at the end.
     */
    ssize_t size;
};
/** Fixed values to use in Managment messages */
struct msgParams {
    uint8_t transportSpecific; /**< transport specific */
    uint8_t domainNumber; /**< domain number */
    uint8_t boundaryHops; /**< boundary hops */
    bool isUnicast; /**< Mark message as unicast */
    /** Use linuxptp Implementation-specific */
    bool useLinuxPTPTlvs;
    PortIdentity_t target; /**< target port ID */
    PortIdentity_t self_id; /**< own port ID */
};
/** Base for all TLV structures */
struct baseData {
};

/* Structure per each mng_vals_e id */
#include "proc.h"

/**
 * @brief Handle PTP management message
 * @details
 *  Handle parse and build of a PTP management massage.
 *  Handle TLV specific dataField by calling a specific callback per TLV id
 */
class message
{
    /** @cond internal
     * Doxygen does not know how to proccess.
     * This is a private section any way.
     */
#define A(n, v, sc, a, sz, f) case##f(n)
#define caseUF(n) bool n##_f(n##_t &data);
  private:
    /* Per tlv ID callback for parse or build or both */
#include "ids.h"
    /**< @endcond */

    mng_vals_e      m_tlv_id;
    actionField_e   m_sendAction;
    actionField_e   m_replyAction;
    size_t          m_msgLen;
    /* Parsing parameters */
    uint16_t        m_sequence;
    bool            m_isUnicast;
    baseData        *m_dataSend;
    /* Used during parsing and build */
    uint8_t        *m_cur;
    ssize_t         m_left;
    size_t          m_size;  /* TLV data size on build */
    bool            m_build; /* true on build */
    MNG_PARSE_ERROR_e m_err; /* Last TLV err */

    msgParams       m_prms;
    PortIdentity_t  m_peer;
    std::unique_ptr<baseData> m_dataGet;

    /* Used for reserved values */
    uint8_t reserved;

    /* For error messages */
    uint16_t m_errorId;
    PTPText_t m_errorDisplay;
    static const managementId_t mng_all_vals[];

    bool allowedAction(mng_vals_e id, actionField_e action);
    /* Parsing functions */
    void move(size_t val) {
        m_cur += val;
        m_left -= val;
        m_size += val;
    }
    bool findTlvId(uint16_t val); /* val in network order */
    bool checkReplyAction(uint8_t actionField);
    bool proc(uint8_t &val);
    bool proc(uint16_t &val);
    bool proc(uint32_t &val);
    bool proc48(uint64_t &val);
    bool proc(uint64_t &val);
    bool proc(int8_t &val);
    bool proc(int16_t &val);
    bool proc(int32_t &val);
    bool proc48(int64_t &val);
    bool proc(int64_t &val);
    bool proc(std::string &str, uint16_t len);
    bool proc(binary &bin, uint16_t len);
    bool proc(uint8_t *val, size_t len);
    bool proc(networkProtocol_e &val);
    bool proc(clockAccuracy_e &val);
    bool proc(faultRecord_e &val);
    bool proc(timeSource_e &val);
    bool proc(portState_e &val);
    bool proc(linuxptpTimeStamp_e &val);
    bool proc(TimeInterval_t &v);
    bool proc(Timestamp_t &d);
    bool proc(ClockIdentity_t &v);
    bool proc(PortIdentity_t &d);
    bool proc(PortAddress_t &d);
    bool proc(ClockQuality_t &d);
    bool proc(PTPText_t &d);
    bool proc(FaultRecord_t &d);
    bool proc(AcceptableMaster_t &d);
    /* linuxptp PORT_STATS_NP statistics use little endian */
    bool procLe(uint64_t &val);
    MNG_PARSE_ERROR_e call_tlv_data();
    /*
     * dataFieldSize() for sending SET/COMMAND
     * Get dataField of current m_tlv_id
     * For id with non fixed size
     * The size is determined by the m_dataSend content
     */
    ssize_t dataFieldSize();

  public:
    message();
    /**
     * Construct a new object using the user msgParams parameters
     * @param[in] prms msgParams parameters
     * @note you may use the parameters from a different message object
     */
    message(msgParams prms);
    /**
     * Get the current msgparams parameters
     * @return msgparams parameters
     */
    msgParams getParams() { return m_prms; }
    /**
     * Set and use a user msgParams parameters
     * @param[in] prms msgParams parameters
     * @return true if parameters are valid and updated
     */
    bool updateParams(msgParams prms);
    /**
     * Get the current TLV id
     * @return current TLV id
     * @note the message object hold a single value from the last setting or
     *  reply parsing.
     */
    mng_vals_e getTlvId() { return m_tlv_id; }
    /**
     * Set target clock ID to use all clocks.
     */
    void setAllClocks();
    /**
     * Query if target clock ID is use all clocks.
     * @return true if target use all clocks
     */
    bool isAllClocks();
    /**
     * Fetch msgParams parameters from configuration file
     * @param[in] cfg referance to configuration file object
     * @param[in] section in configuration file
     * @return true on success
     * @note calling without section will fetch value from @"global@" section
     */
    bool useConfig(configFile &cfg, std::string section = "");
    /**
     * Convert parse error code to string
     * @param[in] err parse code
     * @return string with the error message
     */
    static const char *err2str_c(MNG_PARSE_ERROR_e err);
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
     * Convert port state to string
     * @param[in] state port state
     * @return string with the port state
     */
    static const char *portState2str_c(portState_e state);
    /**
     * Convert linuxptp time stamp type to string
     * @param[in] type time stamp type
     * @return string with the Linux time stamp type
     */
    static const char *ts2str_c(linuxptpTimeStamp_e type);
    /**
     * Check if leap 61 seconds flag is enabled
     * @param[in] flags
     * @return 1 if flag on or 0 if not
     */
    static uint8_t is_LI_61(uint8_t flags) { return flags & f_LI_61 ? 1 : 0; }
    /**
     * Check if leap 59 seconds flag is enabled
     * @param[in] flags
     * @return 1 if flag on or 0 if not
     */
    static uint8_t is_LI_59(uint8_t flags) { return flags & f_LI_59 ? 1 : 0; }
    /**
     * Check if UTC offset is valid flag is enabled
     * @param[in] flags
     * @return 1 if flag on or 0 if not
     */
    static uint8_t is_UTCV(uint8_t flags)  { return flags & f_UTCV  ? 1 : 0; }
    /**
     * Check if is PTP instance flag is enabled
     * @param[in] flags
     * @return 1 if flag on or 0 if not
     */
    static uint8_t is_PTP(uint8_t flags)   { return flags & f_PTP   ? 1 : 0; }
    /**
     * Check if timescale is traceable flag is enabled
     * @param[in] flags
     * @return 1 if flag on or 0 if not
     */
    static uint8_t is_TTRA(uint8_t flags)  { return flags & f_TTRA  ? 1 : 0; }
    /**
     * Check if frequency is traceable flag is enabled
     * @param[in] flags
     * @return 1 if flag on or 0 if not
     */
    static uint8_t is_FTRA(uint8_t flags)  { return flags & f_FTRA  ? 1 : 0; }
    /**
     * Check management TLV id uses empty dataField
     * @param[in] id management TLV id
     * @return true if dataField is empty
     */
    static bool isEmpty(mng_vals_e id);
    /**
     * Set message object management TLV id and action with empty dataField
     * @param[in] actionField for sending
     * @param[in] tlv_id management TLV id
     * @return true if setting is leagal
     * @note the setting is valid for send only
     */
    bool setAction(actionField_e actionField, mng_vals_e tlv_id);
    /**
     * Set message object management TLV id, action and data for dataField
     * @param[in] actionField for sending
     * @param[in] tlv_id management TLV id
     * @param[in] dataSend referece t TLV id
     * @return true if setting is leagal
     * @note the setting is valid for send only
     * @attention
     *  The caller must use the proper structure with the TLV id!
     *  Mismatch will probably cause a crash to your application.
     *  The library does @b NOT perform any error catchig of any kind!
     */
    bool setAction(actionField_e actionField, mng_vals_e tlv_id,
        baseData &dataSend);
    /**
     * Build a raw message for send based on last setAction call
     * @param[in, out] buf memory buffer to fill with raw PTP message
     * @param[in] bufSize buffer size
     * @param[in] sequence message sequence
     * @return parse error state
     * @note the message is initialize with NULL_PTP_MANAGEMENT managment ID
     * @note usually the user increase the sequence so it can be compared
     *  with replied message
     * @note if raw message is larger then buffer size the function
     *   return MNG_PARSE_ERROR_TOO_SMALL
     */
    MNG_PARSE_ERROR_e build(void *buf, size_t bufSize, uint16_t sequence);
    /**
     * Get last sent management action
     * @return send management action
     */
    actionField_e getSendAction() { return m_sendAction; }
    /**
     * Get last reply management action
     * @return reply management action
     * @note set on parse
     */
    actionField_e getReplyAction() { return m_replyAction; }
    /**
     * Get last sent message sized
     * @return message size
     */
    size_t getMsgLen() { return m_msgLen; }
    /**
     * Get planned message to send sized
     * @return planned message size
     * @note the planned message size is based on the managemant TLV id,
     *  action and the last dataSend the user set.
     * User can use the size to allocate proper buffer for sending
     */
    ssize_t getMsgPlanedLen();
    /* Parsed message functions */
    /**
     * Parse a recieved raw socket
     * @param[in] buf memory buffer containing the raw PTP message
     * @param[in] msgSize recieved size of PTP message
     * @return parse error state
     */
    MNG_PARSE_ERROR_e parse(void *buf, ssize_t msgSize);
    /**
     * Is last parsed message a unicast or not
     * @return true if parsed message is unicast
     */
    bool isUnicast() { return m_isUnicast; }
    /**
     * Get last parsed message sequance number
     * @return parsed sequance number
     */
    uint16_t getSequence() { return m_sequence; }
    /**
     * Get last parsed message peer port ID
     * @return parsed message peer port ID
     */
    const PortIdentity_t &getPeer() { return m_peer; }
    /**
     * Get last parsed message dataField
     * @return pointer to last parsed message dataField
     * @note user need to cast to proper structure depends on
     *  management TLV ID.
     * @note User @b should not try to free this memory block
     */
    const baseData *getData() { return m_dataGet.get(); }
    /**
     * Get management error code ID
     * Relevant only when parsed message return MNG_PARSE_ERROR_MSG
     * @return error code
     */
    managementErrorId_e getErrId() { return (managementErrorId_e)m_errorId; }
    /**
     * Get management error message
     * Relevant only when parsed message return MNG_PARSE_ERROR_MSG
     * @return error message
     */
    const std::string &getErrDisplay() { return m_errorDisplay.textField; }
    /**
     * Get management error message
     * Relevant only when parsed message return MNG_PARSE_ERROR_MSG
     * @return error message
     */
    const char *getErrDisplay_c() { return m_errorDisplay.str(); }
    /**
     * Get this library version
     * @return this library version
     */
    static const char *getVersion();
    /**
     * Get this library major version
     * @return this library version major
     */
    static int getVersionMajor();
    /**
     * Get this library minor version
     * @return this library version minor
     */
    static int getVersionMinor();
};

/* For SWIG */
#undef A
#undef caseUF

#endif /*__MSG_H*/
