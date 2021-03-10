/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* st_ids.h PTP managment messages structure per ID
 *
 * Authors: Erez Geva <ErezGeva2@gmail.com>
 *
 */

#ifndef __ST_IDS_H
#define __ST_IDS_H

typedef uint8_t  UInteger8_t;
typedef uint16_t UInteger16_t;
typedef uint32_t UInteger32_t;
typedef uint64_t UInteger64_t;
typedef int8_t   Integer8_t;
typedef int16_t  Integer16_t;
typedef int32_t  Integer32_t;
typedef int64_t  Integer64_t;
typedef uint8_t  Octet_t;


#define A(n) }; struct n##_t : public baseData {
struct baseData {

A(CLOCK_DESCRIPTION)
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
A(USER_DESCRIPTION)
    PTPText_t userDescription;
const uint16_t INITIALIZE_EVENT = 0x0000;
A(INITIALIZE)
    uint16_t initializationKey;
A(FAULT_LOG)
    UInteger16_t numberOfFaultRecords;
    std::vector<FaultRecord_t> faultRecords;
A(TIME)
    Timestamp_t currentTime;
A(CLOCK_ACCURACY)
    clockAccuracy_e clockAccuracy;
A(DEFAULT_DATA_SET)
    uint8_t flags; // 0: TSC defaultDS.twoStepFlag
                   // 1: SO  defaultDS.slaveOnly
    UInteger16_t numberPorts;
    UInteger8_t priority1;
    ClockQuality_t clockQuality;
    UInteger8_t priority2;
    ClockIdentity_t clockIdentity;
    UInteger8_t domainNumber;
A(PRIORITY1)
    UInteger8_t priority1;
A(PRIORITY2)
    UInteger8_t priority2;
A(DOMAIN)
    UInteger8_t domainNumber;
A(SLAVE_ONLY)
    uint8_t flags; // 0: SO defaultDS.slaveOnly
A(CURRENT_DATA_SET)
    UInteger16_t stepsRemoved;
    TimeInterval_t offsetFromMaster;
    TimeInterval_t meanPathDelay;
A(PARENT_DATA_SET)
    PortIdentity_t parentPortIdentity;
    uint8_t flags; // 0: PS parentDS.parentStats
    UInteger16_t observedParentOffsetScaledLogVariance;
    Integer32_t observedParentClockPhaseChangeRate;
    UInteger8_t grandmasterPriority1;
    ClockQuality_t grandmasterClockQuality;
    UInteger8_t grandmasterPriority2;
    ClockIdentity_t grandmasterIdentity;
A(TIME_PROPERTIES_DATA_SET)
    Integer16_t currentUtcOffset;
    uint8_t flags;   // 0: LI-61 timePropertiesDS.leap61
                     // 1: LI-59 timePropertiesDS.leap59
                     // 2: UTCV  timePropertiesDS.currentUtcOffsetValid
                     // 3: PTP   timePropertiesDS.ptpTimescale
                     // 4: TTRA  timePropertiesDS.timeTraceable
                     // 5: FTRA  timePropertiesDS.frequencyTraceable
    timeSource_e timeSource;
A(UTC_PROPERTIES)
    Integer16_t currentUtcOffset;
    uint8_t flags;   // 0: LI-61 timePropertiesDS.leap61
                     // 1: LI-59 timePropertiesDS.leap59
                     // 2: UTCV  timePropertiesDS.currentUtcOffsetValid
A(TIMESCALE_PROPERTIES)
    uint8_t flags;   // 3: PTP   timePropertiesDS.ptpTimescale
    timeSource_e timeSource;
A(TRACEABILITY_PROPERTIES)
    uint8_t flags;   // 4: TTRA  timePropertiesDS.timeTraceable
                     // 5: FTRA  timePropertiesDS.frequencyTraceable
A(PORT_DATA_SET)
    PortIdentity_t portIdentity;
    portState_e  portState;
    Integer8_t logMinDelayReqInterval;
    TimeInterval_t peerMeanPathDelay;
    Integer8_t  logAnnounceInterval;
    UInteger8_t announceReceiptTimeout;
    Integer8_t logSyncInterval;
    uint8_t delayMechanism; // 1 E2E - end to end, delay request-response
                            // 2 P2P - peer to peer, peer delay
                            // 0xfe  - disabled
    Integer8_t logMinPdelayReqInterval;
    uint8_t versionNumber; // minor? | major version number
A(LOG_ANNOUNCE_INTERVAL)
    Integer8_t logAnnounceInterval;
A(ANNOUNCE_RECEIPT_TIMEOUT)
    UInteger8_t announceReceiptTimeout;
A(LOG_SYNC_INTERVAL)
    Integer8_t logSyncInterval;
A(DELAY_MECHANISM)
    uint8_t delayMechanism; // 1 E2E - end to end, delay request-response
                            // 2 P2P - peer to peer, peer delay
                            // 0xfe  - disabled
A(LOG_MIN_PDELAY_REQ_INTERVAL)
    Integer8_t logMinPdelayReqInterval;
A(VERSION_NUMBER)
    uint8_t versionNumber; // minor? | major version number
A(TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
    ClockIdentity_t clockIdentity;
    UInteger16_t numberPorts;
    uint8_t delayMechanism;
    UInteger8_t primaryDomain;
A(PRIMARY_DOMAIN)
    UInteger8_t primaryDomain;
A(TRANSPARENT_CLOCK_PORT_DATA_SET)
    PortIdentity_t portIdentity;
    uint8_t flags; // 0 FLT transparentClockPortDS.faultyFlag
    Integer8_t logMinPdelayReqInterval;
    TimeInterval_t peerMeanPathDelay;
A(MASTER_ONLY)
    uint8_t flags; // 0 MO - portDS.masterOnly
A(UNICAST_NEGOTIATION_ENABLE)
    uint8_t flags; // 0 EN - unicastNegotiationPortDS.enable
A(ALTERNATE_MASTER)
    uint8_t flags; // 0 S - alternateMasterPortDS.
                   //            transmitAlternateMulticastSync
    Integer8_t logAlternateMulticastSyncInterval;
    UInteger8_t numberOfAlternateMasters;
A(UNICAST_MASTER_TABLE)
    Integer8_t logQueryInterval;
    UInteger16_t actualTableSize;
    std::vector<PortAddress_t> PortAddress;
A(UNICAST_MASTER_MAX_TABLE_SIZE)
    UInteger16_t maxTableSize;
A(ACCEPTABLE_MASTER_TABLE_ENABLED)
    uint8_t flags; // 0 EN acceptableMasterPortDS.enable.
A(EXT_PORT_CONFIG_PORT_DATA_SET)
    uint8_t flags; // 0 EN acceptableMasterPortDS.enable.
    portState_e desiredState;
A(PATH_TRACE_ENABLE)
    uint8_t flags; // 0 EN pathTraceDS.enable
A(ALTERNATE_TIME_OFFSET_ENABLE)
    UInteger8_t keyField;
    uint8_t flags; // 0 EN alternateTimescaleOffsetsDS.list[keyField].enable
A(GRANDMASTER_CLUSTER_TABLE)
    Integer8_t logQueryInterval;
    UInteger8_t actualTableSize;
    std::vector<PortAddress_t> PortAddress;
A(ACCEPTABLE_MASTER_TABLE)
    Integer16_t actualTableSize;
    std::vector<AcceptableMaster_t> list;
A(ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
    UInteger16_t maxTableSize;
A(ALTERNATE_TIME_OFFSET_NAME)
    UInteger8_t keyField;
    PTPText_t displayName;
A(ALTERNATE_TIME_OFFSET_MAX_KEY)
    UInteger8_t maxKey;
A(ALTERNATE_TIME_OFFSET_PROPERTIES)
    UInteger8_t keyField;
    Integer32_t currentOffset;
    Integer32_t jumpSeconds;
    uint64_t timeOfNextJump; // 48 bits
A(EXTERNAL_PORT_CONFIGURATION_ENABLED)
    uint8_t flags; // 0 EPC defaultDS.externalPortConfigurationEnabled
A(HOLDOVER_UPGRADE_ENABLE)
    uint8_t flags; // 0 EN holdoverUpgradeDS.enable
A(PATH_TRACE_LIST)
    std::vector<ClockIdentity_t> pathSequence;
// linuxptp TLVs (in Implementation-specific C000–DFFF)
A(TIME_STATUS_NP)
    #define P41 ((double)(1ULL << 41))
    int64_t master_offset; /*nanoseconds*/
    int64_t ingress_time;  /*nanoseconds*/
    Integer32_t cumulativeScaledRateOffset;
    Integer32_t scaledLastGmPhaseChange;
    UInteger16_t gmTimeBaseIndicator;
    //  ScaledNs lastGmPhaseChange;
    uint16_t nanoseconds_msb;
    uint64_t nanoseconds_lsb;
    uint16_t fractional_nanoseconds;
    Integer32_t gmPresent;
    ClockIdentity_t gmIdentity;
A(GRANDMASTER_SETTINGS_NP)
    ClockQuality_t clockQuality;
    Integer16_t currentUtcOffset;
    uint8_t flags;
    timeSource_e timeSource; // u8
A(PORT_DATA_SET_NP)
    UInteger32_t neighborPropDelayThresh; /*nanoseconds*/
    Integer32_t asCapable;
A(SUBSCRIBE_EVENTS_NP)
    #define EVENT_BITMASK_CNT 64
    #define NOTIFY_PORT_STATE 0
    #define NOTIFY_TIME_SYNC  1
    #define EVENT_BIT(bitmask, event)\
        (bitmask[event / 8] & (1 << (event % 8))?"on":"off")
    uint16_t duration; /* seconds */
    uint8_t bitmask[EVENT_BITMASK_CNT];
A(PORT_PROPERTIES_NP)
    PortIdentity_t portIdentity;
    portState_e portState; // u8
    linuxptpTimesTamp_e timestamping; // u8
    PTPText_t interface;
A(PORT_STATS_NP)
    PortIdentity_t portIdentity;
    // PortStats stats
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
    uint64_t rxMsgType[MAX_MESSAGE_TYPES];
    uint64_t txMsgType[MAX_MESSAGE_TYPES];
A(SYNCHRONIZATION_UNCERTAIN_NP)
    uint8_t val;

}; // Close last one
#undef A

#endif /*__ST_IDS_H*/
