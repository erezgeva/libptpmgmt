/* SPDX-License-Identifier: GPL-3.0-or-later */

/** @file
 * @brief dump received TLVs
 *
 * @author Erez Geva <ErezGeva2@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#include <cstring>
#include <functional>
#include "msg.h"

static const char toksep[] = " \t\n\a\r"; // while spaces
#define IDENT "\n\t\t"
#define dump(n) \
    static inline void dump_##n(message &m) {\
        n##_t &d = *(n##_t*)m.getData();\
        printf(
#define dump_end_print \
        );\
    }
#define end_print \
        )
#define dump_end \
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
    dump_end_print
dump(USER_DESCRIPTION)
    IDENT "userDescription  %s",
    m.pppText2str_c(d.userDescription)
    dump_end_print
dump(INITIALIZE)
    IDENT "initializationKey %u",
    d.initializationKey
    dump_end_print
dump(FAULT_LOG)
    IDENT "numberOfFaultRecords %u",
    d.numberOfFaultRecords
    end_print;
    uint16_t i = 0;
    for(auto &rec : d.faultRecords) {
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
    dump_end
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
    dump_end_print
dump(CURRENT_DATA_SET)
    IDENT "stepsRemoved     %u"
    IDENT "offsetFromMaster %.1f"
    IDENT "meanPathDelay    %.1f",
    d.stepsRemoved,
    m.getInterval(d.offsetFromMaster),
    m.getInterval(d.meanPathDelay)
    dump_end_print
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
    dump_end_print
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
    dump_end_print
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
    dump_end_print
dump(PRIORITY1)
    IDENT "priority1 %u",
    d.priority1
    dump_end_print
dump(PRIORITY2)
    IDENT "priority2 %u",
    d.priority2
    dump_end_print
dump(DOMAIN)
    IDENT "domainNumber %u",
    d.domainNumber
    dump_end_print
dump(SLAVE_ONLY)
    IDENT "slaveOnly %u",
    /*
     * According to IEEE 1588, the flag is on bit 0
     * As linuxptp put the flag on the wrong bit
     * We test both bit 0 and bit 1.
     * As bit 1 is not due to be used, it should be safe
     */
    (m.getParams().useLinuxPTPTlvs ?
        ((d.flags & 0x3) > 0 ? 1 : 0) :
        (d.flags & 1))
    dump_end_print
dump(LOG_ANNOUNCE_INTERVAL)
    IDENT "logAnnounceInterval %d",
    d.logAnnounceInterval
    dump_end_print
dump(ANNOUNCE_RECEIPT_TIMEOUT)
    IDENT "announceReceiptTimeout %u",
    d.announceReceiptTimeout
    dump_end_print
dump(LOG_SYNC_INTERVAL)
    IDENT "logSyncInterval %d",
    d.logSyncInterval
    dump_end_print
dump(VERSION_NUMBER)
    IDENT "versionNumber %u",
    d.versionNumber
    dump_end_print
dump(TIME)
    IDENT "currentTime %s",
    m.c2str(d.currentTime).c_str()
    dump_end_print
dump(CLOCK_ACCURACY)
    IDENT "clockAccuracy 0x%x",
    d.clockAccuracy
    dump_end_print
dump(UTC_PROPERTIES)
    IDENT "currentUtcOffset      %d"
    IDENT "leap61                %u"
    IDENT "leap59                %u"
    IDENT "currentUtcOffsetValid %u",
    d.currentUtcOffset,
    m.is_LI_61(d.flags),
    m.is_LI_59(d.flags),
    m.is_UTCV(d.flags)
    dump_end_print
dump(TRACEABILITY_PROPERTIES)
    IDENT "timeTraceable      %u"
    IDENT "frequencyTraceable %u",
    m.is_TTRA(d.flags),
    m.is_FTRA(d.flags)
    dump_end_print
dump(TIMESCALE_PROPERTIES)
    IDENT "ptpTimescale %u",
    m.is_PTP(d.flags)
    dump_end_print
dump(UNICAST_NEGOTIATION_ENABLE)
    IDENT "unicastNegotiationPortDS %sabled",
    d.flags & 1 ? "e" : "dis"
    dump_end_print
dump(PATH_TRACE_LIST)
    "numberOfPathSequences %zu",
    d.pathSequence.size()
    end_print;
    uint16_t i = 0;
    for(auto &rec : d.pathSequence)
        printf(IDENT "[%u] %s", i++, m.c2str(rec).c_str());
    dump_end
dump(PATH_TRACE_ENABLE)
    IDENT "pathTraceDS %sabled",
    d.flags & 1 ? "e" : "dis"
    dump_end_print
dump(GRANDMASTER_CLUSTER_TABLE)
    IDENT "logQueryInterval %d"
    IDENT "actualTableSize  %u",
    d.logQueryInterval,
    d.actualTableSize
    end_print;
    uint16_t i = 0;
    for(auto &rec : d.PortAddress)
        printf(IDENT "[%u] %s", i++, m.c2str(rec).c_str());
    dump_end
dump(UNICAST_MASTER_TABLE)
    IDENT "logQueryInterval %d"
    IDENT "actualTableSize  %u",
    d.logQueryInterval,
    d.actualTableSize
    end_print;
    uint16_t i = 0;
    for(auto &rec : d.PortAddress)
        printf(IDENT "[%u] %s", i++, m.c2str(rec).c_str());
    dump_end
dump(UNICAST_MASTER_MAX_TABLE_SIZE)
    IDENT "maxTableSize %u",
    d.maxTableSize
    dump_end_print
dump(ACCEPTABLE_MASTER_TABLE)
    IDENT "actualTableSize %d",
    d.actualTableSize
    end_print;
    uint16_t i = 0;
    for(auto &rec : d.list) {
        printf(IDENT "[%u] acceptablePortIdentity %s"
               IDENT "[%u] alternatePriority1     %u",
                i, m.c2str(rec.acceptablePortIdentity).c_str(),
                i, rec.alternatePriority1);
        i++;
    }
    dump_end
dump(ACCEPTABLE_MASTER_TABLE_ENABLED)
    IDENT "acceptableMasterPortDS %sabled",
    d.flags & 1 ? "e" : "dis"
    dump_end_print
dump(ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
    IDENT "maxTableSize %u",
    d.maxTableSize
    dump_end_print
dump(ALTERNATE_MASTER)
    IDENT "transmitAlternateMulticastSync    %sabled"
    IDENT "logAlternateMulticastSyncInterval %d"
    IDENT "numberOfAlternateMasters          %u",
    d.flags & 1 ? "e" : "dis",
    d.logAlternateMulticastSyncInterval,
    d.numberOfAlternateMasters
    dump_end_print
dump(ALTERNATE_TIME_OFFSET_ENABLE)
    IDENT "alternateTimescaleOffsetsDS[%u] %sabled",
    d.keyField, d.flags & 1 ? "e" : "dis"
    dump_end_print
dump(ALTERNATE_TIME_OFFSET_NAME)
    IDENT "[%u] %s",
    d.keyField, m.pppText2str_c(d.displayName)
    dump_end_print
dump(ALTERNATE_TIME_OFFSET_MAX_KEY)
    IDENT "maxKey %u",
    d.maxKey
    dump_end_print
dump(ALTERNATE_TIME_OFFSET_PROPERTIES)
    IDENT "keyField       %u"
    IDENT "currentOffset  %d"
    IDENT "jumpSeconds    %d"
    IDENT "timeOfNextJump %ju",
    d.keyField,
    d.currentOffset,
    d.jumpSeconds,
    d.timeOfNextJump
    dump_end_print
dump(TRANSPARENT_CLOCK_PORT_DATA_SET)
    IDENT "portIdentity            %s"
    IDENT "transparentClockPortDS  %s"
    IDENT "logMinPdelayReqInterval %i"
    IDENT "peerMeanPathDelay       %ju",
    m.c2str(d.portIdentity).c_str(),
    d.flags & 1 ? "true" : "false",
    d.logMinPdelayReqInterval,
    (uint64_t)m.getInterval(d.peerMeanPathDelay)
    dump_end_print
dump(LOG_MIN_PDELAY_REQ_INTERVAL)
    IDENT "logMinPdelayReqInterval %d",
    d.logMinPdelayReqInterval
    dump_end_print
dump(TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
    IDENT "clockIdentity  %s"
    IDENT "numberPorts    %u"
    IDENT "delayMechanism %u"
    IDENT "primaryDomain  %u",
    m.c2str(d.clockIdentity).c_str(),
    d.numberPorts,
    d.delayMechanism,
    d.primaryDomain
    dump_end_print
dump(PRIMARY_DOMAIN)
    IDENT "primaryDomain %u",
    d.primaryDomain
    dump_end_print
dump(DELAY_MECHANISM)
    IDENT "delayMechanism %u",
    d.delayMechanism
    dump_end_print
dump(EXTERNAL_PORT_CONFIGURATION_ENABLED)
    IDENT "externalPortConfiguration %sabled",
    d.flags & 1 ? "e" : "dis"
    dump_end_print
dump(MASTER_ONLY)
    IDENT "masterOnly %s",
    d.flags & 1 ? "true" : "false"
    dump_end_print
dump(HOLDOVER_UPGRADE_ENABLE)
    IDENT "holdoverUpgradeDS %sabled",
    d.flags & 1 ? "e" : "dis"
    dump_end_print
dump(EXT_PORT_CONFIG_PORT_DATA_SET)
    IDENT "acceptableMasterPortDS %sabled"
    IDENT "desiredState           %s",
    d.flags & 1 ? "e" : "dis",
    m.portState2str_c(d.desiredState)
    dump_end_print
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
    dump_end_print
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
    dump_end_print
dump(PORT_DATA_SET_NP)
    IDENT "neighborPropDelayThresh %u"
    IDENT "asCapable               %d",
    d.neighborPropDelayThresh,
    d.asCapable
    dump_end_print
dump(SUBSCRIBE_EVENTS_NP)
    IDENT "duration          %u"
    IDENT "NOTIFY_PORT_STATE %s"
    IDENT "NOTIFY_TIME_SYNC  %s",
    d.duration,
    EVENT_BIT(d.bitmask, NOTIFY_PORT_STATE),
    EVENT_BIT(d.bitmask, NOTIFY_TIME_SYNC)
    dump_end_print
dump(PORT_PROPERTIES_NP)
    IDENT "portIdentity            %s"
    IDENT "portState               %s"
    IDENT "timestamping            %s"
    IDENT "interface               %s",
    m.c2str(d.portIdentity).c_str(),
    m.portState2str_c(d.portState),
    m.ts2str_c(d.timestamping) + 3 /* Remove the 'TS_' prefix*/,
    m.pppText2str_c(d.interface)
    dump_end_print
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
    dump_end_print
dump(SYNCHRONIZATION_UNCERTAIN_NP)
    IDENT "uncertain %u",
    d.val
    dump_end_print

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
        n##_t *dp = new n##_t;\
        if(dp == nullptr)\
            return dp;\
        n##_t &d = *dp;\
        std::map<std::string, val_key_t> keys;
#define build_end \
        return dp;\
    }

/* structure for special parser */
struct val_key_t {
    long num;     // number value
    bool can_str; // value can be string
    bool got_num; // flag indicate parsed number successfully
    int base;     // number value base to use in parsing, 0 for automatic
    long def;     // Default number value
    long max;     // maximum number value
    long min;     // minimum number value
    bool req;     // key is required
    char *str;    // string value
    // Callback function for special value parser
    std::function<bool (long &num, char *cur)> handle;
};

static bool parseKeys(std::map<std::string, val_key_t> &keys)
{
    // Update unrequired keys with default value
    for(auto it = keys.begin(); it != keys.end(); it++) {
        if(!it->second.req)
            it->second.num = it->second.def;
    }
    // Each key should be once
    for(size_t cnt; cnt < keys.size() * 2; cnt++) {
        char *cur = strtok(nullptr, toksep);
        // No more keys, we can continue
        if(cur == nullptr || *cur == 0)
            break;
        auto it = keys.begin();
        for(; it != keys.end(); it++) {
            if(strcasecmp(it->first.c_str(), cur) == 0)
                break;
        }
        // Unknown key
        if(it == keys.end())
            return true;
        val_key_t &key = it->second;
        cur = strtok(nullptr, toksep);
        // key must have value
        if(cur == nullptr || *cur == 0)
            return true;
        if(key.handle != nullptr) {
            key.str = cur;
            char *end;
            long num = strtol(cur, &end, key.base);
            // Make sure value is integer and in range
            if(*end == 0 && num >= key.min && num <= key.max) {
                key.got_num = true;
                key.num = num;
            } else if(!key.can_str)
                // Allow value if it can be string
                return true;
        } else if(key.handle(key.num, cur))
            return true;
        // We have the value mark it
        key.req = false;
    }
    for(auto it = keys.begin(); it != keys.end(); it++) {
        // did we forget required fields
        if(it->second.req)
            return true;
    }
    return false; // No errors!
}
static bool getTimeSource(long &num, char *cur)
{
    if(strcasecmp("ATOMIC_CLOCK", cur) == 0)
        num = ATOMIC_CLOCK;
    else if(strcasecmp("GNSS", cur) == 0)
        num = GNSS;
    else if(strcasecmp("TERRESTRIAL_RADIO", cur) == 0)
        num = TERRESTRIAL_RADIO;
    else if(strcasecmp("SERIAL_TIME_CODE", cur) == 0)
        num = SERIAL_TIME_CODE;
    else if(strcasecmp("PTP", cur) == 0)
        num = PTP;
    else if(strcasecmp("NTP", cur) == 0)
        num = NTP;
    else if(strcasecmp("HAND_SET", cur) == 0)
        num = HAND_SET;
    else if(strcasecmp("OTHER", cur) == 0)
        num = OTHER;
    else if(strcasecmp("INTERNAL_OSCILLATOR", cur) == 0)
        num = INTERNAL_OSCILLATOR;
    else { // Fallback into hex integer
        char *end;
        num = strtol(cur, &end, 16);
        if(*end != 0 || num < 0 || num > UINT8_MAX)
            return true;
    }
    return false; // No errors!
}
static bool getOn(long &num, char *cur)
{
    if(strcasecmp("on", cur) == 0)
        num = 1;
    else
        num = 0;
    return false; // No errors!
}

build(USER_DESCRIPTION);
    keys["userDescription"] = { .can_str = true };
    if (parseKeys(keys))
        return nullptr;
    d.userDescription.textField = keys["userDescription"].str;
    build_end
build(INITIALIZE);
    keys["initializationKey"] = { .def = INITIALIZE_EVENT, .max = UINT16_MAX };
    if (parseKeys(keys))
        return nullptr;
    d.initializationKey = keys["initializationKey"].num;
    build_end
build(PRIORITY1)
    keys["priority1"] = { .def = 128, .max = 255 };
    if (parseKeys(keys))
        return nullptr;
    d.priority1 = keys["priority1"].num;
    build_end
build(PRIORITY2)
    keys["priority2"] = { .def = 128, .max = 255 };
    if (parseKeys(keys))
        return nullptr;
    d.priority2 = keys["priority2"].num;
    build_end
build(DOMAIN)
    keys["domainNumber"] = { .max = UINT8_MAX };
    if (parseKeys(keys))
        return nullptr;
    d.domainNumber = keys["domainNumber"].num;
    build_end
build(PRIMARY_DOMAIN)
    keys["primaryDomain"] = { .max = UINT8_MAX };
    if (parseKeys(keys))
        return nullptr;
    d.primaryDomain = keys["primaryDomain"].num;
    build_end
/* libnuxptp specific implementation */
build(GRANDMASTER_SETTINGS_NP)
    keys["clockClass"] = { .max = UINT8_MAX, .req = true };
    keys["clockAccuracy"] = { .base = 16, .max = UINT8_MAX, .req = true };
    keys["offsetScaledLogVariance"] = { .base = 16, .max = UINT16_MAX, .req = true };
    keys["currentUtcOffset"] = { .max = INT16_MAX, .req = true };
    keys["leap61"] = { .max = UINT8_MAX };
    keys["leap59"] = { .max = UINT8_MAX };
    keys["currentUtcOffsetValid"] = { .max = UINT8_MAX };
    keys["ptpTimescale"] = { .max = UINT8_MAX };
    keys["timeTraceable"] = { .max = UINT8_MAX };
    keys["frequencyTraceable"] = { .max = UINT8_MAX };
    keys["timeSource"] = { .req = true };
    keys["timeSource"].handle = getTimeSource;
    if (parseKeys(keys))
        return nullptr;
    uint8_t flags = 0;
    if (keys["leap61"].num)
        flags |= f_LI_61;
    if (keys["leap59"].num)
        flags |= f_LI_59;
    if (keys["currentUtcOffsetValid"].num)
        flags |= f_UTCV;
    if (keys["ptpTimescale"].num)
        flags |= f_PTP;
    if (keys["timeTraceable"].num)
        flags |= f_TTRA;
    if (keys["frequencyTraceable"].num)
        flags |= f_FTRA;
    d.clockQuality.clockClass = keys["clockClass"].num;
    d.clockQuality.clockAccuracy = (clockAccuracy_e)keys["clockAccuracy"].num;
    d.clockQuality.offsetScaledLogVariance = keys["offsetScaledLogVariance"].num;
    d.currentUtcOffset = keys["currentUtcOffset"].num;
    d.flags = flags;
    d.timeSource = (timeSource_e)keys["timeSource"].num;
    build_end
build(SUBSCRIBE_EVENTS_NP)
    keys["duration"] = { .max = UINT16_MAX, .req = true };
    keys["NOTIFY_PORT_STATE"].handle = getOn;
    keys["NOTIFY_TIME_SYNC"].handle = getOn;
    if (parseKeys(keys))
        return nullptr;
    memset(d.bitmask, 0, sizeof(d.bitmask));
    if(keys["NOTIFY_PORT_STATE"].num)
        EVENT_BIT_SET(d.bitmask, NOTIFY_PORT_STATE);
    if(keys["NOTIFY_TIME_SYNC"].num)
        EVENT_BIT_SET(d.bitmask, NOTIFY_TIME_SYNC);
    d.duration = keys["duration"].num;
    build_end
build(SYNCHRONIZATION_UNCERTAIN_NP)
    keys["duration"] = { .max = UINT8_MAX, .req = true };
    if (parseKeys(keys))
        return nullptr;
    d.val = keys["duration"].num;
    build_end
build(PORT_DATA_SET_NP)
    keys["neighborPropDelayThresh"] = { .max = UINT32_MAX, .req = true };
    keys["asCapable"] = { .max = INT32_MAX, .req = true };
    if (parseKeys(keys))
        return nullptr;
    d.neighborPropDelayThresh = keys["neighborPropDelayThresh"].num;
    d.asCapable = keys["asCapable"].num;
    build_end

#define caseBuild(n) case n: return build_##n()
baseData *call_data(mng_vals_e id)
{
    switch(id) {
        caseBuild(USER_DESCRIPTION);
        caseBuild(INITIALIZE);
        caseBuild(PRIORITY1);
        caseBuild(PRIORITY2);
        caseBuild(DOMAIN);
        caseBuild(PRIMARY_DOMAIN);
        caseBuild(GRANDMASTER_SETTINGS_NP);
        caseBuild(SUBSCRIBE_EVENTS_NP);
        caseBuild(SYNCHRONIZATION_UNCERTAIN_NP);
        caseBuild(PORT_DATA_SET_NP);
        default: return nullptr;
    }
}
/* TODO add these set TLVs
A(SLAVE_ONLY,                          2008, clock, use_GS,     2,     UF)
A(LOG_ANNOUNCE_INTERVAL,               2009, port,  use_GS,     2,     UF)
A(ANNOUNCE_RECEIPT_TIMEOUT,            200a, port,  use_GS,     2,     UF)
A(LOG_SYNC_INTERVAL,                   200b, port,  use_GS,     2,     UF)
A(VERSION_NUMBER,                      200c, port,  use_GS,     2,     UF)
A(CLOCK_ACCURACY,                      2010, clock, use_GS,     2,     UF)
A(UTC_PROPERTIES,                      2011, clock, use_GS,     4,     UF)
A(TRACEABILITY_PROPERTIES,             2012, clock, use_GS,     2,     UF)
A(TIMESCALE_PROPERTIES,                2013, clock, use_GS,     2,     UF)
A(UNICAST_NEGOTIATION_ENABLE,          2014, port,  use_GS,     2,     UF)
A(PATH_TRACE_ENABLE,                   2016, clock, use_GS,     2,     UF)
A(ACCEPTABLE_MASTER_TABLE_ENABLED,     201b, port,  use_GS,     2,     UF)
A(ALTERNATE_MASTER,                    201d, port,  use_GS,     4,     UF)
A(ALTERNATE_TIME_OFFSET_ENABLE,        201e, clock, use_GS,     2,     UF)
A(ALTERNATE_TIME_OFFSET_PROPERTIES,    2021, clock, use_GS,    16,     UF)
A(LOG_MIN_PDELAY_REQ_INTERVAL,         6001, port,  use_GS,     2,     UF)
A(DELAY_MECHANISM,                     6000, port,  use_GS,     2,     UF)
A(EXTERNAL_PORT_CONFIGURATION_ENABLED, 3000, clock, use_GS,     2,     UF)
A(MASTER_ONLY,                         3001, port,  use_GS,     2,     UF)
A(HOLDOVER_UPGRADE_ENABLE,             3002, clock, use_GS,     2,     UF)
A(EXT_PORT_CONFIG_PORT_DATA_SET,       3003, port,  use_GS,     2,     UF)
*/
