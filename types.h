/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief types, enumerators, and structers used by PTP management messages
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#ifndef __PTPMGMT_TYPES_H
#define __PTPMGMT_TYPES_H

#include <string>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <map>
#include "bin.h"

#ifndef SWIG
namespace ptpmgmt
{
#endif

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

/** IEEE 1588 Protocol 2 x 4 bits integer */
typedef uint8_t Nibble_t;
/** IEEE 1588 Protocol unsigned 8 bits integer */
typedef uint8_t UInteger8_t;
/** IEEE 1588 Protocol unsigned 16 bits integer */
typedef uint16_t UInteger16_t;
/** IEEE 1588 Protocol unsigned 32 bits integer */
typedef uint32_t UInteger32_t;
/** IEEE 1588 Protocol unsigned 48 bits integer */
typedef uint64_t UInteger48_t;
/** UInteger48_t octets size */
const size_t sizeof_UInteger48_t = 6;
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
/** Integer48_t octets size */
const size_t sizeof_Integer48_t = 6;
/** IEEE 1588 Protocol signed 64 bits integer */
typedef int64_t  Integer64_t;
/** IEEE 1588 protocol octet */
typedef uint8_t  Octet_t;
/** IEEE Std 754 binary64 (64-bit double-precision floating-point format) */
typedef double Float64_t;

/** Parsing and building errors */
enum MNG_PARSE_ERROR_e {
    MNG_PARSE_ERROR_OK,          /**< no error */
    MNG_PARSE_ERROR_MSG,         /**< Error message */
    MNG_PARSE_ERROR_SIG,         /**< Signaling message */
    MNG_PARSE_ERROR_INVALID_ID,  /**< Invalid TLV mng id or action for TLV */
    MNG_PARSE_ERROR_INVALID_TLV, /**< Wrong TLV header */
    MNG_PARSE_ERROR_SIZE_MISS,   /**< size mismatch of field with length */
    MNG_PARSE_ERROR_TOO_SMALL,   /**< buffer is too small */
    MNG_PARSE_ERROR_SIZE,        /**< size is even */
    MNG_PARSE_ERROR_VAL,         /**< Value is out of range or invalid */
    MNG_PARSE_ERROR_HEADER,      /**< Wrong value in header */
    MNG_PARSE_ERROR_ACTION,      /**< Wrong action value */
    MNG_PARSE_ERROR_UNSUPPORT,   /**< Do not know how to parse the TLV data */
    MNG_PARSE_ERROR_MEM,         /**< fail to allocate TLV data */
};
/** PTP messages type
 * @note: 4 bits
 */
enum msgType_e : Nibble_t {
    /* Event messages */
    Sync                    = 0, /**< Synchronization event message */
    Delay_Req               = 1, /**< Delay request event message */
    Pdelay_Req              = 2, /**< Peer delay request event message */
    Pdelay_Resp             = 3, /**< Peer delay response event message */
    /* General messages */
    Follow_Up               = 0x8, /**< Follow up message */
    Delay_Resp              = 0x9, /**< Delay response */
    Pdelay_Resp_Follow_Up   = 0xa, /**< Peer delay response follow up message */
    Announce                = 0xb, /**< Announce message*/
    Signaling               = 0xc, /**< Signaling message */
    Management              = 0xd, /**< Management message */
};
/**
 * PTP messages TLV types
 * @note: With new signaling TLVs from "IEEE Std 1588-2019@"
 */
enum tlvType_e : uint16_t {
    MANAGEMENT                              = 0x0001, /**< Management TLV */
    MANAGEMENT_ERROR_STATUS                 = 0x0002, /**< Management Error TLV */
    ORGANIZATION_EXTENSION                  = 0x0003, /**< Organization extension */
    REQUEST_UNICAST_TRANSMISSION            = 0x0004, /**< Request unicast */
    GRANT_UNICAST_TRANSMISSION              = 0x0005, /**< Grant unicast */
    CANCEL_UNICAST_TRANSMISSION             = 0x0006, /**< Cancel unicast */
    ACKNOWLEDGE_CANCEL_UNICAST_TRANSMISSION = 0x0007, /**< Ack cancel unicast */
    PATH_TRACE                              = 0x0008, /**< Path trace */
    ALTERNATE_TIME_OFFSET_INDICATOR         = 0x0009, /**< Alternate tine offset */
    /* Obsolete valuse 2000, 2001, 2002, 2003 */
    ORGANIZATION_EXTENSION_PROPAGATE        = 0x4000, /**< Organization extension */
    ENHANCED_ACCURACY_METRICS               = 0x4001, /**< Enhanced accuracy */
    ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE = 0x8000, /**< Organization extension */
    /** Layer 1 synchronization */
    L1_SYNC                                 = 0x8001,
    PORT_COMMUNICATION_AVAILABILITY         = 0x8002, /**< Port communication */
    PROTOCOL_ADDRESS                        = 0x8003, /**< Protocol address */
    SLAVE_RX_SYNC_TIMING_DATA               = 0x8004, /**< Client RX sync time */
    SLAVE_RX_SYNC_COMPUTED_DATA             = 0x8005, /**< Client RX sync */
    SLAVE_TX_EVENT_TIMESTAMPS               = 0x8006, /**< Client TX event */
    CUMULATIVE_RATE_RATIO                   = 0x8007, /**< Cumulative rate */
    TLV_PAD                                 = 0x8008, /**< Padding TLV, ignored */
    AUTHENTICATION                          = 0x8009, /**< Authentication */
    /** Client delay time
     * note: linuxptp Experimental value */
    SLAVE_DELAY_TIMING_DATA_NP              = 0x7f00,
};
/** PTP management action */
enum actionField_e : uint8_t {
    GET = 0,            /**< Send a get message */
    SET = 1,            /**< Send a set message */
    RESPONSE = 2,       /**< Receive response for a get or a set message */
    COMMAND = 3,        /**< Send command message */
    ACKNOWLEDGE = 4,    /**< Receive acknowledge on command */
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
/** PTP Management Error IDs */
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
 * @details A PTP clock could act as more than a single type
 * @note: Ruby's wrapping, capitalize first letter of enumerators values
 */
enum clockType_e : uint16_t {
    ordinaryClock       = 0x8000, /**< ordinary clock */
    boundaryClock       = 0x4000, /**< boundary clock */
    p2pTransparentClock = 0x2000, /**< peer-to-peer transparent clock */
    e2eTransparentClock = 0x1000, /**< end-to-end transparent clock */
    managementClock     = 0x0800, /**< management node (deprecated) */
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
    Accurate_within_1ps   = 0x17, /**< higher than 1 picosecond */
    Accurate_within_2_5ps = 0x18, /**< higher than 2@.5 picoseconds */
    Accurate_within_10ps  = 0x19, /**< higher than 10 picoseconds */
    Accurate_within_25ps  = 0x1a, /**< higher than 25 picoseconds */
    Accurate_within_100ps = 0x1b, /**< higher than 100 picoseconds */
    Accurate_within_250ps = 0x1c, /**< higher than 250 picoseconds */
    Accurate_within_1ns   = 0x1d, /**< higher than 1 nanosecond */
    Accurate_within_2_5ns = 0x1e, /**< higher than 2@.5 nanoseconds */
    Accurate_within_10ns  = 0x1f, /**< higher than 10 nanoseconds */
    Accurate_within_25ns  = 0x20, /**< higher than 25 nanoseconds */
    Accurate_within_100ns = 0x21, /**< higher than 100 nanoseconds */
    Accurate_within_250ns = 0x22, /**< higher than 250 nanoseconds */
    Accurate_within_1us   = 0x23, /**< higher than 1 microsecond */
    Accurate_within_2_5us = 0x24, /**< higher than 2@.5 microseconds */
    Accurate_within_10us  = 0x25, /**< higher than 10 microseconds */
    Accurate_within_25us  = 0x26, /**< higher than 25 microseconds */
    Accurate_within_100us = 0x27, /**< higher than 100 microseconds */
    Accurate_within_250us = 0x28, /**< higher than 250 microseconds */
    Accurate_within_1ms   = 0x29, /**< higher than 1 millisecond */
    Accurate_within_2_5ms = 0x2a, /**< higher than 2@.5 milliseconds */
    Accurate_within_10ms  = 0x2b, /**< higher than 10 milliseconds */
    Accurate_within_25ms  = 0x2c, /**< higher than 25 milliseconds */
    Accurate_within_100ms = 0x2d, /**< higher than 100 milliseconds */
    Accurate_within_250ms = 0x2e, /**< higher than 250 milliseconds */
    Accurate_within_1s    = 0x2f, /**< higher than 1 seconds */
    Accurate_within_10s   = 0x30, /**< higher than 10 seconds */
    Accurate_more_10s     = 0x31, /**< lower than 10 seconds */
    Accurate_Unknown      = 0xef, /**< Unknown */
};
/** Fault record severity code */
enum faultRecord_e : uint8_t {
    F_Emergency     = 0x00, /**< system is unusable */
    F_Alert         = 0x01, /**< immediate action needed */
    F_Critical      = 0x02, /**< critical conditions */
    F_Error         = 0x03, /**< error conditions */
    F_Warning       = 0x04, /**< warning conditions */
    F_Notice        = 0x05, /**< normal but significant condition */
    F_Informational = 0x06, /**< informational messages */
    F_Debug         = 0x07, /**< debug-level messages */
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
    HAND_SET            = 0x60, /**< Manually set */
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
    PRE_SOURCE   = 5, /**< Pre source */
    MASTER       = 6, /**< Source */
    SOURCE       = 6, /**< Source */
    PASSIVE      = 7, /**< Passive */
    UNCALIBRATED = 8, /**< Uncalibrated */
    SLAVE        = 9, /**< Client */
    CLIENT       = 9, /**< Client */
};
/** Specify Management TLV implementation-specific to use
 * @note: Ruby's wrapping, capitalize first letter of enumerators values
 */
enum implementSpecific_e {
    noImplementSpecific, /**< Do not use any implementation-specific */
    linuxptp,            /**< linuxptp project */
};
/** linuxptp timestamp
 *  Specify the underlaying Linux time stamps type that the daemon receive
 */
enum linuxptpTimeStamp_e : uint8_t {
    /** Using system clock based with an offset that is maintain by driver */
    TS_SOFTWARE,
    /** network interface have a PTP Hardware clock, and the
        driver read the value */
    TS_HARDWARE,
    /** Old hardware */
    TS_LEGACY_HW,
    /** One step PTP, the driver writes the time stamp into the frame */
    TS_ONESTEP,
    /** One step PTP plus insert time to PDelay_Resp */
    TS_P2P1STEP,
};
/** linuxptp Power Profile version
 *  Specify the which power system profile to use
 */
enum linuxptpPowerProfileVersion_e : uint16_t {
    /** Do not use power system */
    IEEE_C37_238_VERSION_NONE,
    /** Use IEEE C37.238-2011 profile */
    IEEE_C37_238_VERSION_2011,
    /** Use IEEE C37.238-2017 profile */
    IEEE_C37_238_VERSION_2017,
};
/** linuxptp client side unicast negotiation state */
enum linuxptpUnicastState_e : uint8_t {
    UC_WAIT, /**< Wait for answer */
    UC_HAVE_ANN, /**< Have answer */
    UC_NEED_SYDY, /**< Need to stand by */
    UC_HAVE_SYDY, /**< In stand by */
};
/** Clock time properties bit mask */
enum : uint8_t {
    /** The last minute of the current UTC day contains 61 seconds */
    F_LI_61 = (1 << 0),
    /** the last minute of the current UTC day contains 59 seconds */
    F_LI_59 = (1 << 1),
    F_UTCV  = (1 << 2), /**< Current UTC offset is valid */
    /** The timescale of the grand source PTP Instance is PTP */
    F_PTP   = (1 << 3),
    F_TTRA  = (1 << 4), /**< timescale is traceable to a primary reference */
    /** The frequency determining the timescale is
        traceable to a primary reference */
    F_FTRA  = (1 << 5),
};
/** PTP Time interval value */
struct TimeInterval_t {
    Integer64_t scaledNanoseconds; /**< nanoseconds * 2^16 */
    /**
     * Get object size
     * @return object size
     */
    static size_t size() { return sizeof(Integer64_t); }
    /**
     * Get interval from time interval in nanoseconds
     * @return scaled time interval in nanoseconds
     */
    double getInterval() const { return (double)scaledNanoseconds / 0x10000; }
    /**
     * Get interval from time interval in nanoseconds, trunc to integer
     * @return scaled time interval in nanoseconds
     */
    int64_t getIntervalInt() const { return scaledNanoseconds >> 16; }
};
/** PTP Time stamp */
struct Timestamp_t {
    UInteger48_t secondsField; /**< seconds */
    UInteger32_t nanosecondsField; /**< nanoseconds */
    /**
     * Get object size
     * @return object size
     */
    static size_t size() { return sizeof_UInteger48_t + sizeof(UInteger32_t); }
    /**
     * Convert to string
     * @return string
     */
    std::string string() const;
};
/** PTP clock ID */
struct ClockIdentity_t {
    Octet_t v[8]; /**< value */
    /**
     * Get object size
     * @return object size
     */
    static size_t size() { return sizeof(Octet_t) * 8; }
    /**
     * Convert to string
     * @return string
     */
    std::string string() const;
    /**
     * Clear clock ID and set all octets to val
     * @param[in] val value to set all octets
     */
    void clear(int val = 0) {memset(v, val, 8);}
    /**
     * Compare to another clock ID
     * @param[in] rhs another clock id
     * @return true if IDs are indentical
     */
    bool operator==(const ClockIdentity_t &rhs) const {
        return memcmp(rhs.v, v, size()) == 0;
    }
    /**
     * Compare to another clock ID
     * @param[in] rhs another clock id
     * @return true if IDs are indentical
     */
    bool eq(const ClockIdentity_t &rhs) const {
        return memcmp(rhs.v, v, size()) == 0;
    }
    /**
     * Compare to another clock ID
     * @param[in] rhs another clock id
     * @return true if ID is smaller
     */
    bool operator<(const ClockIdentity_t &rhs) const {
        return memcmp(rhs.v, v, size()) < 0;
    }
    /**
     * Compare to another clock ID
     * @param[in] rhs another clock id
     * @return true if ID is smaller
     */
    bool less(const ClockIdentity_t &rhs) const {
        return memcmp(rhs.v, v, size()) < 0;
    }
};
/** PTP port ID */
struct PortIdentity_t {
    ClockIdentity_t clockIdentity; /**< clock ID */
    UInteger16_t portNumber; /**< port number */
    /**
     * Get object size
     * @return object size
     */
    static size_t size() { return ClockIdentity_t::size() + sizeof(UInteger16_t); }
    /**
     * Convert to string
     * @return string
     */
    std::string string() const;
    /**
     * Clear port ID and set all octets to val
     */
    void clear() {clockIdentity.clear(); portNumber = 0;}
    /**
     * Compare to another port ID
     * @param[in] rhs another port id
     * @return true if IDs are indentical
     */
    bool operator==(const PortIdentity_t &rhs) const {
        return clockIdentity == rhs.clockIdentity && portNumber == rhs.portNumber;
    }
    /**
     * Compare to another port ID
     * @param[in] rhs another port id
     * @return true if IDs are indentical
     */
    bool eq(const PortIdentity_t &rhs) const {
        return clockIdentity == rhs.clockIdentity && portNumber == rhs.portNumber;
    }
    /**
     * Compare to another port ID
     * @param[in] rhs another port id
     * @return true if ID is smaller
     */
    bool operator<(const PortIdentity_t &rhs) const;
    /**
     * Compare to another port ID
     * @param[in] rhs another port id
     * @return true if ID is smaller
     */
    bool less(const PortIdentity_t &rhs) const { return *this < rhs; }
};
/** PTP port address */
struct PortAddress_t {
    networkProtocol_e networkProtocol; /**< network protocol */
    /**
     * address length
     * @note User can ignore the parameter.
     *  The user should use the length of addressField.
     *  The parameter is used during proccess.
     */
    UInteger16_t addressLength;
    Binary addressField; /**< binary from address */
    /**
     * Get object size
     * @return object size
     */
    size_t size() const {
        return sizeof(networkProtocol_e) + sizeof(UInteger16_t) +
            addressField.length();
    }
    /**
     * Convert to string
     * @return string
     */
    std::string string() const;
    /**
     * Compare to another port address
     * @param[in] rhs another port address
     * @return true if addresses are indentical
     */
    bool operator==(const PortAddress_t &rhs) const {
        return networkProtocol == rhs.networkProtocol &&
            addressField == rhs.addressField;
    }
    /**
     * Compare to another port address
     * @param[in] rhs another port address
     * @return true if addresses are indentical
     */
    bool eq(const PortAddress_t &rhs) const {
        return networkProtocol == rhs.networkProtocol &&
            addressField == rhs.addressField;
    }
    /**
     * Compare to another port address
     * @param[in] rhs another port address
     * @return true if address is smaller
     */
    bool operator<(const PortAddress_t &rhs) const;
    /**
     * Compare to another port address
     * @param[in] rhs another port address
     * @return true if address is smaller
     */
    bool less(const PortAddress_t &rhs) const { return *this < rhs; }
};
/** PTP clock quality */
struct ClockQuality_t {
    UInteger8_t clockClass; /**< clock class */
    clockAccuracy_e clockAccuracy; /**< clock accuracy */
    uint16_t offsetScaledLogVariance; /**< variance of the clock's phase */
    /**
     * Get object size
     * @return object size
     */
    static size_t size() {
        return sizeof(UInteger8_t) + sizeof(clockAccuracy_e) + sizeof(uint16_t);
    }
};
/** PTP text value */
struct PTPText_t {
    /**
     * string length
     * @note User can ignore the parameter.
     *  The user should use the length of textField.
     *  The parameter is used during proccess.
     */
    uint8_t lengthField;
    std::string textField; /**< string value */
    /**
     * Get object size
     * @return object size
     */
    size_t size() const { return sizeof(uint8_t) + textField.length(); }
    /**
     * Get string
     * @return pointer to string
     */
    const char *string() const { return textField.c_str(); }
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
    size_t size() const {
        return sizeof(uint16_t) + Timestamp_t::size() + sizeof(faultRecord_e) +
            faultName.size() + faultValue.size() + faultDescription.size();
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
    static size_t size() { return PortIdentity_t::size() + sizeof(uint8_t); }
};
/** Properties of a PTP management TLV */
struct ManagementId_t {
    uint16_t value;   /**< managementId value */
    uint8_t scope;   /**< Applies port or clock using scope_e */
    uint8_t allowed; /**< Allowed actions, bits from allowAction_e */
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
/** Fixed values to use in Management messages */
struct MsgParams {
    uint8_t transportSpecific; /**< transport specific */
    uint8_t domainNumber; /**< domain number */
    uint8_t boundaryHops; /**< boundary hops */
    bool isUnicast; /**< Mark message as unicast */
    implementSpecific_e implementSpecific; /**< Implementation-specific to use */
    PortIdentity_t target; /**< target port ID */
    PortIdentity_t self_id; /**< own port ID */
    bool useZeroGet; /**< send get with zero dataField */
    bool rcvSignaling; /**< parse signaling messages */
    bool filterSignaling; /**< filter signaling messages TLVs */
    /** when filter TLVs in signalling messages
     * allow TLVs that are in the map, the bool value is ignored */
    std::map<tlvType_e, bool> allowSigTlvs;
    void allowSigTlv(tlvType_e type); /**< Add TLV type to allowed signalling */
    void removeSigTlv(tlvType_e
        type); /**< Remove TLV type from allowed signalling */
    bool isSigTlv(tlvType_e type)
    const; /**< Query if TLV type is allowed signalling */
};
/** Base for all Management TLV structures */
struct BaseMngTlv {
};
/** Base for all Signaling TLV structures */
struct BaseSigTlv {
};
/** Master record in unicast master table */
struct LinuxptpUnicastMaster_t {
    PortIdentity_t portIdentity; /**< Master port ID */
    ClockQuality_t clockQuality; /**< Master clock quality */
    uint8_t selected; /**< Master is in use */
    linuxptpUnicastState_e portState; /**< State of master in unicast table */
    UInteger8_t priority1; /**< Master first priority */
    UInteger8_t priority2; /**< Master second priority */
    PortAddress_t portAddress; /**< Master port address */
    /**
     * Get object size
     * @return object size
     */
    size_t size() const {
        return PortIdentity_t::size() + ClockQuality_t::size() +
            sizeof(uint8_t) + sizeof(linuxptpUnicastState_e) + sizeof(UInteger8_t) +
            sizeof(UInteger8_t) + portAddress.size();
    }
};

#ifndef SWIG
}; /* namespace ptpmgmt */
#endif

#endif /* __PTPMGMT_TYPES_H */
