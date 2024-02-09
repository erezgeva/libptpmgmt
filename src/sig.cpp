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

extern "C" {
#include "c/sig.h"
}

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
    return proc(d.bcHopCount) || proc(d.tcHopCount) || proc(reserved) ||
        proc(reserved) || proc(d.maxGmInaccuracy) || proc(d.varGmInaccuracy) ||
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
        proc(reserved))
        return true;
    return vector_o(d.list);
}
A(SLAVE_TX_EVENT_TIMESTAMPS)
{
    if(proc(d.sourcePortIdentity) || proc(d.eventMessageType) || proc(reserved))
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
    size_t _s = sizeof(ptpmgmt_##n##_t) * (sz);\
    x = malloc(_s);\
    if(x == nullptr) {e = true;return;}\
    memset(x, 0, _s);\
    ptpmgmt_##n##_t *m = (ptpmgmt_##n##_t *)x

C1(ORGANIZATION_EXTENSION)
{
    memcpy(a.organizationId, d.organizationId, 3);
    memcpy(a.organizationSubType, d.organizationSubType, 3);
    a.dataField = const_cast<uint8_t *>(d.dataField.get());
}
C1(PATH_TRACE)
{
    C1_tbl(ClockIdentity, d.pathSequence.size() + 1);
    size_t i = 0;
    for(; i < d.pathSequence.size(); i++)
        memcpy(m[i].v, d.pathSequence[i].v, ClockIdentity_t::size());
    a.pathSequence = m;
}
C1(ALTERNATE_TIME_OFFSET_INDICATOR)
{
    a.keyField = d.keyField;
    a.currentOffset = d.currentOffset;
    a.jumpSeconds = d.jumpSeconds;
    a.timeOfNextJump = d.timeOfNextJump;
    a.displayName.lengthField = d.displayName.lengthField;
    a.displayName.textField = d.displayName.textField.c_str();
}
C1(ENHANCED_ACCURACY_METRICS)
{
    a.bcHopCount = d.bcHopCount;
    a.tcHopCount = d.tcHopCount;
    a.maxGmInaccuracy.scaledNanoseconds = d.maxGmInaccuracy.scaledNanoseconds;
    a.varGmInaccuracy = d.varGmInaccuracy;
    a.maxTransientInaccuracy.scaledNanoseconds =
        d.maxTransientInaccuracy.scaledNanoseconds;
    a.varTransientInaccuracy = d.varTransientInaccuracy;
    a.maxDynamicInaccuracy.scaledNanoseconds =
        d.maxDynamicInaccuracy.scaledNanoseconds;
    a.varDynamicInaccuracy = d.varDynamicInaccuracy;
    a.maxStaticInstanceInaccuracy.scaledNanoseconds =
        d.maxStaticInstanceInaccuracy.scaledNanoseconds;
    a.varStaticInstanceInaccuracy = d.varStaticInstanceInaccuracy;
    a.maxStaticMediumInaccuracy.scaledNanoseconds =
        d.maxStaticMediumInaccuracy.scaledNanoseconds;
    a.varStaticMediumInaccuracy = d.varStaticMediumInaccuracy;
}
C1(L1_SYNC)
{
    a.flags1 = d.flags1;
    a.flags2 = d.flags2;
}
C1(PORT_COMMUNICATION_AVAILABILITY)
{
    a.syncMessageAvailability = d.syncMessageAvailability;
    a.delayRespMessageAvailability = d.delayRespMessageAvailability;
}
C1(PROTOCOL_ADDRESS)
{
    a.portProtocolAddress.networkProtocol =
        (ptpmgmt_networkProtocol_e)d.portProtocolAddress.networkProtocol;
    a.portProtocolAddress.addressLength = d.portProtocolAddress.addressLength;
    a.portProtocolAddress.addressField =
        const_cast<uint8_t *>(d.portProtocolAddress.addressField.get());
}
C1(SLAVE_RX_SYNC_TIMING_DATA)
{
    memcpy(a.syncSourcePortIdentity.clockIdentity.v,
        d.syncSourcePortIdentity.clockIdentity.v, ClockIdentity_t::size());
    a.syncSourcePortIdentity.portNumber = d.syncSourcePortIdentity.portNumber;
    C1_tbl(SLAVE_RX_SYNC_TIMING_DATA_rec, d.list.size() + 1);
    size_t i = 0;
    for(; i < d.list.size(); i++) {
        const SLAVE_RX_SYNC_TIMING_DATA_rec_t &f = d.list[i];
        m[i].sequenceId = f.sequenceId;
        m[i].syncOriginTimestamp.secondsField = f.syncOriginTimestamp.secondsField;
        m[i].syncOriginTimestamp.nanosecondsField =
            f.syncOriginTimestamp.nanosecondsField;
        m[i].totalCorrectionField.scaledNanoseconds =
            f.totalCorrectionField.scaledNanoseconds;
        m[i].scaledCumulativeRateOffset = f.scaledCumulativeRateOffset;
        m[i].syncEventIngressTimestamp.secondsField =
            f.syncEventIngressTimestamp.secondsField;
        m[i].syncEventIngressTimestamp.nanosecondsField =
            f.syncEventIngressTimestamp.nanosecondsField;
    }
    a.list = m;
}
C1(SLAVE_RX_SYNC_COMPUTED_DATA)
{
    memcpy(a.sourcePortIdentity.clockIdentity.v,
        d.sourcePortIdentity.clockIdentity.v, ClockIdentity_t::size());
    a.sourcePortIdentity.portNumber = d.sourcePortIdentity.portNumber;
    a.computedFlags = d.computedFlags;
    C1_tbl(SLAVE_RX_SYNC_COMPUTED_DATA_rec, d.list.size() + 1);
    size_t i = 0;
    for(; i < d.list.size(); i++) {
        const SLAVE_RX_SYNC_COMPUTED_DATA_rec_t &f = d.list[i];
        m[i].sequenceId = f.sequenceId;
        m[i].offsetFromMaster.scaledNanoseconds =
            f.offsetFromMaster.scaledNanoseconds;
        m[i].meanPathDelay.scaledNanoseconds = f.meanPathDelay.scaledNanoseconds;
        m[i].scaledNeighborRateRatio = f.scaledNeighborRateRatio;
    }
    a.list = m;
}
C1(SLAVE_TX_EVENT_TIMESTAMPS)
{
    memcpy(a.sourcePortIdentity.clockIdentity.v,
        d.sourcePortIdentity.clockIdentity.v, ClockIdentity_t::size());
    a.sourcePortIdentity.portNumber = d.sourcePortIdentity.portNumber;
    a.eventMessageType = (ptpmgmt_msgType_e)d.eventMessageType;
    C1_tbl(SLAVE_TX_EVENT_TIMESTAMPS_rec, d.list.size() + 1);
    size_t i = 0;
    for(; i < d.list.size(); i++) {
        const SLAVE_TX_EVENT_TIMESTAMPS_rec_t &f = d.list[i];
        m[i].sequenceId = f.sequenceId;
        m[i].eventEgressTimestamp.secondsField =
            f.eventEgressTimestamp.secondsField;
        m[i].eventEgressTimestamp.nanosecondsField =
            f.eventEgressTimestamp.nanosecondsField;
    }
    a.list = m;
}
C1(CUMULATIVE_RATE_RATIO)
{
    a.scaledCumulativeRateRatio = d.scaledCumulativeRateRatio;
}
C1(MANAGEMENT_ERROR_STATUS)
{
    a.managementId = (enum ptpmgmt_mng_vals_e)d.managementId;
    a.managementErrorId = (enum ptpmgmt_managementErrorId_e)d.managementErrorId;
    a.displayData.lengthField = d.displayData.lengthField;
    a.displayData.textField = d.displayData.textField.c_str();
}
C1(MANAGEMENT)
{
    x = cpp2cMngTlv(d.managementId, d.tlvData.get(), x2);
    if(x == nullptr) {
        e = true;
        return;
    }
    a.managementId = (enum ptpmgmt_mng_vals_e)d.managementId;
    a.tlvData = x;
}
C1(SLAVE_DELAY_TIMING_DATA_NP)
{
    memcpy(a.sourcePortIdentity.clockIdentity.v,
        d.sourcePortIdentity.clockIdentity.v, ClockIdentity_t::size());
    a.sourcePortIdentity.portNumber = d.sourcePortIdentity.portNumber;
    C1_tbl(SLAVE_DELAY_TIMING_DATA_NP_rec, d.list.size() + 1);
    size_t i = 0;
    for(; i < d.list.size(); i++) {
        const SLAVE_DELAY_TIMING_DATA_NP_rec_t &f = d.list[i];
        m[i].sequenceId = f.sequenceId;
        m[i].delayOriginTimestamp.secondsField =
            f.delayOriginTimestamp.secondsField;
        m[i].delayOriginTimestamp.nanosecondsField =
            f.delayOriginTimestamp.nanosecondsField;
        m[i].totalCorrectionField.scaledNanoseconds =
            f.totalCorrectionField.scaledNanoseconds;
        m[i].delayResponseTimestamp.secondsField =
            f.delayResponseTimestamp.secondsField;
        m[i].delayResponseTimestamp.nanosecondsField =
            f.delayResponseTimestamp.nanosecondsField;
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
    ptpmgmt_SMPTE_ORGANIZATION_EXTENSION_t &a = *
        (ptpmgmt_SMPTE_ORGANIZATION_EXTENSION_t *)x;
    memcpy(a.organizationId, d.organizationId, 3);
    memcpy(a.organizationSubType, d.organizationSubType, 3);
    a.defaultSystemFrameRate_numerator = d.defaultSystemFrameRate_numerator;
    a.defaultSystemFrameRate_denominator = d.defaultSystemFrameRate_denominator;
    a.masterLockingStatus = (ptpmgmt_SMPTEmasterLockingStatus_e)
        d.masterLockingStatus;
    a.timeAddressFlags = d.timeAddressFlags;
    a.currentLocalOffset = d.currentLocalOffset;
    a.jumpSeconds = d.jumpSeconds;
    a.timeOfNextJump = d.timeOfNextJump;
    a.timeOfNextJam = d.timeOfNextJam;
    a.timeOfPreviousJam = d.timeOfPreviousJam;
    a.previousJamLocalOffset = d.previousJamLocalOffset;
    a.daylightSaving = d.daylightSaving;
    a.leapSecondJump = d.leapSecondJump;
    return x;
}

__PTPMGMT_NAMESPACE_END
