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
        str = string((char *)m_cur, len);
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

C1(CLOCK_DESCRIPTION)
{
    a.clockType = d.clockType;
    a.physicalLayerProtocol.lengthField = d.physicalLayerProtocol.lengthField;
    a.physicalLayerProtocol.textField = d.physicalLayerProtocol.textField.c_str();
    a.physicalAddressLength = d.physicalAddressLength;
    a.physicalAddress = const_cast<uint8_t *>(d.physicalAddress.get());
    a.protocolAddress.networkProtocol =
        (ptpmgmt_networkProtocol_e)d.protocolAddress.networkProtocol;
    a.protocolAddress.addressLength = d.protocolAddress.addressLength;
    a.protocolAddress.addressField =
        const_cast<uint8_t *>(d.protocolAddress.addressField.get());
    memcpy(a.manufacturerIdentity, d.manufacturerIdentity, 3);
    a.productDescription.lengthField = d.productDescription.lengthField;
    a.productDescription.textField = d.productDescription.textField.c_str();
    a.revisionData.lengthField = d.revisionData.lengthField;
    a.revisionData.textField = d.revisionData.textField.c_str();
    a.userDescription.lengthField = d.userDescription.lengthField;
    a.userDescription.textField = d.userDescription.textField.c_str();
    memcpy(a.profileIdentity, d.profileIdentity, 6);
}
C1(USER_DESCRIPTION)
{
    a.userDescription.lengthField = d.userDescription.lengthField;
    a.userDescription.textField = d.userDescription.textField.c_str();
}
C1(INITIALIZE)
{
    a.initializationKey = d.initializationKey;
}
C1(FAULT_LOG)
{
    C1_tbl(FaultRecord, numberOfFaultRecords, faultRecords);
    for(int i = 0; i < d.numberOfFaultRecords; i++) {
        const FaultRecord_t &f = d.faultRecords[i];
        m[i].faultRecordLength = f.faultRecordLength;
        m[i].faultTime.secondsField = f.faultTime.secondsField;
        m[i].faultTime.nanosecondsField = f.faultTime.nanosecondsField;
        m[i].severityCode = (ptpmgmt_faultRecord_e)f.severityCode;
        m[i].faultName.lengthField = f.faultName.lengthField;
        m[i].faultName.textField = f.faultName.textField.c_str();
        m[i].faultValue.lengthField = f.faultValue.lengthField;
        m[i].faultValue.textField = f.faultValue.textField.c_str();
        m[i].faultDescription.lengthField = f.faultDescription.lengthField;
        m[i].faultDescription.textField = f.faultDescription.textField.c_str();
    }
    a.faultRecords = m;
}
C1(DEFAULT_DATA_SET)
{
    a.flags = d.flags;
    a.numberPorts = d.numberPorts;
    a.priority1 = d.priority1;
    a.clockQuality.clockClass = d.clockQuality.clockClass;
    a.clockQuality.clockAccuracy =
        (ptpmgmt_clockAccuracy_e)d.clockQuality.clockAccuracy;
    a.clockQuality.offsetScaledLogVariance =
        d.clockQuality.offsetScaledLogVariance;
    a.priority2 = d.priority2;
    memcpy(a.clockIdentity.v, d.clockIdentity.v, ClockIdentity_t::size());
    a.domainNumber = d.domainNumber;
}
C1(CURRENT_DATA_SET)
{
    a.stepsRemoved = d.stepsRemoved;
    a.offsetFromMaster.scaledNanoseconds = d.offsetFromMaster.scaledNanoseconds;
    a.meanPathDelay.scaledNanoseconds = d.meanPathDelay.scaledNanoseconds;
}
C1(PARENT_DATA_SET)
{
    a.parentPortIdentity.portNumber = d.parentPortIdentity.portNumber;
    memcpy(a.parentPortIdentity.clockIdentity.v,
        d.parentPortIdentity.clockIdentity.v, ClockIdentity_t::size());
    a.flags = d.flags;
    a.observedParentOffsetScaledLogVariance =
        d.observedParentOffsetScaledLogVariance;
    a.observedParentClockPhaseChangeRate = d.observedParentClockPhaseChangeRate;
    a.grandmasterPriority1 = d.grandmasterPriority1;
    a.grandmasterClockQuality.clockClass = d.grandmasterClockQuality.clockClass;
    a.grandmasterClockQuality.clockAccuracy =
        (ptpmgmt_clockAccuracy_e)d.grandmasterClockQuality.clockAccuracy;
    a.grandmasterClockQuality.offsetScaledLogVariance =
        d.grandmasterClockQuality.offsetScaledLogVariance;
    a.grandmasterPriority2 = d.grandmasterPriority2;
    memcpy(a.grandmasterIdentity.v, d.grandmasterIdentity.v,
        ClockIdentity_t::size());
}
C1(TIME_PROPERTIES_DATA_SET)
{
    a.currentUtcOffset = d.currentUtcOffset;
    a.flags = d.flags;
    a.timeSource = (ptpmgmt_timeSource_e)d.timeSource;
}
C1(PORT_DATA_SET)
{
    a.portIdentity.portNumber = d.portIdentity.portNumber;
    memcpy(a.portIdentity.clockIdentity.v, d.portIdentity.clockIdentity.v,
        ClockIdentity_t::size());
    a.portState = (ptpmgmt_portState_e)d.portState;
    a.logMinDelayReqInterval = d.logMinDelayReqInterval;
    a.peerMeanPathDelay.scaledNanoseconds = d.peerMeanPathDelay.scaledNanoseconds;
    a.logAnnounceInterval = d.logAnnounceInterval;
    a.announceReceiptTimeout = d.announceReceiptTimeout;
    a.logSyncInterval = d.logSyncInterval;
    a.delayMechanism = (ptpmgmt_delayMechanism_e)d.delayMechanism;
    a.logMinPdelayReqInterval = d.logMinPdelayReqInterval;
    a.versionNumber = d.versionNumber;
}
C1(PRIORITY1)
{
    a.priority1 = d.priority1;
}
C1(PRIORITY2)
{
    a.priority2 = d.priority2;
}
C1(DOMAIN)
{
    a.domainNumber = d.domainNumber;
}
C1(SLAVE_ONLY)
{
    a.flags = d.flags;
}
C1(LOG_ANNOUNCE_INTERVAL)
{
    a.logAnnounceInterval = d.logAnnounceInterval;
}
C1(ANNOUNCE_RECEIPT_TIMEOUT)
{
    a.announceReceiptTimeout = d.announceReceiptTimeout;
}
C1(LOG_SYNC_INTERVAL)
{
    a.logSyncInterval = d.logSyncInterval;
}
C1(VERSION_NUMBER)
{
    a.versionNumber = d.versionNumber;
}
C1(TIME)
{
    a.currentTime.secondsField = d.currentTime.secondsField;
    a.currentTime.nanosecondsField = d.currentTime.nanosecondsField;
}
C1(CLOCK_ACCURACY)
{
    a.clockAccuracy = (ptpmgmt_clockAccuracy_e)d.clockAccuracy;
}
C1(UTC_PROPERTIES)
{
    a.currentUtcOffset = d.currentUtcOffset;
    a.flags = d.flags;
}
C1(TRACEABILITY_PROPERTIES)
{
    a.flags = d.flags;
}
C1(TIMESCALE_PROPERTIES)
{
    a.flags = d.flags;
    a.timeSource = (ptpmgmt_timeSource_e)d.timeSource;
}
C1(UNICAST_NEGOTIATION_ENABLE)
{
    a.flags = d.flags;
}
C1(PATH_TRACE_LIST)
{
    size_t s = sizeof(ptpmgmt_ClockIdentity_t) * (d.pathSequence.size() + 1);
    x = malloc(s);
    if(x == nullptr) {
        e = true;
        return;
    }
    memset(x, 0, s);
    ptpmgmt_ClockIdentity_t *m = (ptpmgmt_ClockIdentity_t *)x;
    size_t i = 0;
    for(; i < d.pathSequence.size(); i++)
        memcpy(m[i].v, d.pathSequence[i].v, ClockIdentity_t::size());
    a.pathSequence = m;
}
C1(PATH_TRACE_ENABLE)
{
    a.flags = d.flags;
}
C1(GRANDMASTER_CLUSTER_TABLE)
{
    a.logQueryInterval = d.logQueryInterval;
    C1_tbl(PortAddress, actualTableSize, PortAddress);
    for(int i = 0; i < d.actualTableSize; i++) {
        m[i].networkProtocol =
            (ptpmgmt_networkProtocol_e)d.PortAddress[i].networkProtocol;
        m[i].addressLength = d.PortAddress[i].addressLength;
        m[i].addressField =
            const_cast<uint8_t *>(d.PortAddress[i].addressField.get());
    }
    a.PortAddress = m;
}
C1(UNICAST_MASTER_TABLE)
{
    a.logQueryInterval = d.logQueryInterval;
    C1_tbl(PortAddress, actualTableSize, PortAddress);
    for(int i = 0; i < d.actualTableSize; i++) {
        m[i].networkProtocol =
            (ptpmgmt_networkProtocol_e)d.PortAddress[i].networkProtocol;
        m[i].addressLength = d.PortAddress[i].addressLength;
        m[i].addressField =
            const_cast<uint8_t *>(d.PortAddress[i].addressField.get());
    }
    a.PortAddress = m;
}
C1(UNICAST_MASTER_MAX_TABLE_SIZE)
{
    a.maxTableSize = d.maxTableSize;
}
C1(ACCEPTABLE_MASTER_TABLE)
{
    C1_tbl(AcceptableMaster, actualTableSize, list);
    for(int i = 0; i < d.actualTableSize; i++) {
        m[i].acceptablePortIdentity.portNumber =
            d.list[i].acceptablePortIdentity.portNumber;
        memcpy(m[i].acceptablePortIdentity.clockIdentity.v,
            d.list[i].acceptablePortIdentity.clockIdentity.v,
            ClockIdentity_t::size());
        m[i].alternatePriority1 = d.list[i].alternatePriority1;
    }
    a.list = m;
}
C1(ACCEPTABLE_MASTER_TABLE_ENABLED)
{
    a.flags = d.flags;
}
C1(ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
{
    a.maxTableSize = d.maxTableSize;
}
C1(ALTERNATE_MASTER)
{
    a.flags = d.flags;
    a.logAlternateMulticastSyncInterval = d.logAlternateMulticastSyncInterval;
    a.numberOfAlternateMasters = d.numberOfAlternateMasters;
}
C1(ALTERNATE_TIME_OFFSET_ENABLE)
{
    a.keyField = d.keyField;
    a.flags = d.flags;
}
C1(ALTERNATE_TIME_OFFSET_NAME)
{
    a.keyField = d.keyField;
    a.displayName.lengthField = d.displayName.lengthField;
    a.displayName.textField = d.displayName.textField.c_str();
}
C1(ALTERNATE_TIME_OFFSET_MAX_KEY)
{
    a.maxKey = d.maxKey;
}
C1(ALTERNATE_TIME_OFFSET_PROPERTIES)
{
    a.keyField = d.keyField;
    a.currentOffset = d.currentOffset;
    a.jumpSeconds = d.jumpSeconds;
    a.timeOfNextJump = d.timeOfNextJump;
}
C1(TRANSPARENT_CLOCK_PORT_DATA_SET)
{
    a.portIdentity.portNumber = d.portIdentity.portNumber;
    memcpy(a.portIdentity.clockIdentity.v, d.portIdentity.clockIdentity.v,
        ClockIdentity_t::size());
    a.flags = d.flags;
    a.logMinPdelayReqInterval = d.logMinPdelayReqInterval;
    a.peerMeanPathDelay.scaledNanoseconds = d.peerMeanPathDelay.scaledNanoseconds;
}
C1(LOG_MIN_PDELAY_REQ_INTERVAL)
{
    a.logMinPdelayReqInterval = d.logMinPdelayReqInterval;
}
C1(TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
{
    memcpy(a.clockIdentity.v, d.clockIdentity.v, ClockIdentity_t::size());
    a.numberPorts = d.numberPorts;
    a.delayMechanism = (ptpmgmt_delayMechanism_e)d.delayMechanism;
    a.primaryDomain = d.primaryDomain;
}
C1(PRIMARY_DOMAIN)
{
    a.primaryDomain = d.primaryDomain;
}
C1(DELAY_MECHANISM)
{
    a.delayMechanism = (ptpmgmt_delayMechanism_e)d.delayMechanism;
}
C1(EXTERNAL_PORT_CONFIGURATION_ENABLED)
{
    a.flags = d.flags;
}
C1(MASTER_ONLY)
{
    a.flags = d.flags;
}
C1(HOLDOVER_UPGRADE_ENABLE)
{
    a.flags = d.flags;
}
C1(EXT_PORT_CONFIG_PORT_DATA_SET)
{
    a.flags = d.flags;
    a.desiredState = (ptpmgmt_portState_e)d.desiredState;
}
C1(TIME_STATUS_NP)
{
    a.master_offset = d.master_offset;
    a.ingress_time = d.ingress_time;
    a.cumulativeScaledRateOffset = d.cumulativeScaledRateOffset;
    a.scaledLastGmPhaseChange = d.scaledLastGmPhaseChange;
    a.gmTimeBaseIndicator = d.gmTimeBaseIndicator;
    a.nanoseconds_msb = d.nanoseconds_msb;
    a.nanoseconds_lsb = d.nanoseconds_lsb;
    a.fractional_nanoseconds = d.fractional_nanoseconds;
    a.gmPresent = d.gmPresent;
    memcpy(a.gmIdentity.v, d.gmIdentity.v, ClockIdentity_t::size());
}
C1(GRANDMASTER_SETTINGS_NP)
{
    a.clockQuality.clockClass = d.clockQuality.clockClass;
    a.clockQuality.clockAccuracy =
        (ptpmgmt_clockAccuracy_e)d.clockQuality.clockAccuracy;
    a.clockQuality.offsetScaledLogVariance =
        d.clockQuality.offsetScaledLogVariance;
    a.currentUtcOffset = d.currentUtcOffset;
    a.flags = d.flags;
    a.timeSource = (ptpmgmt_timeSource_e)d.timeSource;
}
C1(PORT_DATA_SET_NP)
{
    a.neighborPropDelayThresh = d.neighborPropDelayThresh;
    a.asCapable = d.asCapable;
}
C1(SUBSCRIBE_EVENTS_NP)
{
    a.duration = d.duration;
    memcpy(a.bitmask, d.bitmask, sizeof(uint8_t) * EVENT_BITMASK_CNT);
}
C1(PORT_PROPERTIES_NP)
{
    a.portIdentity.portNumber = d.portIdentity.portNumber;
    memcpy(a.portIdentity.clockIdentity.v, d.portIdentity.clockIdentity.v,
        ClockIdentity_t::size());
    a.portState = (ptpmgmt_portState_e)d.portState;
    a.timestamping = (ptpmgmt_linuxptpTimeStamp_e)d.timestamping;
    a.interface.lengthField = d.interface.lengthField;
    a.interface.textField = d.interface.textField.c_str();
}
C1(PORT_STATS_NP)
{
    a.portIdentity.portNumber = d.portIdentity.portNumber;
    memcpy(a.portIdentity.clockIdentity.v, d.portIdentity.clockIdentity.v,
        ClockIdentity_t::size());
    memcpy(a.rxMsgType, d.rxMsgType, sizeof(uint64_t) * MAX_MESSAGE_TYPES);
    memcpy(a.txMsgType, d.txMsgType, sizeof(uint64_t) * MAX_MESSAGE_TYPES);
}
C1(SYNCHRONIZATION_UNCERTAIN_NP)
{
    a.val = d.val;
}
C1(PORT_SERVICE_STATS_NP)
{
    a.portIdentity.portNumber = d.portIdentity.portNumber;
    memcpy(a.portIdentity.clockIdentity.v, d.portIdentity.clockIdentity.v,
        ClockIdentity_t::size());
    a.announce_timeout = d.announce_timeout;
    a.sync_timeout = d.sync_timeout;
    a.delay_timeout = d.delay_timeout;
    a.unicast_service_timeout = d.unicast_service_timeout;
    a.unicast_request_timeout = d.unicast_request_timeout;
    a.master_announce_timeout = d.master_announce_timeout;
    a.master_sync_timeout = d.master_sync_timeout;
    a.qualification_timeout = d.qualification_timeout;
    a.sync_mismatch = d.sync_mismatch;
    a.followup_mismatch = d.followup_mismatch;
}
C1(UNICAST_MASTER_TABLE_NP)
{
    C1_tbl(LinuxptpUnicastMaster, actualTableSize, unicastMasters);
    for(int i = 0; i < d.actualTableSize; i++) {
        const LinuxptpUnicastMaster_t &f = d.unicastMasters[i];
        m[i].portIdentity.portNumber = f.portIdentity.portNumber;
        memcpy(m[i].portIdentity.clockIdentity.v, f.portIdentity.clockIdentity.v,
            ClockIdentity_t::size());
        m[i].clockQuality.clockClass = f.clockQuality.clockClass;
        m[i].clockQuality.clockAccuracy =
            (ptpmgmt_clockAccuracy_e)f.clockQuality.clockAccuracy;
        m[i].clockQuality.offsetScaledLogVariance =
            f.clockQuality.offsetScaledLogVariance;
        m[i].selected = f.selected;
        m[i].portState = (ptpmgmt_linuxptpUnicastState_e)f.portState;
        m[i].priority1 = f.priority1;
        m[i].priority2 = f.priority2;
        m[i].portAddress.networkProtocol =
            (ptpmgmt_networkProtocol_e)f.portAddress.networkProtocol;
        m[i].portAddress.addressLength = f.portAddress.addressLength;
        m[i].portAddress.addressField =
            const_cast<uint8_t *>(f.portAddress.addressField.get());
    }
    a.unicastMasters = m;
}
C1(PORT_HWCLOCK_NP)
{
    a.portIdentity.portNumber = d.portIdentity.portNumber;
    memcpy(a.portIdentity.clockIdentity.v, d.portIdentity.clockIdentity.v,
        ClockIdentity_t::size());
    a.phc_index = d.phc_index;
    a.flags = d.flags;
}
C1(POWER_PROFILE_SETTINGS_NP)
{
    a.version = (ptpmgmt_linuxptpPowerProfileVersion_e)d.version;
    a.grandmasterID = d.grandmasterID;
    a.grandmasterTimeInaccuracy = d.grandmasterTimeInaccuracy;
    a.networkTimeInaccuracy = d.networkTimeInaccuracy;
    a.totalTimeInaccuracy = d.totalTimeInaccuracy;
}
C1(CMLDS_INFO_NP)
{
    a.meanLinkDelay.scaledNanoseconds = d.meanLinkDelay.scaledNanoseconds;
    a.scaledNeighborRateRatio = d.scaledNeighborRateRatio;
    a.as_capable = d.as_capable;
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

C2(CLOCK_DESCRIPTION)
{
    a.clockType = d.clockType;
    a.physicalLayerProtocol.lengthField = d.physicalLayerProtocol.lengthField;
    if(d.physicalLayerProtocol.textField != nullptr &&
        d.physicalLayerProtocol.lengthField > 0)
        a.physicalLayerProtocol.textField =
            string(d.physicalLayerProtocol.textField,
                d.physicalLayerProtocol.lengthField);
    a.physicalAddressLength = d.physicalAddressLength;
    if(d.physicalAddress != nullptr && d.physicalAddressLength > 0)
        a.physicalAddress.setBin(d.physicalAddress, d.physicalAddressLength);
    a.protocolAddress.networkProtocol =
        (networkProtocol_e)d.protocolAddress.networkProtocol;
    a.protocolAddress.addressLength = d.protocolAddress.addressLength;
    if(d.protocolAddress.addressField != nullptr &&
        d.protocolAddress.addressLength > 0)
        a.protocolAddress.addressField.setBin(d.protocolAddress.addressField,
            d.protocolAddress.addressLength);
    memcpy(a.manufacturerIdentity, d.manufacturerIdentity, 3);
    a.productDescription.lengthField = d.productDescription.lengthField;
    if(d.productDescription.textField != nullptr &&
        d.productDescription.lengthField > 0)
        a.productDescription.textField = string(d.productDescription.textField,
                d.productDescription.lengthField);
    a.revisionData.lengthField = d.revisionData.lengthField;
    if(d.revisionData.textField != nullptr && d.revisionData.lengthField > 0)
        a.revisionData.textField = string(d.revisionData.textField,
                d.revisionData.lengthField);
    a.userDescription.lengthField = d.userDescription.lengthField;
    if(d.userDescription.textField != nullptr && d.userDescription.lengthField > 0)
        a.userDescription.textField = string(d.userDescription.textField,
                d.userDescription.lengthField);
    memcpy(a.profileIdentity, d.profileIdentity, 6);
}
C2(USER_DESCRIPTION)
{
    a.userDescription.lengthField = d.userDescription.lengthField;
    if(d.userDescription.textField != nullptr && d.userDescription.lengthField > 0)
        a.userDescription.textField = string(d.userDescription.textField,
                d.userDescription.lengthField);
}
C2(INITIALIZE)
{
    a.initializationKey = d.initializationKey;
}
C2(FAULT_LOG)
{
    a.numberOfFaultRecords = d.numberOfFaultRecords;
    a.faultRecords.resize(d.numberOfFaultRecords);
    for(int i = 0; i < d.numberOfFaultRecords; i++) {
        FaultRecord_t &r = a.faultRecords[i];
        const ptpmgmt_FaultRecord_t &f = d.faultRecords[i];
        r.faultRecordLength = f.faultRecordLength;
        r.faultTime.secondsField = f.faultTime.secondsField;
        r.faultTime.nanosecondsField = f.faultTime.nanosecondsField;
        r.severityCode = (faultRecord_e)f.severityCode;
        r.faultName.lengthField = f.faultName.lengthField;
        if(f.faultName.textField != nullptr && f.faultName.lengthField > 0)
            r.faultName.textField = string(f.faultName.textField,
                    f.faultName.lengthField);
        r.faultValue.lengthField = f.faultValue.lengthField;
        if(f.faultValue.textField != nullptr && f.faultValue.lengthField > 0)
            r.faultValue.textField = string(f.faultValue.textField,
                    f.faultValue.lengthField);
        r.faultDescription.lengthField = f.faultDescription.lengthField;
        if(f.faultDescription.textField != nullptr &&
            f.faultDescription.lengthField > 0)
            r.faultDescription.textField = string(f.faultDescription.textField,
                    f.faultDescription.lengthField);
    }
}
C2(DEFAULT_DATA_SET)
{
    a.flags = d.flags;
    a.numberPorts = d.numberPorts;
    a.priority1 = d.priority1;
    a.clockQuality.clockClass = d.clockQuality.clockClass;
    a.clockQuality.clockAccuracy =
        (clockAccuracy_e)d.clockQuality.clockAccuracy;
    a.clockQuality.offsetScaledLogVariance =
        d.clockQuality.offsetScaledLogVariance;
    a.priority2 = d.priority2;
    memcpy(a.clockIdentity.v, d.clockIdentity.v, ClockIdentity_t::size());
    a.domainNumber = d.domainNumber;
}
C2(CURRENT_DATA_SET)
{
    a.stepsRemoved = d.stepsRemoved;
    a.offsetFromMaster.scaledNanoseconds = d.offsetFromMaster.scaledNanoseconds;
    a.meanPathDelay.scaledNanoseconds = d.meanPathDelay.scaledNanoseconds;
}
C2(PARENT_DATA_SET)
{
    a.parentPortIdentity.portNumber = d.parentPortIdentity.portNumber;
    memcpy(a.parentPortIdentity.clockIdentity.v,
        d.parentPortIdentity.clockIdentity.v, ClockIdentity_t::size());
    a.flags = d.flags;
    a.observedParentOffsetScaledLogVariance =
        d.observedParentOffsetScaledLogVariance;
    a.observedParentClockPhaseChangeRate = d.observedParentClockPhaseChangeRate;
    a.grandmasterPriority1 = d.grandmasterPriority1;
    a.grandmasterClockQuality.clockClass = d.grandmasterClockQuality.clockClass;
    a.grandmasterClockQuality.clockAccuracy =
        (clockAccuracy_e)d.grandmasterClockQuality.clockAccuracy;
    a.grandmasterClockQuality.offsetScaledLogVariance =
        d.grandmasterClockQuality.offsetScaledLogVariance;
    a.grandmasterPriority2 = d.grandmasterPriority2;
    memcpy(a.grandmasterIdentity.v, d.grandmasterIdentity.v,
        ClockIdentity_t::size());
}
C2(TIME_PROPERTIES_DATA_SET)
{
    a.currentUtcOffset = d.currentUtcOffset;
    a.flags = d.flags;
    a.timeSource = (timeSource_e)d.timeSource;
}
C2(PORT_DATA_SET)
{
    a.portIdentity.portNumber = d.portIdentity.portNumber;
    memcpy(a.portIdentity.clockIdentity.v, d.portIdentity.clockIdentity.v,
        ClockIdentity_t::size());
    a.portState = (portState_e)d.portState;
    a.logMinDelayReqInterval = d.logMinDelayReqInterval;
    a.peerMeanPathDelay.scaledNanoseconds = d.peerMeanPathDelay.scaledNanoseconds;
    a.logAnnounceInterval = d.logAnnounceInterval;
    a.announceReceiptTimeout = d.announceReceiptTimeout;
    a.logSyncInterval = d.logSyncInterval;
    a.delayMechanism = (delayMechanism_e)d.delayMechanism;
    a.logMinPdelayReqInterval = d.logMinPdelayReqInterval;
    a.versionNumber = d.versionNumber;
}
C2(PRIORITY1)
{
    a.priority1 = d.priority1;
}
C2(PRIORITY2)
{
    a.priority2 = d.priority2;
}
C2(DOMAIN)
{
    a.domainNumber = d.domainNumber;
}
C2(SLAVE_ONLY)
{
    a.flags = d.flags;
}
C2(LOG_ANNOUNCE_INTERVAL)
{
    a.logAnnounceInterval = d.logAnnounceInterval;
}
C2(ANNOUNCE_RECEIPT_TIMEOUT)
{
    a.announceReceiptTimeout = d.announceReceiptTimeout;
}
C2(LOG_SYNC_INTERVAL)
{
    a.logSyncInterval = d.logSyncInterval;
}
C2(VERSION_NUMBER)
{
    a.versionNumber = d.versionNumber;
}
C2(TIME)
{
    a.currentTime.secondsField = d.currentTime.secondsField;
    a.currentTime.nanosecondsField = d.currentTime.nanosecondsField;
}
C2(CLOCK_ACCURACY)
{
    a.clockAccuracy = (clockAccuracy_e)d.clockAccuracy;
}
C2(UTC_PROPERTIES)
{
    a.currentUtcOffset = d.currentUtcOffset;
    a.flags = d.flags;
}
C2(TRACEABILITY_PROPERTIES)
{
    a.flags = d.flags;
}
C2(TIMESCALE_PROPERTIES)
{
    a.flags = d.flags;
    a.timeSource = (timeSource_e)d.timeSource;
}
C2(UNICAST_NEGOTIATION_ENABLE)
{
    a.flags = d.flags;
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
            a.pathSequence.push_back(r);
        }
}
C2(PATH_TRACE_ENABLE)
{
    a.flags = d.flags;
}
C2(GRANDMASTER_CLUSTER_TABLE)
{
    a.logQueryInterval = d.logQueryInterval;
    a.actualTableSize = d.actualTableSize;
    a.PortAddress.resize(d.actualTableSize);
    for(int i = 0; i < d.actualTableSize; i++) {
        PortAddress_t &r = a.PortAddress[i];
        r.networkProtocol = (networkProtocol_e)d.PortAddress[i].networkProtocol;
        r.addressLength = d.PortAddress[i].addressLength;
        if(d.PortAddress[i].addressField != nullptr && r.addressLength > 0)
            r.addressField.setBin(d.PortAddress[i].addressField, r.addressLength);
    }
}
C2(UNICAST_MASTER_TABLE)
{
    a.logQueryInterval = d.logQueryInterval;
    a.actualTableSize = d.actualTableSize;
    a.PortAddress.resize(d.actualTableSize);
    for(int i = 0; i < d.actualTableSize; i++) {
        PortAddress_t &r = a.PortAddress[i];
        r.networkProtocol = (networkProtocol_e)d.PortAddress[i].networkProtocol;
        r.addressLength = d.PortAddress[i].addressLength;
        if(d.PortAddress[i].addressField != nullptr && r.addressLength > 0)
            r.addressField.setBin(d.PortAddress[i].addressField, r.addressLength);
    }
}
C2(UNICAST_MASTER_MAX_TABLE_SIZE)
{
    a.maxTableSize = d.maxTableSize;
}
C2(ACCEPTABLE_MASTER_TABLE)
{
    a.actualTableSize = d.actualTableSize;
    a.list.resize(d.actualTableSize);
    for(int i = 0; i < d.actualTableSize; i++) {
        AcceptableMaster_t &r = a.list[i];
        r.acceptablePortIdentity.portNumber =
            d.list[i].acceptablePortIdentity.portNumber;
        memcpy(r.acceptablePortIdentity.clockIdentity.v,
            d.list[i].acceptablePortIdentity.clockIdentity.v,
            ClockIdentity_t::size());
        r.alternatePriority1 = d.list[i].alternatePriority1;
    }
}
C2(ACCEPTABLE_MASTER_TABLE_ENABLED)
{
    a.flags = d.flags;
}
C2(ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
{
    a.maxTableSize = d.maxTableSize;
}
C2(ALTERNATE_MASTER)
{
    a.flags = d.flags;
    a.logAlternateMulticastSyncInterval = d.logAlternateMulticastSyncInterval;
    a.numberOfAlternateMasters = d.numberOfAlternateMasters;
}
C2(ALTERNATE_TIME_OFFSET_ENABLE)
{
    a.keyField = d.keyField;
    a.flags = d.flags;
}
C2(ALTERNATE_TIME_OFFSET_NAME)
{
    a.keyField = d.keyField;
    a.displayName.lengthField = d.displayName.lengthField;
    if(d.displayName.textField != nullptr && d.displayName.lengthField > 0)
        a.displayName.textField = string(d.displayName.textField,
                d.displayName.lengthField);
}
C2(ALTERNATE_TIME_OFFSET_MAX_KEY)
{
    a.maxKey = d.maxKey;
}
C2(ALTERNATE_TIME_OFFSET_PROPERTIES)
{
    a.keyField = d.keyField;
    a.currentOffset = d.currentOffset;
    a.jumpSeconds = d.jumpSeconds;
    a.timeOfNextJump = d.timeOfNextJump;
}
C2(TRANSPARENT_CLOCK_PORT_DATA_SET)
{
    a.portIdentity.portNumber = d.portIdentity.portNumber;
    memcpy(a.portIdentity.clockIdentity.v, d.portIdentity.clockIdentity.v,
        ClockIdentity_t::size());
    a.flags = d.flags;
    a.logMinPdelayReqInterval = d.logMinPdelayReqInterval;
    a.peerMeanPathDelay.scaledNanoseconds = d.peerMeanPathDelay.scaledNanoseconds;
}
C2(LOG_MIN_PDELAY_REQ_INTERVAL)
{
    a.logMinPdelayReqInterval = d.logMinPdelayReqInterval;
}
C2(TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
{
    memcpy(a.clockIdentity.v, d.clockIdentity.v, ClockIdentity_t::size());
    a.numberPorts = d.numberPorts;
    a.delayMechanism = (delayMechanism_e)d.delayMechanism;
    a.primaryDomain = d.primaryDomain;
}
C2(PRIMARY_DOMAIN)
{
    a.primaryDomain = d.primaryDomain;
}
C2(DELAY_MECHANISM)
{
    a.delayMechanism = (delayMechanism_e)d.delayMechanism;
}
C2(EXTERNAL_PORT_CONFIGURATION_ENABLED)
{
    a.flags = d.flags;
}
C2(MASTER_ONLY)
{
    a.flags = d.flags;
}
C2(HOLDOVER_UPGRADE_ENABLE)
{
    a.flags = d.flags;
}
C2(EXT_PORT_CONFIG_PORT_DATA_SET)
{
    a.flags = d.flags;
    a.desiredState = (portState_e)d.desiredState;
}
C2(TIME_STATUS_NP)
{
    a.master_offset = d.master_offset;
    a.ingress_time = d.ingress_time;
    a.cumulativeScaledRateOffset = d.cumulativeScaledRateOffset;
    a.scaledLastGmPhaseChange = d.scaledLastGmPhaseChange;
    a.gmTimeBaseIndicator = d.gmTimeBaseIndicator;
    a.nanoseconds_msb = d.nanoseconds_msb;
    a.nanoseconds_lsb = d.nanoseconds_lsb;
    a.fractional_nanoseconds = d.fractional_nanoseconds;
    a.gmPresent = d.gmPresent;
    memcpy(a.gmIdentity.v, d.gmIdentity.v, ClockIdentity_t::size());
}
C2(GRANDMASTER_SETTINGS_NP)
{
    a.clockQuality.clockClass = d.clockQuality.clockClass;
    a.clockQuality.clockAccuracy =
        (clockAccuracy_e)d.clockQuality.clockAccuracy;
    a.clockQuality.offsetScaledLogVariance =
        d.clockQuality.offsetScaledLogVariance;
    a.currentUtcOffset = d.currentUtcOffset;
    a.flags = d.flags;
    a.timeSource = (timeSource_e)d.timeSource;
}
C2(PORT_DATA_SET_NP)
{
    a.neighborPropDelayThresh = d.neighborPropDelayThresh;
    a.asCapable = d.asCapable;
}
C2(SUBSCRIBE_EVENTS_NP)
{
    a.duration = d.duration;
    memcpy(a.bitmask, d.bitmask, sizeof(uint8_t) * EVENT_BITMASK_CNT);
}
C2(PORT_PROPERTIES_NP)
{
    a.portIdentity.portNumber = d.portIdentity.portNumber;
    memcpy(a.portIdentity.clockIdentity.v, d.portIdentity.clockIdentity.v,
        ClockIdentity_t::size());
    a.portState = (portState_e)d.portState;
    a.timestamping = (linuxptpTimeStamp_e)d.timestamping;
    a.interface.lengthField = d.interface.lengthField;
    if(d.interface.textField != nullptr && d.interface.lengthField > 0)
        a.interface.textField = string(d.interface.textField,
                d.interface.lengthField);
}
C2(PORT_STATS_NP)
{
    a.portIdentity.portNumber = d.portIdentity.portNumber;
    memcpy(a.portIdentity.clockIdentity.v, d.portIdentity.clockIdentity.v,
        ClockIdentity_t::size());
    memcpy(a.rxMsgType, d.rxMsgType, sizeof(uint64_t) * MAX_MESSAGE_TYPES);
    memcpy(a.txMsgType, d.txMsgType, sizeof(uint64_t) * MAX_MESSAGE_TYPES);
}
C2(SYNCHRONIZATION_UNCERTAIN_NP)
{
    a.val = d.val;
}
C2(PORT_SERVICE_STATS_NP)
{
    a.portIdentity.portNumber = d.portIdentity.portNumber;
    memcpy(a.portIdentity.clockIdentity.v, d.portIdentity.clockIdentity.v,
        ClockIdentity_t::size());
    a.announce_timeout = d.announce_timeout;
    a.sync_timeout = d.sync_timeout;
    a.delay_timeout = d.delay_timeout;
    a.unicast_service_timeout = d.unicast_service_timeout;
    a.unicast_request_timeout = d.unicast_request_timeout;
    a.master_announce_timeout = d.master_announce_timeout;
    a.master_sync_timeout = d.master_sync_timeout;
    a.qualification_timeout = d.qualification_timeout;
    a.sync_mismatch = d.sync_mismatch;
    a.followup_mismatch = d.followup_mismatch;
}
C2(UNICAST_MASTER_TABLE_NP)
{
    a.actualTableSize = d.actualTableSize;
    a.unicastMasters.resize(d.actualTableSize);
    for(int i = 0; i < d.actualTableSize; i++) {
        LinuxptpUnicastMaster_t &r = a.unicastMasters[i];
        const ptpmgmt_LinuxptpUnicastMaster_t &f = d.unicastMasters[i];
        r.portIdentity.portNumber = f.portIdentity.portNumber;
        memcpy(r.portIdentity.clockIdentity.v, f.portIdentity.clockIdentity.v,
            ClockIdentity_t::size());
        r.clockQuality.clockClass = f.clockQuality.clockClass;
        r.clockQuality.clockAccuracy =
            (clockAccuracy_e)f.clockQuality.clockAccuracy;
        r.clockQuality.offsetScaledLogVariance =
            f.clockQuality.offsetScaledLogVariance;
        r.selected = f.selected;
        r.portState = (linuxptpUnicastState_e)f.portState;
        r.priority1 = f.priority1;
        r.priority2 = f.priority2;
        r.portAddress.networkProtocol =
            (networkProtocol_e)f.portAddress.networkProtocol;
        r.portAddress.addressLength = f.portAddress.addressLength;
        if(f.portAddress.addressField != nullptr && f.portAddress.addressLength > 0)
            r.portAddress.addressField.setBin(f.portAddress.addressField,
                f.portAddress.addressLength);
    }
}
C2(PORT_HWCLOCK_NP)
{
    a.portIdentity.portNumber = d.portIdentity.portNumber;
    memcpy(a.portIdentity.clockIdentity.v, d.portIdentity.clockIdentity.v,
        ClockIdentity_t::size());
    a.phc_index = d.phc_index;
    a.flags = d.flags;
}
C2(POWER_PROFILE_SETTINGS_NP)
{
    a.version = (linuxptpPowerProfileVersion_e)d.version;
    a.grandmasterID = d.grandmasterID;
    a.grandmasterTimeInaccuracy = d.grandmasterTimeInaccuracy;
    a.networkTimeInaccuracy = d.networkTimeInaccuracy;
    a.totalTimeInaccuracy = d.totalTimeInaccuracy;
}
C2(CMLDS_INFO_NP)
{
    a.meanLinkDelay.scaledNanoseconds = d.meanLinkDelay.scaledNanoseconds;
    a.scaledNeighborRateRatio = d.scaledNeighborRateRatio;
    a.as_capable = d.as_capable;
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
