/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* ids.cpp parse, buid or both function per PTP managmet id
 *
 * Authors: Erez Geva <ErezGeva2@gmail.com>
 *
 */
#include "msg.h"
#include <cstring>

#define sbuffer(a) buffer((a), sizeof(a))
#define vector_b(type, vec)\
    if (m_build) {\
        for(type##_t &rec: d.vec) {\
            if (type##_f(rec))\
                return true;\
        }\
    }
#define vector_f(type, cnt, vec)\
    vector_b(type, vec)\
    else {\
        for (uint16_t i = 0; i < d.cnt; i++) {\
            type##_t rec;\
            if (type##_f(rec))\
                return true;\
            d.vec.push_back(rec);\
        }\
    }
#define vector_l(pre_size, type, vec)\
    size_t ret = pre_size;\
    for(type##_t &rec: d.vec)\
        ret += message::type##_l(rec);\
    return ret;

/* size functions per id */
#define CS(n) case n: size = n##_s(*(n##_t*)m_dataSend)
#define S(n) }static inline size_t n##_s(n##_t &d){
// Do not call, use to close the first S(n)
static inline size_t __dummy_f1__() { return 0;

S(USER_DESCRIPTION)
    return message::PTPText_l(d.userDescription);
S(FAULT_LOG)
    vector_l(2, FaultRecord, faultRecords)
S(UNICAST_MASTER_TABLE)
    vector_l(3, PortAddress, PortAddress)
S(GRANDMASTER_CLUSTER_TABLE)
    vector_l(2, PortAddress, PortAddress)
S(ACCEPTABLE_MASTER_TABLE)
    return 2 + sizeof(AcceptableMaster_t) * d.list.size();
S(ALTERNATE_TIME_OFFSET_NAME)
    return 1 + message::PTPText_l(d.displayName);
S(PATH_TRACE_LIST)
    return ClockIdentity_s * d.pathSequence.size();
// linuxptp TLVs (in Implementation-specific C000–DFFF)
S(PORT_PROPERTIES_NP)
    return 2 + sizeof(PortIdentity_t) + message::PTPText_l(d.interface);

} // Close last S() function
ssize_t message::dataFieldSize()
{
    // That should not happen, precaustion
    if (m_tlv_id < FIRST_MNG_ID || m_tlv_id > LAST_MNG_ID)
        return -1; // Not supported
    ssize_t size = mng_all_vals[m_tlv_id].size;
    if (size != -2)
        return size; // Fixed or not supported
    if (m_dataSend == nullptr)
        return -2; // can not calculate without data
    // Calculate variable length dataField
    switch(m_tlv_id)
    {
        CS(USER_DESCRIPTION);
        CS(FAULT_LOG);
        CS(UNICAST_MASTER_TABLE);
        CS(GRANDMASTER_CLUSTER_TABLE);
        CS(ACCEPTABLE_MASTER_TABLE);
        CS(ALTERNATE_TIME_OFFSET_NAME);
        CS(PATH_TRACE_LIST);
        CS(PORT_PROPERTIES_NP);
        default: // precaustion
            return -2; // Do not know how to calculate (GET only)
    }
    if (size & 1)
        size++;
    return size;
}

/*
 * Function used for both build and parse
 * Function can use:
 *  d        - the referance to id structure
 *  m_build  - flag for build true, parse false
 *  m_err    - error if function return true
 *  m_left   - number of Octets left in dataField the function may read
 *  reserved - uint8_t value for reserved fields
 * The function should return TRUE on ERROR!!
 * The function always end with returning false
 * The main build function will add a pad at the end to make size even
 * Function here can skip the last reserved or pad
 */

#define A(n) return false;}bool message::n##_##f(n##_t &d){
static inline bool __dummy_f2__() { // Do not call, use to close the first A(n)

A(CLOCK_DESCRIPTION)
    d.physicalAddressLength = d.physicalAddress.length();
    return
        u16(d.clockType) ||
        PTPText_f(d.physicalLayerProtocol) ||
        u16(d.physicalAddressLength) ||
        buffer(d.physicalAddress, d.physicalAddressLength) ||
        PortAddress_f(d.protocolAddress) ||
        sbuffer(d.manufacturerIdentity) ||
        u8(reserved) ||
        PTPText_f(d.productDescription) ||
        PTPText_f(d.revisionData) ||
        PTPText_f(d.userDescription) ||
        sbuffer(d.profileIdentity);
A(USER_DESCRIPTION)
    return PTPText_f(d.userDescription);
A(INITIALIZE)
    return u16(d.initializationKey);
A(FAULT_LOG)
    if (u16(d.numberOfFaultRecords))
        return true;
    vector_f(FaultRecord, numberOfFaultRecords, faultRecords)
A(TIME)
    return Timestamp_f(d.currentTime);
A(CLOCK_ACCURACY)
    uint8_t acc = d.clockAccuracy;
    if (u8(acc))
        return true;
    d.clockAccuracy = (clockAccuracy_e)acc;
A(DEFAULT_DATA_SET)
    return u8(d.flags) || u8(reserved) || u16(d.numberPorts) ||
           u8(d.priority1) || ClockQuality_f(d.clockQuality) ||
           u8(d.priority2) || ClockIdentity_f(d.clockIdentity) ||
           u8(d.domainNumber);
A(PRIORITY1)
    return u8(d.priority1);
A(PRIORITY2)
    return u8(d.priority2);
A(DOMAIN)
    return u8(d.domainNumber);
A(SLAVE_ONLY)
    return u8(d.flags);
A(CURRENT_DATA_SET)
    return u16(d.stepsRemoved) || TimeInterval_f(d.offsetFromMaster) ||
           TimeInterval_f(d.meanPathDelay);
A(PARENT_DATA_SET)
    return PortIdentity_f(d.parentPortIdentity) || u8(d.flags) ||
           u8(reserved) || u16(d.observedParentOffsetScaledLogVariance) ||
           i32(d.observedParentClockPhaseChangeRate) ||
           u8(d.grandmasterPriority1) ||
           ClockQuality_f(d.grandmasterClockQuality) ||
           u8(d.grandmasterPriority2) ||
           ClockIdentity_f(d.grandmasterIdentity);
A(TIME_PROPERTIES_DATA_SET)
    uint8_t src = d.timeSource;
    if (i16(d.currentUtcOffset) || u8(d.flags) || u8(src))
        return true;
    d.timeSource = (timeSource_e)src;
A(UTC_PROPERTIES)
    return i16(d.currentUtcOffset) || u8(d.flags);
A(TIMESCALE_PROPERTIES)
    uint8_t src = d.timeSource;
    if (u8(d.flags) || u8(src))
        return true;
    d.timeSource = (timeSource_e)src;
A(TRACEABILITY_PROPERTIES)
    return u8(d.flags);
A(PORT_DATA_SET)
    uint8_t state = d.portState;
    if (PortIdentity_f(d.portIdentity) || u8(state) ||
        i8(d.logMinDelayReqInterval) || TimeInterval_f(d.peerMeanPathDelay) ||
        i8(d.logAnnounceInterval) || u8(d.announceReceiptTimeout) ||
        i8(d.logSyncInterval) || u8(d.delayMechanism) ||
        i8(d.logMinPdelayReqInterval) || u8(d.versionNumber))
        return true;
    d.portState = (portState_e)state;
A(LOG_ANNOUNCE_INTERVAL)
    return i8(d.logAnnounceInterval);
A(ANNOUNCE_RECEIPT_TIMEOUT)
    return u8(d.announceReceiptTimeout);
A(LOG_SYNC_INTERVAL)
    return i8(d.logSyncInterval);
A(DELAY_MECHANISM)
    return u8(d.delayMechanism);
A(LOG_MIN_PDELAY_REQ_INTERVAL)
    return i8(d.logMinPdelayReqInterval);
A(VERSION_NUMBER)
    return u8(d.versionNumber);
A(TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
    return ClockIdentity_f(d.clockIdentity) || u16(d.numberPorts) ||
           u8(d.delayMechanism) || u8(d.primaryDomain);
A(PRIMARY_DOMAIN)
    return u8(d.primaryDomain);
A(TRANSPARENT_CLOCK_PORT_DATA_SET)
    return PortIdentity_f(d.portIdentity) || u8(d.flags) ||
           i8(d.logMinPdelayReqInterval) ||
           TimeInterval_f(d.peerMeanPathDelay);
A(MASTER_ONLY)
    return u8(d.flags);
A(UNICAST_NEGOTIATION_ENABLE)
    return u8(d.flags);
A(ALTERNATE_MASTER)
    return u8(d.flags) || i8(d.logAlternateMulticastSyncInterval) ||
           u8(d.numberOfAlternateMasters);
A(UNICAST_MASTER_TABLE)
    d.actualTableSize = d.PortAddress.size();
    if (i8(d.logQueryInterval) || u16(d.actualTableSize))
        return true;
    vector_f(PortAddress, actualTableSize, PortAddress)
A(UNICAST_MASTER_MAX_TABLE_SIZE)
    return u16(d.maxTableSize);
A(ACCEPTABLE_MASTER_TABLE_ENABLED)
    return u8(d.flags);
A(EXT_PORT_CONFIG_PORT_DATA_SET)
    uint8_t state = d.desiredState;
    if (u8(d.flags) || u8(state))
        return true;
    d.desiredState = (portState_e)state;
A(PATH_TRACE_ENABLE)
    return u8(d.flags);
A(ALTERNATE_TIME_OFFSET_ENABLE)
    return u8(d.keyField) || u8(d.flags);
A(GRANDMASTER_CLUSTER_TABLE)
    d.actualTableSize = d.PortAddress.size();
    if (i8(d.logQueryInterval) || u8(d.actualTableSize))
        return true;
    vector_f(PortAddress, actualTableSize, PortAddress)
A(ACCEPTABLE_MASTER_TABLE)
    d.actualTableSize = d.list.size();
    if (i16(d.actualTableSize))
        return true;
    vector_f(AcceptableMaster, actualTableSize, list)
A(ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
    return u16(d.maxTableSize);
A(ALTERNATE_TIME_OFFSET_NAME)
    return u8(d.keyField) || PTPText_f(d.displayName);
A(ALTERNATE_TIME_OFFSET_MAX_KEY)
    return u8(d.maxKey);
A(ALTERNATE_TIME_OFFSET_PROPERTIES)
    return u8(d.keyField) || i32(d.currentOffset) ||
           i32(d.jumpSeconds) || u48(d.timeOfNextJump);
A(EXTERNAL_PORT_CONFIGURATION_ENABLED)
    return u8(d.flags);
A(HOLDOVER_UPGRADE_ENABLE)
    return u8(d.flags);
A(PATH_TRACE_LIST)
    vector_b(ClockIdentity, pathSequence)
    else {
        int i = 0;
        while(m_left >= (ssize_t)ClockIdentity_s) {
            ClockIdentity_t rec;
            if (ClockIdentity_f(rec))
                return true;
            memcpy(d.pathSequence[i++], rec, ClockIdentity_s);
        }
    }
// linuxptp TLVs (in Implementation-specific C000–DFFF)
A(TIME_STATUS_NP)
    return i64(d.master_offset) || i64(d.ingress_time) ||
           i32(d.cumulativeScaledRateOffset) ||
           i32(d.scaledLastGmPhaseChange) ||
           u16(d.gmTimeBaseIndicator) ||
           u16(d.nanoseconds_msb) || u64(d.nanoseconds_lsb) ||
           u16(d.fractional_nanoseconds) || i32(d.gmPresent) ||
           ClockIdentity_f(d.gmIdentity);
A(GRANDMASTER_SETTINGS_NP)
    uint8_t src = d.timeSource;
    if (ClockQuality_f(d.clockQuality) || i16(d.currentUtcOffset) ||
        u8(d.flags) || u8(src))
        return true;
    d.timeSource = (timeSource_e)src;
A(PORT_DATA_SET_NP)
    return u32(d.neighborPropDelayThresh) || i32(d.asCapable);
A(SUBSCRIBE_EVENTS_NP)
    return u16(d.duration) || buffer(d.bitmask, EVENT_BITMASK_CNT);
A(PORT_PROPERTIES_NP)
    uint8_t state = d.portState;
    uint8_t ts = d.timestamping;
    if (PortIdentity_f(d.portIdentity) || u8(state) || u8(ts) ||
        PTPText_f(d.interface))
        return true;
    d.portState = (portState_e)state;
    d.timestamping = (linuxptpTimesTamp_e)ts;
A(PORT_STATS_NP)
    if (PortIdentity_f(d.portIdentity))
        return true;
    /*
    int i;
    for(i = 0; i < MAX_MESSAGE_TYPES; i++) {
        if (u64(d.rxMsgType[i]))
            return true;
    }
    for(i = 0; i < MAX_MESSAGE_TYPES; i++) {
        if (u64(d.txMsgType[i]))
            return true;
    }
    */
    // TODO this seems a bug and need a fixing in ptp4l daemon
    // Message statistics use host order and NOT network order!
    return buffer((uint8_t*)d.rxMsgType, sizeof(d.rxMsgType)) ||
           buffer((uint8_t*)d.txMsgType, sizeof(d.txMsgType));
A(SYNCHRONIZATION_UNCERTAIN_NP)
    return u8(d.val);

return true;} // Close last A() function
