/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Parse, buid or both function per PTP managmet id
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#include "comp.h"
#include <cmath>
#include <limits>
#ifdef HAVE_BYTESWAP_H
#include <byteswap.h>
#endif
#include "msg.h"
#include "c/types.h"
#include "c/proc.h"

#if __FLOAT_WORD_ORDER__ == __ORDER_BIG_ENDIAN__
#define ptpm_ordMod USE_BIG // Prefer network order
#define ptpm_have_USE_BIG
#elif __FLOAT_WORD_ORDER__ == __BYTE_ORDER__
#define ptpm_ordMod USE_HOST; // Prefer host order!
#elif __FLOAT_WORD_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define ptpm_ordMod USE_LT
#define ptpm_have_USE_LT
#endif

__PTPMGMT_NAMESPACE_BEGIN

/**
 * Constants for proc(Float64_t)
 */
const uint16_t sig_16bits = 1U << 15;            // bit 16 negative sign bit
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
            n##_t *a = dynamic_cast<n##_t *>(tlv);\
            if(a == nullptr)\
                return MNG_PARSE_ERROR_MISMATCH_TLV;\
            if(n##_f(*a))\
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
        }; break
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
template <typename T> bool MsgProc::procB8(T &val)
{
    if(m_left < (ssize_t)sizeof(T))
        return true;
    if(m_build)
        *(T *)m_cur = val;
    else
        val = *(T *)m_cur;
    move(sizeof(T));
    return false;
}
#define B8(t) template bool MsgProc::procB8<t>(t &)
B8(uint8_t);
B8(int8_t);
template <typename T, typename U> bool MsgProc::procBN(T &val)
{
    if(m_left < (ssize_t)sizeof(T))
        return true;
    U v;
    if(m_build) {
        v = cpu_to_net((U)val);
        memcpy(m_cur, &v, sizeof(T));
    } else {
        memcpy(&v, m_cur, sizeof(T));
        val = (T)net_to_cpu(v);
    }
    move(sizeof(T));
    return false;
}
#define BN_(t, u) template bool MsgProc::procBN<t,u>(t &)
#define BN(n) BN_(uint##n##_t,uint##n##_t); BN_(int##n##_t,uint##n##_t)
BN(16);
BN(32);
BN(64);
bool MsgProc::proc48(UInteger48_t &val)
{
    uint16_t high = 0;
    uint32_t low = 0;
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
    uint16_t high = 0;
    uint32_t low = 0;
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
        #ifdef ptpm_have_USE_BIG
        USE_BIG, // float is big endian (network order)
        #endif // ptpm_have_USE_BIG
        #ifdef ptpm_have_USE_LT
        USE_LT, // float is little endian
        #endif // ptpm_have_USE_LT
    } ordMod;
    if(use64) {
        #ifdef ptpm_ordMod
        ordMod = ptpm_ordMod;
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
            if(signbit(val)) {
                num = sig_64bits; // add sign bit
                if(isfinite(val))
                    val = fabsl(val);
            } else
                num = 0;
            long double norm;
            switch(fpclassify(val)) {
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
            #ifdef ptpm_have_USE_BIG
        case USE_BIG:
            if(m_build)
                *(uint64_t *)m_cur = num;
            else
                num = *(uint64_t *)m_cur;
            move(8);
            break;
            #endif // ptpm_have_USE_BIG
        case USE_HOST:
            if(proc(num)) // host order to network order
                return true;
            break;
            #ifdef ptpm_have_USE_LT
        case USE_LT:
            if(m_build)
                *(uint64_t *)m_cur = bswap_64(num);
            else
                num = bswap_64(*(uint64_t *)m_cur);
            move(8);
            break;
            #endif // ptpm_have_USE_LT
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
                    val = numeric_limits<Float64_t>::quiet_NaN();
            } else if(exp == ieee754_exp_sub) // Subnormal or zero
                val = exp2l(ieee754_exp_min) * mnt / ieee754_mnt_base;
            else // Normal
                val = exp2l(exp) * (mnt + ieee754_mnt_base) / ieee754_mnt_base;
            if(num & sig_64bits) // Negative
                val = copysign(val, -1);
        }
    }
    return false;
}
bool MsgProc::proc(string &str, uint16_t len)
{
    if(m_build) // On build ignore length variable
        len = str.length();
    if(m_left < (ssize_t)len)
        return true;
    if(m_build)
        memcpy(m_cur, str.c_str(), len);
    else
        str.assign((char *)m_cur, len);
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
    uint8_t v = 0;
    if(m_build)
        v = val;
    bool ret = proc(v);
    if(!m_build)
        val = (T)v;
    return ret;
}
#define E8(t) template bool MsgProc::procE8<t>(t &)
E8(clockAccuracy_e);
E8(faultRecord_e);
E8(timeSource_e);
E8(portState_e);
E8(delayMechanism_e);
E8(msgType_e);
E8(SMPTEmasterLockingStatus_e);
E8(linuxptpTimeStamp_e);
E8(linuxptpUnicastState_e);
template <typename T> bool MsgProc::procE16(T &val)
{
    uint16_t v = 0;
    if(m_build)
        v = val;
    bool ret = proc(v);
    if(!m_build)
        val = (T)v;
    return ret;
}
#define E16(t) template bool MsgProc::procE16<t>(t &)
E16(networkProtocol_e);
E16(linuxptpPowerProfileVersion_e);
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
    size_t sz = d.textField.length();
    d.lengthField = sz;
    return sz > UINT8_MAX || proc(d.lengthField) ||
        proc(d.textField, d.lengthField);
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
        for(T &rec : vec) {\
            if(proc(rec))\
                return true;\
        }\
    } else
template <typename T> bool MsgProc::vector_f(uint32_t count, vector<T> &vec)
{
    vector_b(vec) {
        vec.reserve(vec.size() + count);
        for(uint32_t i = 0; i < count; i++) {
            T rec = {};
            if(proc(rec))
                return true;
            vec.push_back(std::move(rec));
        }
    }
    return false;
}
#define vf(t) template bool MsgProc::vector_f<t>(uint32_t, vector<t> &)
vf(FaultRecord_t);
vf(PortAddress_t);
vf(AcceptableMaster_t);
vf(LinuxptpUnicastMaster_t);
template <typename T> bool MsgProc::vector_o(vector<T> &vec)
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
#define vo(t) template bool MsgProc::vector_o<t>(vector<t> &)
vo(ClockIdentity_t);
vo(SLAVE_RX_SYNC_TIMING_DATA_rec_t);
vo(SLAVE_RX_SYNC_COMPUTED_DATA_rec_t);
vo(SLAVE_TX_EVENT_TIMESTAMPS_rec_t);
vo(SLAVE_DELAY_TIMING_DATA_NP_rec_t);

template <typename T> size_t vector_l(size_t ret, vector<T> &vec) PURE;

// For Octets arrays
#define oproc(a) proc(a, sizeof a)
#define fproc procFlags(d.flags, d.flagsMask)

// size of variable length list
template <typename T> size_t vector_l(size_t ret, const vector<T> &vec)
{
    for(const T &rec : vec)
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
}

ssize_t Message::dataFieldSize(const BaseMngTlv *data) const
{
#define _ptpmCaseUFS(n) case n: {\
            if(data != nullptr) {\
                const n##_t *a=dynamic_cast<const n##_t*>(data);\
                if(a != nullptr)\
                    return n##_s(*a);\
            }\
            n##_t empty;\
            return n##_s(empty); }
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
        procRes() || proc(d.productDescription) ||
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
    return fproc || procRes() || proc(d.numberPorts) ||
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
    return proc(d.parentPortIdentity) || fproc || procRes() ||
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
}
A(UNICAST_MASTER_TABLE_NP)
{
    d.actualTableSize = d.unicastMasters.size();
    if(proc(d.actualTableSize))
        return true;
    return vector_f(d.actualTableSize, d.unicastMasters);
}
A(PORT_HWCLOCK_NP)
{
    return proc(d.portIdentity) || proc(d.phc_index) || proc(d.flags);
}
A(POWER_PROFILE_SETTINGS_NP)
{
    return proc(d.version) || proc(d.grandmasterID) ||
        proc(d.grandmasterTimeInaccuracy) || proc(d.networkTimeInaccuracy) ||
        proc(d.totalTimeInaccuracy);
}
A(CMLDS_INFO_NP)
{
    return proc(d.meanLinkDelay) || proc(d.scaledNeighborRateRatio) ||
        proc(d.as_capable);
}
A(PORT_CORRECTIONS_NP)
{
    return proc(d.egressLatency) || proc(d.ingressLatency) ||
        proc(d.delayAsymmetry);
}
A(EXTERNAL_GRANDMASTER_PROPERTIES_NP)
{
    return proc(d.gmIdentity) || proc(d.stepsRemoved);
}

#undef A
#define C1(n)\
    static inline void n##_c1(const n##_t &d, ptpmgmt_##n##_t &a, void *&x, bool &e)

#define C1_tbl(n, nm, sz)\
    a.nm = d.nm;\
    if((ssize_t)d.nm != (ssize_t)d.sz.size()) {\
        e = true;\
        return;\
    }\
    if(d.nm == 0)\
        return;\
    x = malloc(sizeof(ptpmgmt_##n##_t) * d.nm);\
    if(x == nullptr) {\
        e = true;\
        return;\
    }\
    ptpmgmt_##n##_t *m = (ptpmgmt_##n##_t *)x

#define C1_MM(n) m[i].n = f.n
#define C1_MC(n, c) a.n = (ptpmgmt_##c##_e)d.n
#define C_M(n) a.n = d.n
#define C_CP(n, s) memcpy(a.n, d.n, s)
#define C_VARA(n) c_st(a.n, d.n)
#define C_VARM(n) c_st(m[i].n, f.n)
#define C_VARR(n) c_st(r.n, f.n)
// Used in sig.cpp
void c_st(ptpmgmt_PTPText_t &a, const PTPText_t &d)
{
    size_t sz = d.textField.size();
    if(sz > 0) {
        a.lengthField = std::min(sz, (size_t)UINT8_MAX);
        a.textField = d.textField.c_str();
    } else {
        a.lengthField = 0;
        a.textField = nullptr;
    }
}
#define C1_B(b, s) c_st(a.b, a.s, d.b, d.s)
static inline void c_st(uint8_t *&b, uint16_t &n, const Binary &d, uint16_t m)
{
    n = d.empty() ? 0 : m;
    b = n > 0 ? const_cast<uint8_t *>(d.get()) : nullptr;
}
// Used in sig.cpp
void c_st(ptpmgmt_PortAddress_t &a, const PortAddress_t &d)
{
    C1_MC(networkProtocol, networkProtocol);
    C1_B(addressField, addressLength);
}
static inline void c_st(ptpmgmt_ClockQuality_t &a, const ClockQuality_t &d)
{
    C_M(clockClass);
    C1_MC(clockAccuracy, clockAccuracy);
    C_M(offsetScaledLogVariance);
}
static inline void c_st(ptpmgmt_PortIdentity_t &a, const PortIdentity_t &d)
{
    C_M(portNumber);
    C_CP(clockIdentity.v, ClockIdentity_t::size());
}

C1(CLOCK_DESCRIPTION)
{
    C_M(clockType);
    C_VARA(physicalLayerProtocol);
    C1_B(physicalAddress, physicalAddressLength);
    C_VARA(protocolAddress);
    C_CP(manufacturerIdentity, 3);
    C_VARA(productDescription);
    C_VARA(revisionData);
    C_VARA(userDescription);
    C_CP(profileIdentity, 6);
}
C1(USER_DESCRIPTION)
{
    C_VARA(userDescription);
}
C1(INITIALIZE)
{
    C_M(initializationKey);
}
C1(FAULT_LOG)
{
    C1_tbl(FaultRecord, numberOfFaultRecords, faultRecords);
    for(int i = 0; i < d.numberOfFaultRecords; i++) {
        const FaultRecord_t &f = d.faultRecords[i];
        C1_MM(faultRecordLength);
        C1_MM(faultTime.secondsField);
        C1_MM(faultTime.nanosecondsField);
        m[i].severityCode = (ptpmgmt_faultRecord_e)f.severityCode;
        C_VARM(faultName);
        C_VARM(faultValue);
        C_VARM(faultDescription);
    }
    a.faultRecords = m;
}
C1(DEFAULT_DATA_SET)
{
    C_M(flags);
    C_M(numberPorts);
    C_M(priority1);
    C_VARA(clockQuality);
    C_M(priority2);
    C_CP(clockIdentity.v, ClockIdentity_t::size());
    C_M(domainNumber);
}
C1(CURRENT_DATA_SET)
{
    C_M(stepsRemoved);
    C_M(offsetFromMaster.scaledNanoseconds);
    C_M(meanPathDelay.scaledNanoseconds);
}
C1(PARENT_DATA_SET)
{
    C_VARA(parentPortIdentity);
    C_M(flags);
    C_M(observedParentOffsetScaledLogVariance);
    C_M(observedParentClockPhaseChangeRate);
    C_M(grandmasterPriority1);
    C_VARA(grandmasterClockQuality);
    C_M(grandmasterPriority2);
    C_CP(grandmasterIdentity.v, ClockIdentity_t::size());
}
C1(TIME_PROPERTIES_DATA_SET)
{
    C_M(currentUtcOffset);
    C_M(flags);
    C1_MC(timeSource, timeSource);
}
C1(PORT_DATA_SET)
{
    C_VARA(portIdentity);
    C1_MC(portState, portState);
    C_M(logMinDelayReqInterval);
    C_M(peerMeanPathDelay.scaledNanoseconds);
    C_M(logAnnounceInterval);
    C_M(announceReceiptTimeout);
    C_M(logSyncInterval);
    C1_MC(delayMechanism, delayMechanism);
    C_M(logMinPdelayReqInterval);
    C_M(versionNumber);
}
C1(PRIORITY1)
{
    C_M(priority1);
}
C1(PRIORITY2)
{
    C_M(priority2);
}
C1(DOMAIN)
{
    C_M(domainNumber);
}
C1(SLAVE_ONLY)
{
    C_M(flags);
}
C1(LOG_ANNOUNCE_INTERVAL)
{
    C_M(logAnnounceInterval);
}
C1(ANNOUNCE_RECEIPT_TIMEOUT)
{
    C_M(announceReceiptTimeout);
}
C1(LOG_SYNC_INTERVAL)
{
    C_M(logSyncInterval);
}
C1(VERSION_NUMBER)
{
    C_M(versionNumber);
}
C1(TIME)
{
    C_M(currentTime.secondsField);
    C_M(currentTime.nanosecondsField);
}
C1(CLOCK_ACCURACY)
{
    C1_MC(clockAccuracy, clockAccuracy);
}
C1(UTC_PROPERTIES)
{
    C_M(currentUtcOffset);
    C_M(flags);
}
C1(TRACEABILITY_PROPERTIES)
{
    C_M(flags);
}
C1(TIMESCALE_PROPERTIES)
{
    C_M(flags);
    C1_MC(timeSource, timeSource);
}
C1(UNICAST_NEGOTIATION_ENABLE)
{
    C_M(flags);
}
C1(PATH_TRACE_LIST)
{
    size_t sz = d.pathSequence.size();
    size_t s = sizeof(ptpmgmt_ClockIdentity_t) * sz;
    x = malloc(s + sizeof(ptpmgmt_ClockIdentity_t));
    if(x == nullptr) {
        e = true;
        return;
    }
    ptpmgmt_ClockIdentity_t *m = (ptpmgmt_ClockIdentity_t *)x;
    if(sz > 0)
        memcpy(m, d.pathSequence.data(), s);
    memset(m + sz, 0, sizeof(ptpmgmt_ClockIdentity_t));
    a.pathSequence = m;
}
C1(PATH_TRACE_ENABLE)
{
    C_M(flags);
}
C1(GRANDMASTER_CLUSTER_TABLE)
{
    C_M(logQueryInterval);
    C1_tbl(PortAddress, actualTableSize, PortAddress);
    for(int i = 0; i < d.actualTableSize; i++)
        c_st(m[i], d.PortAddress[i]);
    a.PortAddress = m;
}
C1(UNICAST_MASTER_TABLE)
{
    C_M(logQueryInterval);
    C1_tbl(PortAddress, actualTableSize, PortAddress);
    for(int i = 0; i < d.actualTableSize; i++)
        c_st(m[i], d.PortAddress[i]);
    a.PortAddress = m;
}
C1(UNICAST_MASTER_MAX_TABLE_SIZE)
{
    C_M(maxTableSize);
}
C1(ACCEPTABLE_MASTER_TABLE)
{
    C1_tbl(AcceptableMaster, actualTableSize, list);
    for(int i = 0; i < d.actualTableSize; i++) {
        const AcceptableMaster_t &f = d.list[i];
        C_VARM(acceptablePortIdentity);
        C1_MM(alternatePriority1);
    }
    a.list = m;
}
C1(ACCEPTABLE_MASTER_TABLE_ENABLED)
{
    C_M(flags);
}
C1(ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
{
    C_M(maxTableSize);
}
C1(ALTERNATE_MASTER)
{
    C_M(flags);
    C_M(logAlternateMulticastSyncInterval);
    C_M(numberOfAlternateMasters);
}
C1(ALTERNATE_TIME_OFFSET_ENABLE)
{
    C_M(keyField);
    C_M(flags);
}
C1(ALTERNATE_TIME_OFFSET_NAME)
{
    C_M(keyField);
    C_VARA(displayName);
}
C1(ALTERNATE_TIME_OFFSET_MAX_KEY)
{
    C_M(maxKey);
}
C1(ALTERNATE_TIME_OFFSET_PROPERTIES)
{
    C_M(keyField);
    C_M(currentOffset);
    C_M(jumpSeconds);
    C_M(timeOfNextJump);
}
C1(TRANSPARENT_CLOCK_PORT_DATA_SET)
{
    C_VARA(portIdentity);
    C_M(flags);
    C_M(logMinPdelayReqInterval);
    C_M(peerMeanPathDelay.scaledNanoseconds);
}
C1(LOG_MIN_PDELAY_REQ_INTERVAL)
{
    C_M(logMinPdelayReqInterval);
}
C1(TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
{
    C_CP(clockIdentity.v, ClockIdentity_t::size());
    C_M(numberPorts);
    C1_MC(delayMechanism, delayMechanism);
    C_M(primaryDomain);
}
C1(PRIMARY_DOMAIN)
{
    C_M(primaryDomain);
}
C1(DELAY_MECHANISM)
{
    C1_MC(delayMechanism, delayMechanism);
}
C1(EXTERNAL_PORT_CONFIGURATION_ENABLED)
{
    C_M(flags);
}
C1(MASTER_ONLY)
{
    C_M(flags);
}
C1(HOLDOVER_UPGRADE_ENABLE)
{
    C_M(flags);
}
C1(EXT_PORT_CONFIG_PORT_DATA_SET)
{
    C_M(flags);
    C1_MC(desiredState, portState);
}
C1(TIME_STATUS_NP)
{
    C_M(master_offset);
    C_M(ingress_time);
    C_M(cumulativeScaledRateOffset);
    C_M(scaledLastGmPhaseChange);
    C_M(gmTimeBaseIndicator);
    C_M(nanoseconds_msb);
    C_M(nanoseconds_lsb);
    C_M(fractional_nanoseconds);
    C_M(gmPresent);
    C_CP(gmIdentity.v, ClockIdentity_t::size());
}
C1(GRANDMASTER_SETTINGS_NP)
{
    C_VARA(clockQuality);
    C_M(currentUtcOffset);
    C_M(flags);
    C1_MC(timeSource, timeSource);
}
C1(PORT_DATA_SET_NP)
{
    C_M(neighborPropDelayThresh);
    C_M(asCapable);
}
C1(SUBSCRIBE_EVENTS_NP)
{
    C_M(duration);
    C_CP(bitmask, sizeof(uint8_t) * EVENT_BITMASK_CNT);
}
C1(PORT_PROPERTIES_NP)
{
    C_VARA(portIdentity);
    C1_MC(portState, portState);
    C1_MC(timestamping, linuxptpTimeStamp);
    C_VARA(interface);
}
C1(PORT_STATS_NP)
{
    C_VARA(portIdentity);
    C_CP(rxMsgType, sizeof(uint64_t) * MAX_MESSAGE_TYPES);
    C_CP(txMsgType, sizeof(uint64_t) * MAX_MESSAGE_TYPES);
}
C1(SYNCHRONIZATION_UNCERTAIN_NP)
{
    C_M(val);
}
C1(PORT_SERVICE_STATS_NP)
{
    C_VARA(portIdentity);
    C_M(announce_timeout);
    C_M(sync_timeout);
    C_M(delay_timeout);
    C_M(unicast_service_timeout);
    C_M(unicast_request_timeout);
    C_M(master_announce_timeout);
    C_M(master_sync_timeout);
    C_M(qualification_timeout);
    C_M(sync_mismatch);
    C_M(followup_mismatch);
}
C1(UNICAST_MASTER_TABLE_NP)
{
    C1_tbl(LinuxptpUnicastMaster, actualTableSize, unicastMasters);
    for(int i = 0; i < d.actualTableSize; i++) {
        const LinuxptpUnicastMaster_t &f = d.unicastMasters[i];
        C_VARM(portIdentity);
        C_VARM(clockQuality);
        C1_MM(selected);
        m[i].portState = (ptpmgmt_linuxptpUnicastState_e)f.portState;
        C1_MM(priority1);
        C1_MM(priority2);
        C_VARM(portAddress);
    }
    a.unicastMasters = m;
}
C1(PORT_HWCLOCK_NP)
{
    C_VARA(portIdentity);
    C_M(phc_index);
    C_M(flags);
}
C1(POWER_PROFILE_SETTINGS_NP)
{
    C1_MC(version, linuxptpPowerProfileVersion);
    C_M(grandmasterID);
    C_M(grandmasterTimeInaccuracy);
    C_M(networkTimeInaccuracy);
    C_M(totalTimeInaccuracy);
}
C1(CMLDS_INFO_NP)
{
    C_M(meanLinkDelay.scaledNanoseconds);
    C_M(scaledNeighborRateRatio);
    C_M(as_capable);
}
C1(PORT_CORRECTIONS_NP)
{
    C_M(egressLatency);
    C_M(ingressLatency);
    C_M(delayAsymmetry);
}
C1(EXTERNAL_GRANDMASTER_PROPERTIES_NP)
{
    C_CP(gmIdentity.v, ClockIdentity_t::size());
    C_M(stepsRemoved);
}

void *cpp2cMngTlv(mng_vals_e tlv_id, const BaseMngTlv *data, void *&x)
{
#define _ptpmCaseUF(n) case n: {\
            m = malloc(sizeof(ptpmgmt_##n##_t));\
            if(m == nullptr)break;\
            const n##_t *a=dynamic_cast<const n##_t*>(data);\
            if(a != nullptr){\
                e = false;\
                n##_c1(*a, *(ptpmgmt_##n##_t *)m, x, e);}}break;
    if(data == nullptr)
        return nullptr;
    void *m = nullptr;
    bool e = true;
    x = nullptr;
    switch(tlv_id) {
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
#include "ids.h"
        case SMPTE_MNG_ID:
            return cpp2cSmpte(data);
        default:
            break;
    }
    if(e) {
        free(m);
        return nullptr;
    }
    return m;
}

#define C2(n)\
    static inline void n##_c2(n##_t &a, const ptpmgmt_##n##_t &d)
#define C2_MR(n) r.n = f.n
#define C2_MC(n, c) a.n = (c##_e)d.n
#define C2_MRC(n, c) r.n = (c##_e)f.n
static inline void c_st(PTPText_t &a, const ptpmgmt_PTPText_t &d)
{
    uint8_t l = d.textField == nullptr ? 0 : d.lengthField;
    a.lengthField = l;
    if(l > 0)
        a.textField.assign(d.textField, l);
}
#define C2_B(b, s) c_st(a.b, a.s, d.b, d.s)
static inline void c_st(Binary &a, uint16_t &n, const uint8_t *d, uint16_t m)
{
    n = d == nullptr ? 0 : m;
    if(n > 0)
        a.setBin(d, n);
}
static inline void c_st(PortAddress_t &a, const ptpmgmt_PortAddress_t &d)
{
    C2_MC(networkProtocol, networkProtocol);
    C2_B(addressField, addressLength);
}
static inline void c_st(ClockQuality_t &a, const ptpmgmt_ClockQuality_t &d)
{
    C_M(clockClass);
    C2_MC(clockAccuracy, clockAccuracy);
    C_M(offsetScaledLogVariance);
}
static inline void c_st(PortIdentity_t &a, const ptpmgmt_PortIdentity_t &d)
{
    C_M(portNumber);
    C_CP(clockIdentity.v, ClockIdentity_t::size());
}

C2(CLOCK_DESCRIPTION)
{
    C_M(clockType);
    C_VARA(physicalLayerProtocol);
    C2_B(physicalAddress, physicalAddressLength);
    C_VARA(protocolAddress);
    C_CP(manufacturerIdentity, 3);
    C_VARA(productDescription);
    C_VARA(revisionData);
    C_VARA(userDescription);
    C_CP(profileIdentity, 6);
}
C2(USER_DESCRIPTION)
{
    C_VARA(userDescription);
}
C2(INITIALIZE)
{
    C_M(initializationKey);
}
C2(FAULT_LOG)
{
    C_M(numberOfFaultRecords);
    a.faultRecords.resize(d.numberOfFaultRecords);
    for(int i = 0; i < d.numberOfFaultRecords; i++) {
        FaultRecord_t &r = a.faultRecords[i];
        const ptpmgmt_FaultRecord_t &f = d.faultRecords[i];
        C2_MR(faultRecordLength);
        C2_MR(faultTime.secondsField);
        C2_MR(faultTime.nanosecondsField);
        C2_MRC(severityCode, faultRecord);
        C_VARR(faultName);
        C_VARR(faultValue);
        C_VARR(faultDescription);
    }
}
C2(DEFAULT_DATA_SET)
{
    C_M(flags);
    C_M(numberPorts);
    C_M(priority1);
    C_VARA(clockQuality);
    C_M(priority2);
    C_CP(clockIdentity.v, ClockIdentity_t::size());
    C_M(domainNumber);
}
C2(CURRENT_DATA_SET)
{
    C_M(stepsRemoved);
    C_M(offsetFromMaster.scaledNanoseconds);
    C_M(meanPathDelay.scaledNanoseconds);
}
C2(PARENT_DATA_SET)
{
    C_VARA(parentPortIdentity);
    C_M(flags);
    C_M(observedParentOffsetScaledLogVariance);
    C_M(observedParentClockPhaseChangeRate);
    C_M(grandmasterPriority1);
    C_VARA(grandmasterClockQuality);
    C_M(grandmasterPriority2);
    C_CP(grandmasterIdentity.v, ClockIdentity_t::size());
}
C2(TIME_PROPERTIES_DATA_SET)
{
    C_M(currentUtcOffset);
    C_M(flags);
    C2_MC(timeSource, timeSource);
}
C2(PORT_DATA_SET)
{
    C_VARA(portIdentity);
    C2_MC(portState, portState);
    C_M(logMinDelayReqInterval);
    C_M(peerMeanPathDelay.scaledNanoseconds);
    C_M(logAnnounceInterval);
    C_M(announceReceiptTimeout);
    C_M(logSyncInterval);
    C2_MC(delayMechanism, delayMechanism);
    C_M(logMinPdelayReqInterval);
    C_M(versionNumber);
}
C2(PRIORITY1)
{
    C_M(priority1);
}
C2(PRIORITY2)
{
    C_M(priority2);
}
C2(DOMAIN)
{
    C_M(domainNumber);
}
C2(SLAVE_ONLY)
{
    C_M(flags);
}
C2(LOG_ANNOUNCE_INTERVAL)
{
    C_M(logAnnounceInterval);
}
C2(ANNOUNCE_RECEIPT_TIMEOUT)
{
    C_M(announceReceiptTimeout);
}
C2(LOG_SYNC_INTERVAL)
{
    C_M(logSyncInterval);
}
C2(VERSION_NUMBER)
{
    C_M(versionNumber);
}
C2(TIME)
{
    C_M(currentTime.secondsField);
    C_M(currentTime.nanosecondsField);
}
C2(CLOCK_ACCURACY)
{
    C2_MC(clockAccuracy, clockAccuracy);
}
C2(UTC_PROPERTIES)
{
    C_M(currentUtcOffset);
    C_M(flags);
}
C2(TRACEABILITY_PROPERTIES)
{
    C_M(flags);
}
C2(TIMESCALE_PROPERTIES)
{
    C_M(flags);
    C2_MC(timeSource, timeSource);
}
C2(UNICAST_NEGOTIATION_ENABLE)
{
    C_M(flags);
}
C2(PATH_TRACE_LIST)
{
    int i = 0;
    const ClockIdentity_t zero = {0};
    if(d.pathSequence != nullptr)
        for(;;) {
            ClockIdentity_t r;
            const Octet_t *v = d.pathSequence[i++].v;
            if(memcmp(v, zero.v, ClockIdentity_t::size()) == 0)
                return;
            memcpy(r.v, v, ClockIdentity_t::size());
            a.pathSequence.push_back(std::move(r));
        }
}
C2(PATH_TRACE_ENABLE)
{
    C_M(flags);
}
C2(GRANDMASTER_CLUSTER_TABLE)
{
    C_M(logQueryInterval);
    C_M(actualTableSize);
    a.PortAddress.resize(d.actualTableSize);
    for(int i = 0; i < d.actualTableSize; i++)
        C_VARA(PortAddress[i]);
}
C2(UNICAST_MASTER_TABLE)
{
    C_M(logQueryInterval);
    C_M(actualTableSize);
    a.PortAddress.resize(d.actualTableSize);
    for(int i = 0; i < d.actualTableSize; i++)
        C_VARA(PortAddress[i]);
}
C2(UNICAST_MASTER_MAX_TABLE_SIZE)
{
    C_M(maxTableSize);
}
C2(ACCEPTABLE_MASTER_TABLE)
{
    C_M(actualTableSize);
    a.list.resize(d.actualTableSize);
    for(int i = 0; i < d.actualTableSize; i++) {
        AcceptableMaster_t &r = a.list[i];
        const ptpmgmt_AcceptableMaster_t &f = d.list[i];
        C_VARR(acceptablePortIdentity);
        C2_MR(alternatePriority1);
    }
}
C2(ACCEPTABLE_MASTER_TABLE_ENABLED)
{
    C_M(flags);
}
C2(ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
{
    C_M(maxTableSize);
}
C2(ALTERNATE_MASTER)
{
    C_M(flags);
    C_M(logAlternateMulticastSyncInterval);
    C_M(numberOfAlternateMasters);
}
C2(ALTERNATE_TIME_OFFSET_ENABLE)
{
    C_M(keyField);
    C_M(flags);
}
C2(ALTERNATE_TIME_OFFSET_NAME)
{
    C_M(keyField);
    C_VARA(displayName);
}
C2(ALTERNATE_TIME_OFFSET_MAX_KEY)
{
    C_M(maxKey);
}
C2(ALTERNATE_TIME_OFFSET_PROPERTIES)
{
    C_M(keyField);
    C_M(currentOffset);
    C_M(jumpSeconds);
    C_M(timeOfNextJump);
}
C2(TRANSPARENT_CLOCK_PORT_DATA_SET)
{
    C_VARA(portIdentity);
    C_M(flags);
    C_M(logMinPdelayReqInterval);
    C_M(peerMeanPathDelay.scaledNanoseconds);
}
C2(LOG_MIN_PDELAY_REQ_INTERVAL)
{
    C_M(logMinPdelayReqInterval);
}
C2(TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
{
    C_CP(clockIdentity.v, ClockIdentity_t::size());
    C_M(numberPorts);
    C2_MC(delayMechanism, delayMechanism);
    C_M(primaryDomain);
}
C2(PRIMARY_DOMAIN)
{
    C_M(primaryDomain);
}
C2(DELAY_MECHANISM)
{
    C2_MC(delayMechanism, delayMechanism);
}
C2(EXTERNAL_PORT_CONFIGURATION_ENABLED)
{
    C_M(flags);
}
C2(MASTER_ONLY)
{
    C_M(flags);
}
C2(HOLDOVER_UPGRADE_ENABLE)
{
    C_M(flags);
}
C2(EXT_PORT_CONFIG_PORT_DATA_SET)
{
    C_M(flags);
    C2_MC(desiredState, portState);
}
C2(TIME_STATUS_NP)
{
    C_M(master_offset);
    C_M(ingress_time);
    C_M(cumulativeScaledRateOffset);
    C_M(scaledLastGmPhaseChange);
    C_M(gmTimeBaseIndicator);
    C_M(nanoseconds_msb);
    C_M(nanoseconds_lsb);
    C_M(fractional_nanoseconds);
    C_M(gmPresent);
    C_CP(gmIdentity.v, ClockIdentity_t::size());
}
C2(GRANDMASTER_SETTINGS_NP)
{
    C_VARA(clockQuality);
    C_M(currentUtcOffset);
    C_M(flags);
    C2_MC(timeSource, timeSource);
}
C2(PORT_DATA_SET_NP)
{
    C_M(neighborPropDelayThresh);
    C_M(asCapable);
}
C2(SUBSCRIBE_EVENTS_NP)
{
    C_M(duration);
    C_CP(bitmask, sizeof(uint8_t) * EVENT_BITMASK_CNT);
}
C2(PORT_PROPERTIES_NP)
{
    C_VARA(portIdentity);
    C2_MC(portState, portState);
    C2_MC(timestamping, linuxptpTimeStamp);
    C_VARA(interface);
}
C2(PORT_STATS_NP)
{
    C_VARA(portIdentity);
    C_CP(rxMsgType, sizeof(uint64_t) * MAX_MESSAGE_TYPES);
    C_CP(txMsgType, sizeof(uint64_t) * MAX_MESSAGE_TYPES);
}
C2(SYNCHRONIZATION_UNCERTAIN_NP)
{
    C_M(val);
}
C2(PORT_SERVICE_STATS_NP)
{
    C_VARA(portIdentity);
    C_M(announce_timeout);
    C_M(sync_timeout);
    C_M(delay_timeout);
    C_M(unicast_service_timeout);
    C_M(unicast_request_timeout);
    C_M(master_announce_timeout);
    C_M(master_sync_timeout);
    C_M(qualification_timeout);
    C_M(sync_mismatch);
    C_M(followup_mismatch);
}
C2(UNICAST_MASTER_TABLE_NP)
{
    C_M(actualTableSize);
    a.unicastMasters.resize(d.actualTableSize);
    for(int i = 0; i < d.actualTableSize; i++) {
        LinuxptpUnicastMaster_t &r = a.unicastMasters[i];
        const ptpmgmt_LinuxptpUnicastMaster_t &f = d.unicastMasters[i];
        C_VARR(portIdentity);
        C_VARR(clockQuality);
        C2_MR(selected);
        C2_MRC(portState, linuxptpUnicastState);
        C2_MR(priority1);
        C2_MR(priority2);
        C_VARR(portAddress);
    }
}
C2(PORT_HWCLOCK_NP)
{
    C_VARA(portIdentity);
    C_M(phc_index);
    C_M(flags);
}
C2(POWER_PROFILE_SETTINGS_NP)
{
    C2_MC(version, linuxptpPowerProfileVersion);
    C_M(grandmasterID);
    C_M(grandmasterTimeInaccuracy);
    C_M(networkTimeInaccuracy);
    C_M(totalTimeInaccuracy);
}
C2(CMLDS_INFO_NP)
{
    C_M(meanLinkDelay.scaledNanoseconds);
    C_M(scaledNeighborRateRatio);
    C_M(as_capable);
}
C2(PORT_CORRECTIONS_NP)
{
    C_M(egressLatency);
    C_M(ingressLatency);
    C_M(delayAsymmetry);
}
C2(EXTERNAL_GRANDMASTER_PROPERTIES_NP)
{
    C_CP(gmIdentity.v, ClockIdentity_t::size());
    C_M(stepsRemoved);
}

BaseMngTlv *c2cppMngTlv(mng_vals_e tlv_id, const void *data)
{
#define _ptpmCaseUF(n) case n:\
        if(data != nullptr) {\
            ptpmgmt_##n##_t *m = (ptpmgmt_##n##_t *)data;\
            if(m == nullptr)break;\
            n##_t *a=new n##_t;\
            if(a != nullptr){n##_c2(*a, *m);return a;}}break;
    switch(tlv_id) {
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
#include "ids.h"
        default:
            break;
    }
    return nullptr;
}

__PTPMGMT_NAMESPACE_END

__PTPMGMT_NAMESPACE_USE;

static inline bool div_event(int event, div_t &d)
{
    if(event < 0 || event >= EVENT_BITMASK_CNT)
        return false;
    d = SUBSCRIBE_EVENTS_NP_t::div_event(event);
    return true;
}

extern "C" {
    void ptpmgmt_setEvent_lnp(ptpmgmt_SUBSCRIBE_EVENTS_NP_t *e, int event)
    {
        div_t d;
        if(div_event(event, d))
            e->bitmask[d.quot] |= d.rem;
    }
    void ptpmgmt_clearEvent_lnp(ptpmgmt_SUBSCRIBE_EVENTS_NP_t *e, int event)
    {
        div_t d;
        if(div_event(event, d))
            e->bitmask[d.quot] &= ~d.rem;
    }
    void ptpmgmt_clearAll_lnp(ptpmgmt_SUBSCRIBE_EVENTS_NP_t *e)
    {
        memset(e->bitmask, 0, EVENT_BITMASK_CNT);
    }
    bool ptpmgmt_getEvent_lnp(const ptpmgmt_SUBSCRIBE_EVENTS_NP_t *e, int event)
    {
        div_t d;
        if(div_event(event, d))
            return (e->bitmask[d.quot] & d.rem) > 0;
        return false;
    }
}
