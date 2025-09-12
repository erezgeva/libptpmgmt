/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief compilation only header
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#ifndef __PTPMGMT_COMPILATION_H
#define __PTPMGMT_COMPILATION_H

#include "config.h"
#include <functional>
#ifdef HAVE_ENDIAN_H
#include <endian.h>
#endif
#if 0
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#endif
#include "name.h"
#include "cfg.h"
#include "err.h"
#include "mngTlvs.h" /* Structures for management TLVs */
#include "sigTlvs.h" /* Structures for signalling TLVs */
#include "json.h"

using namespace std;
/* ************************************************************************** */
/* compilation macros */

#define caseItem(a) a: return #a
#define caseItemOff(a) a: return (#a) + off

#define stringify(s) #s
/* Need 2 levels to stringify macros value instead of macro name */
#define stringifyVal(a) stringify(a)

#define m_init(n) n{{0}}

#if __cplusplus >= 201703L /* C++17 */
#define FALLTHROUGH [[fallthrough]]
#define MAYBE_UNUSED(_expr) [[maybe_unused]] _expr
#endif /* __cplusplus >= 201703L */
#if __cplusplus >= 202002L /* C++20 */
/* branch prediction */
#define LIKELY_COND(_expr) (_expr) [[likely]]
#define UNLIKELY_COND(_expr) (_expr) [[unlikely]]
#endif /* __cplusplus >= 202002L */
#ifdef __GNUC__
/* GNU GCC
 * gcc.gnu.org/onlinedocs/gcc-11.3.0/gcc/Type-Attributes.html
 * Keil GNU mode
 * www.keil.com/support/man/docs/armcc/armcc_chr1359125007083.htm
 * www.keil.com/support/man/docs/armclang_ref/armclang_ref_chr1393328521340.htm
 */
#define PACK(__definition__) __definition__ __attribute__((packed))
#define PURE __attribute__((pure))
#define WEAK __attribute__((weak))
#ifndef MAYBE_UNUSED
#define MAYBE_UNUSED(_expr) _expr __attribute__((unused))
#endif
#define PRINT_FORMAT(_a, _b) __attribute__((format(printf,_a,_b)))
#define ON_EXIT_ATTR  __attribute__((destructor))
#ifndef LIKELY_COND
/* branch prediction */
#define LIKELY_COND(_expr) (__builtin_expect((_expr), true))
#define UNLIKELY_COND(_expr) (__builtin_expect((_expr), false))
#endif
#ifdef __clang__
#ifndef FALLTHROUGH
#define FALLTHROUGH [[clang::fallthrough]]
#endif
#ifndef __FLOAT_WORD_ORDER__
#define __FLOAT_WORD_ORDER__ __BYTE_ORDER__
#endif /* __FLOAT_WORD_ORDER__ */
#elif __GNUC__ > 6
#ifndef FALLTHROUGH
#define FALLTHROUGH __attribute__((fallthrough))
#endif
#endif /* __clang__ */
#elif defined _MSC_VER
/* For MSVC:
 * http://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
 * http://docs.microsoft.com/en-us/cpp/preprocessor/pack
 */
#define PACK(__definition__) __pragma(pack(push, 1)) \
    __definition__ __pragma(pack(pop))
#else
#error Unknown compiler
#endif

#ifndef PACK
#define PACK
#endif
#ifndef PURE
#define PURE
#endif
#ifndef WEAK
#define WEAK
#endif
#ifndef MAYBE_UNUSED
#define MAYBE_UNUSED(_expr) _expr
#endif
#ifndef PRINT_FORMAT
#define PRINT_FORMAT(_a, _b)
#endif
#ifndef ON_EXIT_ATTR
#define ON_EXIT_ATTR
#endif
#ifndef FALLTHROUGH
#define FALLTHROUGH
#endif
#ifndef LIKELY_COND
#define LIKELY_COND(_expr) (_expr)
#define UNLIKELY_COND(_expr) (_expr)
#endif

/* Use namespace */
#define __PTPMGMT_NAMESPACE ptpmgmt
#define __PTPMGMT_NAMESPACE_USE using namespace ptpmgmt
#define __PTPMGMT_C_BEGIN extern "C" {
#define __PTPMGMT_C_END }

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
#define PTPMGMT_ERROR_CLR do{ Error::clear(); } while(0)

__PTPMGMT_NAMESPACE_BEGIN

/* ************************************************************************** */
/*  provide functions to convert unsigned integers from
    host byte order to network byte order */

/**
 * convert unsigned 16 bits integer from host order to network order
 * @param[in] value host order unsigned 16 bits integer
 * @return network order unsigned 16 bits integer
 */
inline uint16_t cpu_to_net16(uint16_t value) {return htobe16(value);}
/**
 * convert unsigned 16 bits integer from host order to network order
 * @param[in] value host order unsigned 16 bits integer
 * @return network order unsigned 16 bits integer
 */
inline uint16_t cpu_to_net(uint16_t value) {return cpu_to_net16(value);}
/**
 * convert unsigned 16 bits integer from network order to host order
 * @param[in] value network order unsigned 16 bits integer
 * @return host order unsigned 16 bits integer
 */
inline uint16_t net_to_cpu16(uint16_t value) {return be16toh(value);}
/**
 * convert unsigned 16 bits integer from network order to host order
 * @param[in] value network order unsigned 16 bits integer
 * @return host order unsigned 16 bits integer
 */
inline uint16_t net_to_cpu(uint16_t value) {return net_to_cpu16(value);}
/**
 * convert unsigned 32 bits integer from host order to network order
 * @param[in] value host order unsigned 32 bits integer
 * @return network order unsigned 32 bits integer
 */
inline uint32_t cpu_to_net32(uint32_t value) {return htobe32(value);}
/**
 * convert unsigned 32 bits integer from host order to network order
 * @param[in] value host order unsigned 32 bits integer
 * @return network order unsigned 32 bits integer
 */
inline uint32_t cpu_to_net(uint32_t value) {return cpu_to_net32(value);}
/**
 * convert unsigned 32 bits integer from network order to host order
 * @param[in] value network order unsigned 32 bits integer
 * @return host order unsigned 32 bits integer
 */
inline uint32_t net_to_cpu32(uint32_t value) {return be32toh(value);}
/**
 * convert unsigned 32 bits integer from network order to host order
 * @param[in] value network order unsigned 32 bits integer
 * @return host order unsigned 32 bits integer
 */
inline uint32_t net_to_cpu(uint32_t value) {return net_to_cpu32(value);}
/**
 * convert unsigned 64 bits integer from host order to network order
 * @param[in] value host order unsigned 64 bits integer
 * @return network order unsigned 64 bits integer
 */
inline uint64_t cpu_to_net64(uint64_t value) {return htobe64(value);}
/**
 * convert unsigned 64 bits integer from host order to network order
 * @param[in] value host order unsigned 64 bits integer
 * @return network order unsigned 64 bits integer
 */
inline uint64_t cpu_to_net(uint64_t value) {return cpu_to_net64(value);}
/**
 * convert unsigned 64 bits integer from network order to host order
 * @param[in] value network order unsigned 64 bits integer
 * @return host order unsigned 64 bits integer
 */
inline uint64_t net_to_cpu64(uint64_t value) {return be64toh(value);}
/**
 * convert unsigned 64 bits integer from network order to host order
 * @param[in] value network order unsigned 64 bits integer
 * @return host order unsigned 64 bits integer
 */
inline uint64_t net_to_cpu(uint64_t value) {return net_to_cpu64(value);}

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
/* A tokener class
 * Internal use, assign buffer once!
 */
static const char spaceToken[] = " \t\n\r"; /* while spaces */

class Token
{
  private:
    char *m_buf;
    char *m_save = nullptr;
    const char *m_sep;
    bool m_alloc = false;
  public:
    ~Token() {
        if(m_alloc)
            free(m_buf);
    }
    Token(const char *sep, char *buf = nullptr) : m_buf(buf), m_sep(sep) {}
    bool dup(const string &str) {
        char *buf = strdup(str.c_str());
        if(buf != nullptr) {
            if(m_alloc)
                free(m_buf);
            m_alloc = true;
            m_buf = buf;
            return false; /* For success */
        }
        return true; /* For failure */
    }
    char *save() {return m_save;}
    char *first() {return strtok_r(m_buf, m_sep, &m_save);}
    char *next() {return strtok_r(nullptr, m_sep, &m_save);}
};

/* ************************************************************************** */
/* Parse and build PTP management TLVs */

/**
 * @brief Handle PTP management message
 * @details
 *  Handle parse and build of a PTP management massage.
 *  Handle TLV specific dataField by calling a specific call-back per TLV id
 */
struct MsgProc {
    bool              m_build = false;  /* true on build */
    uint8_t          *m_cur = nullptr;
    ssize_t           m_left = 0;
    size_t            m_size = 0;   /* TLV data size on build */
    /* Use the common error on build and parsing */
    MNG_PARSE_ERROR_e m_err = MNG_PARSE_ERROR_TOO_SMALL; /* Last TLV err */

    MNG_PARSE_ERROR_e call_tlv_data(mng_vals_e id, BaseMngTlv *&tlv);
    MNG_PARSE_ERROR_e parseSig();

#define _ptpmCaseUF(n) bool n##_f(n##_t &tlv);
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
    /* Per tlv ID call-back for parse or build or both */
#include "ids.h"
    /* Parse functions for signalling messages */
#define _ptpmParseFunc(n) bool n##_f(n##_t &tlv)
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
    _ptpmParseFunc(SMPTE_ORGANIZATION_EXTENSION);
    _ptpmParseFunc(SLAVE_DELAY_TIMING_DATA_NP);
#undef _ptpmParseFunc

    /* Parsing functions */
    void move(size_t val) {
        m_cur += val;
        m_left -= val;
        m_size += val;
    }
    /* val in network order */
    template <typename T> bool procB8(T &val);
    bool proc(int8_t &val) { return procB8(val); }
    bool proc(uint8_t &val) { return procB8(val); }
    bool procRes() {uint8_t v = 0; return proc(v); }
    template <typename T> bool procBU(T &val);
    bool proc(uint16_t &val) { return procBU(val); }
    bool proc(uint32_t &val) { return procBU(val); }
    bool proc(uint64_t &val) { return procBU(val); }
    template <typename T, typename U> bool procBN(T &val);
    bool proc(int16_t &val) { return procBN<int16_t, uint16_t>(val); }
    bool proc(int32_t &val) { return procBN<int32_t, uint32_t>(val); }
    bool proc(int64_t &val) { return procBN<int64_t, uint64_t>(val); }
    bool proc48(uint64_t &val);
    bool proc48(int64_t &val);
    bool proc(Float64_t &val);
    bool proc(string &str, uint16_t len);
    bool proc(Binary &bin, uint16_t len);
    bool proc(uint8_t *val, size_t len);
    /* For Enumerators using 8 bits */
    template <typename T> bool procE8(T &val);
    bool proc(clockAccuracy_e &val) { return procE8(val); }
    bool proc(faultRecord_e &val) { return procE8(val); }
    bool proc(timeSource_e &val) { return procE8(val); }
    bool proc(portState_e &val) { return procE8(val); }
    bool proc(delayMechanism_e &val) { return procE8(val); }
    bool proc(msgType_e &val) { return procE8(val); }
    bool proc(SMPTEmasterLockingStatus_e &val) { return procE8(val); }
    bool proc(linuxptpTimeStamp_e &val) { return procE8(val); }
    bool proc(linuxptpUnicastState_e &val) { return procE8(val); }
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
    bool proc(LinuxptpUnicastMaster_t &d);
    bool proc(SLAVE_RX_SYNC_TIMING_DATA_rec_t &rec);
    bool proc(SLAVE_RX_SYNC_COMPUTED_DATA_rec_t &rec);
    bool proc(SLAVE_TX_EVENT_TIMESTAMPS_rec_t &rec);
    bool proc(SLAVE_DELAY_TIMING_DATA_NP_rec_t &rec);
    bool procFlags(uint8_t &flags, const uint8_t flagsMask);
    /* linuxptp PORT_STATS_NP statistics use little endian */
    bool procLe(uint64_t &val);
    /* list process with count */
    template <typename T> bool vector_f(uint32_t count, vector<T> &vec);
    /* countless list process */
    template <typename T> bool vector_o(vector<T> &vec);
};

void *cpp2cMngTlv(mng_vals_e tlv_id, const BaseMngTlv *tlv, void *&x);
BaseMngTlv *c2cppMngTlv(mng_vals_e tlv_id, const void *tlv);
void *cpp2cSigTlv(tlvType_e tlv_id, const BaseSigTlv *tlv, void *&x,
    void *&x2);
void *cpp2cSmpte(const BaseMngTlv *tlv);

/* ************************************************************************** */
/* JSON base structure used by both build and parse of JSON */

struct JsonProc {
    /* This structure implement single method for use by both from and to */
    bool procData(mng_vals_e managementId, const BaseMngTlv *&tlv);
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
    _ptpmProcType(delayMechanism_e)
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
#undef _ptpmProcType
    virtual bool procBinary(const char *name, Binary &val, uint16_t &len) = 0;
    virtual bool procBinary(const char *name, uint8_t *val, size_t len) = 0;
    virtual bool procFlag(const char *name, uint8_t &flags, int mask) = 0;
    virtual void procZeroFlag(uint8_t &flags) = 0;
#define _ptpmProcVector(type) \
    virtual bool procArray(const char *name, vector<type> &val) = 0;
    _ptpmProcVector(ClockIdentity_t)
    _ptpmProcVector(PortAddress_t)
    _ptpmProcVector(FaultRecord_t)
    _ptpmProcVector(AcceptableMaster_t)
    _ptpmProcVector(LinuxptpUnicastMaster_t)
#undef _ptpmProcVector
};

/* ************************************************************************** */
/* HMAC structure used by wraper libraries
 * https://en.wikipedia.org/wiki/HMAC
 * https://en.wikipedia.org/wiki/One-key_MAC
 * CMAC a variation of CBC-MAC
 */

/* Used by HMAC libraries */
struct HMAC_Key {
    HMAC_t m_type;
    Binary m_key;
    virtual ~HMAC_Key();
    virtual bool init() = 0;
    virtual bool digest(const void *hData, size_t len, Binary &mac) = 0;
    virtual bool verify(const void *hData, size_t len, Binary &mac) = 0;
};

/* structure for linking */
struct HMAC_lib {
    function<HMAC_Key *()> m_alloc_key; /* Alocate HMAC_Key object */
    function<void ()> m_unload; /* unload library */
    const char *m_name; /**< Used in static only */
};

static const size_t HMAC_MAX_MAC_SIZE = 64;
static const size_t HMAC_MAC_SIZE_16 = 16;
static const size_t HMAC_MAC_SIZE_32 = 32;
/**
 * Library binding use C, find it easily with dlsym()
 *  and '-uptpm_hmac' for static link.
 */
#define HMAC_DECL(cls) \
    HMAC_lib me = { [](){return new cls;}, [](){Unload();}, HLIB_NAME }; \
    extern "C" { HMAC_lib *ptpm_hmac() { return Load() ? &me : nullptr; } }

const char *hmac_loadLibrary();
bool hmac_selectLib(const string &libMatch);
bool hmac_isLibShared();
void hmac_freeLib();
size_t hmac_count();
HMAC_Key *hmac_allocHMAC(HMAC_t type, const Binary &key);

__PTPMGMT_NAMESPACE_END

#endif /* __PTPMGMT_COMPILATION_H */
