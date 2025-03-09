/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Parse signalling TLVs
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#include "comp.h"
#include "c/sig.h"

__PTPMGMT_NAMESPACE_BEGIN

#define A(n) bool MsgProc::n##_f(n##_t &d)
A(ORGANIZATION_EXTENSION)
{
    return proc(d.organizationId, 3) ||
        proc(d.organizationSubType, 3) ||
        proc(d.dataField, m_left);
}
A(PATH_TRACE)
{
    return vector_o(d.pathSequence);
}
A(ALTERNATE_TIME_OFFSET_INDICATOR)
{
    return proc(d.keyField) || proc(d.currentOffset) || proc(d.jumpSeconds) ||
        proc48(d.timeOfNextJump) || proc(d.displayName);
}
A(ENHANCED_ACCURACY_METRICS)
{
    return proc(d.bcHopCount) || proc(d.tcHopCount) || procRes() ||
        procRes() || proc(d.maxGmInaccuracy) || proc(d.varGmInaccuracy) ||
        proc(d.maxTransientInaccuracy) || proc(d.varTransientInaccuracy) ||
        proc(d.maxDynamicInaccuracy) || proc(d.varDynamicInaccuracy) ||
        proc(d.maxStaticInstanceInaccuracy) ||
        proc(d.varStaticInstanceInaccuracy) ||
        proc(d.maxStaticMediumInaccuracy) || proc(d.varStaticMediumInaccuracy);
}
A(L1_SYNC)
{
    return procFlags(d.flags1, d.flagsMask1) || procFlags(d.flags2, d.flagsMask2);
}
A(PORT_COMMUNICATION_AVAILABILITY)
{
    return procFlags(d.syncMessageAvailability, d.flagsMask1) ||
        procFlags(d.delayRespMessageAvailability, d.flagsMask2);
}
A(PROTOCOL_ADDRESS)
{
    return proc(d.portProtocolAddress);
}
A(SLAVE_RX_SYNC_TIMING_DATA)
{
    if(proc(d.syncSourcePortIdentity))
        return true;
    return vector_o(d.list);
}
A(SLAVE_RX_SYNC_COMPUTED_DATA)
{
    if(proc(d.sourcePortIdentity) || procFlags(d.computedFlags, d.flagsMask) ||
        procRes())
        return true;
    return vector_o(d.list);
}
A(SLAVE_TX_EVENT_TIMESTAMPS)
{
    if(proc(d.sourcePortIdentity) || proc(d.eventMessageType) || procRes())
        return true;
    return vector_o(d.list);
}
A(CUMULATIVE_RATE_RATIO)
{
    return proc(d.scaledCumulativeRateRatio);
}
A(SMPTE_ORGANIZATION_EXTENSION)
{
    bool ret = proc(d.organizationId, 3);
    if(ret)
        return true;
    if(memcmp(d.organizationId, "\x68\x97\xe8", 3) != 0) {
        m_err = MNG_PARSE_ERROR_INVALID_ID;
        return true;
    }
    return proc(d.organizationSubType, 3) ||
        proc(d.defaultSystemFrameRate_numerator) ||
        proc(d.defaultSystemFrameRate_denominator) ||
        proc(d.masterLockingStatus) || proc(d.timeAddressFlags) ||
        proc(d.currentLocalOffset) || proc(d.jumpSeconds) ||
        proc48(d.timeOfNextJump) || proc48(d.timeOfNextJam) ||
        proc48(d.timeOfPreviousJam) || proc(d.previousJamLocalOffset) ||
        proc(d.daylightSaving) || proc(d.leapSecondJump);
}
bool MsgProc::proc(SLAVE_RX_SYNC_TIMING_DATA_rec_t &d)
{
    return proc(d.sequenceId) || proc(d.syncOriginTimestamp) ||
        proc(d.totalCorrectionField) || proc(d.scaledCumulativeRateOffset) ||
        proc(d.syncEventIngressTimestamp);
}
bool MsgProc::proc(SLAVE_RX_SYNC_COMPUTED_DATA_rec_t &d)
{
    return proc(d.sequenceId) || proc(d.offsetFromMaster) ||
        proc(d.meanPathDelay) || proc(d.scaledNeighborRateRatio);
}
bool MsgProc::proc(SLAVE_TX_EVENT_TIMESTAMPS_rec_t &d)
{
    return proc(d.sequenceId) || proc(d.eventEgressTimestamp);
}
A(SLAVE_DELAY_TIMING_DATA_NP)
{
    if(proc(d.sourcePortIdentity))
        return true;
    return vector_o(d.list);
}
bool MsgProc::proc(SLAVE_DELAY_TIMING_DATA_NP_rec_t &d)
{
    return proc(d.sequenceId) || proc(d.delayOriginTimestamp) ||
        proc(d.totalCorrectionField) || proc(d.delayResponseTimestamp);
}

#define C1(n)\
    static inline void n##_c1(const n##_t &d, ptpmgmt_##n##_t &a,\
        void *&x, void *&x2, bool &e)
#define C1_tbl(n, sz)\
    size_t _s = sizeof(ptpmgmt_##n##_t) * ((sz) + 1);\
    x = malloc(_s);\
    if(x == nullptr) {e = true;return;}\
    memset(x, 0, _s);\
    ptpmgmt_##n##_t *m = (ptpmgmt_##n##_t *)x;\
    size_t i = 0;\
    for(; i < (sz); i++)

#define C1_M(n) a.n = d.n
#define C1_CP(n, s) memcpy(a.n, d.n, s)
#define C1_MM(n) m[i].n = f.n
#define C1_MC(n, c) a.n = (ptpmgmt_##c##_e)d.n
#define C1_VARA(n) c_st(a.n, d.n)
#define C1_VARM(n) c_st(m[i].n, f.n)
// From proc.cpp
void c_st(ptpmgmt_PTPText_t &a, const PTPText_t &d);
void c_st(ptpmgmt_PortAddress_t &a, const PortAddress_t &d);
static inline void c_st(ptpmgmt_PortIdentity_t &a, const PortIdentity_t &d)
{
    C1_M(portNumber);
    C1_CP(clockIdentity.v, ClockIdentity_t::size());
}
static inline void c_st(ptpmgmt_Timestamp_t &a, const Timestamp_t &d)
{
    C1_M(secondsField);
    C1_M(nanosecondsField);
}
static inline void c_st(ptpmgmt_TimeInterval_t &a, const TimeInterval_t &d)
{
    C1_M(scaledNanoseconds);
}

C1(ORGANIZATION_EXTENSION)
{
    C1_CP(organizationId, 3);
    C1_CP(organizationSubType, 3);
    a.dataField = d.dataField.empty() ? nullptr :
        const_cast<uint8_t *>(d.dataField.get());
}
C1(PATH_TRACE)
{
    C1_tbl(ClockIdentity, d.pathSequence.size()) {
        memcpy(m[i].v, d.pathSequence[i].v, ClockIdentity_t::size());
        ;
    }
    a.pathSequence = m;
}
C1(ALTERNATE_TIME_OFFSET_INDICATOR)
{
    C1_M(keyField);
    C1_M(currentOffset);
    C1_M(jumpSeconds);
    C1_M(timeOfNextJump);
    C1_VARA(displayName);
}
C1(ENHANCED_ACCURACY_METRICS)
{
    C1_M(bcHopCount);
    C1_M(tcHopCount);
    C1_VARA(maxGmInaccuracy);
    C1_M(varGmInaccuracy);
    C1_VARA(maxTransientInaccuracy);
    C1_M(varTransientInaccuracy);
    C1_VARA(maxDynamicInaccuracy);
    C1_M(varDynamicInaccuracy);
    C1_VARA(maxStaticInstanceInaccuracy);
    C1_M(varStaticInstanceInaccuracy);
    C1_VARA(maxStaticMediumInaccuracy);
    C1_M(varStaticMediumInaccuracy);
}
C1(L1_SYNC)
{
    C1_M(flags1);
    C1_M(flags2);
}
C1(PORT_COMMUNICATION_AVAILABILITY)
{
    C1_M(syncMessageAvailability);
    C1_M(delayRespMessageAvailability);
}
C1(PROTOCOL_ADDRESS)
{
    C1_VARA(portProtocolAddress);
}
C1(SLAVE_RX_SYNC_TIMING_DATA)
{
    C1_VARA(syncSourcePortIdentity);
    C1_tbl(SLAVE_RX_SYNC_TIMING_DATA_rec, d.list.size()) {
        const SLAVE_RX_SYNC_TIMING_DATA_rec_t &f = d.list[i];
        C1_MM(sequenceId);
        C1_VARM(syncOriginTimestamp);
        C1_VARM(totalCorrectionField);
        C1_MM(scaledCumulativeRateOffset);
        C1_VARM(syncEventIngressTimestamp);
    }
    a.list = m;
}
C1(SLAVE_RX_SYNC_COMPUTED_DATA)
{
    C1_VARA(sourcePortIdentity);
    C1_M(computedFlags);
    C1_tbl(SLAVE_RX_SYNC_COMPUTED_DATA_rec, d.list.size()) {
        const SLAVE_RX_SYNC_COMPUTED_DATA_rec_t &f = d.list[i];
        C1_MM(sequenceId);
        C1_VARM(offsetFromMaster);
        C1_VARM(meanPathDelay);
        C1_MM(scaledNeighborRateRatio);
    }
    a.list = m;
}
C1(SLAVE_TX_EVENT_TIMESTAMPS)
{
    C1_VARA(sourcePortIdentity);
    C1_MC(eventMessageType, msgType);
    C1_tbl(SLAVE_TX_EVENT_TIMESTAMPS_rec, d.list.size()) {
        const SLAVE_TX_EVENT_TIMESTAMPS_rec_t &f = d.list[i];
        C1_MM(sequenceId);
        C1_VARM(eventEgressTimestamp);
    }
    a.list = m;
}
C1(CUMULATIVE_RATE_RATIO)
{
    C1_M(scaledCumulativeRateRatio);
}
C1(MANAGEMENT_ERROR_STATUS)
{
    C1_MC(managementId, mng_vals);
    C1_MC(managementErrorId, managementErrorId);
    C1_VARA(displayData);
}
C1(MANAGEMENT)
{
    x = cpp2cMngTlv(d.managementId, d.tlvData.get(), x2);
    if(x == nullptr) {
        e = true;
        return;
    }
    C1_MC(managementId, mng_vals);
    a.tlvData = x;
}
C1(SLAVE_DELAY_TIMING_DATA_NP)
{
    C1_VARA(sourcePortIdentity);
    C1_tbl(SLAVE_DELAY_TIMING_DATA_NP_rec, d.list.size()) {
        const SLAVE_DELAY_TIMING_DATA_NP_rec_t &f = d.list[i];
        C1_MM(sequenceId);
        C1_VARM(delayOriginTimestamp);
        C1_VARM(totalCorrectionField);
        C1_VARM(delayResponseTimestamp);
    }
    a.list = m;
}

void *cpp2cSigTlv(tlvType_e tlv_id, const BaseSigTlv *data, void *&x, void *&x2)
{
    if(data == nullptr)
        return nullptr;
    void *a = nullptr;
    x = nullptr;
    x2 = nullptr;
    bool e = true;
#define caseBuild(n) n: { \
            a = malloc(sizeof(ptpmgmt_##n##_t));\
            if(a == nullptr)break;\
            const n##_t *d=dynamic_cast<const n##_t*>(data);\
            if(d != nullptr){\
                e = false;\
                n##_c1(*d, *(ptpmgmt_##n##_t *)a, x, x2, e);}}break;
    switch(tlv_id) {
        case ORGANIZATION_EXTENSION_PROPAGATE:
            FALLTHROUGH;
        case ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE:
            FALLTHROUGH;
        case caseBuild(ORGANIZATION_EXTENSION);
        case caseBuild(PATH_TRACE);
        case caseBuild(ALTERNATE_TIME_OFFSET_INDICATOR);
        case caseBuild(ENHANCED_ACCURACY_METRICS);
        case caseBuild(L1_SYNC);
        case caseBuild(PORT_COMMUNICATION_AVAILABILITY);
        case caseBuild(PROTOCOL_ADDRESS);
        case caseBuild(SLAVE_RX_SYNC_TIMING_DATA);
        case caseBuild(SLAVE_RX_SYNC_COMPUTED_DATA);
        case caseBuild(SLAVE_TX_EVENT_TIMESTAMPS);
        case caseBuild(CUMULATIVE_RATE_RATIO);
        case caseBuild(MANAGEMENT_ERROR_STATUS);
        case caseBuild(MANAGEMENT);
        case caseBuild(SLAVE_DELAY_TIMING_DATA_NP);
        default:
            break;
    }
    if(e) {
        free(a);
        return nullptr;
    }
    return a;
}

void *cpp2cSmpte(const BaseMngTlv *tlv)
{
    if(tlv == nullptr)
        return nullptr;
    const SMPTE_ORGANIZATION_EXTENSION_t *d0 =
        dynamic_cast<const SMPTE_ORGANIZATION_EXTENSION_t *>(tlv);
    if(d0 == nullptr)
        return nullptr;
    void *x = malloc(sizeof(ptpmgmt_SMPTE_ORGANIZATION_EXTENSION_t));
    if(x == nullptr)
        return nullptr;
    const SMPTE_ORGANIZATION_EXTENSION_t &d = *d0;
    auto &a = *(ptpmgmt_SMPTE_ORGANIZATION_EXTENSION_t *)x;
    C1_CP(organizationId, 3);
    C1_CP(organizationSubType, 3);
    C1_M(defaultSystemFrameRate_numerator);
    C1_M(defaultSystemFrameRate_denominator);
    C1_MC(masterLockingStatus, SMPTEmasterLockingStatus);
    C1_M(timeAddressFlags);
    C1_M(currentLocalOffset);
    C1_M(jumpSeconds);
    C1_M(timeOfNextJump);
    C1_M(timeOfNextJam);
    C1_M(timeOfPreviousJam);
    C1_M(previousJamLocalOffset);
    C1_M(daylightSaving);
    C1_M(leapSecondJump);
    return x;
}

__PTPMGMT_NAMESPACE_END
