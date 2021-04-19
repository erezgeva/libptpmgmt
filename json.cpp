/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief convert management messages to json
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */
#include <stack>
#include <math.h>
#include "json.h"
#include "err.h"
#ifdef PMC_USE_CJSON
#include <json.h>
#endif /*PMC_USE_CJSON*/
#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC 1000000000L
#endif

struct JsonProc {
    bool procData(mng_vals_e managementId, const BaseMngTlv *&data);
#define procType(type) \
    virtual bool procValue(const char *name, type &val) = 0;
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
    procType(networkProtocol_e)
    procType(clockAccuracy_e)
    procType(faultRecord_e)
    procType(timeSource_e)
    procType(portState_e)
    procType(linuxptpTimeStamp_e)
    procType(TimeInterval_t)
    procType(Timestamp_t)
    procType(ClockIdentity_t)
    procType(PortIdentity_t)
    procType(PortAddress_t)
    procType(ClockQuality_t)
    procType(PTPText_t)
    procType(FaultRecord_t)
    procType(AcceptableMaster_t)
    virtual bool procBinary(const char *name, Binary &val, uint16_t &len) = 0;
    virtual bool procBinary(const char *name, uint8_t *val, size_t len) = 0;
    virtual bool procFlag(const char *name, uint8_t &flags, int mask) = 0;
#define procVector(type) \
    virtual bool procArray(const char *name, std::vector<type> &val) = 0;
    procVector(ClockIdentity_t)
    procVector(PortAddress_t)
    procVector(FaultRecord_t)
    procVector(AcceptableMaster_t)
#undef procType
#undef procVector
};

struct JsonProcToJson : public JsonProc {
    Message &m_msg;
    std::string m_result;
    std::stack<bool> m_first_vals;
    int m_base_indent;
    bool m_first;
    JsonProcToJson(Message &msg, int indent);
    bool data2json(mng_vals_e managementId, const BaseMngTlv *data);
    void sig2json(tlvType_e tlvType, BaseSigTlv *tlv);
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
#define procProperty(name)\
    procValue(#name, val.name)
    void procValue(ClockIdentity_t &val) {
        indent();
        m_result += '"';
        m_result += val.string();
        m_result += '"';
    }
    void procValue(const char *name, const PortIdentity_t &val) {
        procObject(name);
        procString("clockIdentity",  val.clockIdentity.string());
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
#undef procType
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

#define procTypeEnum(type, func)\
    bool procValue(const char *name, type val) {\
        procString(name, m_msg.func(val));\
        return true;\
    }
    procTypeEnum(msgType_e, type2str_c)
    procTypeEnum(tlvType_e, tlv2str_c)
    procTypeEnum(mng_vals_e, mng2str_c)
    procTypeEnum(managementErrorId_e, errId2str_c)
#undef procTypeEnum
#define procTypeEnum(type, func)\
    bool procValue(const char *name, type &val) {\
        procString(name, m_msg.func(val));\
        return true;\
    }
    procTypeEnum(networkProtocol_e, netProt2str_c)
    bool procValue(const char *name, clockAccuracy_e &val) {
        if(val < Accurate_within_1ps) {
            char buf[10];
            snprintf(buf, sizeof(buf), "0x%x", val);
            procString(name, buf);
        } else
            procString(name, m_msg.clockAcc2str_c(val));
        return true;
    }
    procTypeEnum(faultRecord_e, faultRec2str_c)
    procTypeEnum(timeSource_e, timeSrc2str_c)
    procTypeEnum(portState_e, portState2str_c)
    procTypeEnum(linuxptpTimeStamp_e, ts2str_c)
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
        procString("clockIdentity",  val.clockIdentity.string());
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
#define procVector(type) \
    bool procArray(const char *name, std::vector<type> &val) {\
        procArray(name);\
        for(auto rec: val) {\
            close();\
            procValue(rec);\
        }\
        closeArray();\
        return true;\
    }
    procVector(ClockIdentity_t)
    procVector(PortAddress_t)
    procVector(FaultRecord_t)
    procVector(AcceptableMaster_t)
    procVector(SLAVE_RX_SYNC_TIMING_DATA_rec_t)
    procVector(SLAVE_RX_SYNC_COMPUTED_DATA_rec_t)
    procVector(SLAVE_TX_EVENT_TIMESTAMPS_rec_t)
    procVector(SLAVE_DELAY_TIMING_DATA_NP_rec_t)
#undef procType
#undef procVector
};

#define JS(n) static inline bool proc_##n(JsonProc &proc, n##_t &d)
#define PROC_VAL(name) proc.procValue(#name, d.name)
#define PROC_BIN(name, len) proc.procBinary(#name, d.name, d.len)
#define PROC_OCT(name) proc.procBinary(#name, d.name, sizeof(d.name))
#define PROC_FLG(name, mask) proc.procFlag(#name, d.flags, mask)
#define PROC_FLG1(name) proc.procFlag(#name, d.flags, 1)
#define PROC_ARR(name) proc.procArray(#name, d.name)

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
            EVENT_BIT_BYTE(d.bitmask, NOTIFY_PORT_STATE),
            EVENT_BIT_MASK(d.bitmask, NOTIFY_PORT_STATE)) &&
        proc.procFlag("NOTIFY_TIME_SYNC",
            EVENT_BIT_BYTE(d.bitmask, NOTIFY_TIME_SYNC),
            EVENT_BIT_MASK(d.bitmask, NOTIFY_TIME_SYNC));
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
#define PROC_FLD(dir, name, macro)\
    proc.procValue(#dir "_" #name, d.dir##MsgType[STAT_##macro])
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

bool JsonProc::procData(mng_vals_e managementId, const BaseMngTlv *&data)
{
#define caseUF(n) case n:\
        if(data == nullptr) {\
            n##_t *d = new n##_t;\
            if(d == nullptr)\
                return false;\
            data = d;\
        }\
        return proc_##n(*this, *(n##_t *)data);
#define A(n, v, sc, a, sz, f) case##f(n)
    switch(managementId) {
#include "ids.h"
        default:
            return false;
    }
}

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

/**
 * Signaling functions
 */
#undef JS
#undef PROC_FLG
#define JS(n) static inline void parse_##n(JsonProcToJson &proc, n##_t &d)
#define PROC_FLG(name, flag, bit) proc.procFlag(#name, d.flag, (1 << bit))

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
    PROC_FLG(txCoherentIsRequired, flags1, 0);
    PROC_FLG(rxCoherentIsRequired, flags1, 1);
    PROC_FLG(congruentIsRequired, flags1, 2);
    PROC_FLG(optParamsEnabled, flags1, 3);
    PROC_FLG(isTxCoherent, flags2, 0);
    PROC_FLG(isRxCoherent, flags2, 1);
    PROC_FLG(isCongruent, flags2, 2);
}
JS(PORT_COMMUNICATION_AVAILABILITY)
{
    proc.procObject("syncMessageAvailability");
    PROC_FLG(multicastCapable, syncMessageAvailability, 0);
    PROC_FLG(unicastCapable, syncMessageAvailability, 1);
    PROC_FLG(unicastNegotiationCapableEnable, syncMessageAvailability, 2);
    PROC_FLG(unicastNegotiationCapable, syncMessageAvailability, 3);
    proc.closeObject();
    proc.procObject("delayRespMessageAvailability");
    PROC_FLG(multicastCapable, delayRespMessageAvailability, 0);
    PROC_FLG(unicastCapable, delayRespMessageAvailability, 1);
    PROC_FLG(unicastNegotiationCapableEnable, delayRespMessageAvailability, 2);
    PROC_FLG(unicastNegotiationCapable, delayRespMessageAvailability, 3);
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
    PROC_FLG(scaledNeighborRateRatioValid, computedFlags, 0);
    PROC_FLG(meanPathDelayValid, computedFlags, 1);
    PROC_FLG(offsetFromMasterValid, computedFlags, 2);
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

void JsonProcToJson::sig2json(tlvType_e tlvType, BaseSigTlv *tlv)
{
#define parseTlvAct(n)\
    parse_##n(*this, *(n##_t *)tlv);\
    break;
#define parseTlv(n) n: parseTlvAct(n)
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

JsonProcToJson::JsonProcToJson(Message &msg, int indent) : m_msg(msg),
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
            msg.traversSigTlvs
            ([this](const Message & msg, tlvType_e tlvType, BaseSigTlv * tlv) {
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

std::string msg2json(Message &msg, int indent)
{
    JsonProcToJson proc(msg, indent);
    return proc.m_result;
}

/**
 * From JSON part
 */
#ifdef PMC_USE_CJSON
#undef PROC_VAL
#define PROC_VAL(key) procValue(#key, d.key)
#define PROC_STR(val) (strcmp(str, #val) == 0)
#define GET_STR\
    if(strVals.count(key) != 1 || strVals[key].empty())\
        return false;\
    const char *str = strVals[key].c_str();

struct JsonProcFromJson : public JsonProc {
    std::map<std::string, std::string> strVals;   // json_type_string
    std::map<std::string, bool> boolVals;         // json_type_boolean
    std::map<std::string, int64_t> intVals;       // json_type_int
    std::map<std::string, double> fltVals;        // json_type_double
    std::map<std::string, array_list *> arrVals;  // json_type_array
    /* Always use objVals last
     * so we can call use jloop() safely
     */
    std::map<std::string, json_object *> objVals; // json_type_object
    std::map<std::string, json_type> allow;       // Allowed type
    std::map<std::string, json_type> found;       // Actual type
    JsonProcFromJson() {
        // Map main part
        allow["sequenceId"] = json_type_int;
        allow["sdoId"] = json_type_int;
        allow["domainNumber"] = json_type_int;
        allow["versionPTP"] = json_type_int;
        allow["minorVersionPTP"] = json_type_int;
        allow["PTPProfileSpecific"] = json_type_int;
        allow["unicastFlag"] = json_type_boolean;
        allow["messageType"] = json_type_string;
        allow["actionField"] = json_type_string;
        allow["tlvType"] = json_type_string;
        allow["managementId"] = json_type_string;
        allow["sourcePortIdentity"] = json_type_object;
        allow["targetPortIdentity"] = json_type_object;
        allow["dataField"] = json_type_object;
    }
    bool convertType(const char *key, json_type to) {
        if(found.count(key) != 1)
            return false;
        if(found[key] == to)
            return true;
        char *end;
        const char *str;
        switch(found[key]) {
            case json_type_int:
                switch(to) {
                    case json_type_double:
                        fltVals[key] = intVals[key];
                        break;
                    case json_type_boolean:
                        boolVals[key] = intVals[key] != 0;
                        break;
                    case json_type_string:
                        strVals[key] = std::to_string(intVals[key]);
                        break;
                    default:
                        return false;
                }
                break;
            case json_type_double:
                switch(to) {
                    case json_type_int:
                        if(!isnormal(fltVals[key]) || fmod(fltVals[key], 1) != 0)
                            return false;
                        intVals[key] = trunc(fltVals[key]);
                        break;
                    case json_type_boolean:
                        boolVals[key] = fltVals[key] != 0;
                        break;
                    case json_type_string:
                        strVals[key] = std::to_string(fltVals[key]);
                        break;
                    default:
                        return false;
                }
                break;
            case json_type_string:
                str = strVals[key].c_str();
                if(*str == 0)
                    return false;
                switch(to) {
                    case json_type_int:
                        intVals[key] = strtoll(str, &end, 0);
                        if(end == str || *end != 0)
                            return false;
                        break;
                    case json_type_double:
                        fltVals[key] = strtod(str, &end);
                        if(end == str || *end != 0)
                            return false;
                        break;
                    case json_type_boolean:
                        if(strcasecmp(str, "true"))
                            boolVals[key] = true;
                        else if(strcasecmp(str, "false"))
                            boolVals[key] = false;
                        else
                            return false;
                    default:
                        return false;
                }
                break;
            default:
                return false;
        }
        found[key] = to;
        return true;
    }
    bool jloop(json_object *jobj, bool withObj = false, bool withAllow = true) {
        strVals.clear();
        boolVals.clear();
        intVals.clear();
        fltVals.clear();
        arrVals.clear();
        if(withObj)
            objVals.clear();
        found.clear();
        auto it = json_object_iter_begin(jobj);
        auto end = json_object_iter_end(jobj);
        for(; !json_object_iter_equal(&it, &end); json_object_iter_next(&it)) {
            const char *key = json_object_iter_peek_name(&it);
            json_object *val = json_object_iter_peek_value(&it);
            json_type type = json_object_get_type(val);
            if(withAllow && allow.count(key) != 1) {
                PMC_ERRORA("Key '%s' in not allowed", key);
                return false;
            }
            if(val == nullptr) {
                PMC_ERRORA("Key '%s' do not have value", key);
                return false;
            }
            if(found.count(key) != 0) {
                PMC_ERRORA("Key '%s' apear twice", key);
                return false;
            }
            switch(type) { // Type are convertable
                case json_type_int:
                    intVals[key] = json_object_get_int64(val);
                    break;
                case json_type_double:
                    fltVals[key] = json_object_get_double(val);
                    break;
                case json_type_string:
                    strVals[key] = json_object_get_string(val);
                    break;
                default:
                    break;
            }
            found[key] = type;
            if(withAllow && !convertType(key, allow[key])) {
                PMC_ERRORA("Key '%s' use wrong type '%s' instead of '%s'",
                    key, json_type_to_name(type), json_type_to_name(allow[key]));
                return false;
            }
            switch(type) { // These type are NOT convertable
                case json_type_boolean:
                    boolVals[key] = json_object_get_boolean(val);
                    break;
                case json_type_array:
                    arrVals[key] = json_object_get_array(val);
                    break;
                case json_type_object:
                    if(withObj)
                        objVals[key] = val;
                    break;
                default:
                    break;
            }
        }
        return true;
    }
    bool mainProc(json_object *jobj) {
        if(!jloop(jobj, true))
            return false;
        // Optional, if value present verify it
#define testOpt(key, val, emsg)\
    if(strVals.count(#key) > 0 && strVals[#key].compare(#val)) {\
        PMC_ERRORA("Message must " emsg", not '%s'", strVals[#key].c_str());\
        return false;\
    }
        testOpt(messageType, Management, "be management")
        testOpt(tlvType, MANAGEMENT, "use management tlv")
        // Mandatory
#define testMand(key, emsg)\
    if(strVals.count(#key) != 1) {\
        PMC_ERROR("message must have " emsg);\
        return false;\
    }
        testMand(actionField, "action field")
        testMand(managementId, "management ID")
        return true;
    }
    bool procMng(mng_vals_e &id, const char *&str) {
        str = strVals["managementId"].c_str();
        for(int i = FIRST_MNG_ID; i <= LAST_MNG_ID; i++) {
            id = (mng_vals_e)i;
            if(strcmp(str, Message::mng2str_c(id)) == 0)
                return true;
        }
        PMC_ERRORA("No such managementId '%s'", str);
        return false;
    }
#undef procType
#define procType(type) \
    bool procValue(const char *key, type &val) {\
        if(!convertType(key, json_type_int) || intVals.count(key) != 1)\
            return false;\
        val = intVals[key];\
        return true;\
    }
    procType(uint8_t)
    procType(uint16_t)
    procType(uint32_t)
    procType(uint64_t)
    procType(int8_t)
    procType(int16_t)
    procType(int32_t)
    procType(int64_t)
#undef procType
#define procType(type) \
    bool procValue(const char *key, type &val) {\
        if(!convertType(key, json_type_double) || fltVals.count(key) != 1)\
            return false;\
        val = fltVals[key];\
        return true;\
    }
    procType(float)
    procType(double)
    procType(long double)
    bool procValue(const char *key, networkProtocol_e &d) {
        GET_STR
        for(int i = UDP_IPv4; i <= PROFINET; i++) {
            networkProtocol_e v = (networkProtocol_e)i;
            if(strcmp(str, Message::netProt2str_c(v)) == 0) {
                d = v;
                return true;
            }
        }
        return false;
    }
    bool procValue(const char *key, clockAccuracy_e &d) {
        if(found.count(key) != 1)
            return false;
        auto type = found[key];
        if(type == json_type_int) {
            auto v = intVals[key];
            if(v == Accurate_Unknown || (v <= Accurate_more_10s && v >= 0))
                d = (clockAccuracy_e)v;
            else
                return false;
        } else if(type == json_type_double) {
            auto v = fltVals[key];
            if(fmod(v, 1) == 0 && (v == Accurate_Unknown ||
                    (v <= Accurate_more_10s && v >= 0)))
                d = (clockAccuracy_e)v;
            else
                return false;
        } else if(type != json_type_string)
            return false;
        const char *str = strVals[key].c_str();
        if(*str == 0)
            return false;
        const char base[] = "Accurate_";
        const size_t size = sizeof(base);
        // Check enumerator values
        if(strlen(str) > size && strncmp(str, base, size) == 0) {
            if(PROC_STR(Accurate_Unknown))
                d = Accurate_Unknown;
            else {
                for(int i = Accurate_within_1ps; i <= Accurate_more_10s; i++) {
                    clockAccuracy_e v = (clockAccuracy_e)i;
                    if(strcmp(str + size, Message::clockAcc2str_c(v) + size) == 0) {
                        d = v;
                        return true;
                    }
                }
                return false;
            }
        } else { // Fall to number value
            char *end;
            long a = strtol(str, &end, 0);
            if(end == str || *end != 0 || a < 0 || a > Accurate_more_10s)
                return false;
            d = (clockAccuracy_e)a;
        }
        return true;
    }
    bool procValue(const char *key, faultRecord_e &d) {
        GET_STR
        for(int i = F_Emergency; i <= F_Debug; i++) {
            faultRecord_e v = (faultRecord_e)i;
            if(strcmp(str, Message::faultRec2str_c(v) + 2) == 0) {
                d = v;
                return true;
            }
        }
        return false;
    }
    bool procValue(const char *key, timeSource_e &d) {
        GET_STR
        for(int i = ATOMIC_CLOCK; i <= INTERNAL_OSCILLATOR; i++) {
            timeSource_e v = (timeSource_e)i;
            if(strcmp(str, Message::timeSrc2str_c(v)) == 0) {
                d = v;
                return true;
            }
        }
        // Check for additional values
        if(strcmp(str, "GPS") != 0) // Renamed to GNSS
            return false;
        d = GPS;
        return true;
    }
    bool procValue(const char *key, portState_e &d) {
        GET_STR
        for(int i = INITIALIZING; i <= CLIENT; i++) {
            portState_e v = (portState_e)i;
            if(strcmp(str, Message::portState2str_c(v)) == 0) {
                d = v;
                return true;
            }
        }
        // Check for additional values
        if(PROC_STR(MASTER)) // Renamed to SOURCE
            d =  MASTER;
        else if(PROC_STR(SLAVE)) // Renamed to CLIENT
            d = SLAVE;
        else
            return false;
        return true;
    }
    bool procValue(const char *key, linuxptpTimeStamp_e &d) {
        GET_STR
        for(int i = TS_SOFTWARE; i <= TS_P2P1STEP; i++) {
            linuxptpTimeStamp_e v = (linuxptpTimeStamp_e)i;
            if(strcmp(str, Message::ts2str_c(v) + 3) == 0) {
                d = v;
                return true;
            }
        }
        return false;
    }
    bool procValue(const char *key, TimeInterval_t &d) {
        if(!convertType(key, json_type_int) || intVals.count(key) != 1)
            return false;
        d.scaledNanoseconds = intVals[key];
        return true;
    }
    bool procValue(const char *key, Timestamp_t &d) {
        if(!convertType(key, json_type_double) || fltVals.count(key) != 1)
            return false;
        double v = fltVals[key];
        d.secondsField = trunc(v);
        v -= d.secondsField;
        d.nanosecondsField = trunc(v * NSEC_PER_SEC);
        return true;
    }
    bool procValue(const char *key, ClockIdentity_t &d) {
        Binary b;
        if(strVals.count(key) != 1 || !b.fromHex(strVals[key]) ||
            b.size() != d.size())
            return false;
        b.copy(d.v);
        return true;
    }
    bool procValue(const char *key, PortIdentity_t &d) {
        if(objVals.count(key) != 1)
            return false;
        allow.clear();
        allow["clockIdentity"] = json_type_string;
        allow["portNumber"] = json_type_int;
        return jloop(objVals[key]) &&
            PROC_VAL(clockIdentity) &&
            PROC_VAL(portNumber);
    }
    bool procValue(const char *key, PortAddress_t &d) {
        if(objVals.count(key) != 1)
            return false;
        allow.clear();
        allow["networkProtocol"] = json_type_string;
        allow["addressField"] = json_type_string;
        return jloop(objVals[key]) &&
            PROC_VAL(networkProtocol) &&
            procBinary("addressField", d.addressField, d.addressLength);
    }
    bool procValue(const char *key, ClockQuality_t &d) {
        if(objVals.count(key) != 1)
            return false;
        allow.clear();
        allow["clockClass"] = json_type_int;
        allow["clockAccuracy"] = json_type_string;
        allow["offsetScaledLogVariance"] = json_type_int;
        return jloop(objVals[key]) &&
            PROC_VAL(clockClass) &&
            PROC_VAL(clockAccuracy) &&
            PROC_VAL(offsetScaledLogVariance);
    }
    bool procValue(const char *key, PTPText_t &d) {
        if(!convertType(key, json_type_string) || strVals.count(key) != 1)
            return false;
        d.textField = strVals[key];
        return true;
    }
    bool procValue(const char *key, FaultRecord_t &d) {
        if(objVals.count(key) != 1)
            return false;
        allow.clear();
        allow["faultRecordLength"] = json_type_int;
        allow["faultTime"] = json_type_double;
        allow["severityCode"] = json_type_string;
        allow["faultName"] = json_type_string;
        allow["faultValue"] = json_type_string;
        allow["faultDescription"] = json_type_string;
        return jloop(objVals[key]) &&
            PROC_VAL(faultRecordLength) &&
            PROC_VAL(faultTime) &&
            PROC_VAL(severityCode) &&
            PROC_VAL(faultName) &&
            PROC_VAL(faultValue) &&
            PROC_VAL(faultDescription);
    }
    bool procValue(const char *key, AcceptableMaster_t &d) {
        if(objVals.count(key) != 1)
            return false;
        allow.clear();
        allow["alternatePriority1"] = json_type_int;
        allow["acceptablePortIdentity"] = json_type_object;
        std::map<std::string, json_object *> org;
        std::swap(org, objVals);
        bool ret = jloop(org[key], true) &&
            PROC_VAL(alternatePriority1) &&
            PROC_VAL(acceptablePortIdentity);
        std::swap(org, objVals);
        return ret;
    }
    bool procBinary(const char *key, Binary &d, uint16_t &len) {
        if(strVals.count(key) != 1 || !d.fromId(strVals[key]) || d.size() == 0)
            return false;
        len = d.size();
        return true;
    }
    bool procBinary(const char *key, uint8_t *d, size_t len) {
        Binary b;
        if(strVals.count(key) != 1 || !b.fromId(strVals[key]) || b.size() != len)
            return false;
        b.copy(d);
        return true;
    }
    bool procFlag(const char *key, uint8_t &flags, int mask) {
        if(!convertType(key, json_type_boolean) || boolVals.count(key) != 1)
            return false;
        if(boolVals[key])
            flags |= mask;
        return true;
    }
    // TODO support set arrays when parsing JSON
#define procVector(type) \
    bool procArray(const char *key, std::vector<type> &d) {\
        return false;\
    }
    procVector(ClockIdentity_t)
    procVector(PortAddress_t)
    procVector(FaultRecord_t)
    procVector(AcceptableMaster_t)
#undef procType
#undef procVector
};

class json_object_delete
{
  private:
    json_object *jobj;
  public:
    json_object_delete(json_object *o) : jobj(o) {}
    ~json_object_delete() { json_object_put(jobj); }
};

bool Json2msg::fromJson(const std::string json)
{
    json_object *jobj = json_tokener_parse(json.c_str());
    if(jobj == nullptr) {
        PMC_ERROR("JSON parse fail");
        return false;
    }
    json_object_delete keep(jobj); // Remove object when function end
    JsonProcFromJson proc;
    if(!proc.mainProc(jobj))
        return false;
    const char *str = proc.strVals["actionField"].c_str();
    if(PROC_STR(GET))
        m_action = GET;
    else if(PROC_STR(SET))
        m_action = SET;
    else if(PROC_STR(COMMAND))
        m_action = COMMAND;
    else {
        PMC_ERRORA("message must have wrong action field '%s'", str);
        return false;
    }
    const char *mngStrID;
    if(!proc.procMng(m_managementId, mngStrID))
        return false;
    // Optional
#define optProc(key, vals)\
    if(proc.vals.count(#key) == 1) {\
        m_have[have_##key] = true;\
        m_##key = proc.vals[#key];\
    }
    optProc(sequenceId, intVals)
    optProc(sdoId, intVals)
    optProc(domainNumber, intVals)
    optProc(versionPTP, intVals)
    optProc(minorVersionPTP, intVals)
    optProc(PTPProfileSpecific, intVals)
    optProc(unicastFlag, boolVals)
#define portProc(key, var)\
    if(proc.objVals.count(#key) == 1) {\
        if(!proc.procValue(#key, m_##var)) {\
            PMC_ERROR("Fail parsing " #key);\
            return false;\
        }\
        m_have[have_##var] = true;\
    }
    portProc(sourcePortIdentity, srcPort)
    portProc(targetPortIdentity, dstPort)
    size_t count = proc.objVals.count("dataField");
    if(m_action == GET) {
        if(count != 0) {
            PMC_ERROR("GET use dataField with zero values only, "
                "do not send dataField over JSON");
            return false;
        }
    } else if(Message::isEmpty(m_managementId)) {
        if(count != 0) {
            PMC_ERRORA("%s do use dataField", mngStrID);
            return false;
        }
    } else {
        if(count != 1) {
            PMC_ERRORA("%s must use dataField", mngStrID);
            return false;
        }
        json_object *dataField = proc.objVals["dataField"];
        if(!proc.jloop(dataField, true, false))
            return false;
        const BaseMngTlv *data = nullptr;
        if(!proc.procData(m_managementId, data)) {
            if(data != nullptr) {
                delete data;
                PMC_ERROR("dataField parse error");
            } else
                PMC_ERRORA("Fail allocate %s_t", mngStrID);
            return false;
        }
        m_tlvData.reset(const_cast<BaseMngTlv *>(data));
    }
    return true;
}
#else /* No PMC_USE_CJSON*/
bool Json2msg::fromJson(const std::string json)
{
    PMC_ERROR("fromJson need JSON-C library support");
    return false;
}
#endif /*PMC_USE_CJSON*/
