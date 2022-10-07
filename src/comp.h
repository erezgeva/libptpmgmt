/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief compilation only header
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#ifndef __PTPMGMT_COMPILATION_H
#define __PTPMGMT_COMPILATION_H

#include <endian.h>
#include "name.h"
#include "err.h"
#include "proc.h" /* Structures for management TLVs */
#include "sig.h" /* Structures for signaling TLVs */
#include "json.h"

/* ************************************************************************** */
/* compilation macroes */

#define caseItem(a) a: return #a
#define caseItemOff(a) a: return #a + off

#define stringify(s) #s
/* Need 2 levels to stringify macros value instead of macro name */
#define stringifyVal(a) stringify(a)

#define DO_PRAGMA(x) _Pragma (#x)

#ifdef __GNUC__
/* GNU GCC
 * gcc.gnu.org/onlinedocs/gcc-11.3.0/gcc/Type-Attributes.html
 * Keil GNU mode
 * www.keil.com/support/man/docs/armcc/armcc_chr1359125007083.htm
 * www.keil.com/support/man/docs/armclang_ref/armclang_ref_chr1393328521340.htm
 */
#define PACK(__definition__) __definition__ __attribute__((packed))
#define PURE __attribute__((pure))
#define MAYBE_UNUSED __attribute__((unused))
#define PRINT_FORMAT(a, b) __attribute__((format(printf,a,b)))
#define DIAG_START  DO_PRAGMA(GCC diagnostic push)
#define DIAG_IGNORE(warn) DO_PRAGMA(GCC diagnostic ignored warn)
#define DIAG_END   DO_PRAGMA(GCC diagnostic pop)
#if __GNUC__ > 6
#define FALLTHROUGH __attribute__((fallthrough))
#endif
#define HAVE_FUNC_COMPARE
#elif defined _MSC_VER
/* For MSVC:
 * http://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
 * http://docs.microsoft.com/en-us/cpp/preprocessor/pack
 */
#define PACK(__definition__) __pragma(pack(push, 1)) \
    __definition__ __pragma(pack(pop))
#define PURE
#define MAYBE_UNUSED
#define PRINT_FORMAT(a, b)
#define DIAG_START
#define DIAG_IGNORE(warn)
#define DIAG_END
#else
#error Unknown compiler
#endif

#ifndef FALLTHROUGH
#define FALLTHROUGH
#endif

/* ************************************************************************** */
/* Error log interfaces */

/**
 * Store error for user
 * @param[in] format message to print
 * @param[in] ... parameters match for format
 */
#define PTPMGMT_ERROR(format, ...) do\
    { Error::error(__FILE__, __LINE__, __func__ ,\
            Error::doFormat(format, ##__VA_ARGS__)); }\
    while(0)
/**
 * store system error for user
 * @param[in] format
 * @param[in] ... parameters match for format
 */
#define PTPMGMT_ERROR_P(format, ...) do\
    { Error::perror(__FILE__, __LINE__, __func__ ,\
            Error::doFormat(format, ##__VA_ARGS__)); }\
    while(0)
/**
 * Clear stored error
 */
#define PTPMGMT_ERROR_CLR { Error::clear(); } while(0)

__PTPMGMT_NAMESPACE_BEGIN

/* ************************************************************************** */
/*  provide functions to convert unsigned integers from
    host byte order to network byte order */

/**
 * convert unsigned 8 bits integer from host order to network order
 * @param[in] value host order unsigned 16 bits integer
 * @return network order unsigned 16 bits integer
 * @note provide function for consistence, as byte do no need conversion!
 */
inline uint16_t cpu_to_net8(uint8_t value) {return value;}
/**
 * convert unsigned 8 bits integer from network order to host order
 * @param[in] value network order unsigned 16 bits integer
 * @return host order unsigned 16 bits integer
 * @note provide function for consistence, as byte do no need conversion!
 */
inline uint16_t net_to_cpu8(uint8_t value) {return value;}
/**
 * convert unsigned 16 bits integer from host order to network order
 * @param[in] value host order unsigned 16 bits integer
 * @return network order unsigned 16 bits integer
 */
inline uint16_t cpu_to_net16(uint16_t value) {return htobe16(value);}
/**
 * convert unsigned 16 bits integer from network order to host order
 * @param[in] value network order unsigned 16 bits integer
 * @return host order unsigned 16 bits integer
 */
inline uint16_t net_to_cpu16(uint16_t value) {return be16toh(value);}
/**
 * convert unsigned 32 bits integer from host order to network order
 * @param[in] value host order unsigned 32 bits integer
 * @return network order unsigned 32 bits integer
 */
inline uint32_t cpu_to_net32(uint32_t value) {return htobe32(value);}
/**
 * convert unsigned 32 bits integer from network order to host order
 * @param[in] value network order unsigned 32 bits integer
 * @return host order unsigned 32 bits integer
 */
inline uint32_t net_to_cpu32(uint32_t value) {return be32toh(value);}
/**
 * convert unsigned 64 bits integer from host order to network order
 * @param[in] value host order unsigned 64 bits integer
 * @return network order unsigned 64 bits integer
 */
inline uint64_t cpu_to_net64(uint64_t value) {return htobe64(value);}
/**
 * convert unsigned 64 bits integer from network order to host order
 * @param[in] value network order unsigned 64 bits integer
 * @return host order unsigned 64 bits integer
 */
inline uint64_t net_to_cpu64(uint64_t value) {return be64toh(value);}

/**
 * convert unsigned 64 bits integer from host order to little endian order
 * @param[in] value host order unsigned 64 bits integer
 * @return little endian order unsigned 64 bits integer
 * @note used for linuxptp statistics
 */
inline uint64_t cpu_to_le64(uint64_t value) {return htole64(value);}
/**
 * convert unsigned 64 bits integer from little endian order to host order
 * @param[in] value little endian order unsigned 64 bits integer
 * @return host order unsigned 64 bits integer
 * @note used for linuxptp statistics
 */
inline uint64_t le_to_cpu64(uint64_t value) {return le64toh(value);}

/* ************************************************************************** */
/* Parse and build PTP management TLVs */

/**
 * @brief Handle PTP management message
 * @details
 *  Handle parse and build of a PTP management massage.
 *  Handle TLV specific dataField by calling a specific call-back per TLV id
 */
struct MsgProc {
    bool              m_build;  /* true on build */
    uint8_t          *m_cur;
    ssize_t           m_left;
    size_t            m_size;   /* TLV data size on build */
    MNG_PARSE_ERROR_e m_err;    /* Last TLV err */
    uint8_t           reserved; /* Used for reserved values */

    MNG_PARSE_ERROR_e call_tlv_data(mng_vals_e id, BaseMngTlv *&tlv);
    MNG_PARSE_ERROR_e parseSig();

#define _ptpmCaseUF(n) bool n##_f(n##_t &data);
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
    /* Per tlv ID call-back for parse or build or both */
#include "ids.h"
    /* Parse functions for signalling messages */
#define _ptpmParseFunc(n) bool n##_f(n##_t &data)
    _ptpmParseFunc(MANAGEMENT_ERROR_STATUS);
    _ptpmParseFunc(ORGANIZATION_EXTENSION);
    _ptpmParseFunc(PATH_TRACE);
    _ptpmParseFunc(ALTERNATE_TIME_OFFSET_INDICATOR);
    _ptpmParseFunc(ENHANCED_ACCURACY_METRICS);
    _ptpmParseFunc(L1_SYNC);
    _ptpmParseFunc(PORT_COMMUNICATION_AVAILABILITY);
    _ptpmParseFunc(PROTOCOL_ADDRESS);
    _ptpmParseFunc(SLAVE_RX_SYNC_TIMING_DATA);
    _ptpmParseFunc(SLAVE_RX_SYNC_COMPUTED_DATA);
    _ptpmParseFunc(SLAVE_TX_EVENT_TIMESTAMPS);
    _ptpmParseFunc(CUMULATIVE_RATE_RATIO);
    _ptpmParseFunc(SLAVE_DELAY_TIMING_DATA_NP);
#undef _ptpmParseFunc

    /* Parsing functions */
    void move(size_t val) {
        m_cur += val;
        m_left -= val;
        m_size += val;
    }
    /* val in network order */
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
    bool proc(Float64_t &val);
    bool proc(std::string &str, uint16_t len);
    bool proc(Binary &bin, uint16_t len);
    bool proc(uint8_t *val, size_t len);
    /* For Enumerators using 8 bits */
    template <typename T> bool procE8(T &val);
    bool proc(clockAccuracy_e &val) { return procE8(val); }
    bool proc(faultRecord_e &val) { return procE8(val); }
    bool proc(timeSource_e &val) { return procE8(val); }
    bool proc(portState_e &val) { return procE8(val); }
    bool proc(msgType_e &val) { return procE8(val); }
    bool proc(linuxptpTimeStamp_e &val) { return procE8(val); }
    /* For Enumerators using 16 bits */
    template <typename T> bool procE16(T &val);
    bool proc(networkProtocol_e &val) { return procE16(val); }
    bool proc(linuxptpPowerProfileVersion_e &val) { return procE16(val); }
    bool proc(TimeInterval_t &v);
    bool proc(Timestamp_t &d);
    bool proc(ClockIdentity_t &v);
    bool proc(PortIdentity_t &d);
    bool proc(PortAddress_t &d);
    bool proc(ClockQuality_t &d);
    bool proc(PTPText_t &d);
    bool proc(FaultRecord_t &d);
    bool proc(AcceptableMaster_t &d);
    bool proc(linuxptpUnicastState_e &val) { return procE16(val); }
    bool proc(LinuxptpUnicastMaster_t &d);
    bool proc(SLAVE_RX_SYNC_TIMING_DATA_rec_t &rec);
    bool proc(SLAVE_RX_SYNC_COMPUTED_DATA_rec_t &rec);
    bool proc(SLAVE_TX_EVENT_TIMESTAMPS_rec_t &rec);
    bool proc(SLAVE_DELAY_TIMING_DATA_NP_rec_t &rec);
    bool procFlags(uint8_t &flags, const uint8_t flagsMask);
    /* linuxptp PORT_STATS_NP statistics use little endian */
    bool procLe(uint64_t &val);
    /* list proccess with count */
    template <typename T> bool vector_f(uint32_t count, std::vector<T> &vec);
    /* countless list proccess */
    template <typename T> bool vector_o(std::vector<T> &vec);
};

/* ************************************************************************** */
/* JSON base structure used by both build and parse of JSON */

struct JsonProc {
    /* This structure implement single method for use by both from and to */
    bool procData(mng_vals_e managementId, const BaseMngTlv *&data);
#define _ptpmProcType(type) \
    virtual bool procValue(const char *name, type &val) = 0;
    _ptpmProcType(uint8_t)
    _ptpmProcType(uint16_t)
    _ptpmProcType(uint32_t)
    _ptpmProcType(uint64_t)
    _ptpmProcType(int8_t)
    _ptpmProcType(int16_t)
    _ptpmProcType(int32_t)
    _ptpmProcType(int64_t)
    _ptpmProcType(float)
    _ptpmProcType(double)
    _ptpmProcType(long double)
    _ptpmProcType(networkProtocol_e)
    _ptpmProcType(clockAccuracy_e)
    _ptpmProcType(faultRecord_e)
    _ptpmProcType(timeSource_e)
    _ptpmProcType(portState_e)
    _ptpmProcType(linuxptpTimeStamp_e)
    _ptpmProcType(linuxptpPowerProfileVersion_e)
    _ptpmProcType(linuxptpUnicastState_e)
    _ptpmProcType(TimeInterval_t)
    _ptpmProcType(Timestamp_t)
    _ptpmProcType(ClockIdentity_t)
    _ptpmProcType(PortIdentity_t)
    _ptpmProcType(PortAddress_t)
    _ptpmProcType(ClockQuality_t)
    _ptpmProcType(PTPText_t)
    _ptpmProcType(FaultRecord_t)
    _ptpmProcType(AcceptableMaster_t)
    _ptpmProcType(LinuxptpUnicastMaster_t)
    virtual bool procBinary(const char *name, Binary &val, uint16_t &len) = 0;
    virtual bool procBinary(const char *name, uint8_t *val, size_t len) = 0;
    virtual bool procFlag(const char *name, uint8_t &flags, int mask) = 0;
#define _ptpmProcVector(type) \
    virtual bool procArray(const char *name, std::vector<type> &val) = 0;
    _ptpmProcVector(ClockIdentity_t)
    _ptpmProcVector(PortAddress_t)
    _ptpmProcVector(FaultRecord_t)
    _ptpmProcVector(AcceptableMaster_t)
    _ptpmProcVector(LinuxptpUnicastMaster_t)
};

/* Used by From Json */
struct JsonProcFrom : public JsonProc {
    virtual bool mainProc(const void *jobj) = 0;
    virtual bool procMng(mng_vals_e &id, const char *&str) = 0;
    virtual const std::string &getActionField() = 0;
    virtual bool getUnicastFlag(bool &unicastFlag) = 0;
    virtual bool getIntVal(const char *key, int64_t &val) = 0;
    virtual bool parsePort(const char *key, bool &have, PortIdentity_t &port) = 0;
    virtual bool haveData() = 0;
    virtual bool parseData() = 0;
};

__PTPMGMT_NAMESPACE_END

#endif /* __PTPMGMT_COMPILATION_H */
