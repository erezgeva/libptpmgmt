/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief parse, buid or both function per PTP managmet id
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#include "msg.h"
#include "comp.h"

namespace ptpmgmt
{

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

#define A(n) bool Message::n##_##f(n##_t &d)

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
#if 0
A(POWER_PROFILE_SETTINGS_NP)
{
    return proc(d.version) || proc(d.grandmasterID) ||
        proc(d.grandmasterTimeInaccuracy) || proc(d.networkTimeInaccuracy) ||
        proc(d.totalTimeInaccuracy);
}
#endif

}; /* namespace ptpmgmt */
