dnl SPDX-License-Identifier: LGPL-3.0-or-later
dnl SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com> */
dnl
dnl @file
dnl @brief Create types for main C++ library and for wrapper C
dnl
dnl @author Erez Geva <ErezGeva2@@gmail.com>
dnl @copyright © 2023 Erez Geva
dnl
dnl Create types for main C++ library and for wrapper C
dnl
include(lang().m4)dnl
/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
cpp_cod(` * @brief Types, enumerators, and structers used by PTP management messages')dnl
c_cod(` * @brief Types, enumerators, and structers used')dnl
c_cod(` *        by C interface to PTP management messages')dnl
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

ics(TYPES)

cpp_st()dnl
incpp(<map>)dnl
incpp("bin.h")dnl
inc("name.h")dnl
c_cod(`#ifdef __PTPMGMT_HAVE_SYS_TYPES_H')dnl
inc(<sys/types.h>)dnl
c_cod(`#else')dnl
c_cod(`/**< @cond internal */')dnl
c_cod(`typedef long ssize_t;')dnl
c_cod(`/**< @endcond */')dnl
c_cod(`#endif /* __PTPMGMT_HAVE_SYS_TYPES_H */')dnl
incb(mngIds)

ns_s()

#ifndef INT48_MIN
/** Minimum value for signed integer 48 bits */
cnst_st() int64_t INT48_MIN = -INT64_C(0x7fffffffffff) - 1;
#endif
#ifndef INT48_MAX
/** Maximum value for signed integer 48 bits */
cnst_st() int64_t INT48_MAX = INT64_C(0x7fffffffffff);
#endif
#ifndef UINT48_MAX
/** Maximum value for unsigned integer 48 bits */
cnst_st() uint64_t UINT48_MAX = UINT64_C(0xffffffffffff);
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
cnst_st() size_t nm(sizeof_UInteger48_t) = 6;
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
cnst_st() size_t nm(sizeof_Integer48_t) = 6;
/** IEEE 1588 Protocol signed 64 bits integer */
typedef int64_t Integer64_t;
/** IEEE 1588 protocol octet */
typedef uint8_t Octet_t;
/** IEEE Std 754 binary64 (64-bit double-precision floating-point format) */
typedef double Float64_t;
/** Float seconds used in Timestamp_t */
typedef long double float_seconds;
/** Float nano-seconds */
typedef long double float_nanoseconds;

/** Parsing and building errors */
enm(MNG_PARSE_ERROR_e) {
    /** No error */
    NM(MNG_PARSE_ERROR_OK),
    /** Error message */
    NM(MNG_PARSE_ERROR_MSG),
    /** Signalling message */
    NM(MNG_PARSE_ERROR_SIG),
    /** SMPTE message */
    NM(MNG_PARSE_ERROR_SMPTE),
    /** Invalid TLV mng id or action for TLV */
    NM(MNG_PARSE_ERROR_INVALID_ID),
    /** Wrong TLV header */
    NM(MNG_PARSE_ERROR_INVALID_TLV),
    /** Mismatch TLV */
    NM(MNG_PARSE_ERROR_MISMATCH_TLV),
    /** Size mismatch of field with length */
    NM(MNG_PARSE_ERROR_SIZE_MISS),
    /** Buffer is too small */
    NM(MNG_PARSE_ERROR_TOO_SMALL),
    /** Size is even */
    NM(MNG_PARSE_ERROR_SIZE),
    /** Value is out of range or invalid */
    NM(MNG_PARSE_ERROR_VAL),
    /** Wrong value in header */
    NM(MNG_PARSE_ERROR_HEADER),
    /** Wrong action value */
    NM(MNG_PARSE_ERROR_ACTION),
    /** Do not know how to parse the TLV data */
    NM(MNG_PARSE_ERROR_UNSUPPORT),
    /** Fail to allocate TLV data */
    NM(MNG_PARSE_ERROR_MEM),
    /** Authentication error */
    NM(MNG_PARSE_ERROR_AUTH),
    /** Received without any Authentication */
    NM(MNG_PARSE_ERROR_AUTH_NONE),
    /** Received Authentication with wrong ICV */
    NM(MNG_PARSE_ERROR_AUTH_WRONG),
    /** Received Authentication with unkown key */
    NM(MNG_PARSE_ERROR_AUTH_NOKEY),
};
/** PTP messages type
 * @note: 4 bits
 */
enm(msgType_e) sz(: Nibble_t) {
    /* Event messages */
    /** Synchronization event message */
    nm(Sync)                    = 0,
    /** Delay request event message */
    nm(Delay_Req)               = 1,
    /** Peer delay request event message */
    nm(Pdelay_Req)              = 2,
    /** Peer delay response event message */
    nm(Pdelay_Resp)             = 3,
    /* General messages */
    /** Follow up message */
    nm(Follow_Up)               = 0x8,
    /** Delay response */
    nm(Delay_Resp)              = 0x9,
    /** Peer delay response follow up message */
    nm(Pdelay_Resp_Follow_Up)   = 0xa,
    /** Announce message*/
    nm(Announce)                = 0xb,
    /** Signalling message */
    nm(Signaling)               = 0xc,
    /** Management message */
    nm(Management)              = 0xd,
};
/**
 * PTP messages TLV types
 * @note: With new signalling TLVs from "IEEE Std 1588-2019@"
 */
enm(tlvType_e) sz(: uint16_t) {
    /** Management TLV */
    NM(MANAGEMENT)                              = 0x0001,
    /** Management Error TLV */
    NM(MANAGEMENT_ERROR_STATUS)                 = 0x0002,
    /** Organization extension */
    NM(ORGANIZATION_EXTENSION)                  = 0x0003,
    /** Request unicast */
    NM(REQUEST_UNICAST_TRANSMISSION)            = 0x0004,
    /** Grant unicast */
    NM(GRANT_UNICAST_TRANSMISSION)              = 0x0005,
    /** Cancel unicast */
    NM(CANCEL_UNICAST_TRANSMISSION)             = 0x0006,
    /** Ack cancel unicast */
    NM(ACKNOWLEDGE_CANCEL_UNICAST_TRANSMISSION) = 0x0007,
    /** Path trace */
    NM(PATH_TRACE)                              = 0x0008,
    /** Alternate time offset */
    NM(ALTERNATE_TIME_OFFSET_INDICATOR)         = 0x0009,
    /* Obsolete valuse 2000, 2001, 2002, 2003 */
    /** Organization extension */
    NM(ORGANIZATION_EXTENSION_PROPAGATE)        = 0x4000,
    /** Enhanced accuracy */
    NM(ENHANCED_ACCURACY_METRICS)               = 0x4001,
    /** Organization extension */
    NM(ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE) = 0x8000,
    /** Layer 1 synchronization */
    NM(L1_SYNC)                                 = 0x8001,
    /** Port communication */
    NM(PORT_COMMUNICATION_AVAILABILITY)         = 0x8002,
    /** Protocol address */
    NM(PROTOCOL_ADDRESS)                        = 0x8003,
    /** TimeReceiver RX sync time */
    NM(SLAVE_RX_SYNC_TIMING_DATA)               = 0x8004,
    /** TimeReceiver RX sync */
    NM(SLAVE_RX_SYNC_COMPUTED_DATA)             = 0x8005,
    /** TimeReceiver TX event */
    NM(SLAVE_TX_EVENT_TIMESTAMPS)               = 0x8006,
    /** Cumulative rate */
    NM(CUMULATIVE_RATE_RATIO)                   = 0x8007,
    /** Padding TLV, ignored */
    NM(TLV_PAD)                                 = 0x8008,
    /** Authentication */
    NM(AUTHENTICATION)                          = 0x8009,
    /** TimeReceiver delay time
     * note: linuxptp Experimental value */
    NM(SLAVE_DELAY_TIMING_DATA_NP)              = 0x7f00,
};
/** PTP management action */
enm(actionField_e) sz(: uint8_t) {
    NM(GET) = 0,            /**< Send a get message */
    NM(SET) = 1,            /**< Send a set message */
    NM(RESPONSE) = 2,       /**< Receive response for a get or a set message */
    NM(COMMAND) = 3,        /**< Send command message */
    NM(ACKNOWLEDGE) = 4,    /**< Receive acknowledge on command */
};
/** PTP Management Error IDs */
enm(managementErrorId_e) sz(: uint16_t) {
    NM(RESPONSE_TOO_BIG) = 0x0001, /**< Response is too big */
    NM(NO_SUCH_ID)       = 0x0002, /**< No such id */
    NM(WRONG_LENGTH)     = 0x0003, /**< Wrong length */
    NM(WRONG_VALUE)      = 0x0004, /**< Wrong value */
    NM(NOT_SETABLE)      = 0x0005, /**< Not setable */
    NM(NOT_SUPPORTED)    = 0x0006, /**< Not supported */
    NM(GENERAL_ERROR)    = 0xfffe, /**< General error */
};
/** PTP clock type bit mask
 * @details A PTP clock could act as more than a single type
 * @note: Ruby's wrapping, capitalize first letter of enumerators values
 */
enm(clockType_e) sz(: uint16_t) {
    nm(ordinaryClock)       = 0x8000, /**< ordinary clock */
    nm(boundaryClock)       = 0x4000, /**< boundary clock */
    nm(p2pTransparentClock) = 0x2000, /**< peer-to-peer transparent clock */
    nm(e2eTransparentClock) = 0x1000, /**< end-to-end transparent clock */
    nm(managementClock)     = 0x0800, /**< management node (deprecated) */
};
/** PTP using a network layer */
enm(networkProtocol_e) sz(: uint16_t) {
    nm(UDP_IPv4)   = 1, /**< UDP over IP version 4 */
    nm(UDP_IPv6)   = 2, /**< UDP over IP version 6 */
    nm(IEEE_802_3) = 3, /**< Raw Ethernet using IEEE 802@.3 */
    nm(DeviceNet)  = 4, /**< DeviceNet */
    nm(ControlNet) = 5, /**< ControlNet */
    nm(PROFINET)   = 6, /**< PROFINET */
};
/** PTP clock accuracy */
enm(clockAccuracy_e) sz(: uint8_t) {
    nm(Accurate_within_1ps)   = 0x17, /**< higher than 1 picosecond */
    nm(Accurate_within_2_5ps) = 0x18, /**< higher than 2@.5 picoseconds */
    nm(Accurate_within_10ps)  = 0x19, /**< higher than 10 picoseconds */
    nm(Accurate_within_25ps)  = 0x1a, /**< higher than 25 picoseconds */
    nm(Accurate_within_100ps) = 0x1b, /**< higher than 100 picoseconds */
    nm(Accurate_within_250ps) = 0x1c, /**< higher than 250 picoseconds */
    nm(Accurate_within_1ns)   = 0x1d, /**< higher than 1 nanosecond */
    nm(Accurate_within_2_5ns) = 0x1e, /**< higher than 2@.5 nanoseconds */
    nm(Accurate_within_10ns)  = 0x1f, /**< higher than 10 nanoseconds */
    nm(Accurate_within_25ns)  = 0x20, /**< higher than 25 nanoseconds */
    nm(Accurate_within_100ns) = 0x21, /**< higher than 100 nanoseconds */
    nm(Accurate_within_250ns) = 0x22, /**< higher than 250 nanoseconds */
    nm(Accurate_within_1us)   = 0x23, /**< higher than 1 microsecond */
    nm(Accurate_within_2_5us) = 0x24, /**< higher than 2@.5 microseconds */
    nm(Accurate_within_10us)  = 0x25, /**< higher than 10 microseconds */
    nm(Accurate_within_25us)  = 0x26, /**< higher than 25 microseconds */
    nm(Accurate_within_100us) = 0x27, /**< higher than 100 microseconds */
    nm(Accurate_within_250us) = 0x28, /**< higher than 250 microseconds */
    nm(Accurate_within_1ms)   = 0x29, /**< higher than 1 millisecond */
    nm(Accurate_within_2_5ms) = 0x2a, /**< higher than 2@.5 milliseconds */
    nm(Accurate_within_10ms)  = 0x2b, /**< higher than 10 milliseconds */
    nm(Accurate_within_25ms)  = 0x2c, /**< higher than 25 milliseconds */
    nm(Accurate_within_100ms) = 0x2d, /**< higher than 100 milliseconds */
    nm(Accurate_within_250ms) = 0x2e, /**< higher than 250 milliseconds */
    nm(Accurate_within_1s)    = 0x2f, /**< higher than 1 seconds */
    nm(Accurate_within_10s)   = 0x30, /**< higher than 10 seconds */
    nm(Accurate_more_10s)     = 0x31, /**< lower than 10 seconds */
    nm(Accurate_Unknown)      = 0xfe, /**< Unknown */
};
/** Fault record severity code */
enm(faultRecord_e) sz(: uint8_t) {
    nm(F_Emergency)     = 0x00, /**< system is unusable */
    nm(F_Alert)         = 0x01, /**< immediate action needed */
    nm(F_Critical)      = 0x02, /**< critical conditions */
    nm(F_Error)         = 0x03, /**< error conditions */
    nm(F_Warning)       = 0x04, /**< warning conditions */
    nm(F_Notice)        = 0x05, /**< normal but significant condition */
    nm(F_Informational) = 0x06, /**< informational messages */
    nm(F_Debug)         = 0x07, /**< debug-level messages */
};
/** Time source type */
enm(timeSource_e) sz(: uint8_t) {
    NM(ATOMIC_CLOCK)        = 0x10, /**< Atomic clock */
    NM(GNSS)                = 0x20, /**< Satellite navigation */
    NM(GPS)                 = 0x20, /**< Satellite navigation */
    NM(TERRESTRIAL_RADIO)   = 0x30, /**< Terrestrial radio */
    NM(SERIAL_TIME_CODE)    = 0x39, /**< Serial time code */
    NM(PTP)                 = 0x40, /**< PTP of a different domain */
    NM(NTP)                 = 0x50, /**< IP Network time protocol */
    NM(HAND_SET)            = 0x60, /**< Manually set */
    NM(OTHER)               = 0x90, /**< Other */
    NM(INTERNAL_OSCILLATOR) = 0xa0, /**< Internal oscillator */
};
/** Port state */
enm(portState_e) sz(: uint8_t) {
    NM(INITIALIZING)         = 1, /**< Initializing */
    NM(FAULTY)               = 2, /**< Faulty */
    NM(DISABLED)             = 3, /**< Disabled */
    NM(LISTENING)            = 4, /**< Listening */
    NM(PRE_MASTER)           = 5, /**< Pre timeTransmitter */
    NM(PRE_TIME_TRANSMITTER) = 5, /**< Pre timeTransmitter */
    NM(MASTER)               = 6, /**< TimeTransmitter */
    NM(TIME_TRANSMITTER)     = 6, /**< TimeTransmitter */
    NM(PASSIVE)              = 7, /**< Passive */
    NM(UNCALIBRATED)         = 8, /**< Uncalibrated */
    NM(SLAVE)                = 9, /**< TimeReceiver */
    NM(TIME_RECEIVER)        = 9, /**< TimeReceiver */
};
/** Delay mechanizem */
enm(delayMechanism_e) sz(: uint8_t) {
    /**
     *  Automatic probing
     *  @note LinuxPTP value
     */
    NM(AUTO)          = 0,
    /** The PTP Port is configured to use the delay request-response mechanism. */
    NM(E2E)           = 1,
    /** The PTP Port is configured to use the peer-to-peer delay mechanism. */
    NM(P2P)           = 2,
    /** The PTP Port does not implement the delay mechanism. */
    NM(NO_MECHANISM)  = 0xfe,
    /**
     *  The PTP Port is configured to use the
     *  Common Mean Link Delay Service option.
     */
    NM(COMMON_P2P)    = 3,
    /** Special Ports do not use either delay mechanism. */
    NM(SPECIAL)       = 4,
};
/** Specify Management TLV implementation-specific to use
 * @note: Ruby's wrapping, capitalize first letter of enumerators values
 */
enm(implementSpecific_e) {
    nm(noImplementSpecific), /**< Do not use any implementation-specific */
    nm(linuxptp),            /**< linuxptp project */
};
/** SMPTE master clock locking status */
enm(SMPTEmasterLockingStatus_e) sz(: uint8_t) {
    NM(SMPTE_NOT_IN_USE)   = 0, /**< Not in use */
    NM(SMPTE_FREE_RUN)     = 1, /**< Free Run */
    /**
     * In response to a disturbance, the grandmaster is re-locking quickly.
     * In this situation, a rapid phase adjustment with a time discontinuity
     * can be expected.
     */
    NM(SMPTE_COLD_LOCKING) = 2,
    /**
     * In response to a disturbance, the grandmaster is re-locking slowly
     * by means of a frequency adjustment, with no phase discontinuity.
     * Time continuity is maintained.
     */
    NM(SMPTE_WARM_LOCKING) = 3,
    NM(SMPTE_LOCKED)       = 4, /**< Locked, in normal operation and stable. */
};
/** linuxptp timestamp
 *  Specify the underlaying Linux time stamps type that the daemon receive
 */
enm(linuxptpTimeStamp_e) sz(: uint8_t) {
    /** Using system clock based with an offset that is maintain by driver */
    NM(TS_SOFTWARE),
    /** network interface have a PTP Hardware clock, and the
        driver read the value */
    NM(TS_HARDWARE),
    /** Old hardware */
    NM(TS_LEGACY_HW),
    /** One step PTP, the driver writes the time stamp into the frame */
    NM(TS_ONESTEP),
    /** One step PTP plus insert time to PDelay_Resp */
    NM(TS_P2P1STEP),
};
/** linuxptp Power Profile version
 *  Specify the which power system profile to use
 */
enm(linuxptpPowerProfileVersion_e) sz(: uint16_t) {
    /** Do not use power system */
    NM(IEEE_C37_238_VERSION_NONE),
    /** Use IEEE C37.238-2011 profile */
    NM(IEEE_C37_238_VERSION_2011),
    /** Use IEEE C37.238-2017 profile */
    NM(IEEE_C37_238_VERSION_2017),
};
/** linuxptp timeReceiver side unicast negotiation state */
enm(linuxptpUnicastState_e) sz(: uint8_t) {
    NM(UC_WAIT), /**< Wait for answer */
    NM(UC_HAVE_ANN), /**< Have answer */
    NM(UC_NEED_SYDY), /**< Need to stand by */
    NM(UC_HAVE_SYDY), /**< In stand by */
};
/** Clock time properties bit mask */
enum sz(: uint8_t) {
    /** The last minute of the current UTC day contains 61 seconds */
    NM(F_LI_61) = (1 << 0),
    /** the last minute of the current UTC day contains 59 seconds */
    NM(F_LI_59) = (1 << 1),
    /** Current UTC offset is valid */
    NM(F_UTCV)  = (1 << 2),
    /** The timescale of the grandmaster PTP Instance is PTP */
    NM(F_PTP)   = (1 << 3),
    /** timescale is traceable to a primary reference */
    NM(F_TTRA)  = (1 << 4),
    /** The frequency determining the timescale is
        traceable to a primary reference */
    NM(F_FTRA)  = (1 << 5),
};
/** PTP Time interval value */
strc(TimeInterval_t) {
    Integer64_t scaledNanoseconds; /**< nanoseconds * 2^16 */
cpp_cod(`    /**')dnl
cpp_cod(`     * Get object size')dnl
cpp_cod(`     * @return object size')dnl
cpp_cod(`     */')dnl
cpp_cod(`    static size_t size() { return sizeof scaledNanoseconds; }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Get interval from time interval in nanoseconds')dnl
cpp_cod(`     * @return scaled time interval in nanoseconds')dnl
cpp_cod(`     */')dnl
cpp_cod(`    float_nanoseconds getInterval() const {')dnl
cpp_cod(`        return (float_nanoseconds)scaledNanoseconds / 0x10000;')dnl
cpp_cod(`    }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Get interval from time interval in nanoseconds, trunc to integer')dnl
cpp_cod(`     * @return scaled time interval in nanoseconds')dnl
cpp_cod(`     */')dnl
cpp_cod(`    int64_t getIntervalInt() const;')dnl
};
/** PTP Time stamp */
strc(Timestamp_t) {
    UInteger48_t secondsField; /**< seconds */
    UInteger32_t nanosecondsField; /**< nanoseconds */
cpp_cod(`    /**')dnl
cpp_cod(`     * Default constructor')dnl
cpp_cod(`     */')dnl
cpp_cod(`    Timestamp_t() : secondsField(0), nanosecondsField(0) {}')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Constructor')dnl
cpp_cod(`     * @param[in] secs Seconds')dnl
cpp_cod(`     * @param[in] nsecs Nanoseconds')dnl
cpp_cod(`     */')dnl
cpp_cod(`    Timestamp_t(int64_t secs, uint32_t nsecs) : secondsField(secs),')dnl
cpp_cod(`        nanosecondsField(nsecs) {}')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Get object size')dnl
cpp_cod(`     * @return object size')dnl
cpp_cod(`     */')dnl
cpp_cod(`    static size_t size() { return sizeof_UInteger48_t + sizeof nanosecondsField; }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Convert to string')dnl
cpp_cod(`     * @return string')dnl
cpp_cod(`     */')dnl
cpp_cod(`    std::string string() const;')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Convert to string of seconds with fractions')dnl
cpp_cod(`     * @note scripts can use the string() method')dnl
cpp_cod(`     */')dnl
cpp_cod(`    operator std::string() const { return string(); }')dnl
cpp_cod(`')dnl
cpp_cod(`    #ifndef SWIG /* standard C++ structures converting */')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Convert from timespec')dnl
cpp_cod(`     * @param[in] ts timespec structure')dnl
cpp_cod(`     * @note scripts should not use the timespec structure')dnl
cpp_cod(`     */')dnl
cpp_cod(`    Timestamp_t(const timespec &ts) {')dnl
cpp_cod(`        secondsField = ts.tv_sec;')dnl
cpp_cod(`        nanosecondsField = ts.tv_nsec;')dnl
cpp_cod(`    }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Convert to timespec')dnl
cpp_cod(`     * @note scripts should not use the timespec structure')dnl
cpp_cod(`     */')dnl
cpp_cod(`    operator timespec() const { timespec ts; toTimespec(ts); return ts; }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Convert to timespec')dnl
cpp_cod(`     * @param[in, out] ts timespec structure')dnl
cpp_cod(`     * @note scripts should not use the timespec structure')dnl
cpp_cod(`     */')dnl
cpp_cod(`    void toTimespec(timespec &ts) const {')dnl
cpp_cod(`        ts.tv_sec = secondsField;')dnl
cpp_cod(`        ts.tv_nsec = nanosecondsField;')dnl
cpp_cod(`    }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Convert from timeval')dnl
cpp_cod(`     * @param[in] tv timeval structure')dnl
cpp_cod(`     * @note scripts should not use the timeval structure')dnl
cpp_cod(`     */')dnl
cpp_cod(`    Timestamp_t(const timeval &tv);')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Convert to timespec')dnl
cpp_cod(`     * @note scripts should not use the timeval structure')dnl
cpp_cod(`     * @note Trunc nanosecods to microseconds')dnl
cpp_cod(`     *       Could result zero microseconds from a small nanosecods value')dnl
cpp_cod(`     */')dnl
cpp_cod(`    operator timeval() const { timeval tv; toTimeval(tv); return tv; }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Convert to timeval')dnl
cpp_cod(`     * @param[in, out] tv timeval structure')dnl
cpp_cod(`     * @note scripts should not use the timeval structure')dnl
cpp_cod(`     * @note Trunc nanosecods to microseconds')dnl
cpp_cod(`     *       Could result zero microseconds from a small nanosecods value')dnl
cpp_cod(`     */')dnl
cpp_cod(`    void toTimeval(timeval &tv) const;')dnl
cpp_cod(`    #endif /* SWIG */')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Convert from seconds with fractions')dnl
cpp_cod(`     * @param[in] seconds with fractions')dnl
cpp_cod(`     */')dnl
cpp_cod(`    Timestamp_t(float_seconds seconds) {fromFloat(seconds);}')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Convert from seconds with fractions')dnl
cpp_cod(`     * @param[in] seconds with fractions')dnl
cpp_cod(`     */')dnl
cpp_cod(`    void fromFloat(float_seconds seconds);')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Convert to seconds with fractions')dnl
cpp_cod(`     * @note scripts can use the toFloat() method')dnl
cpp_cod(`     */')dnl
cpp_cod(`    operator float_seconds() const { return toFloat(); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Convert to seconds with fractions')dnl
cpp_cod(`     * @return seconds with fractions')dnl
cpp_cod(`     */')dnl
cpp_cod(`    float_seconds toFloat() const;')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Convert from nanoseconds')dnl
cpp_cod(`     * @param[in] nanoseconds')dnl
cpp_cod(`     */')dnl
cpp_cod(`    void fromNanoseconds(uint64_t nanoseconds);')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Convert to nanoseconds')dnl
cpp_cod(`     * @return nanoseconds')dnl
cpp_cod(`     */')dnl
cpp_cod(`    uint64_t toNanoseconds() const;')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare to another clock time')dnl
cpp_cod(`     * @param[in] ts another clock time')dnl
cpp_cod(`     * @return true if the same time')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool operator==(const Timestamp_t &ts) const { return eq(ts); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare to another clock time')dnl
cpp_cod(`     * @param[in] ts another clock time')dnl
cpp_cod(`     * @return true if the same time')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool eq(const Timestamp_t &ts) const {')dnl
cpp_cod(`        return secondsField == ts.secondsField &&')dnl
cpp_cod(`            nanosecondsField == ts.nanosecondsField;')dnl
cpp_cod(`    }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare to seconds with fractions')dnl
cpp_cod(`     * @param[in] seconds')dnl
cpp_cod(`     * @return true if the same time')dnl
cpp_cod(`     * @note due to pressision, the compare use +-1 nanosecond of value')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool operator==(float_seconds seconds) const { return eq(seconds); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare to seconds with fractions')dnl
cpp_cod(`     * @param[in] seconds')dnl
cpp_cod(`     * @return true if the same time')dnl
cpp_cod(`     * @note due to pressision, the compare use +-1 nanosecond of value')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool eq(float_seconds seconds) const;')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare to another clock time')dnl
cpp_cod(`     * @param[in] ts another clock time')dnl
cpp_cod(`     * @return true if smaller then other time')dnl
cpp_cod(`     * @note when compare to a number,')dnl
cpp_cod(`     *       the number will be converted as seconds with fractions')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool operator<(const Timestamp_t &ts) const { return less(ts); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare to another clock time')dnl
cpp_cod(`     * @param[in] ts another clock time')dnl
cpp_cod(`     * @return true if smaller then other time')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool less(const Timestamp_t &ts) const {')dnl
cpp_cod(`        return secondsField < ts.secondsField ||')dnl
cpp_cod(`            (secondsField == ts.secondsField &&')dnl
cpp_cod(`                nanosecondsField < ts.nanosecondsField);')dnl
cpp_cod(`    }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare to seconds with fractions')dnl
cpp_cod(`     * @param[in] seconds')dnl
cpp_cod(`     * @return true if smaller')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool operator<(float_seconds seconds) const { return less(seconds); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare to seconds with fractions')dnl
cpp_cod(`     * @param[in] seconds')dnl
cpp_cod(`     * @return true if smaller')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool less(float_seconds seconds) const;')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Add another clock time')dnl
cpp_cod(`     * @param[in] ts another clock time')dnl
cpp_cod(`     * @return reference to itself')dnl
cpp_cod(`     */')dnl
cpp_cod(`    Timestamp_t &operator+(const Timestamp_t &ts) { return add(ts); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Add another clock time')dnl
cpp_cod(`     * @param[in] ts another clock time')dnl
cpp_cod(`     * @return reference to itself')dnl
cpp_cod(`     */')dnl
cpp_cod(`    Timestamp_t &operator+=(const Timestamp_t &ts) { return add(ts); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Add another clock time')dnl
cpp_cod(`     * @param[in] ts another clock time')dnl
cpp_cod(`     * @return reference to itself')dnl
cpp_cod(`     */')dnl
cpp_cod(`    Timestamp_t &add(const Timestamp_t &ts);')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Add a seconds with fractions')dnl
cpp_cod(`     * @param[in] seconds')dnl
cpp_cod(`     * @return reference to itself')dnl
cpp_cod(`     */')dnl
cpp_cod(`    Timestamp_t &operator+(float_seconds seconds) { return add(seconds); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Add a seconds with fractions')dnl
cpp_cod(`     * @param[in] seconds')dnl
cpp_cod(`     * @return reference to itself')dnl
cpp_cod(`     */')dnl
cpp_cod(`    Timestamp_t &operator+=(float_seconds seconds) { return add(seconds); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Add a seconds with fractions')dnl
cpp_cod(`     * @param[in] seconds')dnl
cpp_cod(`     * @return reference to itself')dnl
cpp_cod(`     */')dnl
cpp_cod(`    Timestamp_t &add(float_seconds seconds);')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Subtract another clock time')dnl
cpp_cod(`     * @param[in] ts another clock time')dnl
cpp_cod(`     * @return reference to itself')dnl
cpp_cod(`     */')dnl
cpp_cod(`    Timestamp_t &operator-(const Timestamp_t &ts) { return subt(ts); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Subtract another clock time')dnl
cpp_cod(`     * @param[in] ts another clock time')dnl
cpp_cod(`     * @return reference to itself')dnl
cpp_cod(`     */')dnl
cpp_cod(`    Timestamp_t &operator-=(const Timestamp_t &ts) { return subt(ts); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Subtract another clock time')dnl
cpp_cod(`     * @param[in] ts another clock time')dnl
cpp_cod(`     * @return reference to itself')dnl
cpp_cod(`     */')dnl
cpp_cod(`    Timestamp_t &subt(const Timestamp_t &ts);')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Subtract seconds with fractions')dnl
cpp_cod(`     * @param[in] seconds')dnl
cpp_cod(`     * @return reference to itself')dnl
cpp_cod(`     */')dnl
cpp_cod(`    Timestamp_t &operator-(float_seconds seconds) { return add(-seconds); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Subtract seconds with fractions')dnl
cpp_cod(`     * @param[in] seconds')dnl
cpp_cod(`     * @return reference to itself')dnl
cpp_cod(`     */')dnl
cpp_cod(`    Timestamp_t &operator-=(float_seconds seconds) { return add(-seconds); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Subtract seconds with fractions')dnl
cpp_cod(`     * @param[in] seconds')dnl
cpp_cod(`     * @return reference to itself')dnl
cpp_cod(`     */')dnl
cpp_cod(`    Timestamp_t &subt(float_seconds seconds) { return add(-seconds); }')dnl
};
/** PTP clock ID */
strc(ClockIdentity_t) {
    Octet_t v[8]; /**< value */
cpp_cod(`    /**')dnl
cpp_cod(`     * Get object size')dnl
cpp_cod(`     * @return object size')dnl
cpp_cod(`     */')dnl
cpp_cod(`    static size_t size() { return sizeof v; }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Convert to string')dnl
cpp_cod(`     * @return string')dnl
cpp_cod(`     */')dnl
cpp_cod(`    std::string string() const;')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Clear clock ID and set all octets to val')dnl
cpp_cod(`     * @param[in] val value to set all octets')dnl
cpp_cod(`     */')dnl
cpp_cod(`    void clear(int val = 0) {memset(v, val, 8);}')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare to another clock ID')dnl
cpp_cod(`     * @param[in] rhs another clock id')dnl
cpp_cod(`     * @return true if IDs are indentical')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool operator==(const ClockIdentity_t &rhs) const { return eq(rhs); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare to another clock ID')dnl
cpp_cod(`     * @param[in] rhs another clock id')dnl
cpp_cod(`     * @return true if IDs are indentical')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool eq(const ClockIdentity_t &rhs) const {')dnl
cpp_cod(`        return memcmp(v, rhs.v, size()) == 0;')dnl
cpp_cod(`    }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare to another clock ID')dnl
cpp_cod(`     * @param[in] rhs another clock id')dnl
cpp_cod(`     * @return true if ID is smaller')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool operator<(const ClockIdentity_t &rhs) const { return less(rhs); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare to another clock ID')dnl
cpp_cod(`     * @param[in] rhs another clock id')dnl
cpp_cod(`     * @return true if ID is smaller')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool less(const ClockIdentity_t &rhs) const {')dnl
cpp_cod(`        return memcmp(v, rhs.v, size()) < 0;')dnl
cpp_cod(`    }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare equal with a binary value')dnl
cpp_cod(`     * @param[in] bin another clock id')dnl
cpp_cod(`     * @return true if binary is equal')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool operator==(const Binary &bin) const { return eq(bin); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare equal with a binary value')dnl
cpp_cod(`     * @param[in] bin another clock id')dnl
cpp_cod(`     * @return true if binary is equal')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool eq(const Binary &bin) const {')dnl
cpp_cod(`        return bin.size() == size() &&')dnl
cpp_cod(`            memcmp(v, bin.get(), size()) == 0;')dnl
cpp_cod(`    }')dnl
};
/** PTP port ID */
strc(PortIdentity_t) {
    strcc(ClockIdentity_t) clockIdentity; /**< clock ID */
    UInteger16_t portNumber; /**< port number */
cpp_cod(`    /**')dnl
cpp_cod(`     * Get object size')dnl
cpp_cod(`     * @return object size')dnl
cpp_cod(`     */')dnl
cpp_cod(`    static size_t size() { return ClockIdentity_t::size() + sizeof portNumber; }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Convert to string')dnl
cpp_cod(`     * @return string')dnl
cpp_cod(`     */')dnl
cpp_cod(`    std::string string() const;')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Clear port ID and set all octets to val')dnl
cpp_cod(`     */')dnl
cpp_cod(`    void clear() {clockIdentity.clear(); portNumber = 0;}')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare to another port ID')dnl
cpp_cod(`     * @param[in] rhs another port id')dnl
cpp_cod(`     * @return true if IDs are indentical')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool operator==(const PortIdentity_t &rhs) const { return eq(rhs); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare to another port ID')dnl
cpp_cod(`     * @param[in] rhs another port id')dnl
cpp_cod(`     * @return true if IDs are indentical')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool eq(const PortIdentity_t &rhs) const {')dnl
cpp_cod(`        return clockIdentity == rhs.clockIdentity && portNumber == rhs.portNumber;')dnl
cpp_cod(`    }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare to another port ID')dnl
cpp_cod(`     * @param[in] rhs another port id')dnl
cpp_cod(`     * @return true if ID is smaller')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool operator<(const PortIdentity_t &rhs) const { return less(rhs); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare to another port ID')dnl
cpp_cod(`     * @param[in] rhs another port id')dnl
cpp_cod(`     * @return true if ID is smaller')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool less(const PortIdentity_t &rhs) const;')dnl
};
/** PTP port address */
strc(PortAddress_t) {
    enmc(networkProtocol_e) networkProtocol; /**< network protocol */
    /**
     * address length
     * @note User can ignore the parameter.
     *  The user should use the length of addressField.
     *  The parameter is used during process.
     */
    UInteger16_t addressLength;
    bintyp()addressField; /**< binary from address */
cpp_cod(`    /**')dnl
cpp_cod(`     * Get object size')dnl
cpp_cod(`     * @return object size')dnl
cpp_cod(`     */')dnl
cpp_cod(`    size_t size() const {')dnl
cpp_cod(`        return sizeof networkProtocol + sizeof addressLength +')dnl
cpp_cod(`            addressField.length();')dnl
cpp_cod(`    }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Convert to string')dnl
cpp_cod(`     * @return string')dnl
cpp_cod(`     */')dnl
cpp_cod(`    std::string string() const;')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare to another port address')dnl
cpp_cod(`     * @param[in] rhs another port address')dnl
cpp_cod(`     * @return true if addresses are indentical')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool operator==(const PortAddress_t &rhs) const { return eq(rhs); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare to another port address')dnl
cpp_cod(`     * @param[in] rhs another port address')dnl
cpp_cod(`     * @return true if addresses are indentical')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool eq(const PortAddress_t &rhs) const {')dnl
cpp_cod(`        return networkProtocol == rhs.networkProtocol &&')dnl
cpp_cod(`            addressField == rhs.addressField;')dnl
cpp_cod(`    }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare to another port address')dnl
cpp_cod(`     * @param[in] rhs another port address')dnl
cpp_cod(`     * @return true if address is smaller')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool operator<(const PortAddress_t &rhs) const { return less(rhs); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Compare to another port address')dnl
cpp_cod(`     * @param[in] rhs another port address')dnl
cpp_cod(`     * @return true if address is smaller')dnl
cpp_cod(`     */')dnl
cpp_cod(`    bool less(const PortAddress_t &rhs) const;')dnl
};
/** PTP clock quality */
strc(ClockQuality_t) {
    UInteger8_t clockClass; /**< clock class */
    enmc(clockAccuracy_e) clockAccuracy; /**< clock accuracy */
    uint16_t offsetScaledLogVariance; /**< variance of the clock's phase */
cpp_cod(`    /**')dnl
cpp_cod(`     * Get object size')dnl
cpp_cod(`     * @return object size')dnl
cpp_cod(`     */')dnl
cpp_cod(`    static size_t size() {')dnl
cpp_cod(`        return sizeof clockClass + sizeof clockAccuracy +')dnl
cpp_cod(`            sizeof offsetScaledLogVariance;')dnl
cpp_cod(`    }')dnl
};
/** PTP text value */
strc(PTPText_t) {
    /**
     * string length
     * @note User can ignore the parameter.
     *  The user should use the length of textField.
     *  The parameter is used during process.
     */
    uint8_t lengthField;
    strtyp()textField; /**< string value */
cpp_cod(`    /**')dnl
cpp_cod(`     * Get object size')dnl
cpp_cod(`     * @return object size')dnl
cpp_cod(`     */')dnl
cpp_cod(`    size_t size() const { return sizeof lengthField + textField.length(); }')dnl
cpp_cod(`    /**')dnl
cpp_cod(`     * Get string')dnl
cpp_cod(`     * @return pointer to string')dnl
cpp_cod(`     */')dnl
cpp_cod(`    const char *string() const { return textField.c_str(); }')dnl
};
/** PTP fault record */
strc(FaultRecord_t) {
    uint16_t faultRecordLength; /**< record length */
    strcc(Timestamp_t) faultTime; /**< time stamp of fault */
    enmc(faultRecord_e) severityCode; /**< severity code */
    strcc(PTPText_t) faultName; /**< name */
    strcc(PTPText_t) faultValue; /**< value */
    strcc(PTPText_t) faultDescription; /**< description */
cpp_cod(`    /**')dnl
cpp_cod(`     * Get object size')dnl
cpp_cod(`     * @return object size')dnl
cpp_cod(`     */')dnl
cpp_cod(`    size_t size() const {')dnl
cpp_cod(`        return sizeof faultRecordLength + faultTime.size() + sizeof severityCode +')dnl
cpp_cod(`            faultName.size() + faultValue.size() + faultDescription.size();')dnl
cpp_cod(`    }')dnl
};
/** PTP Acceptable timeTransmitter */
strc(AcceptableMaster_t) {
    strcc(PortIdentity_t) acceptablePortIdentity; /**< acceptable port ID */
    uint8_t alternatePriority1; /**< alternate priority 1 */
cpp_cod(`    /**')dnl
cpp_cod(`     * Get object size')dnl
cpp_cod(`     * @return object size')dnl
cpp_cod(`     */')dnl
cpp_cod(`    static size_t size() {')dnl
cpp_cod(`        return PortIdentity_t::size() + sizeof alternatePriority1;')dnl
cpp_cod(`    }')dnl
};
/** Properties of a PTP management TLV */
strc(ManagementId_t) {
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

/** Receive Authentication modes */
enm(MsgParams_RcvAuth_e) sz(: uint8_t) {
    NM(RCV_AUTH_NONE) = 0, /**< Ignore all Authentication TLVs */
    /** Proccess Authentication TLVs of All Managment packts */
    NM(RCV_AUTH_MNG) = (1 << 0),
    /** Proccess last Authentication TLVs of Signaling */
    NM(RCV_AUTH_SIG_LAST) = (1 << 1),
    /** Proccess all Authentication TLVs of Signaling */
    NM(RCV_AUTH_SIG_ALL) = (1 << 2),
    NM(RCV_AUTH_ALL) = 0x7, /**< Proccess all Authentication TLVs */
    /** Continure proccess, with Authentication errors */
    NM(RCV_AUTH_IGNORE) = (1 << 3),
};

c_cod(`/** pointer to ptpmgmt_MsgParams structure */')dnl
c_cod(`typedef struct ptpmgmt_MsgParams *ptpmgmt_pMsgParams;')dnl
c_cod(`/** pointer to constant ptpmgmt_MsgParams structure */')dnl
c_cod(`typedef const struct ptpmgmt_MsgParams *ptpmgmt_cpMsgParams;')dnl
c_cod(`')dnl
c_cod(`/**')dnl
c_cod(` * Alocate new ptpmgmt_MsgParams structure')dnl
c_cod(` * @return new ptpmgmt_MsgParams structure or null in case of error')dnl
c_cod(` * @note: C interface')dnl
c_cod(` */')dnl
c_cod(`ptpmgmt_pMsgParams ptpmgmt_MsgParams_alloc();')dnl
c_cod(`')dnl
/** Fixed values to use in Management messages */
strc(MsgParams) {
c_cod(`    /**< @cond internal */')dnl
c_cod(`    void *_this; /**< pointer to actual C++ MsgParams structure */')dnl
c_cod(`    /**< @endcond */')dnl
    uint8_t transportSpecific; /**< transport specific */
    uint8_t domainNumber; /**< domain number */
    uint8_t boundaryHops; /**< boundary hops */
    uint8_t minorVersion; /**< PTP minor version (We support 2.x) */
    bool isUnicast; /**< Mark message as unicast */
    /** Implementation-specific to use */
    enmc(implementSpecific_e) implementSpecific;
    strcc(PortIdentity_t) target; /**< target port ID */
    strcc(PortIdentity_t) self_id; /**< own port ID */
    bool useZeroGet; /**< send get with zero dataField */
    bool rcvSignaling; /**< parse signalling messages */
    bool filterSignaling; /**< use filter for signalling messages TLVs */
    bool rcvSMPTEOrg; /**< parse SMPTE Organization Extension TLV */
    /**
     * Send with Authentication TLV
     * User must call Message::useAuth() to setup the key for send
     */
    bool sendAuth;
    /**
     * Receive Authentication mode
     * Use values from MsgParams_RcvAuth_e
     * User must call Message::useAuth() to setup the spp pool
     */
    uint8_t rcvAuth;
cpp_cod(`    /** empty constructor */')dnl
cpp_cod(`    MsgParams();')dnl
    /**
     * Add TLV type to allowed signalling filter
c_cod(`     * @param[in, out] mp pointer to MsgParams wrapper structure')dnl
     * @param[in] type of TLV to allowed
     */
cpp_cod(`    void allowSigTlv(tlvType_e type);')dnl
c_cod(`    void (*allowSigTlv)(ptpmgmt_pMsgParams mp, enum ptpmgmt_tlvType_e type);')dnl
    /**
     * Remove TLV type from allowed signalling filter
c_cod(`     * @param[in, out] mp pointer to MsgParams wrapper structure')dnl
     * @param[in] type of TLV to remove
     */
cpp_cod(`    void removeSigTlv(tlvType_e type);')dnl
c_cod(`    void (*removeSigTlv)(ptpmgmt_pMsgParams mp, enum ptpmgmt_tlvType_e type);')dnl
    /**
     * Query if TLV type is allowed signalling filter
c_cod(`     * @param[in] mp pointer to MsgParams wrapper structure')dnl
     * @param[in] type of TLV to query
     * @return true if TLV is allowed
     */
cpp_cod(`    bool isSigTlv(tlvType_e type) const;')dnl
c_cod(`    bool (*isSigTlv)(ptpmgmt_cpMsgParams m, enum ptpmgmt_tlvType_e type);')dnl
    /**
     * Query how many signalling TLVs are allowd in filter
c_cod(`     * @param[in] mp pointer to MsgParams wrapper structure')dnl
     * @return number of allowed TLVs
     */
cpp_cod(`    size_t countSigTlvs() const;')dnl
c_cod(`    size_t (*countSigTlvs)(ptpmgmt_cpMsgParams m);')dnl
c_cod(`    /**')dnl
c_cod(`     * Free structure object')dnl
c_cod(`     * @param[in, out] mp pointer to MsgParams wrapper structure')dnl
c_cod(`     */')dnl
c_cod(`    void (*free)(ptpmgmt_pMsgParams mp);')dnl
cpp_cod(`  private:')dnl
cpp_cod(`    /** when filter TLVs in signalling messages')dnl
cpp_cod(`     * allow TLVs that are in the map, the bool value is ignored */')dnl
cpp_cod(`    std::map<tlvType_e, bool> allowSigTlvs;')dnl
};
cpp_cod(`/** Base for all Management TLV structures */')dnl
cpp_cod(`struct BaseMngTlv {')dnl
cpp_cod(`    virtual ~BaseMngTlv() = default;')dnl
cpp_cod(`};')dnl
cpp_cod(`/** Base for all Signalling TLV structures */')dnl
cpp_cod(`struct BaseSigTlv {')dnl
cpp_cod(`    virtual ~BaseSigTlv() = default;')dnl
cpp_cod(`};')dnl
/** Master record in unicast master table */
strc(LinuxptpUnicastMaster_t) {
    strcc(PortIdentity_t) portIdentity; /**< Master port ID */
    strcc(ClockQuality_t) clockQuality; /**< Master clock quality */
    uint8_t selected; /**< Master is in use */
    /** State of master in unicast table */
    enmc(linuxptpUnicastState_e) portState;
    UInteger8_t priority1; /**< Master first priority */
    UInteger8_t priority2; /**< Master second priority */
    strcc(PortAddress_t) portAddress; /**< Master port address */
cpp_cod(`    /**')dnl
cpp_cod(`     * Get object size')dnl
cpp_cod(`     * @return object size')dnl
cpp_cod(`     */')dnl
cpp_cod(`    size_t size() const {')dnl
cpp_cod(`        return portIdentity.size() + clockQuality.size() +')dnl
cpp_cod(`            sizeof selected + sizeof portState + sizeof priority1 +')dnl
cpp_cod(`            sizeof priority2 + portAddress.size();')dnl
cpp_cod(`    }')dnl
};

ns_e()
cpp_en(types)dnl

ice(TYPES)
