/* SPDX-License-Identifier: GPL-3.0-or-later */

/* pmc_dump.cpp dump received TLVs
 *
 * Authors: Erez Geva <ErezGeva2@gmail.com>
 *
 */

#include <cstring>
#include "msg.h"

static const char toksep[] = " \t\n\a\r"; // while spaces
#define IDENT "\n\t\t"
#define dump(n) \
static inline void dump_##n(message &m) {\
    n##_t &d = *(n##_t*)m.getData();\
    printf(
#define func_end_print \
    );\
}
#define end_print \
    )
#define func_end \
}

dump(CLOCK_DESCRIPTION)
    IDENT "clockType             0x%4x"
    IDENT "physicalLayerProtocol %s"
    IDENT "physicalAddress       %s"
    IDENT "protocolAddress       %s %s"
    IDENT "manufacturerId        %s"
    IDENT "productDescription    %s"
    IDENT "revisionData          %s"
    IDENT "userDescription       %s"
    IDENT "profileId             %s",
    d.clockType,
    m.pppText2str_c(d.physicalLayerProtocol),
    m.b2str(d.physicalAddress).c_str(),
    m.netProt2str_c(d.protocolAddress.networkProtocol),
    m.c2str(d.protocolAddress).c_str(),
    m.b2str(d.manufacturerIdentity, sizeof(d.manufacturerIdentity)).c_str(),
    m.pppText2str_c(d.productDescription), m.pppText2str_c(d.revisionData),
    m.pppText2str_c(d.userDescription),
    m.b2str(d.profileIdentity, sizeof(d.profileIdentity)).c_str()
    func_end_print
dump(USER_DESCRIPTION)
    IDENT "userDescription  %s",
    m.pppText2str_c(d.userDescription)
    func_end_print
dump(INITIALIZE)
    IDENT "initializationKey %u",
    d.initializationKey
    func_end_print
dump(FAULT_LOG)
    IDENT "numberOfFaultRecords %u",
    d.numberOfFaultRecords
    end_print;
    uint16_t i = 0;
    for(auto &rec: d.faultRecords) {
        printf(IDENT "[%u] faultTime        %s"
               IDENT "[%u] severityCode     %s"
               IDENT "[%u] faultName        %s"
               IDENT "[%u] faultValue       %s"
               IDENT "[%u] faultDescription %s",
               i, m.c2str(rec.faultTime).c_str(),
               i, m.faultRec2str_c(rec.severityCode),
               i, m.pppText2str_c(rec.faultName),
               i, m.pppText2str_c(rec.faultValue),
               i, m.pppText2str_c(rec.faultDescription));
        i++;
    }
    func_end
dump(DEFAULT_DATA_SET)
    IDENT "twoStepFlag             %u"
    IDENT "slaveOnly               %u"
    IDENT "numberPorts             %u"
    IDENT "priority1               %u"
    IDENT "clockClass              %u"
    IDENT "clockAccuracy           0x%x"
    IDENT "offsetScaledLogVariance 0x%x"
    IDENT "priority2               %u"
    IDENT "clockIdentity           %s"
    IDENT "domainNumber            %u",
    d.flags & 1,
    (d.flags >> 1) & 1,
    d.numberPorts,
    d.priority1,
    d.clockQuality.clockClass,
    d.clockQuality.clockAccuracy,
    d.clockQuality.offsetScaledLogVariance,
    d.priority2,
    m.c2str(d.clockIdentity).c_str(),
    d.domainNumber
    func_end_print
dump(CURRENT_DATA_SET)
    IDENT "stepsRemoved     %u"
    IDENT "offsetFromMaster %.1f"
    IDENT "meanPathDelay    %.1f",
    d.stepsRemoved,
    m.getInterval(d.offsetFromMaster),
    m.getInterval(d.meanPathDelay)
    func_end_print
dump(PARENT_DATA_SET)
    IDENT "parentPortIdentity                    %s"
    IDENT "parentStats                           %u"
    IDENT "observedParentOffsetScaledLogVariance 0x%x"
    IDENT "observedParentClockPhaseChangeRate    0x%x"
    IDENT "grandmasterPriority1                  %u"
    IDENT "gm.ClockClass                         %u"
    IDENT "gm.ClockAccuracy                      0x%x"
    IDENT "gm.OffsetScaledLogVariance            0x%x"
    IDENT "grandmasterPriority2                  %u"
    IDENT "grandmasterIdentity                   %s",
    m.c2str(d.parentPortIdentity).c_str(),
    d.flags & 1,
    d.observedParentOffsetScaledLogVariance,
    d.observedParentClockPhaseChangeRate,
    d.grandmasterPriority1,
    d.grandmasterClockQuality.clockClass,
    d.grandmasterClockQuality.clockAccuracy,
    d.grandmasterClockQuality.offsetScaledLogVariance,
    d.grandmasterPriority2,
    m.c2str(d.grandmasterIdentity).c_str()
    func_end_print
dump(TIME_PROPERTIES_DATA_SET)
    IDENT "currentUtcOffset      %d"
    IDENT "leap61                %u"
    IDENT "leap59                %u"
    IDENT "currentUtcOffsetValid %u"
    IDENT "ptpTimescale          %u"
    IDENT "timeTraceable         %u"
    IDENT "frequencyTraceable    %u"
    IDENT "timeSource            %s",
    d.currentUtcOffset,
    m.is_LI_61(d.flags),
    m.is_LI_59(d.flags),
    m.is_UTCV(d.flags),
    m.is_PTP(d.flags),
    m.is_TTRA(d.flags),
    m.is_FTRA(d.flags),
    m.timeSrc2str_c(d.timeSource)
    func_end_print
dump(PORT_DATA_SET)
    IDENT "portIdentity            %s"
    IDENT "portState               %s"
    IDENT "logMinDelayReqInterval  %d"
    IDENT "peerMeanPathDelay       %ju"
    IDENT "logAnnounceInterval     %d"
    IDENT "announceReceiptTimeout  %u"
    IDENT "logSyncInterval         %d"
    IDENT "delayMechanism          %u"
    IDENT "logMinPdelayReqInterval %d"
    IDENT "versionNumber           %u",
    m.c2str(d.portIdentity).c_str(),
    m.portState2str_c(d.portState),
    d.logMinDelayReqInterval,
    (uint64_t)m.getInterval(d.peerMeanPathDelay),
    d.logAnnounceInterval,
    d.announceReceiptTimeout,
    d.logSyncInterval,
    d.delayMechanism,
    d.logMinPdelayReqInterval,
    d.versionNumber
    func_end_print
dump(PRIORITY1)
    IDENT "priority1 %u",
    d.priority1
    func_end_print
dump(PRIORITY2)
    IDENT "priority2 %u",
    d.priority2
    func_end_print
dump(DOMAIN)
    IDENT "domainNumber %u",
    d.domainNumber
    func_end_print
dump(SLAVE_ONLY)
    IDENT "slaveOnly %u",
    d.flags & 1
    func_end_print
dump(LOG_ANNOUNCE_INTERVAL)
    IDENT "logAnnounceInterval %d",
    d.logAnnounceInterval
    func_end_print
dump(ANNOUNCE_RECEIPT_TIMEOUT)
    IDENT "announceReceiptTimeout %u",
    d.announceReceiptTimeout
    func_end_print
dump(LOG_SYNC_INTERVAL)
    IDENT "logSyncInterval %d",
    d.logSyncInterval
    func_end_print
dump(VERSION_NUMBER)
    IDENT "versionNumber %u",
    d.versionNumber
    func_end_print
dump(TIME)
    IDENT "currentTime %s",
    m.c2str(d.currentTime).c_str()
    func_end_print
dump(CLOCK_ACCURACY)
    IDENT "clockAccuracy 0x%x",
    d.clockAccuracy
    func_end_print
dump(UTC_PROPERTIES)
    IDENT "currentUtcOffset      %d"
    IDENT "leap61                %u"
    IDENT "leap59                %u"
    IDENT "currentUtcOffsetValid %u",
    d.currentUtcOffset,
    m.is_LI_61(d.flags),
    m.is_LI_59(d.flags),
    m.is_UTCV(d.flags)
    func_end_print
dump(TRACEABILITY_PROPERTIES)
    IDENT "timeTraceable      %u"
    IDENT "frequencyTraceable %u",
    m.is_TTRA(d.flags),
    m.is_FTRA(d.flags)
    func_end_print
dump(TIMESCALE_PROPERTIES)
    IDENT "ptpTimescale %u",
    m.is_PTP(d.flags)
    func_end_print
dump(UNICAST_NEGOTIATION_ENABLE)
    IDENT "unicastNegotiationPortDS %sabled",
    d.flags & 1 ?"e":"dis"
    func_end_print
dump(PATH_TRACE_LIST)
    "numberOfPathSequences %zu",
    d.pathSequence.size()
    end_print;
    uint16_t i = 0;
    for(auto &rec: d.pathSequence)
        printf(IDENT "[%u] %s" , i++, m.c2str(rec).c_str());
    func_end
dump(PATH_TRACE_ENABLE)
    IDENT "pathTraceDS %sabled",
    d.flags & 1 ?"e":"dis"
    func_end_print
dump(GRANDMASTER_CLUSTER_TABLE)
    IDENT "logQueryInterval %d"
    IDENT "actualTableSize  %u",
    d.logQueryInterval,
    d.actualTableSize
    end_print;
    uint16_t i = 0;
    for(auto &rec: d.PortAddress)
        printf(IDENT "[%u] %s" , i++, m.c2str(rec).c_str());
    func_end
dump(UNICAST_MASTER_TABLE)
    IDENT "logQueryInterval %d"
    IDENT "actualTableSize  %u",
    d.logQueryInterval,
    d.actualTableSize
    end_print;
    uint16_t i = 0;
    for(auto &rec: d.PortAddress)
        printf(IDENT "[%u] %s" , i++, m.c2str(rec).c_str());
    func_end
dump(UNICAST_MASTER_MAX_TABLE_SIZE)
    IDENT "maxTableSize %u",
    d.maxTableSize
    func_end_print
dump(ACCEPTABLE_MASTER_TABLE)
    IDENT "actualTableSize %d",
    d.actualTableSize
    end_print;
    uint16_t i = 0;
    for(auto &rec: d.list) {
        printf(IDENT "[%u] acceptablePortIdentity %s"
               IDENT "[%u] alternatePriority1     %u",
                i, m.c2str(rec.acceptablePortIdentity).c_str(),
                i, rec.alternatePriority1);
        i++;
    }
    func_end
dump(ACCEPTABLE_MASTER_TABLE_ENABLED)
    IDENT "acceptableMasterPortDS %sabled",
    d.flags & 1 ?"e":"dis"
    func_end_print
dump(ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
    IDENT "maxTableSize %u",
    d.maxTableSize
    func_end_print
dump(ALTERNATE_MASTER)
    IDENT "transmitAlternateMulticastSync    %sabled"
    IDENT "logAlternateMulticastSyncInterval %d"
    IDENT "numberOfAlternateMasters          %u",
    d.flags & 1 ?"e":"dis",
    d.logAlternateMulticastSyncInterval,
    d.numberOfAlternateMasters
    func_end_print
dump(ALTERNATE_TIME_OFFSET_ENABLE)
    IDENT "alternateTimescaleOffsetsDS[%u] %sabled",
    d.keyField, d.flags & 1 ?"e":"dis"
    func_end_print
dump(ALTERNATE_TIME_OFFSET_NAME)
    IDENT "[%u] %s",
    d.keyField, m.pppText2str_c(d.displayName)
    func_end_print
dump(ALTERNATE_TIME_OFFSET_MAX_KEY)
    IDENT "maxKey %u",
    d.maxKey
    func_end_print
dump(ALTERNATE_TIME_OFFSET_PROPERTIES)
    IDENT "keyField       %u"
    IDENT "currentOffset  %d"
    IDENT "jumpSeconds    %d"
    IDENT "timeOfNextJump %ju",
    d.keyField,
    d.currentOffset,
    d.jumpSeconds,
    d.timeOfNextJump
    func_end_print
dump(TRANSPARENT_CLOCK_PORT_DATA_SET)
    IDENT "portIdentity            %s"
    IDENT "transparentClockPortDS  %s"
    IDENT "logMinPdelayReqInterval %i"
    IDENT "peerMeanPathDelay       %ju",
    m.c2str(d.portIdentity).c_str(),
    d.flags & 1 ? "true" : "false",
    d.logMinPdelayReqInterval,
    (uint64_t)m.getInterval(d.peerMeanPathDelay)
    func_end_print
dump(LOG_MIN_PDELAY_REQ_INTERVAL)
    IDENT "logMinPdelayReqInterval %d",
    d.logMinPdelayReqInterval
    func_end_print
dump(TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
    IDENT "clockIdentity  %s"
    IDENT "numberPorts    %u"
    IDENT "delayMechanism %u"
    IDENT "primaryDomain  %u",
    m.c2str(d.clockIdentity).c_str(),
    d.numberPorts,
    d.delayMechanism,
    d.primaryDomain
    func_end_print
dump(PRIMARY_DOMAIN)
    IDENT "primaryDomain %u",
    d.primaryDomain
    func_end_print
dump(DELAY_MECHANISM)
    IDENT "delayMechanism %u",
    d.delayMechanism
    func_end_print
dump(EXTERNAL_PORT_CONFIGURATION_ENABLED)
    IDENT "externalPortConfiguration %sabled",
    d.flags & 1 ?"e":"dis"
    func_end_print
dump(MASTER_ONLY)
    IDENT "masterOnly %s",
    d.flags & 1 ?"true":"false"
    func_end_print
dump(HOLDOVER_UPGRADE_ENABLE)
    IDENT "holdoverUpgradeDS %sabled",
    d.flags & 1 ?"e":"dis"
    func_end_print
dump(EXT_PORT_CONFIG_PORT_DATA_SET)
    IDENT "acceptableMasterPortDS %sabled"
    IDENT "desiredState           %s",
    d.flags & 1 ?"e":"dis",
    m.portState2str_c(d.desiredState)
    func_end_print
dump(TIME_STATUS_NP)
    IDENT "master_offset              %jd"
    IDENT "ingress_time               %jd"
    IDENT "cumulativeScaledRateOffset %+.9f"
    IDENT "scaledLastGmPhaseChange    %d"
    IDENT "gmTimeBaseIndicator        %u"
    IDENT "lastGmPhaseChange          0x%04hx'%016jd.%04hx"
    IDENT "gmPresent                  %s"
    IDENT "gmIdentity                 %s",
    d.master_offset,
    d.ingress_time,
    (double)d.cumulativeScaledRateOffset / P41,
    d.scaledLastGmPhaseChange,
    d.gmTimeBaseIndicator,
    // lastGmPhaseChange
    d.nanoseconds_msb,
    d.nanoseconds_lsb,
    d.fractional_nanoseconds,
    d.gmPresent ? "true" : "false",
    m.c2str(d.gmIdentity).c_str()
    func_end_print
dump(GRANDMASTER_SETTINGS_NP)
    IDENT "clockClass              %u"
    IDENT "clockAccuracy           0x%x"
    IDENT "offsetScaledLogVariance 0x%x"
    IDENT "currentUtcOffset        %d"
    IDENT "leap61                  %u"
    IDENT "leap59                  %u"
    IDENT "currentUtcOffsetValid   %u"
    IDENT "ptpTimescale            %u"
    IDENT "timeTraceable           %u"
    IDENT "frequencyTraceable      %u"
    IDENT "timeSource              %s",
    d.clockQuality.clockClass,
    d.clockQuality.clockAccuracy,
    d.clockQuality.offsetScaledLogVariance,
    d.currentUtcOffset,
    m.is_LI_61(d.flags),
    m.is_LI_59(d.flags),
    m.is_UTCV(d.flags),
    m.is_PTP(d.flags),
    m.is_TTRA(d.flags),
    m.is_FTRA(d.flags),
    m.timeSrc2str_c(d.timeSource)
    func_end_print
dump(PORT_DATA_SET_NP)
    IDENT "neighborPropDelayThresh %u"
    IDENT "asCapable               %d",
    d.neighborPropDelayThresh,
    d.asCapable
    func_end_print
dump(SUBSCRIBE_EVENTS_NP)
    IDENT "duration          %u"
    IDENT "NOTIFY_PORT_STATE %s"
    IDENT "NOTIFY_TIME_SYNC  %s",
    d.duration,
    EVENT_BIT(d.bitmask, NOTIFY_PORT_STATE),
    EVENT_BIT(d.bitmask, NOTIFY_TIME_SYNC)
    func_end_print
dump(PORT_PROPERTIES_NP)
    IDENT "portIdentity            %s"
    IDENT "portState               %s"
    IDENT "timestamping            %s"
    IDENT "interface               %s",
    m.c2str(d.portIdentity).c_str(),
    m.portState2str_c(d.portState),
    m.ts2str_c(d.timestamping) + 3 /* Remove the 'TS_' prefix*/,
    m.pppText2str_c(d.interface)
    func_end_print
dump(PORT_STATS_NP)
    IDENT "portIdentity              %s"
    IDENT "rx_Sync                   %ju"
    IDENT "rx_Delay_Req              %ju"
    IDENT "rx_Pdelay_Req             %ju"
    IDENT "rx_Pdelay_Resp            %ju"
    IDENT "rx_Follow_Up              %ju"
    IDENT "rx_Delay_Resp             %ju"
    IDENT "rx_Pdelay_Resp_Follow_Up  %ju"
    IDENT "rx_Announce               %ju"
    IDENT "rx_Signaling              %ju"
    IDENT "rx_Management             %ju"
    IDENT "tx_Sync                   %ju"
    IDENT "tx_Delay_Req              %ju"
    IDENT "tx_Pdelay_Req             %ju"
    IDENT "tx_Pdelay_Resp            %ju"
    IDENT "tx_Follow_Up              %ju"
    IDENT "tx_Delay_Resp             %ju"
    IDENT "tx_Pdelay_Resp_Follow_Up  %ju"
    IDENT "tx_Announce               %ju"
    IDENT "tx_Signaling              %ju"
    IDENT "tx_Management             %ju",
    m.c2str(d.portIdentity).c_str(),
    d.rxMsgType[STAT_SYNC],
    d.rxMsgType[STAT_DELAY_REQ],
    d.rxMsgType[STAT_PDELAY_REQ],
    d.rxMsgType[STAT_PDELAY_RESP],
    d.rxMsgType[STAT_FOLLOW_UP],
    d.rxMsgType[STAT_DELAY_RESP],
    d.rxMsgType[STAT_PDELAY_RESP_FOLLOW_UP],
    d.rxMsgType[STAT_ANNOUNCE],
    d.rxMsgType[STAT_SIGNALING],
    d.rxMsgType[STAT_MANAGEMENT],
    d.txMsgType[STAT_SYNC],
    d.txMsgType[STAT_DELAY_REQ],
    d.txMsgType[STAT_PDELAY_REQ],
    d.txMsgType[STAT_PDELAY_RESP],
    d.txMsgType[STAT_FOLLOW_UP],
    d.txMsgType[STAT_DELAY_RESP],
    d.txMsgType[STAT_PDELAY_RESP_FOLLOW_UP],
    d.txMsgType[STAT_ANNOUNCE],
    d.txMsgType[STAT_SIGNALING],
    d.txMsgType[STAT_MANAGEMENT]
    func_end_print
dump(SYNCHRONIZATION_UNCERTAIN_NP)
    IDENT "uncertain %u",
    d.val
    func_end_print

void call_dump(message &m)
{
    switch(m.getTlvId()) {
        #define caseUF(n) case n: dump_##n(m); break;
        #define A(n, v, sc, a, sz, f) case##f(n)
        #include "ids.h"
        default: break;
    }
    printf("\n");
}

#define build(n) \
static inline baseData *build_##n() {\
    n##_t *d = new n##_t;\
    if(d == nullptr)\
        return d;
#define next_token \
    cur = strtok(nullptr, toksep);\
    if (cur == nullptr || *cur == 0)\
        return true
#define build_end \
    return d;\
}
static inline bool getNum(const char *name, long &val, int base, long max,
                          long min = 0)
{
    char *cur;
    next_token;
    if(strcasecmp(name, cur) != 0)
        return true;
    next_token;
    char *end;
    val = strtol(cur, &end, base);
    if (*end != 0 || val < min || val > max)
        return true;
    return false;
}
inline bool getTimeSource(timeSource_e &val)
{
    char *cur;
    next_token;
    if(strcasecmp("timeSource", cur) != 0)
        return true;
    next_token;
         if(strcasecmp("ATOMIC_CLOCK", cur) == 0) val = ATOMIC_CLOCK;
    else if(strcasecmp("GNSS", cur) == 0) val = GNSS;
    else if(strcasecmp("TERRESTRIAL_RADIO", cur) == 0) val = TERRESTRIAL_RADIO;
    else if(strcasecmp("SERIAL_TIME_CODE", cur) == 0) val = SERIAL_TIME_CODE;
    else if(strcasecmp("PTP", cur) == 0) val = PTP;
    else if(strcasecmp("NTP", cur) == 0) val = NTP;
    else if(strcasecmp("HAND_SET", cur) == 0) val = HAND_SET;
    else if(strcasecmp("OTHER", cur) == 0) val = OTHER;
    else if(strcasecmp("INTERNAL_OSCILLATOR", cur) == 0) val = INTERNAL_OSCILLATOR;
    else {
        char *end;
        long v1 = strtol(cur, &end, 16);
        if (*end != 0 || v1 < 0 || v1 > UINT8_MAX)
            return true;
        val = (timeSource_e)v1;
    }
    return false;
}

build(PRIORITY1)
    long priority1;
    if(getNum("priority1", priority1, 0, 255))
        return nullptr;
    d->priority1 = priority1;
    build_end
build(PRIORITY2)
    long priority2;
    if(getNum("priority2", priority2, 0, 255))
        return nullptr;
    d->priority2 = priority2;
    build_end
/* libnuxptp specific implementation */
build(GRANDMASTER_SETTINGS_NP)
    long clockClass, clockAccuracy, offsetScaledLogVariance, currentUtcOffset,
         leap61, leap59, currentUtcOffsetValid, ptpTimescale, timeTraceable,
         frequencyTraceable;
    timeSource_e timeSource;
    if(getNum("clockClass", clockClass, 0, UINT8_MAX) ||
       getNum("clockAccuracy", clockAccuracy, 16, UINT8_MAX) ||
       getNum("offsetScaledLogVariance", offsetScaledLogVariance, 16, UINT16_MAX) ||
       getNum("currentUtcOffset", currentUtcOffset, 0, INT16_MAX) ||
       getNum("leap61", leap61, 0, UINT8_MAX) ||
       getNum("leap59", leap59, 0, UINT8_MAX) ||
       getNum("currentUtcOffsetValid", currentUtcOffsetValid, 0, UINT8_MAX) ||
       getNum("ptpTimescale", ptpTimescale, 0, UINT8_MAX) ||
       getNum("timeTraceable", timeTraceable, 0, UINT8_MAX) ||
       getNum("frequencyTraceable", frequencyTraceable, 0, UINT8_MAX) ||
       getTimeSource(timeSource))
        return nullptr;
    uint8_t flags = 0;
    if (leap61)
        flags |= (1 << 0);
    if (leap59)
        flags |= (1 << 1);
    if (currentUtcOffsetValid)
        flags |= (1 << 2);
    if (ptpTimescale)
        flags |= (1 << 3);
    if (timeTraceable)
        flags |= (1 << 4);
    if (frequencyTraceable)
        flags |= (1 << 5);
    d->clockQuality.clockClass = clockClass;
    d->clockQuality.clockAccuracy = (clockAccuracy_e)clockAccuracy;
    d->clockQuality.offsetScaledLogVariance = offsetScaledLogVariance;
    d->currentUtcOffset = currentUtcOffset;
    d->flags = flags;
    d->timeSource = timeSource;
    build_end
build(SUBSCRIBE_EVENTS_NP)
    return nullptr; // TODO
    long val;
    if(getNum("duration", val, 0, UINT16_MAX))
        return nullptr;
    d->duration = val;
    build_end
build(SYNCHRONIZATION_UNCERTAIN_NP)
    long uncertain;
    if(getNum("uncertain", uncertain, 0, UINT8_MAX))
        return nullptr;
    d->val = uncertain;
    build_end
build(PORT_DATA_SET_NP)
    long neighborPropDelayThresh, asCapable;
    if(getNum("neighborPropDelayThresh", neighborPropDelayThresh, 0, UINT32_MAX) ||
       getNum("asCapable", asCapable, 0, INT32_MAX))
        return nullptr;
    d->neighborPropDelayThresh = neighborPropDelayThresh;
    d->asCapable = asCapable;
    build_end

#define caseBuild(n) case n: return build_##n()
baseData *call_data(mng_vals_e id)
{
    switch(id) {
        caseBuild(PRIORITY1);
        caseBuild(PRIORITY2);
        caseBuild(GRANDMASTER_SETTINGS_NP);
        caseBuild(SUBSCRIBE_EVENTS_NP);
        caseBuild(SYNCHRONIZATION_UNCERTAIN_NP);
        caseBuild(PORT_DATA_SET_NP);
        default: return nullptr;
    }
}
