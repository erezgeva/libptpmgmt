/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief convert management messages to json
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#include <stack>
#include <cmath>
#include <mutex>
#include <dlfcn.h>
#include "jsonDef.h"
#include "err.h"

#define stringify0(s) #s
#define stringify1(a) stringify0(a)

namespace ptpmgmt
{

#define JS(n) static inline bool proc_##n(JsonProc &proc, n##_t &d)
#define PROC_VAL(name) proc.procValue(#name, d.name)
#define PROC_BIN(name, len) proc.procBinary(#name, d.name, d.len)
#define PROC_OCT(name) proc.procBinary(#name, d.name, sizeof(d.name))
#define PROC_FLG(name, mask) proc.procFlag(#name, d.flags, mask)
#define PROC_FLG1(name) proc.procFlag(#name, d.flags, 1)
#define PROC_FLGB(name, flag, bit) proc.procFlag(#name, d.flag, (1 << bit))
#define PROC_ARR(name) proc.procArray(#name, d.name)
#define PROC_FLD(dir, name, macro)\
    proc.procValue(#dir "_" #name, d.dir##MsgType[STAT_##macro])

JS(CLOCK_DESCRIPTION)
{
    return
        PROC_VAL(clockType) &&
        PROC_VAL(physicalLayerProtocol) &&
        PROC_BIN(physicalAddress, physicalAddressLength) &&
        PROC_VAL(protocolAddress) &&
        PROC_OCT(manufacturerIdentity) &&
        PROC_VAL(productDescription) &&
        PROC_VAL(revisionData) &&
        PROC_VAL(userDescription) &&
        PROC_OCT(profileIdentity);
}
JS(USER_DESCRIPTION)
{
    return
        PROC_VAL(userDescription);
}
JS(INITIALIZE)
{
    return
        PROC_VAL(initializationKey);
}
JS(FAULT_LOG)
{
    return
        PROC_VAL(numberOfFaultRecords) &&
        PROC_ARR(faultRecords);
}
JS(DEFAULT_DATA_SET)
{
    return
        PROC_FLG(twoStepFlag, 1) &&
        PROC_FLG(slaveOnly, 2) &&
        PROC_VAL(numberPorts) &&
        PROC_VAL(priority1) &&
        PROC_VAL(clockQuality) &&
        PROC_VAL(priority2) &&
        PROC_VAL(clockIdentity) &&
        PROC_VAL(domainNumber);
}
JS(CURRENT_DATA_SET)
{
    return
        PROC_VAL(stepsRemoved) &&
        PROC_VAL(offsetFromMaster) &&
        PROC_VAL(meanPathDelay);
}
JS(PARENT_DATA_SET)
{
    return
        PROC_VAL(parentPortIdentity) &&
        PROC_FLG1(parentStats) &&
        PROC_VAL(observedParentOffsetScaledLogVariance) &&
        PROC_VAL(observedParentClockPhaseChangeRate) &&
        PROC_VAL(grandmasterPriority1) &&
        PROC_VAL(grandmasterClockQuality) &&
        PROC_VAL(grandmasterPriority2) &&
        PROC_VAL(grandmasterIdentity);
}
JS(TIME_PROPERTIES_DATA_SET)
{
    return
        PROC_VAL(currentUtcOffset) &&
        PROC_FLG(leap61, F_LI_61) &&
        PROC_FLG(leap59, F_LI_59) &&
        PROC_FLG(currentUtcOffsetValid, F_UTCV) &&
        PROC_FLG(ptpTimescale, F_PTP) &&
        PROC_FLG(timeTraceable, F_TTRA) &&
        PROC_FLG(frequencyTraceable, F_FTRA) &&
        PROC_VAL(timeSource);
}
JS(PORT_DATA_SET)
{
    return
        PROC_VAL(portIdentity) &&
        PROC_VAL(portState) &&
        PROC_VAL(logMinDelayReqInterval) &&
        PROC_VAL(peerMeanPathDelay) &&
        PROC_VAL(logAnnounceInterval) &&
        PROC_VAL(announceReceiptTimeout) &&
        PROC_VAL(logSyncInterval) &&
        PROC_VAL(delayMechanism) &&
        PROC_VAL(logMinPdelayReqInterval) &&
        PROC_VAL(versionNumber);
}
JS(PRIORITY1)
{
    return
        PROC_VAL(priority1);
}
JS(PRIORITY2)
{
    return
        PROC_VAL(priority2);
}
JS(DOMAIN)
{
    return
        PROC_VAL(domainNumber);
}
JS(SLAVE_ONLY)
{
    return
        PROC_FLG1(slaveOnly);
}
JS(LOG_ANNOUNCE_INTERVAL)
{
    return
        PROC_VAL(logAnnounceInterval);
}
JS(ANNOUNCE_RECEIPT_TIMEOUT)
{
    return
        PROC_VAL(announceReceiptTimeout);
}
JS(LOG_SYNC_INTERVAL)
{
    return
        PROC_VAL(logSyncInterval);
}
JS(VERSION_NUMBER)
{
    return
        PROC_VAL(versionNumber);
}
JS(TIME)
{
    return
        PROC_VAL(currentTime);
}
JS(CLOCK_ACCURACY)
{
    return
        PROC_VAL(clockAccuracy);
}
JS(UTC_PROPERTIES)
{
    return
        PROC_VAL(currentUtcOffset) &&
        PROC_FLG(leap61, F_LI_61) &&
        PROC_FLG(leap59, F_LI_59) &&
        PROC_FLG(currentUtcOffsetValid, F_UTCV);
}
JS(TRACEABILITY_PROPERTIES)
{
    return
        PROC_FLG(timeTraceable, F_TTRA) &&
        PROC_FLG(frequencyTraceable, F_FTRA);
}
JS(TIMESCALE_PROPERTIES)
{
    return
        PROC_FLG(ptpTimescale, F_PTP) &&
        PROC_VAL(timeSource);
}
JS(UNICAST_NEGOTIATION_ENABLE)
{
    return
        PROC_FLG1(unicastNegotiationPortDS);
}
JS(PATH_TRACE_LIST)
{
    return
        PROC_ARR(pathSequence);
}
JS(PATH_TRACE_ENABLE)
{
    return
        PROC_FLG1(pathTraceDS);
}
JS(GRANDMASTER_CLUSTER_TABLE)
{
    return
        PROC_VAL(logQueryInterval) &&
        PROC_VAL(actualTableSize) &&
        PROC_ARR(PortAddress);
}
JS(UNICAST_MASTER_TABLE)
{
    return
        PROC_VAL(logQueryInterval) &&
        PROC_VAL(actualTableSize) &&
        PROC_ARR(PortAddress);
}
JS(UNICAST_MASTER_MAX_TABLE_SIZE)
{
    return
        PROC_VAL(maxTableSize);
}
JS(ACCEPTABLE_MASTER_TABLE)
{
    return
        PROC_VAL(actualTableSize) &&
        PROC_ARR(list);
}
JS(ACCEPTABLE_MASTER_TABLE_ENABLED)
{
    return
        PROC_FLG1(acceptableMasterPortDS);
}
JS(ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
{
    return
        PROC_VAL(maxTableSize);
}
JS(ALTERNATE_MASTER)
{
    return
        PROC_FLG1(transmitAlternateMulticastSync) &&
        PROC_VAL(logAlternateMulticastSyncInterval) &&
        PROC_VAL(numberOfAlternateMasters);
}
JS(ALTERNATE_TIME_OFFSET_ENABLE)
{
    return
        PROC_VAL(keyField) &&
        PROC_FLG1(alternateTimescaleOffsetsDS);
}
JS(ALTERNATE_TIME_OFFSET_NAME)
{
    return
        PROC_VAL(keyField) &&
        PROC_VAL(displayName);
}
JS(ALTERNATE_TIME_OFFSET_MAX_KEY)
{
    return
        PROC_VAL(maxKey);
}
JS(ALTERNATE_TIME_OFFSET_PROPERTIES)
{
    return
        PROC_VAL(keyField) &&
        PROC_VAL(currentOffset) &&
        PROC_VAL(jumpSeconds) &&
        PROC_VAL(timeOfNextJump);
}
JS(TRANSPARENT_CLOCK_PORT_DATA_SET)
{
    return
        PROC_VAL(portIdentity) &&
        PROC_FLG1(transparentClockPortDS) &&
        PROC_VAL(logMinPdelayReqInterval) &&
        PROC_VAL(peerMeanPathDelay);
}
JS(LOG_MIN_PDELAY_REQ_INTERVAL)
{
    return
        PROC_VAL(logMinPdelayReqInterval);
}
JS(TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
{
    return
        PROC_VAL(clockIdentity) &&
        PROC_VAL(numberPorts) &&
        PROC_VAL(delayMechanism) &&
        PROC_VAL(primaryDomain);
}
JS(PRIMARY_DOMAIN)
{
    return
        PROC_VAL(primaryDomain);
}
JS(DELAY_MECHANISM)
{
    return
        PROC_VAL(delayMechanism);
}
JS(EXTERNAL_PORT_CONFIGURATION_ENABLED)
{
    return
        PROC_FLG1(externalPortConfiguration);
}
JS(MASTER_ONLY)
{
    return
        PROC_FLG1(masterOnly);
}
JS(HOLDOVER_UPGRADE_ENABLE)
{
    return
        PROC_FLG1(holdoverUpgradeDS);
}
JS(EXT_PORT_CONFIG_PORT_DATA_SET)
{
    return
        PROC_FLG1(acceptableMasterPortDS) &&
        PROC_VAL(desiredState);
}
// linuxptp implementation specific TLVs
JS(TIME_STATUS_NP)
{
    return
        PROC_VAL(master_offset) &&
        PROC_VAL(ingress_time) &&
        PROC_VAL(cumulativeScaledRateOffset) &&
        PROC_VAL(scaledLastGmPhaseChange) &&
        PROC_VAL(gmTimeBaseIndicator) &&
        PROC_VAL(nanoseconds_msb) &&
        PROC_VAL(nanoseconds_lsb) &&
        PROC_VAL(fractional_nanoseconds) &&
        PROC_VAL(gmPresent) &&
        PROC_VAL(gmIdentity);
}
JS(GRANDMASTER_SETTINGS_NP)
{
    return
        PROC_VAL(clockQuality) &&
        PROC_VAL(currentUtcOffset) &&
        PROC_FLG(leap61, F_LI_61) &&
        PROC_FLG(leap59, F_LI_59) &&
        PROC_FLG(currentUtcOffsetValid, F_UTCV) &&
        PROC_FLG(ptpTimescale, F_PTP) &&
        PROC_FLG(timeTraceable, F_TTRA) &&
        PROC_FLG(frequencyTraceable, F_FTRA) &&
        PROC_VAL(timeSource);
}
JS(PORT_DATA_SET_NP)
{
    return
        PROC_VAL(neighborPropDelayThresh) &&
        PROC_VAL(asCapable);
}
JS(SUBSCRIBE_EVENTS_NP)
{
    return
        PROC_VAL(duration) &&
        proc.procFlag("NOTIFY_PORT_STATE",
            d.byteEvent(NOTIFY_PORT_STATE),
            d.maskEvent(NOTIFY_PORT_STATE)) &&
        proc.procFlag("NOTIFY_TIME_SYNC",
            d.byteEvent(NOTIFY_TIME_SYNC),
            d.maskEvent(NOTIFY_TIME_SYNC));
}
JS(PORT_PROPERTIES_NP)
{
    return
        PROC_VAL(portIdentity) &&
        PROC_VAL(portState) &&
        PROC_VAL(timestamping) &&
        PROC_VAL(interface);
}
JS(PORT_STATS_NP)
{
    return
        PROC_VAL(portIdentity) &&
        PROC_FLD(rx, Sync, SYNC) &&
        PROC_FLD(rx, Delay_Req, DELAY_REQ) &&
        PROC_FLD(rx, Pdelay_Req, PDELAY_REQ) &&
        PROC_FLD(rx, Pdelay_Resp, PDELAY_RESP) &&
        PROC_FLD(rx, Follow_Up, FOLLOW_UP) &&
        PROC_FLD(rx, Delay_Resp, DELAY_RESP) &&
        PROC_FLD(rx, Pdelay_Resp_Follow_Up, PDELAY_RESP_FOLLOW_UP) &&
        PROC_FLD(rx, Announce, ANNOUNCE) &&
        PROC_FLD(rx, Signaling, SIGNALING) &&
        PROC_FLD(rx, Management, MANAGEMENT) &&
        PROC_FLD(tx, Sync, SYNC) &&
        PROC_FLD(tx, Delay_Req, DELAY_REQ) &&
        PROC_FLD(tx, Pdelay_Req, PDELAY_REQ) &&
        PROC_FLD(tx, Pdelay_Resp, PDELAY_RESP) &&
        PROC_FLD(tx, Follow_Up, FOLLOW_UP) &&
        PROC_FLD(tx, Delay_Resp, DELAY_RESP) &&
        PROC_FLD(tx, Pdelay_Resp_Follow_Up, PDELAY_RESP_FOLLOW_UP) &&
        PROC_FLD(tx, Announce, ANNOUNCE) &&
        PROC_FLD(tx, Signaling, SIGNALING) &&
        PROC_FLD(tx, Management, MANAGEMENT);
}
JS(SYNCHRONIZATION_UNCERTAIN_NP)
{
    return
        PROC_VAL(val);
}
JS(PORT_SERVICE_STATS_NP)
{
    return
        PROC_VAL(portIdentity) &&
        PROC_VAL(announce_timeout) &&
        PROC_VAL(sync_timeout) &&
        PROC_VAL(delay_timeout) &&
        PROC_VAL(unicast_service_timeout) &&
        PROC_VAL(unicast_request_timeout) &&
        PROC_VAL(master_announce_timeout) &&
        PROC_VAL(master_sync_timeout) &&
        PROC_VAL(qualification_timeout) &&
        PROC_VAL(sync_mismatch) &&
        PROC_VAL(followup_mismatch);
}
JS(UNICAST_MASTER_TABLE_NP)
{
    return
        PROC_VAL(actualTableSize) &&
        PROC_ARR(unicastMasters);
}
JS(PORT_HWCLOCK_NP)
{
    return
        PROC_VAL(portIdentity) &&
        PROC_VAL(phc_index) &&
        PROC_VAL(flags);
};
#if 0
JS(POWER_PROFILE_SETTINGS_NP)
{
    return
        PROC_VAL(version) &&
        PROC_VAL(grandmasterID) &&
        PROC_VAL(grandmasterTimeInaccuracy) &&
        PROC_VAL(networkTimeInaccuracy) &&
        PROC_VAL(totalTimeInaccuracy);
}
#endif

/* Finish Proccess functions of JsonProc */
#undef JS

#define procProperty(name)\
    procValue(#name, val.name)
#define procType(type) \
    void procValue(const char *name, const type &val) {\
        startName(name, " ");\
        m_result += std::to_string(val);\
    }\
    bool procValue(const char *name, type &val) {\
        startName(name, " ");\
        m_result += std::to_string(val);\
        return true;\
    }
#define procTypeEnum(type, func)\
    bool procValue(const char *name, type val) {\
        procString(name, m_msg.func(val));\
        return true;\
    }
#define procTypeEnumR(type, func)\
    bool procValue(const char *name, type &val) {\
        procString(name, m_msg.func(val));\
        return true;\
    }
#define procVector(type) \
    bool procArray(const char *name, std::vector<type> &val) {\
        procArray(name);\
        for(auto &rec : val) {\
            close();\
            procValue(rec);\
        }\
        closeArray();\
        return true;\
    }

bool JsonProc::procData(mng_vals_e managementId, const BaseMngTlv *&data)
{
#define _ptpmCaseUF(n) case n:\
        if(data == nullptr) {\
            n##_t *d = new n##_t;\
            if(d == nullptr)\
                return false;\
            data = d;\
        }\
        return proc_##n(*this, *dynamic_cast<n##_t *>\
                (const_cast<BaseMngTlv *>(data)));
    switch(managementId) {
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
#include "ids.h"
        default:
            return false;
    }
}

struct JsonProcToJson : public JsonProc {
    const Message &m_msg;
    std::string m_result;
    std::stack<bool> m_first_vals;
    int m_base_indent;
    bool m_first;
    JsonProcToJson(const Message &msg, int indent);
    bool data2json(mng_vals_e managementId, const BaseMngTlv *data);
    void sig2json(tlvType_e tlvType, const BaseSigTlv *tlv);
    void close() {
        if(!m_first)
            m_result += ',';
        m_result += '\n';
        m_first = false;
    }
    void indent() {
        for(size_t i = 0; i < m_first_vals.size() + m_base_indent; i++)
            m_result += "  ";
    }
    void startName(const char *name, const char *end) {
        close();
        indent();
        m_result += '"';
        m_result += name;
        m_result += "\" :";
        m_result += end;
    }
    void startObject() {
        indent();
        m_result += "{";
        m_first_vals.push(m_first);
        m_first = true;
    }
    void closeObject() {
        m_result += '\n';
        m_first = m_first_vals.top();
        m_first_vals.pop();
        indent();
        m_result += "}";
    }
    void procObject(const char *name) {
        startName(name, "\n");
        startObject();
    }
    void startArray() {
        indent();
        m_result += "[";
        m_first_vals.push(m_first);
        m_first = true;
    }
    void closeArray() {
        m_result += '\n';
        m_first = m_first_vals.top();
        m_first_vals.pop();
        indent();
        m_result += "]";
    }
    void procArray(const char *name) {
        startName(name, "\n");
        startArray();
    }
    void procString(const char *name, const std::string &val) {
        startName(name, " \"");
        m_result += val;
        m_result += '"';
    }
    void procValue(const char *name, const std::string &val) {
        startName(name, " ");
        m_result += val;
    }
    void procValue(const char *name, const Binary &val) {
        procString(name, val.toId());
    }
    void procBool(const char *name, const bool &val) {
        startName(name, " ");
        m_result += val ? "true" : "false";
    }
    void procNull(const char *name) {
        startName(name, " ");
        m_result += "null";
    }
    void procValue(ClockIdentity_t &val) {
        indent();
        m_result += '"';
        m_result += val.string();
        m_result += '"';
    }
    void procValue(const char *name, const PortIdentity_t &val) {
        procObject(name);
        procString("clockIdentity", val.clockIdentity.string());
        procValue("portNumber", val.portNumber);
        closeObject();
    }
    void procValue(PortAddress_t &val) {
        startObject();
        procProperty(networkProtocol);
        procProperty(addressField);
        closeObject();
    }
    void procValue(FaultRecord_t &val) {
        startObject();
        procProperty(faultRecordLength);
        procProperty(faultTime);
        procProperty(severityCode);
        procProperty(faultName);
        procProperty(faultValue);
        procProperty(faultDescription);
        closeObject();
    }
    void procValue(AcceptableMaster_t &val) {
        startObject();
        procProperty(acceptablePortIdentity);
        procProperty(alternatePriority1);
        closeObject();
    }
    void procValue(LinuxptpUnicastMaster_t &val) {
        startObject();
        procProperty(portIdentity);
        procProperty(clockQuality);
        procProperty(selected);
        procProperty(portState);
        procProperty(priority1);
        procProperty(priority2);
        procProperty(portAddress);
        closeObject();
    }
    void procValue(SLAVE_RX_SYNC_TIMING_DATA_rec_t &val) {
        startObject();
        procProperty(sequenceId);
        procProperty(syncOriginTimestamp);
        procProperty(totalCorrectionField);
        procProperty(scaledCumulativeRateOffset);
        procProperty(syncEventIngressTimestamp);
        closeObject();
    }
    void procValue(SLAVE_RX_SYNC_COMPUTED_DATA_rec_t &val) {
        startObject();
        procProperty(sequenceId);
        procProperty(offsetFromMaster);
        procProperty(meanPathDelay);
        procProperty(scaledNeighborRateRatio);
        closeObject();
    }
    void procValue(SLAVE_TX_EVENT_TIMESTAMPS_rec_t &val) {
        startObject();
        procProperty(sequenceId);
        procProperty(eventEgressTimestamp);
        closeObject();
    }
    void procValue(SLAVE_DELAY_TIMING_DATA_NP_rec_t &val) {
        startObject();
        procProperty(sequenceId);
        procProperty(delayOriginTimestamp);
        procProperty(totalCorrectionField);
        procProperty(delayResponseTimestamp);
        closeObject();
    }
    procType(uint8_t)
    procType(uint16_t)
    procType(uint32_t)
    procType(uint64_t)
    procType(int8_t)
    procType(int16_t)
    procType(int32_t)
    procType(int64_t)
    procType(float)
    procType(double)
    procType(long double)
    procTypeEnum(msgType_e, type2str_c)
    procTypeEnum(tlvType_e, tlv2str_c)
    procTypeEnum(mng_vals_e, mng2str_c)
    procTypeEnum(managementErrorId_e, errId2str_c)
    procTypeEnumR(networkProtocol_e, netProt2str_c)
    bool procValue(const char *name, clockAccuracy_e &val) {
        if(val < Accurate_within_1ps) {
            char buf[10];
            snprintf(buf, sizeof(buf), "0x%x", val);
            procString(name, buf);
        } else
            procString(name, m_msg.clockAcc2str_c(val));
        return true;
    }
    procTypeEnumR(faultRecord_e, faultRec2str_c)
    procTypeEnumR(timeSource_e, timeSrc2str_c)
    procTypeEnumR(portState_e, portState2str_c)
    procTypeEnumR(linuxptpTimeStamp_e, ts2str_c)
    procTypeEnumR(linuxptpPowerProfileVersion_e, pwr2str_c)
    procTypeEnumR(linuxptpUnicastState_e, us2str_c)
    bool procValue(const char *name, TimeInterval_t &val) {
        procValue(name, val.scaledNanoseconds);
        return true;
    }
    bool procValue(const char *name, Timestamp_t &val) {
        procValue(name, val.string());
        return true;
    }
    bool procValue(const char *name, ClockIdentity_t &val) {
        procString(name, val.string());
        return true;
    }
    bool procValue(const char *name, PortIdentity_t &val) {
        procObject(name);
        procString("clockIdentity", val.clockIdentity.string());
        procProperty(portNumber);
        closeObject();
        return true;
    }
    bool procValue(const char *name, PortAddress_t &val) {
        startName(name, "\n");
        procValue(val);
        return true;
    }
    bool procValue(const char *name, ClockQuality_t &val) {
        procObject(name);
        procProperty(clockClass);
        procProperty(clockAccuracy);
        procProperty(offsetScaledLogVariance);
        closeObject();
        return true;
    }
    bool procValue(const char *name, PTPText_t &val) {
        procString(name, val.textField);
        return true;
    }
    bool procValue(const char *name, FaultRecord_t &val) {
        startName(name, "\n");
        procValue(val);
        return true;
    }
    bool procValue(const char *name, AcceptableMaster_t &val) {
        startName(name, "\n");
        procValue(val);
        return true;
    }
    bool procValue(const char *name, LinuxptpUnicastMaster_t &val) {
        startName(name, "\n");
        procValue(val);
        return true;
    }
    bool procBinary(const char *name, Binary &val, uint16_t &) {
        procString(name, val.toId());
        return true;
    }
    bool procBinary(const char *name, uint8_t *val, size_t len) {
        procString(name, Binary::bufToId(val, len));
        return true;
    }
    bool procFlag(const char *name, uint8_t &flags, int mask) {
        procBool(name, (flags & mask) > 0);
        return true;
    }
    procVector(ClockIdentity_t)
    procVector(PortAddress_t)
    procVector(FaultRecord_t)
    procVector(AcceptableMaster_t)
    procVector(LinuxptpUnicastMaster_t)
    procVector(SLAVE_RX_SYNC_TIMING_DATA_rec_t)
    procVector(SLAVE_RX_SYNC_COMPUTED_DATA_rec_t)
    procVector(SLAVE_TX_EVENT_TIMESTAMPS_rec_t)
    procVector(SLAVE_DELAY_TIMING_DATA_NP_rec_t)
};

bool JsonProcToJson::data2json(mng_vals_e managementId, const BaseMngTlv *data)
{
    if(data == nullptr)
        procNull("dataField");
    else {
        procObject("dataField");
        procData(managementId, data);
        closeObject();
    }
    return true;
}

/* Signaling functions */
#define JS(n) static inline void parse_##n(JsonProcToJson &proc, n##_t &d)
#define parseTlv(n)\
    n:\
    parse_##n(*this, *(n##_t *)tlv);\
    break;

JS(MANAGEMENT)
{
    PROC_VAL(managementId);
    proc.data2json(d.managementId, d.tlvData.get());
}
JS(MANAGEMENT_ERROR_STATUS)
{
    PROC_VAL(managementId);
    PROC_VAL(managementErrorId);
    PROC_VAL(displayData);
}
JS(ORGANIZATION_EXTENSION)
{
    PROC_OCT(organizationId);
    PROC_OCT(organizationSubType);
    PROC_VAL(dataField);
}
JS(PATH_TRACE)
{
    PROC_ARR(pathSequence);
}
JS(ALTERNATE_TIME_OFFSET_INDICATOR)
{
    PROC_VAL(keyField);
    PROC_VAL(currentOffset);
    PROC_VAL(jumpSeconds);
    PROC_VAL(timeOfNextJump);
    PROC_VAL(displayName);
}
JS(ENHANCED_ACCURACY_METRICS)
{
    PROC_VAL(bcHopCount);
    PROC_VAL(tcHopCount);
    PROC_VAL(maxGmInaccuracy);
    PROC_VAL(varGmInaccuracy);
    PROC_VAL(maxTransientInaccuracy);
    PROC_VAL(varTransientInaccuracy);
    PROC_VAL(maxDynamicInaccuracy);
    PROC_VAL(varDynamicInaccuracy);
    PROC_VAL(maxStaticInstanceInaccuracy);
    PROC_VAL(varStaticInstanceInaccuracy);
    PROC_VAL(maxStaticMediumInaccuracy);
    PROC_VAL(varStaticMediumInaccuracy);
}
JS(L1_SYNC)
{
    PROC_FLGB(txCoherentIsRequired, flags1, 0);
    PROC_FLGB(rxCoherentIsRequired, flags1, 1);
    PROC_FLGB(congruentIsRequired, flags1, 2);
    PROC_FLGB(optParamsEnabled, flags1, 3);
    PROC_FLGB(isTxCoherent, flags2, 0);
    PROC_FLGB(isRxCoherent, flags2, 1);
    PROC_FLGB(isCongruent, flags2, 2);
}
JS(PORT_COMMUNICATION_AVAILABILITY)
{
    proc.procObject("syncMessageAvailability");
    PROC_FLGB(multicastCapable, syncMessageAvailability, 0);
    PROC_FLGB(unicastCapable, syncMessageAvailability, 1);
    PROC_FLGB(unicastNegotiationCapableEnable, syncMessageAvailability, 2);
    PROC_FLGB(unicastNegotiationCapable, syncMessageAvailability, 3);
    proc.closeObject();
    proc.procObject("delayRespMessageAvailability");
    PROC_FLGB(multicastCapable, delayRespMessageAvailability, 0);
    PROC_FLGB(unicastCapable, delayRespMessageAvailability, 1);
    PROC_FLGB(unicastNegotiationCapableEnable, delayRespMessageAvailability, 2);
    PROC_FLGB(unicastNegotiationCapable, delayRespMessageAvailability, 3);
    proc.closeObject();
}
JS(PROTOCOL_ADDRESS)
{
    PROC_VAL(portProtocolAddress);
}
JS(SLAVE_RX_SYNC_TIMING_DATA)
{
    PROC_VAL(syncSourcePortIdentity);
    PROC_ARR(list);
}
JS(SLAVE_RX_SYNC_COMPUTED_DATA)
{
    PROC_VAL(sourcePortIdentity);
    PROC_FLGB(scaledNeighborRateRatioValid, computedFlags, 0);
    PROC_FLGB(meanPathDelayValid, computedFlags, 1);
    PROC_FLGB(offsetFromMasterValid, computedFlags, 2);
    PROC_ARR(list);
}
JS(SLAVE_TX_EVENT_TIMESTAMPS)
{
    PROC_VAL(sourcePortIdentity);
    PROC_VAL(eventMessageType);
    PROC_ARR(list);
}
JS(CUMULATIVE_RATE_RATIO)
{
    PROC_VAL(scaledCumulativeRateRatio);
}
// linuxptp implementation specific
JS(SLAVE_DELAY_TIMING_DATA_NP)
{
    PROC_VAL(sourcePortIdentity);
    PROC_ARR(list);
}

void JsonProcToJson::sig2json(tlvType_e tlvType, const BaseSigTlv *tlv)
{
    close();
    startObject();
    procValue("tlvType", tlvType);
    switch(tlvType) {
        case parseTlv(MANAGEMENT);
        case parseTlv(MANAGEMENT_ERROR_STATUS);
        case ORGANIZATION_EXTENSION_PROPAGATE:
        case ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE:
        case parseTlv(ORGANIZATION_EXTENSION);
        case parseTlv(PATH_TRACE);
        case parseTlv(ALTERNATE_TIME_OFFSET_INDICATOR);
        case parseTlv(ENHANCED_ACCURACY_METRICS);
        case parseTlv(L1_SYNC);
        case parseTlv(PORT_COMMUNICATION_AVAILABILITY);
        case parseTlv(PROTOCOL_ADDRESS);
        case parseTlv(SLAVE_RX_SYNC_TIMING_DATA);
        case parseTlv(SLAVE_RX_SYNC_COMPUTED_DATA);
        case parseTlv(SLAVE_TX_EVENT_TIMESTAMPS);
        case parseTlv(CUMULATIVE_RATE_RATIO);
        // linuxptp implementation specific
        case parseTlv(SLAVE_DELAY_TIMING_DATA_NP);
        default:
            break;
    }
    closeObject();
}

JsonProcToJson::JsonProcToJson(const Message &msg, int indent) : m_msg(msg),
    m_base_indent(indent), m_first(false)
{
    startObject();
    procValue("sequenceId", msg.getSequence());
    procValue("sdoId", msg.getSdoId());
    procValue("domainNumber", msg.getDomainNumber());
    procValue("versionPTP", msg.getVersionPTP());
    procValue("minorVersionPTP", msg.getMinorVersionPTP());
    procBool("unicastFlag", msg.isUnicast());
    procValue("PTPProfileSpecific", msg.getPTPProfileSpecific());
    msgType_e type = msg.getType();
    procValue("messageType", type);
    procValue("sourcePortIdentity", msg.getPeer());
    procValue("targetPortIdentity", msg.getTarget());
    tlvType_e tlvType;
    mng_vals_e managementId;
    switch(type) {
        case Management:
            tlvType = msg.getMngType();
            managementId = msg.getTlvId();
            procString("actionField", msg.act2str_c(msg.getReplyAction()));
            procValue("tlvType", tlvType);
            procValue("managementId", managementId);
            switch(tlvType) {
                case MANAGEMENT:
                    data2json(managementId, msg.getData());
                    break;
                case MANAGEMENT_ERROR_STATUS:
                    procValue("managementErrorId", msg.getErrId());
                    procString("displayData", msg.getErrDisplay());
                    break;
                default:
                    break;
            }
            break;
        case Signaling:
            procArray("TLVs");
            msg.traversSigTlvs([this](const Message & msg, tlvType_e tlvType,
            const BaseSigTlv * tlv) {
                sig2json(tlvType, tlv);
                return false;
            });
            closeArray();
            break;
        default:
            break;
    }
    closeObject();
}

std::string msg2json(const Message &msg, int indent)
{
    JsonProcToJson proc(msg, indent);
    return proc.m_result;
}

// From JSON part
static int Json2msgCount = 0; // Count how many objects exist
#ifdef PIC // Shared library code
static void *jsonLib = nullptr;
static const char *useLib = nullptr;
static std::mutex jsonLoadLock; // Lock loading and unloading
#undef JSON_C_SLINK // No static link!
#define funcName(fname) ptpm_json_##fname##_p
#define funcDeclare0(fret, fname, fargs)\
    typedef fret(*fname##_t)(fargs);\
    fname##_t fname##_p = nullptr
#define funcAssign0(fname)\
    fname##_p = (fname##_t)dlsym(jsonLib, #fname);\
    if(fname##_p == nullptr)\
        return false
#define funcAssign(fname) funcAssign0(ptpm_json_##fname)
#else // PIC
#define funcName(fname) ptpm_json_##fname
#define funcDeclare0(fret, fname, fargs) fret fname(fargs)
#endif // PIC

#if defined JSON_C_SLINK || defined PIC
#define funcDeclare(fret, fname, fargs) funcDeclare0(fret, ptpm_json_##fname, fargs)
extern "C" {
    funcDeclare(void *, parse, const char *json);
    funcDeclare(void, free, void *jobj);
    funcDeclare(JsonProcFrom *, alloc_proc,);
}
#endif // funcDeclare

#ifdef PIC
static void doLibRm()
{
    if(dlclose(jsonLib) != 0)
        PTPMGMT_ERRORA("Fail to unload the libptpmngt from JSON library: %s",
            dlerror());
}
static void doLibNull()
{
    jsonLib = nullptr;
    useLib = nullptr;
    funcName(parse) = nullptr;
    funcName(free) = nullptr;
    funcName(alloc_proc) = nullptr;
}
static inline bool loadFuncs()
{
    funcAssign(parse);
    funcAssign(free);
    funcAssign(alloc_proc);
    return true;
}
static bool tryLib(const char *name)
{
    jsonLib = dlopen(name, RTLD_LAZY);
    if(jsonLib != nullptr) {
        if(loadFuncs()) {
            useLib = name;
            return true;
        }
        doLibRm();
    }
    return false;
}
static inline bool loadMatchLibrary(const char *libMatch, const char *found)
{
    if(found == nullptr) {
        PTPMGMT_ERRORA("Fail to find any library to matche pattern '%s'", libMatch);
        return false;
    } else if(jsonLib == nullptr) {
        // Try loading
        if(!tryLib(found)) {
            doLibNull(); // Ensure all pointers stay null
            PTPMGMT_ERRORA("Fail loading the matched fromJson library '%s' for "
                "pattern '%s'", found, libMatch);
            return false;
        }
    }
    // Already load, just check if it is the library we want
    else if(useLib != found) {  // We compare pointers, not strings!
        PTPMGMT_ERRORA("Already load a different library '%s', not the "
            "matched '%s' to pattern '%s'", useLib, found, libMatch);
        return false;
    }
    return true;
}
#endif // PIC

#ifndef JSON_C_SLINK
static bool doLoadLibrary(const char *libMatch = nullptr)
{
    #ifdef PIC
    std::unique_lock<std::mutex> lock(jsonLoadLock);
    const char *list[] = { JSON_C nullptr };
    if(libMatch != nullptr) {
        const char *found = nullptr;
        for(const char **cur = list; *cur != nullptr; cur++) {
            if(strcasestr(*cur, libMatch) != nullptr) {
                if(found != nullptr) {
                    PTPMGMT_ERRORA("Found multiple libraries match to pattern '%s'",
                        libMatch);
                    return false;
                }
                found = *cur;
            }
        }
        return loadMatchLibrary(libMatch, found);
    }
    if(jsonLib != nullptr)
        return true;
    for(const char **cur = list; *cur != nullptr; cur++) {
        if(tryLib(*cur))
            return true;
    }
    doLibNull(); // Ensure all pointers stay null
    PTPMGMT_ERROR("fail loading a fromJson library");
    #else // PIC
    PTPMGMT_ERROR("Static library compiled without fromJson library");
    #endif // PIC
    return false;
}
#endif // JSON_C_SLINK

Json2msg::Json2msg():
    m_managementId(NULL_PTP_MANAGEMENT),
    m_action(GET),
    m_have{0}
{
    Json2msgCount++;
}
Json2msg::~Json2msg()
{
    Json2msgCount--;
    #ifdef PIC
    std::unique_lock<std::mutex> lock(jsonLoadLock);
    if(Json2msgCount <= 0) {
        if(jsonLib != nullptr) {
            doLibRm();
            doLibNull(); // mark all pointers null
        }
        Json2msgCount = 0;
    }
    #endif // PIC
}

const char *Json2msg::loadLibrary()
{
    #ifdef PIC
    return useLib;
    #elif defined JSON_C_SLINK
    return stringify1(JSON_C_SLINK);
    #else
    return nullptr;
    #endif
}

bool Json2msg::selectLib(const std::string &libMatch)
{
    #ifndef JSON_C_SLINK
    if(doLoadLibrary(libMatch.c_str()))
        return true;
    #endif // JSON_C_SLINK
    return false;
}
bool Json2msg::isLibShared()
{
    #ifdef PIC
    return true;
    #else
    return false;
    #endif // PIC
}

bool Json2msg::fromJson(const std::string &json)
{
    #ifndef JSON_C_SLINK
    if(!doLoadLibrary())
        return false;
    #endif // JSON_C_SLINK
    #ifdef funcDeclare
    void *jobj = funcName(parse)(json.c_str());
    if(jobj == nullptr) {
        PTPMGMT_ERROR("JSON parse fail");
        return false;
    }
    bool ret = fromJsonObj(jobj);
    funcName(free)(jobj);
    return ret;
    #else // funcDeclare
    return false;
    #endif // funcDeclare
}
bool Json2msg::fromJsonObj(const void *jobj)
{
    #ifndef JSON_C_SLINK
    if(!doLoadLibrary())
        return false;
    #endif // JSON_C_SLINK
    #ifdef funcDeclare
    std::unique_ptr<JsonProcFrom> hold;
    JsonProcFrom *pproc = funcName(alloc_proc)();
    if(pproc == nullptr) {
        PTPMGMT_ERROR("fromJsonObj fail allocation of JsonProcFrom");
        return false;
    }
    hold.reset(pproc); // delete the object once we return :-)
    if(!pproc->mainProc(jobj))
        return false;
    const std::string &str = pproc->getActionField();
    if(str.empty()) {
        PTPMGMT_ERROR("Message do not have an action field");
        return false;
    } else if(str == "GET")
        m_action = GET;
    else if(str == "SET")
        m_action = SET;
    else if(str == "COMMAND")
        m_action = COMMAND;
    else {
        PTPMGMT_ERRORA("Message have wrong action field '%s'", str.c_str());
        return false;
    }
    const char *mngStrID;
    if(!pproc->procMng(m_managementId, mngStrID))
        return false;
    // Optional
    int64_t val;
#define optProc(key)\
    if(pproc->getIntVal(#key, val)) {\
        m_have[have_##key] = true;\
        m_##key = val;\
    }
    optProc(sequenceId)
    optProc(sdoId)
    optProc(domainNumber)
    optProc(versionPTP)
    optProc(minorVersionPTP)
    optProc(PTPProfileSpecific)
    if(pproc->getUnicastFlag(m_unicastFlag))
        m_have[have_unicastFlag] = true;
    bool have;
#define portProc(key, var)\
    if(pproc->parsePort(#key, have, m_##var)) {\
        if(have)\
            m_have[have_##var] = true;\
    } else\
        return false;
    portProc(sourcePortIdentity, srcPort)
    portProc(targetPortIdentity, dstPort)
    bool have_data = pproc->haveData();
    if(m_action == GET) {
        if(have_data) {
            PTPMGMT_ERROR("GET use dataField with zero values only, "
                "do not send dataField over JSON");
            return false;
        }
    } else if(Message::isEmpty(m_managementId)) {
        if(have_data) {
            PTPMGMT_ERRORA("%s do use dataField", mngStrID);
            return false;
        }
    } else {
        if(!have_data) {
            PTPMGMT_ERRORA("%s must use dataField", mngStrID);
            return false;
        }
        if(!pproc->parseData())
            return false;
        const BaseMngTlv *data = nullptr;
        if(!pproc->procData(m_managementId, data)) {
            if(data != nullptr) {
                delete data;
                PTPMGMT_ERROR("dataField parse error");
            } else
                PTPMGMT_ERRORA("Fail allocate %s_t", mngStrID);
            return false;
        }
        m_tlvData.reset(const_cast<BaseMngTlv *>(data));
    }
    return true;
    #else // funcDeclare
    return false;
    #endif // funcDeclare
}

}; /* namespace ptpmgmt */
