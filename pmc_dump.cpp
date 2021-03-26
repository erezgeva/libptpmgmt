/* SPDX-License-Identifier: GPL-3.0-or-later */

/** @file
 * @brief dump received TLVs
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#include <cstring>
#include <functional>
#include "msg.h"

static const char toksep[] = " \t\n\r"; // while spaces
#define IDENT "\n\t\t"
#define dump(n) static inline void dump_##n(message &m, n##_t *dp) {\
        if(dp == nullptr)\
            dp = (n##_t*)m.getData();\
        if(dp == nullptr)\
            return;\
        n##_t &d = *dp;
#define dump_end }

dump(CLOCK_DESCRIPTION)
{
    printf(
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
        d.physicalLayerProtocol.str(),
        d.physicalAddress.toId().c_str(),
        m.netProt2str_c(d.protocolAddress.networkProtocol),
        d.protocolAddress.str().c_str(),
        binary::bufToId(d.manufacturerIdentity,
            sizeof(d.manufacturerIdentity)).c_str(),
        d.productDescription.str(), d.revisionData.str(), d.userDescription.str(),
        binary::bufToId(d.profileIdentity, sizeof(d.profileIdentity)).c_str());
    dump_end;
}
dump(USER_DESCRIPTION)
{
    printf(IDENT "userDescription  %s", d.userDescription.str());
    dump_end;
}
dump(INITIALIZE)
{
    printf(IDENT "initializationKey %u", d.initializationKey);
    dump_end;
}
dump(FAULT_LOG)
{
    printf(IDENT "numberOfFaultRecords %u", d.numberOfFaultRecords);
    uint16_t i = 0;
    for(auto &rec : d.faultRecords) {
        printf(
            IDENT "[%u] faultTime        %s"
            IDENT "[%u] severityCode     %s"
            IDENT "[%u] faultName        %s"
            IDENT "[%u] faultValue       %s"
            IDENT "[%u] faultDescription %s",
            i, rec.faultTime.str().c_str(),
            i, m.faultRec2str_c(rec.severityCode),
            i, rec.faultName.str(),
            i, rec.faultValue.str(),
            i, rec.faultDescription.str());
        i++;
    }
    dump_end;
}
dump(DEFAULT_DATA_SET)
{
    printf(
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
        d.clockIdentity.str().c_str(),
        d.domainNumber);
    dump_end;
}
dump(CURRENT_DATA_SET)
{
    printf(
        IDENT "stepsRemoved     %u"
        IDENT "offsetFromMaster %.1f"
        IDENT "meanPathDelay    %.1f",
        d.stepsRemoved,
        d.offsetFromMaster.getInterval(),
        d.meanPathDelay.getInterval());
    dump_end;
}
dump(PARENT_DATA_SET)
{
    printf(
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
        d.parentPortIdentity.str().c_str(),
        d.flags & 1,
        d.observedParentOffsetScaledLogVariance,
        d.observedParentClockPhaseChangeRate,
        d.grandmasterPriority1,
        d.grandmasterClockQuality.clockClass,
        d.grandmasterClockQuality.clockAccuracy,
        d.grandmasterClockQuality.offsetScaledLogVariance,
        d.grandmasterPriority2,
        d.grandmasterIdentity.str().c_str());
    dump_end;
}
dump(TIME_PROPERTIES_DATA_SET)
{
    printf(
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
        m.timeSrc2str_c(d.timeSource));
    dump_end;
}
dump(PORT_DATA_SET)
{
    printf(
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
        d.portIdentity.str().c_str(),
        m.portState2str_c(d.portState),
        d.logMinDelayReqInterval,
        (uint64_t)d.peerMeanPathDelay.getInterval(),
        d.logAnnounceInterval,
        d.announceReceiptTimeout,
        d.logSyncInterval,
        d.delayMechanism,
        d.logMinPdelayReqInterval,
        d.versionNumber);
    dump_end;
}
dump(PRIORITY1)
{
    printf(IDENT "priority1 %u", d.priority1);
    dump_end;
}
dump(PRIORITY2)
{
    printf(IDENT "priority2 %u", d.priority2);
    dump_end;
}
dump(DOMAIN)
{
    printf(IDENT "domainNumber %u", d.domainNumber);
    dump_end;
}
dump(SLAVE_ONLY)
{
    printf(IDENT "slaveOnly %u",
        /*
         * According to IEEE 1588, the flag is on bit 0
         * As linuxptp put the flag on the wrong bit
         * We test both bit 0 and bit 1.
         * As bit 1 is not due to be used, it should be safe
         */
        (m.getParams().useLinuxPTPTlvs ?
            ((d.flags & 0x3) > 0 ? 1 : 0) :
            (d.flags & 1)));
    dump_end;
}
dump(LOG_ANNOUNCE_INTERVAL)
{
    printf(IDENT "logAnnounceInterval %d", d.logAnnounceInterval);
    dump_end;
}
dump(ANNOUNCE_RECEIPT_TIMEOUT)
{
    printf(IDENT "announceReceiptTimeout %u", d.announceReceiptTimeout);
    dump_end;
}
dump(LOG_SYNC_INTERVAL)
{
    printf(IDENT "logSyncInterval %d", d.logSyncInterval);
    dump_end;
}
dump(VERSION_NUMBER)
{
    printf(IDENT "versionNumber %u", d.versionNumber);
    dump_end;
}
dump(TIME)
{
    printf(IDENT "currentTime %s", d.currentTime.str().c_str());
    dump_end;
}
dump(CLOCK_ACCURACY)
{
    printf(IDENT "clockAccuracy 0x%x", d.clockAccuracy);
    dump_end;
}
dump(UTC_PROPERTIES)
{
    printf(
        IDENT "currentUtcOffset      %d"
        IDENT "leap61                %u"
        IDENT "leap59                %u"
        IDENT "currentUtcOffsetValid %u",
        d.currentUtcOffset,
        m.is_LI_61(d.flags),
        m.is_LI_59(d.flags),
        m.is_UTCV(d.flags));
    dump_end;
}
dump(TRACEABILITY_PROPERTIES)
{
    printf(
        IDENT "timeTraceable      %u"
        IDENT "frequencyTraceable %u",
        m.is_TTRA(d.flags),
        m.is_FTRA(d.flags));
    dump_end;
}
dump(TIMESCALE_PROPERTIES)
{
    printf(IDENT "ptpTimescale %u", m.is_PTP(d.flags));
    dump_end;
}
dump(UNICAST_NEGOTIATION_ENABLE)
{
    printf(IDENT "unicastNegotiationPortDS %sabled", d.flags & 1 ? "e" : "dis");
    dump_end;
}
dump(PATH_TRACE_LIST)
{
    uint16_t i = 0;
    for(auto &rec : d.pathSequence)
        printf(IDENT "[%u] %s", i++, rec.str().c_str());
    dump_end;
}
dump(PATH_TRACE_ENABLE)
{
    printf(IDENT "pathTraceDS %sabled", d.flags & 1 ? "e" : "dis");
    dump_end;
}
dump(GRANDMASTER_CLUSTER_TABLE)
{
    printf(
        IDENT "logQueryInterval %d"
        IDENT "actualTableSize  %u",
        d.logQueryInterval,
        d.actualTableSize);
    uint16_t i = 0;
    for(auto &rec : d.PortAddress)
        printf(IDENT "[%u] %s", i++, rec.str().c_str());
    dump_end;
}
dump(UNICAST_MASTER_TABLE)
{
    printf(
        IDENT "logQueryInterval %d"
        IDENT "actualTableSize  %u",
        d.logQueryInterval,
        d.actualTableSize);
    uint16_t i = 0;
    for(auto &rec : d.PortAddress)
        printf(IDENT "[%u] %s", i++, rec.str().c_str());
    dump_end;
}
dump(UNICAST_MASTER_MAX_TABLE_SIZE)
{
    printf(IDENT "maxTableSize %u", d.maxTableSize);
    dump_end;
}
dump(ACCEPTABLE_MASTER_TABLE)
{
    printf(IDENT "actualTableSize %d", d.actualTableSize);
    uint16_t i = 0;
    for(auto &rec : d.list) {
        printf(
            IDENT "[%u] acceptablePortIdentity %s"
            IDENT "[%u] alternatePriority1     %u",
            i, rec.acceptablePortIdentity.str().c_str(),
            i, rec.alternatePriority1);
        i++;
    }
    dump_end;
}
dump(ACCEPTABLE_MASTER_TABLE_ENABLED)
{
    printf(IDENT "acceptableMasterPortDS %sabled", d.flags & 1 ? "e" : "dis");
    dump_end;
}
dump(ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
{
    printf(IDENT "maxTableSize %u", d.maxTableSize);
    dump_end;
}
dump(ALTERNATE_MASTER)
{
    printf(
        IDENT "transmitAlternateMulticastSync    %sabled"
        IDENT "logAlternateMulticastSyncInterval %d"
        IDENT "numberOfAlternateMasters          %u",
        d.flags & 1 ? "e" : "dis",
        d.logAlternateMulticastSyncInterval,
        d.numberOfAlternateMasters);
    dump_end;
}
dump(ALTERNATE_TIME_OFFSET_ENABLE)
{
    printf(
        IDENT "alternateTimescaleOffsetsDS[%u] %sabled", d.keyField,
        d.flags & 1 ? "e" : "dis");
    dump_end;
}
dump(ALTERNATE_TIME_OFFSET_NAME)
{
    printf(IDENT "[%u] %s", d.keyField, d.displayName.str());
    dump_end;
}
dump(ALTERNATE_TIME_OFFSET_MAX_KEY)
{
    printf(IDENT "maxKey %u", d.maxKey);
    dump_end;
}
dump(ALTERNATE_TIME_OFFSET_PROPERTIES)
{
    printf(
        IDENT "keyField       %u"
        IDENT "currentOffset  %d"
        IDENT "jumpSeconds    %d"
        IDENT "timeOfNextJump %ju",
        d.keyField,
        d.currentOffset,
        d.jumpSeconds,
        d.timeOfNextJump);
    dump_end;
}
dump(TRANSPARENT_CLOCK_PORT_DATA_SET)
{
    printf(
        IDENT "portIdentity            %s"
        IDENT "transparentClockPortDS  %s"
        IDENT "logMinPdelayReqInterval %i"
        IDENT "peerMeanPathDelay       %ju",
        d.portIdentity.str().c_str(),
        d.flags & 1 ? "true" : "false",
        d.logMinPdelayReqInterval,
        (uint64_t)d.peerMeanPathDelay.getInterval());
    dump_end;
}
dump(LOG_MIN_PDELAY_REQ_INTERVAL)
{
    printf(IDENT "logMinPdelayReqInterval %d", d.logMinPdelayReqInterval);
    dump_end;
}
dump(TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
{
    printf(
        IDENT "clockIdentity  %s"
        IDENT "numberPorts    %u"
        IDENT "delayMechanism %u"
        IDENT "primaryDomain  %u",
        d.clockIdentity.str().c_str(),
        d.numberPorts,
        d.delayMechanism,
        d.primaryDomain);
    dump_end;
}
dump(PRIMARY_DOMAIN)
{
    printf(IDENT "primaryDomain %u", d.primaryDomain);
    dump_end;
}
dump(DELAY_MECHANISM)
{
    printf(IDENT "delayMechanism %u", d.delayMechanism);
    dump_end;
}
dump(EXTERNAL_PORT_CONFIGURATION_ENABLED)
{
    printf(IDENT "externalPortConfiguration %sabled", d.flags & 1 ? "e" : "dis");
    dump_end;
}
dump(MASTER_ONLY)
{
    printf(IDENT "masterOnly %s", d.flags & 1 ? "true" : "false");
    dump_end;
}
dump(HOLDOVER_UPGRADE_ENABLE)
{
    printf(IDENT "holdoverUpgradeDS %sabled", d.flags & 1 ? "e" : "dis");
    dump_end;
}
dump(EXT_PORT_CONFIG_PORT_DATA_SET)
{
    printf(
        IDENT "acceptableMasterPortDS %sabled"
        IDENT "desiredState           %s",
        d.flags & 1 ? "e" : "dis",
        m.portState2str_c(d.desiredState));
    dump_end;
}
dump(TIME_STATUS_NP)
{
    printf(
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
        d.gmIdentity.str().c_str());
    dump_end;
}
dump(GRANDMASTER_SETTINGS_NP)
{
    printf(
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
        m.timeSrc2str_c(d.timeSource));
    dump_end;
}
dump(PORT_DATA_SET_NP)
{
    printf(
        IDENT "neighborPropDelayThresh %u"
        IDENT "asCapable               %d",
        d.neighborPropDelayThresh,
        d.asCapable);
    dump_end;
}
dump(SUBSCRIBE_EVENTS_NP)
{
    printf(
        IDENT "duration          %u"
        IDENT "NOTIFY_PORT_STATE %s"
        IDENT "NOTIFY_TIME_SYNC  %s",
        d.duration,
        EVENT_BIT(d.bitmask, NOTIFY_PORT_STATE),
        EVENT_BIT(d.bitmask, NOTIFY_TIME_SYNC));
    dump_end;
}
dump(PORT_PROPERTIES_NP)
{
    printf(
        IDENT "portIdentity            %s"
        IDENT "portState               %s"
        IDENT "timestamping            %s"
        IDENT "interface               %s",
        d.portIdentity.str().c_str(),
        m.portState2str_c(d.portState),
        m.ts2str_c(d.timestamping) + 3 /* Remove the 'TS_' prefix*/,
        d.interface.str());
    dump_end;
}
dump(PORT_STATS_NP)
{
    printf(
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
        d.portIdentity.str().c_str(),
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
        d.txMsgType[STAT_MANAGEMENT]);
    dump_end;
}
dump(SYNCHRONIZATION_UNCERTAIN_NP)
{
    printf(IDENT "uncertain %u", d.val);
    dump_end;
}

void call_dump(message &msg, baseData *data)
{
#define caseUF(n) case n: dump_##n(msg, (n##_t *)data); break;
#define A(n, v, sc, a, sz, f) case##f(n)
    switch(msg.getTlvId()) {
#include "ids.h"
        default:
            break;
    }
    printf("\n");
}

#define build(n)\
    static inline baseData *build_##n(message &m, char *save) {\
        n##_t *dp = new n##_t;\
        if(dp == nullptr)\
            return nullptr;\
        n##_t &d = *dp;
#define defKeys \
        std::map<std::string, val_key_t> keys;
#define parseKeys \
    if(parseKeysFunc(m, keys, save))\
        return nullptr;
#define build_end } return dp;

/* structure for parser */
struct val_key_t {
    bool can_str;        // [in]  check for quoted string & ignore wrong number
    int base;            // [in]  base for strtol()
    int64_t def;         // [in]  Default number value
    int64_t max;         // [in]  maximum number value
    int64_t min;         // [in]  minimum number value
    bool req;            // [in]  key is required
    int64_t num;         // [out] number value
    bool got_num;        // [out] flag indicate parsed number successfully
    const char *str_val; // [out] string value
    // Callback function for special value parser
    std::function<bool (val_key_t &key)> handle;
};
/*
 * @return
 * 0 No more tokens
 * 1 No quate string call strtok_r
 * 2 found quated token
 * -1 error
 */
static int parse_quate(char *save, char *&lastStr, char *&tkn)
{
    char *cur;
    // Was parse_quate last?
    if(lastStr != nullptr)
        cur = lastStr; // last call of parse_quate
    else
        cur = save; // last call of strtok_r
    // skip separator characters
    for(; strchr(toksep, *cur) != nullptr; cur++);
    if(*cur == 0) // No more tokens, error after a key
        return 0; // No more tokens
    // Do we have quoted string token?
    if(*cur != '"' && *cur != '\'')
        return 1;
    int close = *cur; // we search for the same char
    cur++; // Skip quote char
    char *start = cur;
    // Find the close and skip escaped close
    for(; (cur = strchr(cur, close)) != nullptr && *(cur - 1) == '\\';
        cur++);
    if(cur == nullptr)
        // quoted string is not close properly, we have error
        return -1; // error
    // We find the closing quote char
    *cur = 0; // close the string
    // Next token will be after the end quote char :-)
    lastStr = cur + 1;
    // We can save token
    tkn = start;
    // Now we need to handle the escapes in the string
    char *rcur, *wcur;
    rcur = wcur = start;
    while(*rcur != 0) {
        if(*rcur == '\\') {
            switch(*++rcur) {
                case 't': // Horizontal tab
                    *wcur = '\t';
                    break;
                case 'n': // Newline, Line feed
                    *wcur = '\n';
                    break;
                case 'r': // Carriage return
                    *wcur = '\r';
                    break;
                case 'a': // Alert, Beep
                    *wcur = 0x07;
                    break;
                case 'b': // Backspace
                    *wcur = 0x08;
                    break;
                case 'e': // Escape char
                    *wcur = 0x1b;
                    break;
                case 'f':// Formfeed, Page break
                    *wcur = 0x0c;
                    break;
                case 'v': // Vertical tab
                    *wcur = 0x0b;
                    break;
                // Other esacapes are ignored
                default:
                    *wcur++ = '\\';
                // self char escape
                case ' ':
                case '"':
                case '?':
                case '\'':
                case '\\':
                    *wcur = *rcur;
                    break;
            }
        } else
            *wcur = *rcur;
        wcur++;
        rcur++;
    }
    *wcur = 0; // close string
    return 2; // Found token
}
static bool parseKeysFunc(message &msg, std::map<std::string, val_key_t> &keys,
    char *orgSave)
{
    char *save = orgSave;
    // Update unrequired keys with default value.
    for(auto it = keys.begin(); it != keys.end(); it++) {
        if(!it->second.req) {
            it->second.str_val = ""; // empty string
            it->second.num = it->second.def;
        }
    }
    bool singleKey;
    bool singleKeyCanStr;
    if(keys.size() == 1) {
        singleKey = true;
        singleKeyCanStr = keys.begin()->second.can_str;
    } else {
        singleKey = false;
        singleKeyCanStr = false;
    }
    // In case we found quated string we point after it.
    // so, we look for the next token on the proper place.
    char *lastStr = nullptr;
    char *tkn; // Token found
    int ret = 1;
    while(ret > 0) {
        ret = 1; // take token using strtok_r
        if(singleKeyCanStr) {
            ret = parse_quate(save, lastStr, tkn);
            if(ret == -1)
                return true; // parse error
            if(ret == 0) // No more tokens we can continue to last part
                break;
            // ret == 1  take token using strtok_r
            // ret == 2  we have quated token
        }
        if(ret == 1) { // take normal token
            tkn = strtok_r(lastStr, toksep, &save);
            // No more tokens, we can continue to last part
            if(tkn == nullptr || *tkn == 0)
                break; // No more tokens we can continue to last part
            lastStr = nullptr; // Clear last quoted string (if was)
        }
        auto it = keys.begin();
        if(!singleKey) { // No need to search single key!
            for(; it != keys.end(); it++) {
                if(strcasecmp(it->first.c_str(), tkn) == 0)
                    break;
            }
        }
        // Unknown key
        if(it == keys.end())
            return true;
        val_key_t &key = it->second;
        ret = 1; // take token using strtok_r
        if(key.can_str) {
            ret = parse_quate(save, lastStr, tkn);
            if(ret == -1)
                return true; // pass error
            if(ret == 0 && !singleKey)
                break; // No more tokens we can continue to last part
            // ret == 0  use last token as value
            // ret == 1  take token using strtok_r
            // ret == 2  we have quated token
        }
        if(ret == 1) { // No quate string, take normal token
            char *ntkn = strtok_r(lastStr, toksep, &save);
            if(ntkn == nullptr || *ntkn == 0) {
                // No more tokens
                if(!singleKey)
                    break; // No more tokens we can continue to last part
                ret = 0; // use last token as value
            } else
                tkn = ntkn; // use new token
            lastStr = nullptr; // Clear last quoted string (if was)
        }
        key.str_val = tkn;
        if(key.handle == nullptr) {
            char *end;
            int64_t num = strtoll(tkn, &end, key.base);
            // Make sure value is integer and in range
            if(*end == 0 && num >= key.min && num <= key.max) {
                // In case we want to use string if number is wrong
                key.got_num = true;
                key.num = num;
            } else if(!key.can_str)
                // worng number, if we can't use the string then we have error.
                return true;
        } else if(key.handle(key))
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
static bool getTimeSource(val_key_t &key)
{
    int64_t num;
    const char *tkn = key.str_val;
    if(strcasecmp("ATOMIC_CLOCK", tkn) == 0)
        num = ATOMIC_CLOCK;
    else if(strcasecmp("GNSS", tkn) == 0)
        num = GNSS;
    else if(strcasecmp("GPS", tkn) == 0)
        num = GNSS;
    else if(strcasecmp("TERRESTRIAL_RADIO", tkn) == 0)
        num = TERRESTRIAL_RADIO;
    else if(strcasecmp("SERIAL_TIME_CODE", tkn) == 0)
        num = SERIAL_TIME_CODE;
    else if(strcasecmp("PTP", tkn) == 0)
        num = PTP;
    else if(strcasecmp("NTP", tkn) == 0)
        num = NTP;
    else if(strcasecmp("HAND_SET", tkn) == 0)
        num = HAND_SET;
    else if(strcasecmp("OTHER", tkn) == 0)
        num = OTHER;
    else if(strcasecmp("INTERNAL_OSCILLATOR", tkn) == 0)
        num = INTERNAL_OSCILLATOR;
    else { // Fallback into hex integer
        char *end;
        num = strtol(tkn, &end, 16);
        if(*end != 0 || num < 0 || num > UINT8_MAX)
            return true;
    }
    key.num = num;
    return false; // No errors!
}
static bool getportState(val_key_t &key)
{
    int64_t num;
    const char *tkn = key.str_val;
    if(strcasecmp("INITIALIZING", tkn) == 0)
        num = INITIALIZING;
    else if(strcasecmp("FAULTY", tkn) == 0)
        num = FAULTY;
    else if(strcasecmp("DISABLED", tkn) == 0)
        num = DISABLED;
    else if(strcasecmp("LISTENING", tkn) == 0)
        num = LISTENING;
    else if(strcasecmp("PRE_MASTER", tkn) == 0)
        num = PRE_MASTER;
    else if(strcasecmp("MASTER", tkn) == 0 || strcasecmp("SOURCE", tkn) == 0)
        num = SOURCE;
    else if(strcasecmp("PASSIVE", tkn) == 0)
        num = PASSIVE;
    else if(strcasecmp("UNCALIBRATED", tkn) == 0)
        num = UNCALIBRATED;
    else if(strcasecmp("SLAVE", tkn) == 0 || strcasecmp("CLIENT", tkn) == 0)
        num = CLIENT;
    else
        return true;
    key.num = num;
    return false; // No errors!
}
static bool getOn(val_key_t &key)
{
    if(strcasecmp("on", key.str_val) == 0)
        key.num = 1;
    else
        key.num = 0;
    return false; // No errors!
}
static bool getEnable(val_key_t &key)
{
    if(strcasecmp("enable", key.str_val) == 0)
        key.num = 1;
    else
        key.num = 0;
    return false; // No errors!
}

build(USER_DESCRIPTION)
{
    defKeys;
    keys["userDescription"] = { .can_str = true };
    parseKeys;
    d.userDescription.textField = keys["userDescription"].str_val;
    build_end;
}
build(INITIALIZE)
{
    defKeys;
    keys["initializationKey"] = { .def = INITIALIZE_EVENT, .max = UINT16_MAX };
    parseKeys;
    d.initializationKey = keys["initializationKey"].num;
    build_end;
}
build(PRIORITY1)
{
    defKeys;
    keys["priority1"] = { .def = 128, .max = 255 };
    parseKeys;
    d.priority1 = keys["priority1"].num;
    build_end;
}
build(PRIORITY2)
{
    defKeys;
    keys["priority2"] = { .def = 128, .max = 255 };
    parseKeys;
    d.priority2 = keys["priority2"].num;
    build_end;
}
build(DOMAIN)
{
    defKeys;
    keys["domainNumber"] = { .max = UINT8_MAX };
    parseKeys;
    d.domainNumber = keys["domainNumber"].num;
    build_end;
}
build(SLAVE_ONLY)
{
    defKeys;
    keys["slaveOnly"] = { .max = UINT8_MAX };
    parseKeys;
    d.flags = keys["slaveOnly"].num > 0 ? 1 : 0;
    build_end;
}
build(LOG_ANNOUNCE_INTERVAL)
{
    defKeys;
    keys["logAnnounceInterval"] = { .def = 1, .max = INT8_MAX };
    parseKeys;
    d.logAnnounceInterval = keys["logAnnounceInterval"].num;
    build_end;
}
build(ANNOUNCE_RECEIPT_TIMEOUT)
{
    defKeys;
    keys["announceReceiptTimeout"] = { .def = 3, .max = UINT8_MAX };
    parseKeys;
    d.announceReceiptTimeout = keys["announceReceiptTimeout"].num;
    build_end;
}
build(LOG_SYNC_INTERVAL)
{
    defKeys;
    keys["logSyncInterval"] = { .max = INT8_MAX };
    parseKeys;
    d.logSyncInterval = keys["logSyncInterval"].num > 0 ? 1 : 0;
    build_end;
}
build(VERSION_NUMBER)
{
    defKeys;
    keys["versionNumber"] = { .def = 2, .max = 0xf, .min = 1 };
    keys["minor"] = { .def = 0, .max = 0xf };
    parseKeys;
    d.versionNumber = (keys["minor"].num << 4) | keys["versionNumber"].num;
    build_end;
}
build(TIME)
{
    defKeys;
    keys["secondsField"] = { .max = UINT48_MAX, .req = 1 };
    keys["nanosecondsField"] = { .max = UINT32_MAX };
    parseKeys;
    d.currentTime.secondsField = keys["secondsField"].num;
    d.currentTime.nanosecondsField = keys["nanosecondsField"].num;
    build_end;
}
build(CLOCK_ACCURACY)
{
    defKeys;
    keys["clockAccuracy"] = { .def = Accurate_Unknown, .max = Accurate_Unknown };
    parseKeys;
    d.clockAccuracy = (clockAccuracy_e)keys["clockAccuracy"].num;
    build_end;
}
build(UTC_PROPERTIES)
{
    defKeys;
    keys["currentUtcOffset"] = { .max = INT16_MAX, .req = true };
    keys["leap61"] = { .max = UINT8_MAX };
    keys["leap59"] = { .max = UINT8_MAX };
    keys["currentUtcOffsetValid"] = { .max = UINT8_MAX };
    parseKeys;
    uint8_t flags = 0;
    if(keys["leap61"].num)
        flags |= f_LI_61;
    if(keys["leap59"].num)
        flags |= f_LI_59;
    if(keys["currentUtcOffsetValid"].num)
        flags |= f_UTCV;
    d.currentUtcOffset = keys["currentUtcOffset"].num;
    d.flags = flags;
    build_end;
}
build(TRACEABILITY_PROPERTIES)
{
    defKeys;
    keys["timeTraceable"] = { .max = UINT8_MAX };
    keys["frequencyTraceable"] = { .max = UINT8_MAX };
    parseKeys;
    uint8_t flags = 0;
    if(keys["timeTraceable"].num)
        flags |= f_TTRA;
    if(keys["frequencyTraceable"].num)
        flags |= f_FTRA;
    d.flags = flags;
    build_end;
}
build(TIMESCALE_PROPERTIES)
{
    defKeys;
    keys["ptpTimescale"] = { .max = UINT8_MAX };
    parseKeys;
    uint8_t flags = 0;
    if(keys["ptpTimescale"].num)
        flags |= f_PTP;
    d.flags = flags;
    build_end;
}
build(UNICAST_NEGOTIATION_ENABLE)
{
    defKeys;
    keys["unicastNegotiationPortDS"].handle = getEnable;
    parseKeys;
    d.flags = keys["unicastNegotiationPortDS"].num;
    build_end;
}
build(PATH_TRACE_ENABLE)
{
    defKeys;
    keys["pathTraceDS"].handle = getEnable;
    parseKeys;
    d.flags = keys["pathTraceDS"].num;
    build_end;
}
// TODO Table sets
// build(GRANDMASTER_CLUSTER_TABLE)
// build(UNICAST_MASTER_TABLE)
// build(ACCEPTABLE_MASTER_TABLE)
build(ACCEPTABLE_MASTER_TABLE_ENABLED)
{
    defKeys;
    keys["acceptableMasterPortDS"].handle = getEnable;
    parseKeys;
    d.flags = keys["acceptableMasterPortDS"].num;
    build_end;
}
build(ALTERNATE_MASTER)
{
    defKeys;
    keys["transmitAlternateMulticastSync"].handle = getEnable;
    keys["logAlternateMulticastSyncInterval"] = { .max = INT8_MAX };
    keys["numberOfAlternateMasters"] = { .max = UINT8_MAX };
    parseKeys;
    d.flags = keys["transmitAlternateMulticastSync"].num;
    d.logAlternateMulticastSyncInterval =
        keys["logAlternateMulticastSyncInterval"].num;
    d.numberOfAlternateMasters = keys["numberOfAlternateMasters"].num;
    build_end;
}
build(ALTERNATE_TIME_OFFSET_ENABLE)
{
    defKeys;
    keys["alternateTimescaleOffsetsDS"].handle = getEnable;
    parseKeys;
    d.flags = keys["alternateTimescaleOffsetsDS"].num;
    build_end;
}
build(ALTERNATE_TIME_OFFSET_NAME)
{
    defKeys;
    keys["keyField"] = { .max = UINT8_MAX, .req = true };
    keys["displayName"] = { .can_str = true, .req = true  };
    parseKeys;
    d.keyField = keys["keyField"].num;
    d.displayName.textField = keys["displayName"].str_val;
    build_end;
}
build(ALTERNATE_TIME_OFFSET_PROPERTIES)
{
    defKeys;
    keys["keyField"] = { .max = UINT8_MAX };
    keys["currentOffset"] = { .max = INT32_MAX };
    keys["jumpSeconds"] = { .max = INT32_MAX  };
    keys["timeOfNextJump"] = { .max = UINT48_MAX  };
    parseKeys;
    d.keyField = keys["keyField"].num;
    d.currentOffset = keys["currentOffset"].num;
    d.jumpSeconds = keys["jumpSeconds"].num;
    d.timeOfNextJump = keys["timeOfNextJump"].num;
    build_end;
}
build(LOG_MIN_PDELAY_REQ_INTERVAL)
{
    defKeys;
    keys["logMinPdelayReqInterval"] = { .max = INT8_MAX };
    parseKeys;
    d.logMinPdelayReqInterval = keys["logMinPdelayReqInterval"].num;
    build_end;
}
build(PRIMARY_DOMAIN)
{
    defKeys;
    keys["primaryDomain"] = { .max = UINT8_MAX };
    parseKeys;
    d.primaryDomain = keys["primaryDomain"].num;
    build_end;
}
build(DELAY_MECHANISM)
{
    defKeys;
    keys["delayMechanism"] = { .max = 0xfe };
    parseKeys;
    d.delayMechanism = keys["delayMechanism"].num;
    build_end;
}
build(EXTERNAL_PORT_CONFIGURATION_ENABLED)
{
    defKeys;
    keys["externalPortConfiguration"].handle = getEnable;
    parseKeys;
    d.flags = keys["externalPortConfiguration"].num;
    build_end;
}
build(MASTER_ONLY)
{
    defKeys;
    keys["masterOnly"].handle = getEnable;
    parseKeys;
    d.flags = keys["masterOnly"].num;
    build_end;
}
build(HOLDOVER_UPGRADE_ENABLE)
{
    defKeys;
    keys["holdoverUpgradeDS"].handle = getEnable;
    parseKeys;
    d.flags = keys["holdoverUpgradeDS"].num;
    build_end;
}
build(EXT_PORT_CONFIG_PORT_DATA_SET)
{
    defKeys;
    keys["acceptableMasterPortDS"].handle = getEnable;
    keys["desiredState"].handle = getportState;
    parseKeys;
    d.flags = keys["acceptableMasterPortDS"].num;
    d.desiredState = (portState_e)keys["desiredState"].num;
    build_end;
}
// libnuxptp specific implementation
build(GRANDMASTER_SETTINGS_NP)
{
    defKeys;
    keys["clockClass"] = { .max = UINT8_MAX, .req = true };
    keys["clockAccuracy"] = { .base = 16, .max = UINT8_MAX, .req = true };
    keys["offsetScaledLogVariance"] = {
        .base = 16,
        .max = UINT16_MAX,
        .req = true
    };
    keys["currentUtcOffset"] = { .max = INT16_MAX, .req = true };
    keys["leap61"] = { .max = UINT8_MAX };
    keys["leap59"] = { .max = UINT8_MAX };
    keys["currentUtcOffsetValid"] = { .max = UINT8_MAX };
    keys["ptpTimescale"] = { .max = UINT8_MAX };
    keys["timeTraceable"] = { .max = UINT8_MAX };
    keys["frequencyTraceable"] = { .max = UINT8_MAX };
    keys["timeSource"] = { .req = true };
    keys["timeSource"].handle = getTimeSource;
    parseKeys;
    uint8_t flags = 0;
    if(keys["leap61"].num)
        flags |= f_LI_61;
    if(keys["leap59"].num)
        flags |= f_LI_59;
    if(keys["currentUtcOffsetValid"].num)
        flags |= f_UTCV;
    if(keys["ptpTimescale"].num)
        flags |= f_PTP;
    if(keys["timeTraceable"].num)
        flags |= f_TTRA;
    if(keys["frequencyTraceable"].num)
        flags |= f_FTRA;
    d.clockQuality.clockClass = keys["clockClass"].num;
    d.clockQuality.clockAccuracy = (clockAccuracy_e)keys["clockAccuracy"].num;
    d.clockQuality.offsetScaledLogVariance = keys["offsetScaledLogVariance"].num;
    d.currentUtcOffset = keys["currentUtcOffset"].num;
    d.flags = flags;
    d.timeSource = (timeSource_e)keys["timeSource"].num;
    build_end;
}
build(PORT_DATA_SET_NP)
{
    defKeys;
    keys["neighborPropDelayThresh"] = { .max = UINT32_MAX, .req = true };
    keys["asCapable"] = { .max = INT32_MAX, .req = true };
    parseKeys;
    d.neighborPropDelayThresh = keys["neighborPropDelayThresh"].num;
    d.asCapable = keys["asCapable"].num;
    build_end;
}
build(SUBSCRIBE_EVENTS_NP)
{
    defKeys;
    keys["duration"] = { .max = UINT16_MAX, .req = true };
    keys["NOTIFY_PORT_STATE"].handle = getOn;
    keys["NOTIFY_TIME_SYNC"].handle = getOn;
    parseKeys;
    memset(d.bitmask, 0, sizeof(d.bitmask));
    if(keys["NOTIFY_PORT_STATE"].num)
        EVENT_BIT_SET(d.bitmask, NOTIFY_PORT_STATE);
    if(keys["NOTIFY_TIME_SYNC"].num)
        EVENT_BIT_SET(d.bitmask, NOTIFY_TIME_SYNC);
    d.duration = keys["duration"].num;
    build_end;
}
build(SYNCHRONIZATION_UNCERTAIN_NP)
{
    defKeys;
    keys["duration"] = { .max = UINT8_MAX, .req = true };
    parseKeys;
    d.val = keys["duration"].num;
    build_end;
}

#if 1
#define callBuild(n) n: return build_##n(msg, save)
#else
// Debug code, dump set instead of send it over network
#define callBuild(n)\
    n: {\
        baseData *d = build_##n(msg, save);\
        if(d != nullptr) {\
            printf("Dump " #n ":");\
            dump_##n(msg, (n##_t*)d);\
            printf("\n");\
        }\
        return nullptr;\
    }
#endif
baseData *call_data(message &msg, mng_vals_e id, char *save)
{
    switch(id) {
        case callBuild(USER_DESCRIPTION);
        case callBuild(INITIALIZE);
        case callBuild(PRIORITY1);
        case callBuild(PRIORITY2);
        case callBuild(DOMAIN);
        case callBuild(SLAVE_ONLY);
        case callBuild(LOG_ANNOUNCE_INTERVAL);
        case callBuild(ANNOUNCE_RECEIPT_TIMEOUT);
        case callBuild(LOG_SYNC_INTERVAL);
        case callBuild(VERSION_NUMBER);
        case callBuild(TIME);
        case callBuild(CLOCK_ACCURACY);
        case callBuild(UTC_PROPERTIES);
        case callBuild(TRACEABILITY_PROPERTIES);
        case callBuild(TIMESCALE_PROPERTIES);
        case callBuild(UNICAST_NEGOTIATION_ENABLE);
        case callBuild(PATH_TRACE_ENABLE);
        case callBuild(ACCEPTABLE_MASTER_TABLE_ENABLED);
        case callBuild(ALTERNATE_MASTER);
        case callBuild(ALTERNATE_TIME_OFFSET_ENABLE);
        case callBuild(ALTERNATE_TIME_OFFSET_NAME);
        case callBuild(ALTERNATE_TIME_OFFSET_PROPERTIES);
        case callBuild(LOG_MIN_PDELAY_REQ_INTERVAL);
        case callBuild(PRIMARY_DOMAIN);
        case callBuild(DELAY_MECHANISM);
        case callBuild(EXTERNAL_PORT_CONFIGURATION_ENABLED);
        case callBuild(MASTER_ONLY);
        case callBuild(HOLDOVER_UPGRADE_ENABLE);
        case callBuild(EXT_PORT_CONFIG_PORT_DATA_SET);
        case callBuild(GRANDMASTER_SETTINGS_NP);
        case callBuild(PORT_DATA_SET_NP);
        case callBuild(SUBSCRIBE_EVENTS_NP);
        case callBuild(SYNCHRONIZATION_UNCERTAIN_NP);
        default:
            return nullptr;
    }
}
