/* SPDX-License-Identifier: GPL-3.0-or-later */

/* pmc.cpp Impleament linuxptp pmc tool using the libpmc library
 *
 * Authors: Erez Geva <ErezGeva2@gmail.com>
 *
 * Created following "IEEE Std 1588-2008", PTP version 2
 */

#include <unistd.h>
#include <string>
#include <sys/select.h>
#include <signal.h>
#include "msg.h"
#include "sock.h"
#include "ptp.h"

// buffer for send and recv
int seq = 0;
const size_t bufSize = 2000;
char buf[bufSize];
configFile cfg;
msgParams prms;
message msg;
sockUnix sk;
const char toksep[] = " \t\n\a\r"; // while spaces

const char*act2str()
{
    switch(msg.getAction())
    {
        case GET: return "GET";
        case SET: return "SET";
        default: return "COMMAND";
    }
}
void dump_head()
{
    printf(
    "sending: %s %s\n"
    "\t%s seq %u RESPONSE MANAGEMENT %s ",
    act2str(), msg.c2str_c(msg.getTlvId()),
    msg.c2str(msg.getPeer()).c_str(),
    msg.getSequence(),
    msg.c2str_c(msg.getTlvId()));
}
void dump_err()
{
    printf(
    "sending: %s %s\n"
    "\t%s seq %u RESPONSE MANAGEMENT_ERROR_STATUS %s\n"
    "\tERROR: %s(%u)\n"
    "\tERROR DISPLAY: %s\n\n",
    act2str(), msg.c2str_c(msg.getTlvId()),
    msg.c2str(msg.getPeer()).c_str(),
    msg.getSequence(),
    msg.c2str_c(msg.getTlvId()),
    msg.c2str_c(msg.getErrId()), msg.getErrId(),
    msg.getErrDisplay_c()
// dump_err END

#define IDENT "\n\t\t"
#define dump(n) \
);} void dump_##n(message &m) {\
    struct n##_t &d = *(n##_t*)m.getData();\
    printf(

dump(CLOCK_DESCRIPTION)
    IDENT "clockType             0x%4x"
    IDENT "physicalLayerProtocol %s"
    IDENT "physicalAddress       %s"
    IDENT "protocolAddress       %s %s"
    IDENT "manufacturerId        %s"
    IDENT "productDescription    %s"
    IDENT "revisionData          %s"
    IDENT "userDescription       %s"
    IDENT "profileId             %s\n",
    d.clockType,
    m.c2str_c(d.physicalLayerProtocol),
    m.b2str(d.physicalAddress).c_str(),
    m.c2str_c(d.protocolAddress.networkProtocol),
    m.c2str(d.protocolAddress).c_str(),
    m.b2str(d.manufacturerIdentity, sizeof(d.manufacturerIdentity)).c_str(),
    m.c2str_c(d.productDescription), m.c2str_c(d.revisionData),
    m.c2str_c(d.userDescription),
    m.b2str(d.profileIdentity, sizeof(d.profileIdentity)).c_str()
dump(USER_DESCRIPTION)
    IDENT "userDescription  %s\n",
    m.c2str_c(d.userDescription)
dump(INITIALIZE)
    IDENT "initializationKey %u\n",
    d.initializationKey
dump(FAULT_LOG)
    IDENT "numberOfFaultRecords %u\n",
    d.numberOfFaultRecords);
    uint16_t i = 0;
    for(auto &rec: d.faultRecords) {
        printf(IDENT "[%u] faultTime        %s"
               IDENT "[%u] severityCode     %s"
               IDENT "[%u] faultName        %s"
               IDENT "[%u] faultValue       %s"
               IDENT "[%u] faultDescription %s",
               i, m.c2str(rec.faultTime).c_str(),
               i, m.c2str_c(rec.severityCode),
               i, m.c2str_c(rec.faultName),
               i, m.c2str_c(rec.faultValue),
               i, m.c2str_c(rec.faultDescription));
        i++;
    }
    printf("\n"
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
    IDENT "domainNumber            %u\n",
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
dump(CURRENT_DATA_SET)
    IDENT "stepsRemoved     %u"
    IDENT "offsetFromMaster %.1f"
    IDENT "meanPathDelay    %.1f\n",
    d.stepsRemoved,
    m.getInterval(d.offsetFromMaster),
    m.getInterval(d.meanPathDelay)
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
    IDENT "grandmasterIdentity                   %s\n",
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
dump(TIME_PROPERTIES_DATA_SET)
    IDENT "currentUtcOffset      %d"
    IDENT "leap61                %u"
    IDENT "leap59                %u"
    IDENT "currentUtcOffsetValid %u"
    IDENT "ptpTimescale          %u"
    IDENT "timeTraceable         %u"
    IDENT "frequencyTraceable    %u"
    IDENT "timeSource            %s\n",
    d.currentUtcOffset,
    m.is_LI_61(d.flags),
    m.is_LI_59(d.flags),
    m.is_UTCV(d.flags),
    m.is_PTP(d.flags),
    m.is_TTRA(d.flags),
    m.is_FTRA(d.flags),
    m.c2str_c(d.timeSource)
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
    IDENT "versionNumber           %u\n",
    m.c2str(d.portIdentity).c_str(),
    m.c2str_c(d.portState),
    d.logMinDelayReqInterval,
    (uint64_t)m.getInterval(d.peerMeanPathDelay),
    d.logAnnounceInterval,
    d.announceReceiptTimeout,
    d.logSyncInterval,
    d.delayMechanism,
    d.logMinPdelayReqInterval,
    d.versionNumber
dump(PRIORITY1)
    IDENT "priority1 %u\n",
    d.priority1
dump(PRIORITY2)
    IDENT "priority2 %u\n",
    d.priority2
dump(DOMAIN)
    IDENT "domainNumber %u\n",
    d.domainNumber
dump(SLAVE_ONLY)
    IDENT "slaveOnly %u\n",
    d.flags & 1
dump(LOG_ANNOUNCE_INTERVAL)
    IDENT "logAnnounceInterval %d\n",
    d.logAnnounceInterval
dump(ANNOUNCE_RECEIPT_TIMEOUT)
    IDENT "announceReceiptTimeout %u\n",
    d.announceReceiptTimeout
dump(LOG_SYNC_INTERVAL)
    IDENT "logSyncInterval %d\n",
    d.logSyncInterval
dump(VERSION_NUMBER)
    IDENT "versionNumber %u\n",
    d.versionNumber
dump(TIME)
    IDENT "currentTime %s\n",
    m.c2str(d.currentTime).c_str()
dump(CLOCK_ACCURACY)
    IDENT "clockAccuracy 0x%x\n",
    d.clockAccuracy
dump(UTC_PROPERTIES)
    IDENT "currentUtcOffset      %d"
    IDENT "leap61                %u"
    IDENT "leap59                %u"
    IDENT "currentUtcOffsetValid %u\n",
    d.currentUtcOffset,
    m.is_LI_61(d.flags),
    m.is_LI_59(d.flags),
    m.is_UTCV(d.flags)
dump(TRACEABILITY_PROPERTIES)
    IDENT "timeTraceable      %u"
    IDENT "frequencyTraceable %u\n",
    m.is_TTRA(d.flags),
    m.is_FTRA(d.flags)
dump(TIMESCALE_PROPERTIES)
    IDENT "ptpTimescale %u\n",
    m.is_PTP(d.flags)
dump(UNICAST_NEGOTIATION_ENABLE)
    IDENT "unicastNegotiationPortDS %sabled\n",
    d.flags & 1 ?"e":"dis"
dump(PATH_TRACE_LIST)
    "size %zu\n",
    d.pathSequence.size());
    uint16_t i = 0;
    for(auto &rec: d.pathSequence)
        printf(IDENT "[%u] %s" , i++, m.c2str(rec).c_str());
    printf("\n"
dump(PATH_TRACE_ENABLE)
    IDENT "pathTraceDS %sabled\n",
    d.flags & 1 ?"e":"dis"
dump(GRANDMASTER_CLUSTER_TABLE)
    IDENT "logQueryInterval %d"
    IDENT "actualTableSize  %u\n",
    d.logQueryInterval,
    d.actualTableSize);
    uint16_t i = 0;
    for(auto &rec: d.PortAddress)
        printf(IDENT "[%u] %s" , i++, m.c2str(rec).c_str());
    printf("\n"
dump(UNICAST_MASTER_TABLE)
    IDENT "logQueryInterval %d"
    IDENT "actualTableSize  %u\n",
    d.logQueryInterval,
    d.actualTableSize);
    uint16_t i = 0;
    for(auto &rec: d.PortAddress)
        printf(IDENT "[%u] %s" , i++, m.c2str(rec).c_str());
    printf("\n"
dump(UNICAST_MASTER_MAX_TABLE_SIZE)
    IDENT "maxTableSize %u\n",
    d.maxTableSize
dump(ACCEPTABLE_MASTER_TABLE)
    IDENT "actualTableSize %d\n",
    d.actualTableSize);
    uint16_t i = 0;
    for(auto &rec: d.list) {
        printf(IDENT "[%u] acceptablePortIdentity %s"
               IDENT "[%u] alternatePriority1     %u",
                i, m.c2str(rec.acceptablePortIdentity).c_str(),
                i, rec.alternatePriority1);
        i++;
    }
    printf("\n"
dump(ACCEPTABLE_MASTER_TABLE_ENABLED)
    IDENT "acceptableMasterPortDS %sabled\n",
    d.flags & 1 ?"e":"dis"
dump(ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
    IDENT "maxTableSize %u\n",
    d.maxTableSize
dump(ALTERNATE_MASTER)
    IDENT "transmitAlternateMulticastSync    %sabled"
    IDENT "logAlternateMulticastSyncInterval %d"
    IDENT "numberOfAlternateMasters          %u\n",
    d.flags & 1 ?"e":"dis",
    d.logAlternateMulticastSyncInterval,
    d.numberOfAlternateMasters
dump(ALTERNATE_TIME_OFFSET_ENABLE)
    IDENT "alternateTimescaleOffsetsDS[%u] %sabled\n",
    d.keyField, d.flags & 1 ?"e":"dis"
dump(ALTERNATE_TIME_OFFSET_NAME)
    IDENT "[%u] %s\n",
    d.keyField, m.c2str_c(d.displayName)
dump(ALTERNATE_TIME_OFFSET_MAX_KEY)
    IDENT "maxKey %u\n",
    d.maxKey
dump(ALTERNATE_TIME_OFFSET_PROPERTIES)
    IDENT "keyField       %u"
    IDENT "currentOffset  %d"
    IDENT "jumpSeconds    %d"
    IDENT "timeOfNextJump %ju\n",
    d.keyField,
    d.currentOffset,
    d.jumpSeconds,
    d.timeOfNextJump
dump(TRANSPARENT_CLOCK_PORT_DATA_SET)
    IDENT "portIdentity            %s"
    IDENT "transparentClockPortDS  %s"
    IDENT "logMinPdelayReqInterval %i"
    IDENT "peerMeanPathDelay       %ju\n",
    m.c2str(d.portIdentity).c_str(),
    d.flags & 1 ? "true" : "false",
    d.logMinPdelayReqInterval,
    (uint64_t)m.getInterval(d.peerMeanPathDelay)
dump(LOG_MIN_PDELAY_REQ_INTERVAL)
    IDENT "logMinPdelayReqInterval %d\n",
    d.logMinPdelayReqInterval
dump(TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
    IDENT "clockIdentity  %s"
    IDENT "numberPorts    %u"
    IDENT "delayMechanism %u"
    IDENT "primaryDomain  %u\n",
    m.c2str(d.clockIdentity).c_str(),
    d.numberPorts,
    d.delayMechanism,
    d.primaryDomain
dump(PRIMARY_DOMAIN)
    IDENT "primaryDomain %u\n",
    d.primaryDomain
dump(DELAY_MECHANISM)
    IDENT "delayMechanism %u\n",
    d.delayMechanism
dump(EXTERNAL_PORT_CONFIGURATION_ENABLED)
    IDENT "externalPortConfiguration %sabled\n",
    d.flags & 1 ?"e":"dis"
dump(MASTER_ONLY)
    IDENT "masterOnly %s\n",
    d.flags & 1 ?"true":"false"
dump(HOLDOVER_UPGRADE_ENABLE)
    IDENT "holdoverUpgradeDS %sabled\n",
    d.flags & 1 ?"e":"dis"
dump(EXT_PORT_CONFIG_PORT_DATA_SET)
    IDENT "acceptableMasterPortDS %sabled"
    IDENT "desiredState           %s\n",
    d.flags & 1 ?"e":"dis",
    m.c2str_c(d.desiredState)
dump(TIME_STATUS_NP)
    IDENT "master_offset              %jd"
    IDENT "ingress_time               %jd"
    IDENT "cumulativeScaledRateOffset %+.9f"
    IDENT "scaledLastGmPhaseChange    %d"
    IDENT "gmTimeBaseIndicator        %u"
    IDENT "lastGmPhaseChange          0x%04hx'%016jd.%04hx"
    IDENT "gmPresent                  %s"
    IDENT "gmIdentity                 %s\n",
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
    IDENT "timeSource              %s\n",
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
    m.c2str_c(d.timeSource)
dump(PORT_DATA_SET_NP)
    IDENT "neighborPropDelayThresh %u"
    IDENT "asCapable               %d\n",
    d.neighborPropDelayThresh,
    d.asCapable
dump(SUBSCRIBE_EVENTS_NP)
    IDENT "duration          %u"
    IDENT "NOTIFY_PORT_STATE %s"
    IDENT "NOTIFY_TIME_SYNC  %s\n",
    d.duration,
    EVENT_BIT(d.bitmask, NOTIFY_PORT_STATE),
    EVENT_BIT(d.bitmask, NOTIFY_TIME_SYNC)
dump(PORT_PROPERTIES_NP)
    IDENT "portIdentity            %s"
    IDENT "portState               %s"
    IDENT "timestamping            %s"
    IDENT "interface               %s\n",
    m.c2str(d.portIdentity).c_str(),
    m.c2str_c(d.portState),
    m.c2str_c(d.timestamping) + 3 /* Remove the 'TS_' prefix*/,
    m.c2str_c(d.interface)
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
    IDENT "tx_Management             %ju\n",
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
dump(SYNCHRONIZATION_UNCERTAIN_NP)
    IDENT "uncertain %u\n",
    d.val

// Close last dump
);}

bool sendAction()
{
    MNG_PARSE_ERROR_e err = msg.build(buf, bufSize, seq);
    if (err != MNG_PARSE_ERROR_OK) {
        printf("build error %s\n", msg.c2str_c(err));
        return false;
    }
    if (!sk.send(buf, msg.getMsgLen()))
        return false;
    seq++;
    return true;
}

bool rcv()
{
    auto cnt = sk.rcv(buf, bufSize);
    if (cnt < 0)
        return false;
    MNG_PARSE_ERROR_e err = msg.parse(buf, cnt);
    if (err == MNG_PARSE_ERROR_MSG)
        dump_err();
    else if (err != MNG_PARSE_ERROR_OK)
        printf("Parse error %s\n", msg.c2str_c(err));
    else {
        dump_head();
        switch(msg.getTlvId()) {
            #define caseUF(n) case n: dump_##n(msg); break;
            #define A(n, v, sc, a, sz, f) case##f(n)
            #include "ids.h"
            default:
                printf("\n");
                break;
        }
        return true;
    }
    return false;
}

bool findId(mng_vals_e &id, char *str)
{
    size_t len = strlen(str);
    if (len == 0)
        return false;
    // Make string uppercase as all commands are uppercase
    for(char *cur = str; *cur; cur++)
        *cur = toupper(*cur);
    if (strstr(str, "NULL") != nullptr) {
        id = NULL_PTP_MANAGEMENT;
        return true; // Any NULL
    }
    int find = 0;
    for(int i = FIRST_MNG_ID; i <= LAST_MNG_ID; i++) {
        const char *sid = message::c2str_c((mng_vals_e)i);
        if (strcmp(sid, str) == 0) {
            id = (mng_vals_e)i;
            return true; // Exect match!
        }
        if (find < 2 && strncmp(sid, str, len) == 0) {
            id = (mng_vals_e)i;
            find++;
        }
    }
    // 1 matach
    return find == 1;
}

void run_line(char *line)
{
    char *cur = strtok(line, toksep);
    if (cur == nullptr)
        return;
    actionField_e action;
    if (strcasecmp(cur, "get") == 0)
        action = GET;
    else if (strcasecmp(cur, "set") == 0)
        action = SET;
    else if (strcasecmp(cur, "cmd") == 0 ||
             strcasecmp(cur, "command") == 0)
        action = COMMAND;
    else if (strcasecmp(cur, "target") == 0) {
        cur = strtok(nullptr, toksep);
        if (cur == nullptr || *cur == 0)
            return;
        if (*cur == '*') {
            msg.setAllPorts();
        } else {
            char *end;
            uint16_t portNumber = strtol(cur, &end, 0);
            if (*end != 0)
                return; // invalid string
            prms.portNumber = portNumber;
            msg.updateParams(prms);
        }
    } else
        return;
    cur = strtok(nullptr, toksep);
    if (cur == nullptr)
        return;
    mng_vals_e id;
    if (!findId(id, cur))
        return;
    if (action == GET || msg.isEmpty(id)) {
        // No data is needed
        if (!msg.setAction(action, id))
            return;
    } else {
        baseData *data = nullptr;
        // TODO Handle data
        if (data == nullptr)
            return;
        if (!msg.setAction(action, id, *data))
            return;
    }
    if(sendAction() && sk.poll(500)) // 500 milli
        rcv();
}

void handle_sig(int)
{
    sk.close();
    exit(0);
}
void handle_sig_ctrl(int)
{
    sk.close();
    printf("\n");
    exit(0);
}

int main(int argc, char *const argv[])
{
    int c;
    std::map<int, std::string> options;
    const char *act_options = "f:b:d:s:t:v";
    const char *def_options = "i:uz";
    std::string opts = act_options;
    opts += def_options;
    while ((c = getopt(argc, argv, opts.c_str())) != -1) {
        switch(c) {
            case ':':
                printf("Wrong option ':'\n");
                return -1;
            case 'v':
                printf("%s\n", message::getVersion());
                return 0;
            default:
                break;
        }
        if (strrchr(act_options, c) != nullptr)
            options[c] = optarg;
        else if(strrchr(def_options, c) == nullptr) {
            printf("Wrong option '%c:'\n", c);
            return -1;
        }
    }

    /* Option that are on by default (though ignored)
       -u     Select the Unix Domain Socket transport.
       -z     Send GET with zero length TLV values.
     * unsuppurted options of pmc
     * we use only unix socket.
       -2     Select the IEEE 802.3 network transport.
       -4     Select the UDP IPv4 network transport.
       -6     Select the UDP IPv6 network transport.
     * interface is not used by unix socket.
       -i     interface
     * No need for help as we mimic pmc.
       -h     Display a help message.
     */

    /* Set defalt values*/
    uint8_t boundaryHops = 1;
    uint8_t domainNumber = cfg.domainNumber();
    uint8_t transportSpecific = cfg.transportSpecific();
    std::string uds_address = cfg.uds_address();

    /* fetch from command line */
    if (options.count('b'))
        boundaryHops = atoi(options['b'].c_str());
    if (options.count('d'))
        domainNumber = atoi(options['b'].c_str());
    if (options.count('t'))
        transportSpecific = strtol(options['t'].c_str(), nullptr, 16);
    if (options.count('s'))
        uds_address = options['s'];

    /* handle configuration file */
    if (options.count('f')) {
        if (!cfg.read_cfg(options['f'].c_str()))
            return -1;
        domainNumber = cfg.domainNumber();
        transportSpecific = cfg.transportSpecific();
        uds_address = cfg.uds_address();
    }

    prms = msg.getParams();
    prms.self_id.portNumber = getpid();
    prms.transportSpecific = transportSpecific;
    prms.domainNumber = domainNumber;
    prms.boundaryHops = boundaryHops;
    msg.updateParams(prms);

    if (!sk.setDefSelfAddress() || !sk.init() || !sk.setPeerAddress(cfg))
    {
        fprintf(stderr, "failed to create transport\n");
        return -1;
    }

    if (optind == argc) {
        // No arguments left, use batch mode
        // Normal Termination (by kill)
        if (signal(SIGTERM, handle_sig) == SIG_ERR)
            fprintf(stderr, "sig term fails %m\n");
        // Control + C
        if (signal(SIGINT, handle_sig_ctrl) == SIG_ERR)
            fprintf(stderr, "sig init fails %m\n");
        char lineBuf[bufSize];
        while(fgets(lineBuf, bufSize, stdin) != nullptr)
            run_line(lineBuf);
    } else for (int index = optind; index < argc; index++)
        run_line(argv[index]);
    return 0;
}

#if 0

# Add all users for testing (so we can test without using root :-)
sed -i '/^#define UDS_FILEMODE/ s#GRP)#GRP|S_IROTH|S_IWOTH)#' uds.c
make ; sudo /home.local/debian/TSN/build/linuxptp/ptp4l -f /etc/linuxptp/ptp4l.conf -i enp0s25

Testings, compare linuxptp-pmc with libpmc-pmc

time for n in ANNOUNCE_RECEIPT_TIMEOUT CLOCK_ACCURACY CLOCK_DESCRIPTION CURRENT_DATA_SET DEFAULT_DATA_SET DELAY_MECHANISM DOMAIN LOG_ANNOUNCE_INTERVAL LOG_MIN_PDELAY_REQ_INTERVAL LOG_SYNC_INTERVAL PARENT_DATA_SET PRIORITY1 PRIORITY2 SLAVE_ONLY TIMESCALE_PROPERTIES TIME_PROPERTIES_DATA_SET TRACEABILITY_PROPERTIES USER_DESCRIPTION VERSION_NUMBER PORT_DATA_SET TIME_STATUS_NP GRANDMASTER_SETTINGS_NP PORT_DATA_SET_NP PORT_PROPERTIES_NP PORT_STATS_NP SUBSCRIBE_EVENTS_NP SYNCHRONIZATION_UNCERTAIN_NP
do sudo /usr/sbin/pmc -u -f /etc/linuxptp/ptp4l.conf "get $n";done > org.txt

real    0m3.189s
user    0m0.167s
sys     0m0.138s

make ; time for n in ANNOUNCE_RECEIPT_TIMEOUT CLOCK_ACCURACY CLOCK_DESCRIPTION CURRENT_DATA_SET DEFAULT_DATA_SET DELAY_MECHANISM DOMAIN LOG_ANNOUNCE_INTERVAL LOG_MIN_PDELAY_REQ_INTERVAL LOG_SYNC_INTERVAL PARENT_DATA_SET PRIORITY1 PRIORITY2 SLAVE_ONLY TIMESCALE_PROPERTIES TIME_PROPERTIES_DATA_SET TRACEABILITY_PROPERTIES USER_DESCRIPTION VERSION_NUMBER PORT_DATA_SET TIME_STATUS_NP GRANDMASTER_SETTINGS_NP PORT_DATA_SET_NP PORT_PROPERTIES_NP PORT_STATS_NP SUBSCRIBE_EVENTS_NP SYNCHRONIZATION_UNCERTAIN_NP
do ./pmc -u -f /etc/linuxptp/ptp4l.conf "get $n";done > new.txt

real    0m0.085s * 37.517647059
user    0m0.040s * 4.175
sys     0m0.033s * 4.18

#endif
