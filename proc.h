/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* proc.h PTP managment messages structure per ID
 *
 * Authors: Erez Geva <ErezGeva2@gmail.com>
 *
 */

#ifndef __PROC_H
#define __PROC_H

typedef uint8_t  UInteger8_t;
typedef uint16_t UInteger16_t;
typedef uint32_t UInteger32_t;
typedef uint64_t UInteger64_t;
typedef int8_t   Integer8_t;
typedef int16_t  Integer16_t;
typedef int32_t  Integer32_t;
typedef int64_t  Integer64_t;
typedef uint8_t  Octet_t;

struct CLOCK_DESCRIPTION_t : public baseData {
    uint16_t clockType;
    PTPText_t physicalLayerProtocol;
    UInteger16_t physicalAddressLength;
    std::string physicalAddress;
    PortAddress_t protocolAddress;
    Octet_t manufacturerIdentity[3];
    PTPText_t productDescription;
    PTPText_t revisionData;
    PTPText_t userDescription;
    Octet_t profileIdentity[6];
};
struct USER_DESCRIPTION_t : public baseData {
    PTPText_t userDescription;
};
const uint16_t INITIALIZE_EVENT = 0x0000;
struct INITIALIZE_t : public baseData {
    uint16_t initializationKey;
};
struct FAULT_LOG_t : public baseData {
    UInteger16_t numberOfFaultRecords;
    std::vector<FaultRecord_t> faultRecords;
};
struct TIME_t : public baseData {
    Timestamp_t currentTime;
};
struct CLOCK_ACCURACY_t : public baseData {
    clockAccuracy_e clockAccuracy;
};
struct DEFAULT_DATA_SET_t : public baseData {
    /*
     * 0: TSC defaultDS.twoStepFlag
     * 1: SO  defaultDS.slaveOnly
     */
    uint8_t flags;
    UInteger16_t numberPorts;
    UInteger8_t priority1;
    ClockQuality_t clockQuality;
    UInteger8_t priority2;
    ClockIdentity_t clockIdentity;
    UInteger8_t domainNumber;
};
struct PRIORITY1_t : public baseData {
    UInteger8_t priority1;
};
struct PRIORITY2_t : public baseData {
    UInteger8_t priority2;
};
struct DOMAIN_t : public baseData {
    UInteger8_t domainNumber;
};
struct SLAVE_ONLY_t : public baseData {
    /*
     * 0: SO defaultDS.slaveOnly
     */
    uint8_t flags;
};
struct CURRENT_DATA_SET_t : public baseData {
    UInteger16_t stepsRemoved;
    TimeInterval_t offsetFromMaster;
    TimeInterval_t meanPathDelay;
};
struct PARENT_DATA_SET_t : public baseData {
    PortIdentity_t parentPortIdentity;
    /*
     * 0: PS parentDS.parentStats
     */
    uint8_t flags;
    UInteger16_t observedParentOffsetScaledLogVariance;
    Integer32_t observedParentClockPhaseChangeRate;
    UInteger8_t grandmasterPriority1;
    ClockQuality_t grandmasterClockQuality;
    UInteger8_t grandmasterPriority2;
    ClockIdentity_t grandmasterIdentity;
};
struct TIME_PROPERTIES_DATA_SET_t : public baseData {
    Integer16_t currentUtcOffset;
    /*
     * 0: LI-61 timePropertiesDS.leap61
     * 1: LI-59 timePropertiesDS.leap59
     * 2: UTCV  timePropertiesDS.currentUtcOffsetValid
     * 3: PTP   timePropertiesDS.ptpTimescale
     * 4: TTRA  timePropertiesDS.timeTraceable
     * 5: FTRA  timePropertiesDS.frequencyTraceable
     */
    uint8_t flags;
    timeSource_e timeSource;
};
struct UTC_PROPERTIES_t : public baseData {
    Integer16_t currentUtcOffset;
    /*
     * 0: LI-61 timePropertiesDS.leap61
     * 1: LI-59 timePropertiesDS.leap59
     * 2: UTCV  timePropertiesDS.currentUtcOffsetValid
     */
    uint8_t flags;
};
struct TIMESCALE_PROPERTIES_t : public baseData {
    /*
     * 3: PTP   timePropertiesDS.ptpTimescale
     */
    uint8_t flags;
    timeSource_e timeSource;
};
struct TRACEABILITY_PROPERTIES_t : public baseData {
    /*
     * 4: TTRA  timePropertiesDS.timeTraceable
     * 5: FTRA  timePropertiesDS.frequencyTraceable
     */
    uint8_t flags;
};
struct PORT_DATA_SET_t : public baseData {
    PortIdentity_t portIdentity;
    portState_e portState;
    Integer8_t logMinDelayReqInterval;
    TimeInterval_t peerMeanPathDelay;
    Integer8_t logAnnounceInterval;
    UInteger8_t announceReceiptTimeout;
    Integer8_t logSyncInterval;
    /*
     * 1 E2E - end to end, delay request-response
     * 2 P2P - peer to peer, peer delay
     * 0xfe  - disabled
     */
    uint8_t delayMechanism;
    Integer8_t logMinPdelayReqInterval;
    uint8_t versionNumber; /* minor? | major version number */
};
struct LOG_ANNOUNCE_INTERVAL_t : public baseData {
    Integer8_t logAnnounceInterval;
};
struct ANNOUNCE_RECEIPT_TIMEOUT_t : public baseData {
    UInteger8_t announceReceiptTimeout;
};
struct LOG_SYNC_INTERVAL_t : public baseData {
    Integer8_t logSyncInterval;
};
struct DELAY_MECHANISM_t : public baseData {
    /*
     * 1 E2E - end to end, delay request-response
     * 2 P2P - peer to peer, peer delay
     * 0xfe  - disabled
     */
    uint8_t delayMechanism;
};
struct LOG_MIN_PDELAY_REQ_INTERVAL_t : public baseData {
    Integer8_t logMinPdelayReqInterval;
};
struct VERSION_NUMBER_t : public baseData {
    uint8_t versionNumber; /* minor? | major version number */
};
struct TRANSPARENT_CLOCK_DEFAULT_DATA_SET_t : public baseData {
    ClockIdentity_t clockIdentity;
    UInteger16_t numberPorts;
    uint8_t delayMechanism;
    UInteger8_t primaryDomain;
};
struct PRIMARY_DOMAIN_t : public baseData {
    UInteger8_t primaryDomain;
};
struct TRANSPARENT_CLOCK_PORT_DATA_SET_t : public baseData {
    PortIdentity_t portIdentity;
    /*
     * 0 FLT transparentClockPortDS.faultyFlag
     */
    uint8_t flags;
    Integer8_t logMinPdelayReqInterval;
    TimeInterval_t peerMeanPathDelay;
};
struct MASTER_ONLY_t : public baseData {
    /*
     * 0 MO - portDS.masterOnly
     */
    uint8_t flags;
};
struct UNICAST_NEGOTIATION_ENABLE_t : public baseData {
    /*
     * 0 EN - unicastNegotiationPortDS.enable
     */
    uint8_t flags;
};
struct ALTERNATE_MASTER_t : public baseData {
    /*
     * 0 S - alternateMasterPortDS.transmitAlternateMulticastSync
     */
    uint8_t flags;
    Integer8_t logAlternateMulticastSyncInterval;
    UInteger8_t numberOfAlternateMasters;
};
struct UNICAST_MASTER_TABLE_t : public baseData {
    Integer8_t logQueryInterval;
    UInteger16_t actualTableSize;
    std::vector<PortAddress_t> PortAddress;
};
struct UNICAST_MASTER_MAX_TABLE_SIZE_t : public baseData {
    UInteger16_t maxTableSize;
};
struct ACCEPTABLE_MASTER_TABLE_ENABLED_t : public baseData {
    /*
     * 0 EN acceptableMasterPortDS.enable.
     */
    uint8_t flags;
};
struct EXT_PORT_CONFIG_PORT_DATA_SET_t : public baseData {
    /*
     * 0 EN acceptableMasterPortDS.enable.
     */
    uint8_t flags;
    portState_e desiredState;
};
struct PATH_TRACE_ENABLE_t : public baseData {
    /*
     * 0 EN pathTraceDS.enable
     */
    uint8_t flags;
};
struct ALTERNATE_TIME_OFFSET_ENABLE_t : public baseData {
    UInteger8_t keyField;
    /*
     * 0 EN alternateTimescaleOffsetsDS.list[keyField].enable
     */
    uint8_t flags;
};
struct GRANDMASTER_CLUSTER_TABLE_t : public baseData {
    Integer8_t logQueryInterval;
    UInteger8_t actualTableSize;
    std::vector<PortAddress_t> PortAddress;
};
struct ACCEPTABLE_MASTER_TABLE_t : public baseData {
    Integer16_t actualTableSize;
    std::vector<AcceptableMaster_t> list;
};
struct ACCEPTABLE_MASTER_MAX_TABLE_SIZE_t : public baseData {
    UInteger16_t maxTableSize;
};
struct ALTERNATE_TIME_OFFSET_NAME_t : public baseData {
    UInteger8_t keyField;
    PTPText_t displayName;
};
struct ALTERNATE_TIME_OFFSET_MAX_KEY_t : public baseData {
    UInteger8_t maxKey;
};
struct ALTERNATE_TIME_OFFSET_PROPERTIES_t : public baseData {
    UInteger8_t keyField;
    Integer32_t currentOffset;
    Integer32_t jumpSeconds;
    uint64_t timeOfNextJump; /* 48 bits */
};
struct EXTERNAL_PORT_CONFIGURATION_ENABLED_t : public baseData {
    /*
     * 0 EPC defaultDS.externalPortConfigurationEnabled
     */
    uint8_t flags;
};
struct HOLDOVER_UPGRADE_ENABLE_t : public baseData {
    /*
     * 0 EN holdoverUpgradeDS.enable
     */
    uint8_t flags;
};
struct PATH_TRACE_LIST_t : public baseData {
    std::vector<ClockIdentity_t> pathSequence;
};
/* linuxptp TLVs (in Implementation-specific C000–DFFF) */
struct TIME_STATUS_NP_t : public baseData {
#define P41 ((double)(1ULL << 41))
    int64_t master_offset; /* nanoseconds */
    int64_t ingress_time;  /* nanoseconds */
    Integer32_t cumulativeScaledRateOffset;
    Integer32_t scaledLastGmPhaseChange;
    UInteger16_t gmTimeBaseIndicator;
    /* ScaledNs lastGmPhaseChange */
    uint16_t nanoseconds_msb;
    uint64_t nanoseconds_lsb;
    uint16_t fractional_nanoseconds;
    Integer32_t gmPresent;
    ClockIdentity_t gmIdentity;
};
struct GRANDMASTER_SETTINGS_NP_t : public baseData {
    ClockQuality_t clockQuality;
    Integer16_t currentUtcOffset;
    uint8_t flags;
    timeSource_e timeSource;
};
struct PORT_DATA_SET_NP_t : public baseData {
    UInteger32_t neighborPropDelayThresh; /* nanoseconds */
    Integer32_t asCapable;
};
struct SUBSCRIBE_EVENTS_NP_t : public baseData {
#define EVENT_BITMASK_CNT 64
#define NOTIFY_PORT_STATE 0
#define NOTIFY_TIME_SYNC  1
#define EVENT_BIT(bitmask, event)\
    (bitmask[event / 8] & (1 << (event % 8))?"on":"off")
    uint16_t duration; /* seconds */
    uint8_t bitmask[EVENT_BITMASK_CNT];
};
struct PORT_PROPERTIES_NP_t : public baseData {
    PortIdentity_t portIdentity;
    portState_e portState;
    linuxptpTimeStamp_e timestamping;
    PTPText_t interface;
};
struct PORT_STATS_NP_t : public baseData {
#define MAX_MESSAGE_TYPES 16
#define STAT_SYNC                    0
#define STAT_DELAY_REQ               1
#define STAT_PDELAY_REQ              2
#define STAT_PDELAY_RESP             3
#define STAT_FOLLOW_UP               8
#define STAT_DELAY_RESP              9
#define STAT_PDELAY_RESP_FOLLOW_UP  10
#define STAT_ANNOUNCE               11
#define STAT_SIGNALING              12
#define STAT_MANAGEMENT             13
    PortIdentity_t portIdentity;
    /* PortStats stats */
    uint64_t rxMsgType[MAX_MESSAGE_TYPES];
    uint64_t txMsgType[MAX_MESSAGE_TYPES];
};
struct SYNCHRONIZATION_UNCERTAIN_NP_t : public baseData {
    uint8_t val;
};

#undef A

#endif /*__PROC_H*/
