/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief Parse, buid or both function per PTP managmet id
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#include <cmath>
#include <limits>
#include <byteswap.h>
#include "comp.h"
#include "msg.h"

__PTPMGMT_NAMESPACE_BEGIN

/**
 * Constants for proc(Float64_t)
 */
const uint16_t sig_16bits = 1U << 15;           // bit 16 negative sign bit
const uint64_t sig_64bits = (uint64_t)1UL << 63; // bit 64 negative sign bit
// constants for IEEE 754 64-bit
// sign bit: 1 bit, exponent: 11 bits, mantissa: 52 bits
// https://en.wikipedia.org/wiki/Double-precision_floating-point_format
const int ieee754_mnt_size = 52; // Exponent location, mantissa size
const int ieee754_exp_size = 11; // Exponent size
const int64_t ieee754_exp_nan = ((int64_t)1L << (ieee754_exp_size - 1)); // 1024
const int64_t ieee754_exp_max = ieee754_exp_nan - 1; // 1023
const int64_t ieee754_exp_sub = -ieee754_exp_max; // -1023 for subnormal
const int64_t ieee754_exp_min = ieee754_exp_sub + 1; // -1022
const int64_t ieee754_exp_bias = ieee754_exp_max; // 1023
// 0x7ff0000000000000
const uint64_t ieee754_exp_mask = (((uint64_t)1 << ieee754_exp_size) - 1) <<
    ieee754_mnt_size;
// 0x0010000000000000
const int64_t ieee754_mnt_base = (int64_t)1 << ieee754_mnt_size;
// 0x000fffffffffffff
const int64_t ieee754_mnt_mask = ieee754_mnt_base - 1;

MNG_PARSE_ERROR_e MsgProc::call_tlv_data(mng_vals_e id, BaseMngTlv *&tlv)
{
#define _ptpmCaseNA(n) case n: return MNG_PARSE_ERROR_OK
#define _ptpmCaseUF(n) case n:\
        if(m_build) {\
            if(n##_f(*dynamic_cast<n##_t *>(tlv)))\
                return m_err;\
        } else {\
            n##_t *t = new n##_t;\
            if(t == nullptr)\
                return MNG_PARSE_ERROR_MEM;\
            if(n##_f(*t)) {\
                delete t;\
                return m_err;\
            }\
            tlv = t;\
        } break
    // The default error on build or parsing
    m_err = MNG_PARSE_ERROR_TOO_SMALL;
    switch(id) {
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n);
#include "ids.h"
        default:
            return MNG_PARSE_ERROR_UNSUPPORT;
    }
    // The mng ID is not supported yet
    return MNG_PARSE_ERROR_OK;
}
#define procInt(n) \
    bool MsgProc::proc(uint##n##_t &val) \
    { \
        if(m_left < (ssize_t)sizeof(uint##n##_t)) \
            return true; \
        if(m_build) \
            *(uint##n##_t *)m_cur = cpu_to_net##n(val); \
        else \
            val = net_to_cpu##n(*(uint##n##_t *)m_cur); \
        move(sizeof(uint##n##_t)); \
        return false; \
    } \
    bool MsgProc::proc(int##n##_t &val) \
    { \
        if(m_left < (ssize_t)sizeof(int##n##_t)) \
            return true; \
        if(m_build) \
            *(uint##n##_t *)m_cur = cpu_to_net##n((uint##n##_t)val); \
        else \
            val = (int##n##_t)net_to_cpu##n(*(uint##n##_t *)m_cur); \
        move(sizeof(int##n##_t)); \
        return false; \
    }
procInt(8)
procInt(16)
procInt(32)
procInt(64)
bool MsgProc::proc48(UInteger48_t &val)
{
    uint16_t high;
    uint32_t low;
    if(m_left < (ssize_t)sizeof_UInteger48_t)
        return true;
    if(m_build) {
        if(val > UINT48_MAX) {
            m_err = MNG_PARSE_ERROR_VAL;
            return true;
        }
        high = (val >> 32) & UINT16_MAX;
        low = val & UINT32_MAX;
    }
    if(proc(high) || proc(low))
        return true;
    if(!m_build)
        val = low | ((UInteger48_t)high << 32);
    return false;
}
bool MsgProc::proc48(Integer48_t &val)
{
    uint16_t high;
    uint32_t low;
    if(m_left < (ssize_t)sizeof_Integer48_t)
        return true;
    if(m_build) {
        if(val < INT48_MIN || val > INT48_MAX) {
            m_err = MNG_PARSE_ERROR_VAL;
            return true;
        }
        high = (val >> 32) & INT16_MAX;
        if(val < 0) // Add sign bit for negative
            high |= sig_16bits;
        low = val & UINT32_MAX;
    }
    if(proc(high) || proc(low))
        return true;
    if(!m_build) {
        UInteger48_t ret = low | (((UInteger48_t)high & INT16_MAX) << 32);
        if(high & sig_16bits) // Add sign bit for negative
            ret |= sig_64bits;
        memcpy(&val, &ret, sizeof ret);
    }
    return false;
}
bool MsgProc::proc(Float64_t &val)
{
    // Float64_t
    // Using IEEE 754 64-bit floating-point
    PTPMGMT_ERROR_CLR;
    if(m_left < 8)
        return true;
    uint64_t num;
    int64_t mnt, exp;
    // see ieee754.h
    #ifdef NO_IEEE_754
    // Most processors support IEEE 754
    // For Hardware that do not use IEEE 754
    bool use64 = false;
    #else
    // true: Float64_t is 64 bits IEEE 754
    // false: calculate IEEE 754
    bool use64 = sizeof num == sizeof(Float64_t);
    #endif
    enum {
        // when calculate always use host order
        USE_HOST, // use host order
        USE_BIG, // float is big endian (network order)
        USE_LT, // float is little endian
    } ordMod;
    if(use64) {
        #if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
        ordMod = USE_BIG; // Prefer network order
        #elif __FLOAT_WORD_ORDER__ == __BYTE_ORDER__
        ordMod = USE_HOST; // Prefer host order!
        #elif __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
        ordMod = USE_LT;
        #else // float use unkown order, we must build it manually
        ordMod = USE_HOST;
        use64 = false;
        #endif
    } else
        ordMod = USE_HOST;
    if(m_build) {
        if(use64) // Float64_t is 64 bits IEEE 754
            memcpy(&num, &val, sizeof num);
        else {
            /* For processors that do not support IEEE 754
             *  or endian is not clear
             * The computed float is in host order
             */
            // Move negative sign bit
            if(std::signbit(val)) {
                num = sig_64bits; // add sign bit
                if(std::isfinite(val))
                    val = fabsl(val);
            } else
                num = 0;
            double norm;
            switch(std::fpclassify(val)) {
                case FP_NAN: // Not a number
                    exp = ieee754_exp_nan;
                    mnt = 1; // Any positive goes
                    break;
                case FP_INFINITE: // Infinity
                    exp = ieee754_exp_nan;
                    mnt = 0;
                    break;
                case FP_ZERO: // Zero
                    exp = ieee754_exp_sub;
                    mnt = 0;
                    break;
                default:
                    FALLTHROUGH;
                case FP_NORMAL:
                    exp = (int64_t)floorl(log2l(val));
                    if(exp > ieee754_exp_max)
                        return true; // Number is too big
                    else if(exp >= ieee754_exp_min) {
                        norm = val / exp2l(exp);
                        if(norm >= 1) {
                            mnt = (int64_t)floorl(norm * ieee754_mnt_base -
                                    ieee754_mnt_base);
                            if(mnt < 0 || mnt >= ieee754_mnt_base) {
                                PTPMGMT_ERROR("wrong calculation of float, "
                                    "mnt out of range");
                                return true; // wrong calculation
                            }
                            break; // Break normal
                        }
                    }
                    FALLTHROUGH;
                case FP_SUBNORMAL: // Subnormal number
                    exp = ieee754_exp_sub;
                    norm = val / exp2l(ieee754_exp_min);
                    mnt = (int64_t)floorl(norm * ieee754_mnt_base);
                    if(mnt < 0 || mnt >= ieee754_mnt_base) {
                        PTPMGMT_ERROR("wrong calculation of float, "
                            "mnt out of range for subnormal");
                        return true; // wrong calculation
                    }
                    // For very small number use the minimum subnormal
                    // As zero is used by zero only!
                    if(mnt == 0)
                        mnt = 1;
                    break;
            }
            // Add exponent value and mantissa
            num |= ((exp + ieee754_exp_bias) << ieee754_mnt_size) |
                (mnt & ieee754_mnt_mask);
        }
    }
    switch(ordMod) {
        case USE_BIG:
            if(m_build)
                *(uint64_t *)m_cur = num;
            else
                num = *(uint64_t *)m_cur;
            move(8);
            break;
        case USE_HOST:
            if(proc(num)) // host order to network order
                return true;
            break;
        case USE_LT:
            if(m_build)
                *(uint64_t *)m_cur = bswap_64(num);
            else
                num = bswap_64(*(uint64_t *)m_cur);
            move(8);
            break;
    }
    if(!m_build) {
        if(use64) // Float64_t is 64 bits IEEE 754
            memcpy(&val, &num, sizeof num);
        else {
            /* For processors that do not support IEEE 754
             *  or endian is not clear
             * The computed float is in host order
             */
            exp = (int64_t)((num & ieee754_exp_mask) >> ieee754_mnt_size) -
                ieee754_exp_bias;
            mnt = num & ieee754_mnt_mask;
            if(exp == ieee754_exp_nan) {
                if(mnt == 0) // infinity
                    val = HUGE_VALL;
                else // NaN
                    val = std::numeric_limits<Float64_t>::quiet_NaN();
            } else if(exp == ieee754_exp_sub) // Subnormal or zero
                val = exp2l(ieee754_exp_min) * mnt / ieee754_mnt_base;
            else // Normal
                val = exp2l(exp) * (mnt + ieee754_mnt_base) / ieee754_mnt_base;
            if(num & sig_64bits) // Negative
                val = std::copysign(val, -1);
        }
    }
    return false;
}
bool MsgProc::proc(std::string &str, uint16_t len)
{
    if(m_build) // On build ignore length variable
        len = str.length();
    if(m_left < (ssize_t)len)
        return true;
    if(m_build)
        memcpy(m_cur, str.c_str(), len);
    else
        str = std::string((char *)m_cur, len);
    move(len);
    return false;
}
bool MsgProc::proc(Binary &bin, uint16_t len)
{
    if(m_build) // On build ignore length variable
        len = bin.length();
    if(m_left < (ssize_t)len)
        return true;
    if(m_build)
        bin.copy(m_cur);
    else
        bin.setBin(m_cur, len);
    move(len);
    return false;
}
bool MsgProc::proc(uint8_t *val, size_t len)
{
    if(m_left < (ssize_t)len)
        return true;
    if(m_build)
        memcpy(m_cur, val, len);
    else
        memcpy(val, m_cur, len);
    move(len);
    return false;
}
template <typename T> bool MsgProc::procE8(T &val)
{
    uint8_t v = val;
    bool ret = proc(v);
    val = (T)v;
    return ret;
}
#define E8(t) template bool MsgProc::procE8<t>(t &)
E8(clockAccuracy_e);
E8(faultRecord_e);
E8(timeSource_e);
E8(portState_e);
E8(msgType_e);
E8(linuxptpTimeStamp_e);
template <typename T> bool MsgProc::procE16(T &val)
{
    uint16_t v = val;
    bool ret = proc(v);
    val = (T)v;
    return ret;
}
#define E16(t) template bool MsgProc::procE16<t>(t &)
E16(networkProtocol_e);
E16(linuxptpUnicastState_e);
bool MsgProc::proc(TimeInterval_t &v)
{
    return proc(v.scaledNanoseconds);
}
bool MsgProc::proc(Timestamp_t &d)
{
    return proc48(d.secondsField) || proc(d.nanosecondsField);
}
bool MsgProc::proc(ClockIdentity_t &v)
{
    return proc(v.v, v.size());
}
bool MsgProc::proc(PortIdentity_t &d)
{
    return proc(d.clockIdentity) || proc(d.portNumber);
}
bool MsgProc::proc(PortAddress_t &d)
{
    d.addressLength = d.addressField.length();
    return proc(d.networkProtocol) || proc(d.addressLength) ||
        proc(d.addressField, d.addressLength);
}
bool MsgProc::proc(ClockQuality_t &d)
{
    return proc(d.clockClass) || proc(d.clockAccuracy) ||
        proc(d.offsetScaledLogVariance);
}
bool MsgProc::proc(PTPText_t &d)
{
    d.lengthField = d.textField.length();
    return proc(d.lengthField) || proc(d.textField, d.lengthField);
}
bool MsgProc::proc(FaultRecord_t &d)
{
    if(m_build)
        d.faultRecordLength = d.size();
    if(proc(d.faultRecordLength) || proc(d.faultTime) || proc(d.severityCode) ||
        proc(d.faultName) || proc(d.faultValue) || proc(d.faultDescription))
        return true;
    if(!m_build && d.faultRecordLength != d.size()) {
        m_err = MNG_PARSE_ERROR_SIZE_MISS;
        return true;
    }
    return false;
}
bool MsgProc::proc(AcceptableMaster_t &d)
{
    return proc(d.acceptablePortIdentity) || proc(d.alternatePriority1);
}
bool MsgProc::proc(LinuxptpUnicastMaster_t &d)
{
    return proc(d.portIdentity) || proc(d.clockQuality) || proc(d.selected) ||
        proc(d.portState) || proc(d.priority1) || proc(d.priority2) ||
        proc(d.portAddress);
}
bool MsgProc::procFlags(uint8_t &flags, const uint8_t flagsMask)
{
    if(m_build) {
        if(flagsMask > 1) // Ensure we use proper bits
            flags &= flagsMask;
        else if(flags > 0) // We have single flag, any positive goes
            flags = 1;
    }
    return proc(flags);
}
bool MsgProc::procLe(uint64_t &val)
{
    if(m_left < (ssize_t)sizeof(uint64_t))
        return true;
    if(m_build)
        *(uint64_t *)m_cur = cpu_to_le64(val);
    else
        val = le_to_cpu64(*(uint64_t *)m_cur);
    move(sizeof(uint64_t));
    return false;
}
// list build part
#define vector_b(vec)\
    if(m_build) {\
        for(auto &rec : vec) {\
            if(proc(rec))\
                return true;\
        }\
    } else
template <typename T> bool MsgProc::vector_f(uint32_t count,
    std::vector<T> &vec)
{
    vector_b(vec) {
        for(uint32_t i = 0; i < count; i++) {
            T rec = {};
            if(proc(rec))
                return true;
            vec.push_back(rec);
        }
    }
    return false;
}
#define vf(t) template bool MsgProc::vector_f<t>(uint32_t, std::vector<t> &)
vf(FaultRecord_t);
vf(PortAddress_t);
vf(AcceptableMaster_t);
vf(LinuxptpUnicastMaster_t);
template <typename T> bool MsgProc::vector_o(std::vector<T> &vec)
{
    vector_b(vec) {
        while(m_left >= (ssize_t)T::size()) {
            T rec = {};
            if(proc(rec))
                return true;
            vec.push_back(rec);
        }
    }
    return false;
}
#define vo(t) template bool MsgProc::vector_o<t>(std::vector<t> &)
vo(ClockIdentity_t);
vo(SLAVE_RX_SYNC_TIMING_DATA_rec_t);
vo(SLAVE_RX_SYNC_COMPUTED_DATA_rec_t);
vo(SLAVE_TX_EVENT_TIMESTAMPS_rec_t);
vo(SLAVE_DELAY_TIMING_DATA_NP_rec_t);

template <typename T> size_t vector_l(size_t ret, std::vector<T> &vec) PURE;

// For Octets arrays
#define oproc(a) proc(a, sizeof a)
#define fproc procFlags(d.flags, d.flagsMask)

// size of variable length list
template <typename T> size_t vector_l(size_t ret, const std::vector<T> &vec)
{
    for(const auto &rec : vec)
        ret += rec.size();
    return ret;
}

// size functions per id
#define S(n)\
    static inline size_t n##_s(const n##_t &d) PURE;\
    static inline size_t n##_s(const n##_t &d)

S(CLOCK_DESCRIPTION)
{
    return 13 + d.physicalAddress.size() + d.protocolAddress.size() +
        d.physicalLayerProtocol.size() + d.productDescription.size() +
        d.revisionData.size() + d.userDescription.size();
}
S(USER_DESCRIPTION)
{
    return d.userDescription.size();
}
S(FAULT_LOG)
{
    return vector_l(2, d.faultRecords);
}
S(PATH_TRACE_LIST)
{
    return ClockIdentity_t::size() * d.pathSequence.size();
}
S(GRANDMASTER_CLUSTER_TABLE)
{
    return vector_l(2, d.PortAddress);
}
S(UNICAST_MASTER_TABLE)
{
    return vector_l(3, d.PortAddress);
}
S(ACCEPTABLE_MASTER_TABLE)
{
    return 2 + AcceptableMaster_t::size() * d.list.size();
}
S(ALTERNATE_TIME_OFFSET_NAME)
{
    return 1 + d.displayName.size();
}
// linuxptp TLVs Implementation-specific C000-DFFF
S(PORT_PROPERTIES_NP)
{
    return 2 + PortIdentity_t::size() + d.interface.size();
}
S(UNICAST_MASTER_TABLE_NP)
{
    return vector_l(2, d.unicastMasters);
};

ssize_t Message::dataFieldSize(const BaseMngTlv *data) const
{
#define _ptpmCaseUFS(n) case n:\
        if(data == nullptr) {\
            n##_t empty;\
            return n##_s(empty);\
        } else\
            return n##_s(*dynamic_cast<const n##_t*>(data));
#define _ptpmCaseUFBS(n) _ptpmCaseUFS(n)
    switch(m_tlv_id) {
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
#include "ids.h"
        default:
            return -2;
    }
}

/*
 * Function used for both build and parse
 * Function can use:
 *  d        - the reference to the TLV own structure
 *  m_build  - is it build (true) or parse (false)
 *  m_err    - error to use when function return true
 *  m_left   - number of octets left in dataField the function may read
 *  reserved - uint8_t value for the function may use for reserved fields
 * The function should return TRUE on ERROR!!
 * The main build function will add a pad at the end to make size even
 */

#define A(n) bool MsgProc::n##_##f(n##_t &d)

A(CLOCK_DESCRIPTION)
{
    d.physicalAddressLength = d.physicalAddress.length();
    return proc(d.clockType) || proc(d.physicalLayerProtocol) ||
        proc(d.physicalAddressLength) ||
        proc(d.physicalAddress, d.physicalAddressLength) ||
        proc(d.protocolAddress) || oproc(d.manufacturerIdentity) ||
        proc(reserved) || proc(d.productDescription) ||
        proc(d.revisionData) || proc(d.userDescription) ||
        oproc(d.profileIdentity);
}
A(USER_DESCRIPTION)
{
    return proc(d.userDescription);
}
A(INITIALIZE)
{
    return proc(d.initializationKey);
}
A(FAULT_LOG)
{
    if(proc(d.numberOfFaultRecords))
        return true;
    return vector_f(d.numberOfFaultRecords, d.faultRecords);
}
A(DEFAULT_DATA_SET)
{
    return fproc || proc(reserved) || proc(d.numberPorts) ||
        proc(d.priority1) || proc(d.clockQuality) || proc(d.priority2) ||
        proc(d.clockIdentity) || proc(d.domainNumber);
}
A(CURRENT_DATA_SET)
{
    return proc(d.stepsRemoved) || proc(d.offsetFromMaster) ||
        proc(d.meanPathDelay);
}
A(PARENT_DATA_SET)
{
    return proc(d.parentPortIdentity) || fproc || proc(reserved) ||
        proc(d.observedParentOffsetScaledLogVariance) ||
        proc(d.observedParentClockPhaseChangeRate) ||
        proc(d.grandmasterPriority1) || proc(d.grandmasterClockQuality) ||
        proc(d.grandmasterPriority2) || proc(d.grandmasterIdentity);
}
A(TIME_PROPERTIES_DATA_SET)
{
    return proc(d.currentUtcOffset) || fproc || proc(d.timeSource);
}
A(PORT_DATA_SET)
{
    return proc(d.portIdentity) || proc(d.portState) ||
        proc(d.logMinDelayReqInterval) || proc(d.peerMeanPathDelay) ||
        proc(d.logAnnounceInterval) || proc(d.announceReceiptTimeout) ||
        proc(d.logSyncInterval) || proc(d.delayMechanism) ||
        proc(d.logMinPdelayReqInterval) || proc(d.versionNumber);
}
A(PRIORITY1)
{
    return proc(d.priority1);
}
A(PRIORITY2)
{
    return proc(d.priority2);
}
A(DOMAIN)
{
    return proc(d.domainNumber);
}
A(SLAVE_ONLY)
{
    return fproc;
}
A(LOG_ANNOUNCE_INTERVAL)
{
    return proc(d.logAnnounceInterval);
}
A(ANNOUNCE_RECEIPT_TIMEOUT)
{
    return proc(d.announceReceiptTimeout);
}
A(LOG_SYNC_INTERVAL)
{
    return proc(d.logSyncInterval);
}
A(VERSION_NUMBER)
{
    return proc(d.versionNumber);
}
A(TIME)
{
    return proc(d.currentTime);
}
A(CLOCK_ACCURACY)
{
    return proc(d.clockAccuracy);
}
A(UTC_PROPERTIES)
{
    return proc(d.currentUtcOffset) || fproc;
}
A(TRACEABILITY_PROPERTIES)
{
    return fproc;
}
A(TIMESCALE_PROPERTIES)
{
    return fproc || proc(d.timeSource);
}
A(UNICAST_NEGOTIATION_ENABLE)
{
    return fproc;
}
A(PATH_TRACE_LIST)
{
    return vector_o(d.pathSequence);
}
A(PATH_TRACE_ENABLE)
{
    return fproc;
}
A(GRANDMASTER_CLUSTER_TABLE)
{
    d.actualTableSize = d.PortAddress.size();
    if(proc(d.logQueryInterval) || proc(d.actualTableSize))
        return true;
    return vector_f(d.actualTableSize, d.PortAddress);
}
A(UNICAST_MASTER_TABLE)
{
    d.actualTableSize = d.PortAddress.size();
    if(proc(d.logQueryInterval) || proc(d.actualTableSize))
        return true;
    return vector_f(d.actualTableSize, d.PortAddress);
}
A(UNICAST_MASTER_MAX_TABLE_SIZE)
{
    return proc(d.maxTableSize);
}
A(ACCEPTABLE_MASTER_TABLE)
{
    d.actualTableSize = d.list.size();
    if(proc(d.actualTableSize))
        return true;
    return vector_f(d.actualTableSize, d.list);
}
A(ACCEPTABLE_MASTER_TABLE_ENABLED)
{
    return fproc;
}
A(ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
{
    return proc(d.maxTableSize);
}
A(ALTERNATE_MASTER)
{
    return fproc || proc(d.logAlternateMulticastSyncInterval) ||
        proc(d.numberOfAlternateMasters);
}
A(ALTERNATE_TIME_OFFSET_ENABLE)
{
    return proc(d.keyField) || fproc;
}
A(ALTERNATE_TIME_OFFSET_NAME)
{
    return proc(d.keyField) || proc(d.displayName);
}
A(ALTERNATE_TIME_OFFSET_MAX_KEY)
{
    return proc(d.maxKey);
}
A(ALTERNATE_TIME_OFFSET_PROPERTIES)
{
    return proc(d.keyField) || proc(d.currentOffset) || proc(d.jumpSeconds) ||
        proc48(d.timeOfNextJump);
}
A(TRANSPARENT_CLOCK_PORT_DATA_SET)
{
    return proc(d.portIdentity) || fproc ||
        proc(d.logMinPdelayReqInterval) || proc(d.peerMeanPathDelay);
}
A(LOG_MIN_PDELAY_REQ_INTERVAL)
{
    return proc(d.logMinPdelayReqInterval);
}
A(TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
{
    return proc(d.clockIdentity) || proc(d.numberPorts) ||
        proc(d.delayMechanism) || proc(d.primaryDomain);
}
A(PRIMARY_DOMAIN)
{
    return proc(d.primaryDomain);
}
A(DELAY_MECHANISM)
{
    return proc(d.delayMechanism);
}
A(EXTERNAL_PORT_CONFIGURATION_ENABLED)
{
    return fproc;
}
A(MASTER_ONLY)
{
    return fproc;
}
A(HOLDOVER_UPGRADE_ENABLE)
{
    return fproc;
}
A(EXT_PORT_CONFIG_PORT_DATA_SET)
{
    return fproc || proc(d.desiredState);
}
// linuxptp TLVs (in Implementation-specific C000-DFFF)
A(TIME_STATUS_NP)
{
    return proc(d.master_offset) || proc(d.ingress_time) ||
        proc(d.cumulativeScaledRateOffset) ||
        proc(d.scaledLastGmPhaseChange) || proc(d.gmTimeBaseIndicator) ||
        proc(d.nanoseconds_msb) || proc(d.nanoseconds_lsb) ||
        proc(d.fractional_nanoseconds) || proc(d.gmPresent) ||
        proc(d.gmIdentity);
}
A(GRANDMASTER_SETTINGS_NP)
{
    return proc(d.clockQuality) || proc(d.currentUtcOffset) || fproc ||
        proc(d.timeSource);
}
A(PORT_DATA_SET_NP)
{
    return proc(d.neighborPropDelayThresh) || proc(d.asCapable);
}
A(SUBSCRIBE_EVENTS_NP)
{
    return proc(d.duration) || proc(d.bitmask, EVENT_BITMASK_CNT);
}
A(PORT_PROPERTIES_NP)
{
    return proc(d.portIdentity) || proc(d.portState) || proc(d.timestamping) ||
        proc(d.interface);
}
A(PORT_STATS_NP)
{
    if(proc(d.portIdentity))
        return true;
    int i;
    // Statistics uses little endian order
    for(i = 0; i < MAX_MESSAGE_TYPES; i++) {
        if(procLe(d.rxMsgType[i]))
            return true;
    }
    for(i = 0; i < MAX_MESSAGE_TYPES; i++) {
        if(procLe(d.txMsgType[i]))
            return true;
    }
    return false;
}
A(SYNCHRONIZATION_UNCERTAIN_NP)
{
    return proc(d.val);
}
A(PORT_SERVICE_STATS_NP)
{
    return proc(d.portIdentity) || procLe(d.announce_timeout) ||
        procLe(d.sync_timeout) || procLe(d.delay_timeout) ||
        procLe(d.unicast_service_timeout) || procLe(d.unicast_request_timeout) ||
        procLe(d.master_announce_timeout) || procLe(d.master_sync_timeout) ||
        procLe(d.qualification_timeout) || procLe(d.sync_mismatch) ||
        procLe(d.followup_mismatch);
};
A(UNICAST_MASTER_TABLE_NP)
{
    d.actualTableSize = d.unicastMasters.size();
    if(proc(d.actualTableSize))
        return true;
    return vector_f(d.actualTableSize, d.unicastMasters);
};
A(PORT_HWCLOCK_NP)
{
    return proc(d.portIdentity) || proc(d.phc_index) || proc(d.flags);
};

__PTPMGMT_NAMESPACE_END
