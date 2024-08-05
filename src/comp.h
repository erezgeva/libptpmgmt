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
#include "name.h"
#include "cfg.h"
#include "err.h"
#include "proc.h" /* Structures for management TLVs */
#include "sig.h" /* Structures for signalling TLVs */
#include "json.h"

using namespace std;
/* ************************************************************************** */
/* compilation macroes */

#define caseItem(a) a: return #a
#define caseItemOff(a) a: return (#a) + off

#define stringify(s) #s
/* Need 2 levels to stringify macros value instead of macro name */
#define stringifyVal(a) stringify(a)

#define DO_PRAGMA(x) _Pragma (#x)

#define m_init(n) n{{0}}

#if __cplusplus >= 201603L /* C++17 */
#define FALLTHROUGH [[fallthrough]]
#define MAYBE_UNUSED(_expr) [[maybe_unused]] _expr
#endif /* __cplusplus >= 201603L */
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
inline uint16_t cpu_to_net(uint16_t value) {return htobe16(value);}
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
inline uint16_t net_to_cpu(uint16_t value) {return be16toh(value);}
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
inline uint32_t cpu_to_net(uint32_t value) {return htobe32(value);}
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
inline uint32_t net_to_cpu(uint32_t value) {return be32toh(value);}
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
inline uint64_t cpu_to_net(uint64_t value) {return htobe64(value);}
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
inline uint64_t net_to_cpu(uint64_t value) {return be64toh(value);}

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
    template <typename T, typename U> bool procBN(T &val);
    bool proc(int16_t &val) { return procBN<int16_t, uint16_t>(val); }
    bool proc(uint16_t &val) { return procBN<uint16_t, uint16_t>(val); }
    bool proc(int32_t &val) { return procBN<int32_t, uint32_t>(val); }
    bool proc(uint32_t &val) { return procBN<uint32_t, uint32_t>(val); }
    bool proc(int64_t &val) { return procBN<int64_t, uint64_t>(val); }
    bool proc(uint64_t &val) { return procBN<uint64_t, uint64_t>(val); }
    bool proc48(uint64_t &val);
    bool proc48(int64_t &val);
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
    template <typename T> bool vector_f(uint32_t count, std::vector<T> &vec);
    /* countless list process */
    template <typename T> bool vector_o(std::vector<T> &vec);
};

void *cpp2cMngTlv(mng_vals_e tlv_id, const BaseMngTlv *data, void *&x);
BaseMngTlv *c2cppMngTlv(mng_vals_e tlv_id, const void *data);
void *cpp2cSigTlv(tlvType_e tlv_id, const BaseSigTlv *data, void *&x,
    void *&x2);
void *cpp2cSmpte(const BaseMngTlv *tlv);

/* ************************************************************************** */
/* map of values with string key and stack of these maps */

template <class T> class mapStackStr
{
  protected:
    class elem_t
    {
      protected:
        char *m_key;
        T m_elem;
        elem_t *m_hashNext = nullptr, *m_topNext = nullptr;
        elem_t(const char *key) : m_key(strdup(key)) {}
        ~elem_t() {
            free(m_key);
        }
        T &getElem() { return m_elem; }
        friend class mapStackStr<T>;
    };
    class map_t
    {
      protected:
        map_t *m_nextMap = nullptr; /* For heap of maps */
        elem_t *m_topElem = nullptr; /* List of all elements, for cleanup */
        elem_t *m_elemHash[UINT8_MAX] = { nullptr }; /* Hash for finding */
        void freeElems() {
            elem_t *n, *c = m_topElem;
            while(c != nullptr) {
                n = c->m_topNext;
                delete c;
                c = n;
            }
        }
        void clean() {
            freeElems();
            m_topElem = nullptr;
            memset(m_elemHash, 0, sizeof m_elemHash);
        }
        ~map_t() {freeElems();}
        static uint8_t hash_f(const char *key) {
            size_t a = std::min(strlen(key), (size_t)8);
            uint8_t h = key[0];
            for(size_t i = 1; i < a; i++)
                h ^= key[i];
            return h;
        }
        elem_t *get(const char *key, bool add) {
            uint8_t h = hash_f(key);
            elem_t *ne, *p = nullptr, *c = m_elemHash[h];
            for(;;) {
                int r;
                if(c == nullptr)
                    r = 1;
                else {
                    r = strcmp(key, c->m_key);
                    if(r == 0)
                        return c;
                }
                if(r > 0) { /* bigger or end of list */
                    if(!add)
                        return nullptr;
                    ne = new elem_t(key);
                    if(ne == nullptr)
                        return nullptr;
                    if(p == nullptr)
                        m_elemHash[h] = ne;
                    else
                        p->m_hashNext = ne;
                    ne->m_hashNext = c;
                    break;
                }
                p = c;
                c = p->m_hashNext;
            }
            ne->m_topNext = m_topElem;
            m_topElem = ne;
            return ne;
        }
        friend class mapStackStr<T>;
    };
    map_t *topMap, *curMap;
  public:
    mapStackStr() : topMap(nullptr), curMap(new map_t) {}
    ~mapStackStr() {
        if(curMap != nullptr)
            delete curMap;
        map_t *n, *c = topMap;
        while(c != nullptr) {
            n = c->m_nextMap;
            delete c;
            c = n;
        }
    }
    bool push() { /* Push into heap */
        if(curMap != nullptr) {
            curMap->m_nextMap = topMap;
            topMap = curMap;
        }
        curMap = new map_t; /* Create new map */
        return curMap != nullptr;
    }
    bool pop() { /* Pop from heap */
        if(topMap == nullptr)
            return false;
        /* Delete current map */
        if(curMap != nullptr)
            delete curMap;
        curMap = topMap;
        topMap = topMap->m_nextMap;
        return true;
    }
    bool have(const char *key) {
        if(curMap == nullptr)
            return false;
        return curMap->get(key, false) != nullptr;
    }
    T &operator [](const char *key) {
        return curMap->get(key, true)->getElem();
    }
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
    virtual bool procBinary(const char *name, Binary &val, uint16_t &len) = 0;
    virtual bool procBinary(const char *name, uint8_t *val, size_t len) = 0;
    virtual bool procFlag(const char *name, uint8_t &flags, int mask) = 0;
    virtual void procZeroFlag(uint8_t &flags) = 0;
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
    virtual bool procMng(mng_vals_e &id) = 0;
    virtual const char *getActionField() = 0;
    virtual bool getUnicastFlag(bool &unicastFlag) = 0;
    virtual bool getIntVal(const char *key, int64_t &val) = 0;
    virtual bool parsePort(const char *key, bool &have, PortIdentity_t &port) = 0;
    virtual bool haveData() = 0;
    virtual bool parseData() = 0;
    virtual ~JsonProcFrom() {}
};

/* structure for linking of From Json library */
struct Json_lib {
    std::function<void *(const char *json)> m_parse;
    std::function<void(void *jobj)> m_free;
    std::function<JsonProcFrom *()> m_alloc_proc;
    const char *m_name; /**< Used in static only */
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
    virtual bool init(HMAC_t _type) = 0;
    virtual bool digest(const void *data, size_t len, Binary &mac) = 0;
    virtual bool verify(const void *data, size_t len, Binary &mac) = 0;
};

/* structure for linking */
struct HMAC_lib {
    std::function<HMAC_Key *()> m_alloc_key;
    const char *m_name; /**< Used in static only */
};

/**
 * Library binding use C, find it easily with dlsym()
 *  and '-uptpm_hmac' for static link.
 */
static const size_t HMAC_MAX_MAC_SIZE = 64;
static const size_t HMAC_MAC_SIZE_16 = 16;
static const size_t HMAC_MAC_SIZE_32 = 32;
#define HMAC_DECL(cls) \
    HMAC_lib me = { [](){return new cls;}, HLIB_NAME }; \
    extern "C" { HMAC_lib *ptpm_hmac() { return &me; } }

const char *hmac_loadLibrary();
bool hmac_selectLib(const std::string &libMatch);
bool hmac_isLibShared();
void hmac_freeLib();
HMAC_Key *hmac_allocHMAC(HMAC_t type, const Binary &key);

__PTPMGMT_NAMESPACE_END

#endif /* __PTPMGMT_COMPILATION_H */
