/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief parse, buid or both function per PTP managmet id
 *
 * @author Erez Geva <ErezGeva2@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */
#include <cstring>
#include "msg.h"

#define sproc(a) proc((uint8_t*)(a), sizeof(a))
#define vector_b(type, vec)\
    if(m_build) {\
        for(type##_t &rec: d.vec) {\
            if(proc(rec)) return true;\
        }\
    } else
#define vector_f(type, cnt, vec) {\
        vector_b(type, vec) {\
            for(uint16_t i = 0; i < d.cnt; i++) {\
                type##_t rec;\
                if(proc(rec)) return true;\
                d.vec.push_back(rec);\
            }\
        }\
        return false;\
    }
#define vector_l(pre_size, type, vec) {\
        size_t ret = pre_size;\
        for(type##_t &rec: d.vec)\
            ret += message::type##_l(rec);\
        return ret;\
    }

/* size functions per id */
#define CS(n) n: return n##_s(*(n##_t*)m_dataSend)
#define S(n) static inline size_t n##_s(n##_t &d)

S(USER_DESCRIPTION)
{
    return message::PTPText_l(d.userDescription);
}
S(FAULT_LOG)
{
    vector_l(2, FaultRecord, faultRecords);
}
S(UNICAST_MASTER_TABLE)
{
    vector_l(3, PortAddress, PortAddress);
}
S(GRANDMASTER_CLUSTER_TABLE)
{
    vector_l(2, PortAddress, PortAddress);
}
S(ACCEPTABLE_MASTER_TABLE)
{
    return 2 + sizeof(AcceptableMaster_t) * d.list.size();
}
S(ALTERNATE_TIME_OFFSET_NAME)
{
    return 1 + message::PTPText_l(d.displayName);
}
S(PATH_TRACE_LIST)
{
    return sizeof(ClockIdentity_t) * d.pathSequence.size();
}
// linuxptp TLVs Implementation-specific C000-DFFF
S(PORT_PROPERTIES_NP)
{
    return 2 + sizeof(PortIdentity_t) + message::PTPText_l(d.interface);
}

ssize_t message::dataFieldSize()
{
    switch(m_tlv_id) {
        // CLOCK_DESCRIPTION is GET only, we can skip it
        case CS(USER_DESCRIPTION);
        case CS(FAULT_LOG);
        case CS(UNICAST_MASTER_TABLE);
        case CS(GRANDMASTER_CLUSTER_TABLE);
        case CS(ACCEPTABLE_MASTER_TABLE);
        case CS(ALTERNATE_TIME_OFFSET_NAME);
        case CS(PATH_TRACE_LIST);
        case CS(PORT_PROPERTIES_NP);
        default:
            return -2;
    }
}

/*
 * Function used for both build and parse
 * Function can use:
 *  d        - the referance to the TLV own structure
 *  m_build  - is it build (true) or parse (false)
 *  m_err    - error to use when function return true
 *  m_left   - number of octets left in dataField the function may read
 *  reserved - uint8_t value for the function may use for reserved fields
 * The function should return TRUE on ERROR!!
 * The main build function will add a pad at the end to make size even
 */

#define A(n) bool message::n##_##f(n##_t &d)

A(CLOCK_DESCRIPTION)
{
    d.physicalAddressLength = d.physicalAddress.length();
    return proc(d.clockType) || proc(d.physicalLayerProtocol) ||
        proc(d.physicalAddressLength) ||
        proc(d.physicalAddress, d.physicalAddressLength) ||
        proc(d.protocolAddress) || sproc(d.manufacturerIdentity) ||
        proc(reserved) || proc(d.productDescription) ||
        proc(d.revisionData) || proc(d.userDescription) ||
        sproc(d.profileIdentity);
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
    vector_f(FaultRecord, numberOfFaultRecords, faultRecords);
}
A(TIME)
{
    return proc(d.currentTime);
}
A(CLOCK_ACCURACY)
{
    return proc(d.clockAccuracy);
}
A(DEFAULT_DATA_SET)
{
    return proc(d.flags) || proc(reserved) || proc(d.numberPorts) ||
        proc(d.priority1) || proc(d.clockQuality) || proc(d.priority2) ||
        proc(d.clockIdentity) || proc(d.domainNumber);
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
    return proc(d.flags);
}
A(CURRENT_DATA_SET)
{
    return proc(d.stepsRemoved) || proc(d.offsetFromMaster) ||
        proc(d.meanPathDelay);
}
A(PARENT_DATA_SET)
{
    return proc(d.parentPortIdentity) || proc(d.flags) || proc(reserved) ||
        proc(d.observedParentOffsetScaledLogVariance) ||
        proc(d.observedParentClockPhaseChangeRate) ||
        proc(d.grandmasterPriority1) || proc(d.grandmasterClockQuality) ||
        proc(d.grandmasterPriority2) || proc(d.grandmasterIdentity);
}
A(TIME_PROPERTIES_DATA_SET)
{
    return proc(d.currentUtcOffset) || proc(d.flags) || proc(d.timeSource);
}
A(UTC_PROPERTIES)
{
    return proc(d.currentUtcOffset) || proc(d.flags);
}
A(TIMESCALE_PROPERTIES)
{
    return proc(d.flags) || proc(d.timeSource);
}
A(TRACEABILITY_PROPERTIES)
{
    return proc(d.flags);
}
A(PORT_DATA_SET)
{
    return proc(d.portIdentity) || proc(d.portState) ||
        proc(d.logMinDelayReqInterval) || proc(d.peerMeanPathDelay) ||
        proc(d.logAnnounceInterval) || proc(d.announceReceiptTimeout) ||
        proc(d.logSyncInterval) || proc(d.delayMechanism) ||
        proc(d.logMinPdelayReqInterval) || proc(d.versionNumber);
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
A(DELAY_MECHANISM)
{
    return proc(d.delayMechanism);
}
A(LOG_MIN_PDELAY_REQ_INTERVAL)
{
    return proc(d.logMinPdelayReqInterval);
}
A(VERSION_NUMBER)
{
    return proc(d.versionNumber);
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
A(TRANSPARENT_CLOCK_PORT_DATA_SET)
{
    return proc(d.portIdentity) || proc(d.flags) ||
        proc(d.logMinPdelayReqInterval) || proc(d.peerMeanPathDelay);
}
A(MASTER_ONLY)
{
    return proc(d.flags);
}
A(UNICAST_NEGOTIATION_ENABLE)
{
    return proc(d.flags);
}
A(ALTERNATE_MASTER)
{
    return proc(d.flags) || proc(d.logAlternateMulticastSyncInterval) ||
        proc(d.numberOfAlternateMasters);
}
A(UNICAST_MASTER_TABLE)
{
    d.actualTableSize = d.PortAddress.size();
    if(proc(d.logQueryInterval) || proc(d.actualTableSize))
        return true;
    vector_f(PortAddress, actualTableSize, PortAddress);
}
A(UNICAST_MASTER_MAX_TABLE_SIZE)
{
    return proc(d.maxTableSize);
}
A(ACCEPTABLE_MASTER_TABLE_ENABLED)
{
    return proc(d.flags);
}
A(EXT_PORT_CONFIG_PORT_DATA_SET)
{
    return proc(d.flags) || proc(d.desiredState);
}
A(PATH_TRACE_ENABLE)
{
    return proc(d.flags);
}
A(ALTERNATE_TIME_OFFSET_ENABLE)
{
    return proc(d.keyField) || proc(d.flags);
}
A(GRANDMASTER_CLUSTER_TABLE)
{
    d.actualTableSize = d.PortAddress.size();
    if(proc(d.logQueryInterval) || proc(d.actualTableSize))
        return true;
    vector_f(PortAddress, actualTableSize, PortAddress);
}
A(ACCEPTABLE_MASTER_TABLE)
{
    d.actualTableSize = d.list.size();
    if(proc(d.actualTableSize))
        return true;
    vector_f(AcceptableMaster, actualTableSize, list);
}
A(ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
{
    return proc(d.maxTableSize);
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
A(EXTERNAL_PORT_CONFIGURATION_ENABLED)
{
    return proc(d.flags);
}
A(HOLDOVER_UPGRADE_ENABLE)
{
    return proc(d.flags);
}
A(PATH_TRACE_LIST)
{
    vector_b(ClockIdentity, pathSequence) {
        for(int i = 0; m_left >= (ssize_t)sizeof(ClockIdentity_t); i++) {
            ClockIdentity_t rec;
            if(proc(rec))
                return true;
            d.pathSequence[i++] = rec;
        }
    }
    return false;
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
    return proc(d.clockQuality) || proc(d.currentUtcOffset) || proc(d.flags) ||
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
