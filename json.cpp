/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief convert management messages to json
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */
#include <stack>
#include "json.h"
#ifdef PMC_USE_CJSON
#include <json.h>
#endif /*PMC_USE_CJSON*/

struct JsonProc {
    bool data2json(mng_vals_e managementId, const BaseMngTlv *data);
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
    bool m_first;
    JsonProcToJson(Message &msg);
    bool data2json(mng_vals_e managementId, const BaseMngTlv *data);
    void sig2json(tlvType_e tlvType, BaseSigTlv *tlv);
    void close() {
        if(!m_first)
            m_result += ',';
        m_result += '\n';
        m_first = false;
    }
    void indent() {
        for(size_t i = 0; i < m_first_vals.size(); i++)
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
    procTypeEnum(clockAccuracy_e, clockAcc2str_c)
    procTypeEnum(faultRecord_e, faultRec2str_c)
    procTypeEnum(timeSource_e, timeSrc2str_c)
    procTypeEnum(portState_e, portState2str_c)
    procTypeEnum(linuxptpTimeStamp_e, ts2str_c)
    bool procValue(const char *name, TimeInterval_t &val) {
        procValue(name, val.getInterval());
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

bool JsonProc::data2json(mng_vals_e managementId, const BaseMngTlv *data)
{
#define caseUF(n) case n: return proc_##n(*this, *(n##_t *)data);
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
        JsonProc::data2json(managementId, data);
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

JsonProcToJson::JsonProcToJson(Message &msg) : m_msg(msg), m_first(false)
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

std::string msg2json(Message &msg)
{
    JsonProcToJson proc(msg);
    return proc.m_result;
}

/**
 * From JSON part
 */
#ifdef PMC_USE_CJSON

struct JsonProcFromJson : public JsonProc {
    std::map<std::string, std::string> strVals;
    std::map<std::string, bool> boolVals;
    std::map<std::string, int64_t> intVals;
    /* Always use objVals last
     * so we can call use jloop() safely
     */
    std::map<std::string, json_object *> objVals;
    std::map<std::string, json_type> allow;
    void mainAllow() {
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
    bool jloop(json_object *jobj, bool withObj) {
        strVals.clear();
        boolVals.clear();
        intVals.clear();
        if(withObj)
            objVals.clear();
        auto it = json_object_iter_begin(jobj);
        auto end = json_object_iter_end(jobj);
        for(; !json_object_iter_equal(&it, &end); json_object_iter_next(&it)) {
            const char *key = json_object_iter_peek_name(&it);
            json_object *val = json_object_iter_peek_value(&it);
            json_type type = json_object_get_type(val);
            if(allow.count(key) < 1) {
                fprintf(stderr, "Key '%s' in not allowed\n", key);
                return false;
            }
            if(val == nullptr) {
                fprintf(stderr, "Key '%s' do not have value\n", key);
                return false;
            }
            if(type != allow[key]) {
                fprintf(stderr, "Key '%s' use wrong type '%s' instead of '%s'\n",
                    key, json_type_to_name(type), json_type_to_name(allow[key]));
                return false;
            }
            ssize_t count;
            switch(type) {
                case json_type_boolean:
                    count = boolVals.count(key);
                    break;
                case json_type_int:
                    count = intVals.count(key);
                    break;
                case json_type_string:
                    count = strVals.count(key);
                    break;
                case json_type_object:
                    if(withObj) {
                        count = objVals.count(key);
                        break;
                    }
                default:
                    count = -1;
                    break;
            }
            if(count != 0) {
                fprintf(stderr, "Key '%s' apear twice\n", key);
                return false;
            }
            switch(type) {
                case json_type_boolean:
                    boolVals[key] = json_object_get_boolean(val);
                    break;
                case json_type_int:
                    intVals[key] = json_object_get_int64(val);
                    break;
                case json_type_string:
                    strVals[key] = json_object_get_string(val);
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
    bool mainCheck() {
        // Optional, if value present check value
        if(strVals.count("messageType") > 0 &&
            strVals["messageType"].compare("Management")) {
            fprintf(stderr, "Message must be management, not '%s'\n",
                strVals["messageType"].c_str());
            return false;
        }
        // Optional, if value present check value
        if(strVals.count("tlvType") > 0 &&
            strVals["tlvType"].compare("MANAGEMENT")) {
            fprintf(stderr, "message must use management tlv, not '%s'\n",
                strVals["tlvType"].c_str());
            return false;
        }
        // Mandatory
        if(strVals.count("actionField") != 1) {
            fprintf(stderr, "message must have action field\n");
            return false;
        }
        if(strVals.count("managementId") != 1) {
            fprintf(stderr, "message must have managementId\n");
            return false;
        }
        return true;
    }
#define procType(type) \
    bool procValue(const char *name, type &val) {\
        if(intVals.count(name) != 1)\
            return false;\
        val = intVals[name];\
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
    bool procValue(const char *name, type &val) {\
        return false;\
    }
    bool procValue(const char *name, networkProtocol_e &val) {
        return false;
    }
    procType(clockAccuracy_e)
    procType(faultRecord_e)
    procType(timeSource_e)
    procType(portState_e)
    procType(linuxptpTimeStamp_e)
    procType(TimeInterval_t)
    procType(Timestamp_t)
    bool procValue(const char *name, ClockIdentity_t &val) {
        if(strVals.count(name) != 1)
            return false;
        Binary b;
        b.fromHex(strVals[name]);
        if(b.size() != val.size())
            return false;
        b.copy(val.v);
        return true;
    }
    bool procValue(const char *name, PortIdentity_t &d) {
        if(objVals.count(name) != 1)
            return false;
        allow.clear();
        allow["clockIdentity"] = json_type_string;
        allow["portNumber"] = json_type_int;
        bool ret = jloop(objVals[name], false) &&
            procValue("clockIdentity", d.clockIdentity) &&
            procValue("portNumber", d.portNumber);
        return ret;
    }
    bool procValue(const char *name, PortAddress_t &d) {
        if(objVals.count(name) != 1)
            return false;
        allow.clear();
        allow["networkProtocol"] = json_type_string;
        allow["addressField"] = json_type_string;
        bool ret = jloop(objVals[name], false) &&
            procValue("networkProtocol", d.networkProtocol) &&
            procBinary("addressField", d.addressField, d.addressLength);
        return ret;
    }
    procType(ClockQuality_t);
    procType(PTPText_t)
    procType(FaultRecord_t);
    procType(AcceptableMaster_t);
    bool procBinary(const char *name, Binary &d, uint16_t &len) {
        if(strVals.count(name) != 1)
            return false;
        d.fromId(strVals[name]);
        len = d.size();
        return true;
    }
    bool procBinary(const char *name, uint8_t *d, size_t len) {
        if(strVals.count(name) != 1)
            return false;
        Binary b;
        b.fromId(strVals[name]);
        if(b.size() != len)
            return false;
        b.copy(d);
        return true;
    }
    bool procFlag(const char *name, uint8_t &flags, int mask) {
        if(boolVals.count(name) != 1)
            return false;
        if(boolVals[name])
            flags |= mask;
        return true;
    }
#define procVector(type) \
    bool procArray(const char *name, std::vector<type> &val) {\
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
        fprintf(stderr, "JSON parse fail\n");
        return false;
    }
    // Remove object when function end
    json_object_delete keep(jobj);
    JsonProcFromJson proc;
    proc.mainAllow();
    if(!proc.jloop(jobj, true) || !proc.mainCheck())
        return false;
    const char *actionField = proc.strVals["actionField"].c_str();
    if(strcmp(actionField, "GET") == 0)
        m_action = GET;
    else if(strcmp(actionField, "SET") == 0)
        m_action = SET;
    else if(strcmp(actionField, "COMMAND") == 0)
        m_action = COMMAND;
    else {
        fprintf(stderr, "message must have wrong action field '%s'\n", actionField);
        return false;
    }
    bool find = false;
    const char *managementId = proc.strVals["managementId"].c_str();
    for(int i = FIRST_MNG_ID; i <= LAST_MNG_ID; i++) {
        mng_vals_e id = (mng_vals_e)i;
        if(strcmp(managementId, Message::mng2str_c(id)) == 0) {
            m_managementId = id;
            find = true;
            break;
        }
    }
    if(!find) {
        fprintf(stderr, "No such managementId '%s'\n", managementId);
        return false;
    }
    // Optional
    if(proc.intVals.count("sequenceId") == 1) {
        m_have[have_sequenceId] = true;
        m_sequenceId = proc.intVals["sequenceId"];
    }
    if(proc.intVals.count("sdoId") == 1) {
        m_have[have_sdoId] = true;
        m_sdoId = proc.intVals["sdoId"];
    }
    if(proc.intVals.count("domainNumber") == 1) {
        m_have[have_domainNumber] = true;
        m_domainNumber = proc.intVals["domainNumber"];
    }
    if(proc.intVals.count("versionPTP") == 1) {
        m_have[have_versionPTP] = true;
        m_versionPTP = proc.intVals["versionPTP"];
    }
    if(proc.intVals.count("minorVersionPTP") == 1) {
        m_have[have_minorVersionPTP] = true;
        m_minorVersionPTP = proc.intVals["minorVersionPTP"];
    }
    if(proc.intVals.count("PTPProfileSpecific") == 1) {
        m_have[have_PTPProfileSpecific] = true;
        m_PTPProfileSpecific = proc.intVals["PTPProfileSpecific"];
    }
    if(proc.boolVals.count("unicastFlag") == 1) {
        m_have[have_isUnicast] = true;
        m_isUnicast = proc.boolVals["unicastFlag"];
    }
    if(proc.boolVals.count("unicastFlag") == 1) {
        m_have[have_isUnicast] = true;
        m_isUnicast = proc.boolVals["unicastFlag"];
    }
    if(proc.objVals.count("sourcePortIdentity") == 1) {
        if(!proc.procValue("sourcePortIdentity", m_srcPort)) {
            fprintf(stderr, "Fail parsing sourcePortIdentity\n");
            return false;
        }
        m_have[have_srcPort] = true;
    }
    if(proc.objVals.count("targetPortIdentity") == 1) {
        if(!proc.procValue("targetPortIdentity", m_dstPort)) {
            fprintf(stderr, "Fail parsing targetPortIdentity\n");
            return false;
        }
        m_have[have_dstPort] = true;
    }
    // TODO    dataField m_tlvData
    return true;
}
#else /* No PMC_USE_CJSON*/
bool Json2msg::fromJson(const std::string json)
{
    fprintf(stderr, "fromJson need JSON-C library support\n");
    return false;
}
#endif /*PMC_USE_CJSON*/
