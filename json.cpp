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
// JSON library type
#define JSON_POBJ json_object*
#define JSON_TYPE json_type
#define JT_INT    json_type_int
#define JT_DOUBLE json_type_double
#define JT_STR    json_type_string
#define JT_BOOL   json_type_boolean
#define JT_ARRAY  json_type_array
#define JT_OBJ    json_type_object
#define JT_NULL   json_type_null
// JSON get functions
#define JG_TNAME(type) json_type_to_name(type)
#define JG_INT(obj)    json_object_get_int64(obj)
#define JG_DOUBLE(obj) json_object_get_double(obj)
#define JG_STR(obj)    json_object_get_string(obj)
#define JG_BOOL(obj)   json_object_get_boolean(obj)
#define JG_TYPE(obj)   json_object_get_type(obj)
// JSON iterate functions
#define JI_END(obj)   json_object_iter_end(obj)
#define JI_BEGIN(obj) json_object_iter_begin(obj)
#define JI_EQ(a, b)   json_object_iter_equal(&(a), &(b))
#define JI_NEXT(it)   json_object_iter_next(&(it))
#define JI_NAME(it)   json_object_iter_peek_name(&(it))
#define JI_VAL(it)    json_object_iter_peek_value(&(it))
// JSON array functions
#define JA_LEN(obj)      json_object_array_length(obj)
#define JA_GET(obj, idx) json_object_array_get_idx(obj, idx)
// JSON parser functions
#define JSON_PARSE(str)    json_tokener_parse(str)
#define JSON_OBJ_FREE(obj) json_object_put(obj)
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
#undef procVector
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
    if(!isType(key, JT_STR) || found[key].strV.empty())\
        return false;\
    const char *str = found[key].strV.c_str();

struct JsonVal {
    JSON_TYPE type;
    std::string strV;
    int64_t intV; // Also boolean
    double fltV;
    JSON_POBJ objV;
    bool blV() { return intV; }
    bool convType(JSON_TYPE to) {
        if(type == to)
            return true;
        char *end;
        const char *str;
        switch(type) {
            case JT_INT:
                switch(to) {
                    case JT_DOUBLE:
                        fltV = intV;
                        break;
                    case JT_BOOL:
                        break;
                    case JT_STR:
                        strV = std::to_string(intV);
                        break;
                    default:
                        return false;
                }
                break;
            case JT_DOUBLE:
                switch(to) {
                    case JT_INT:
                        if(!isnormal(fltV) || fmod(fltV, 1) != 0)
                            return false;
                        intV = trunc(fltV);
                        break;
                    case JT_BOOL:
                        intV = fltV != 0;
                        break;
                    case JT_STR:
                        strV = std::to_string(fltV);
                        break;
                    default:
                        return false;
                }
                break;
            case JT_STR:
                str = strV.c_str();
                if(*str == 0)
                    return false;
                switch(to) {
                    case JT_INT:
                        intV = strtoll(str, &end, 0);
                        if(end == str || *end != 0)
                            return false;
                        break;
                    case JT_DOUBLE:
                        fltV = strtod(str, &end);
                        if(end == str || *end != 0)
                            return false;
                        break;
                    case JT_BOOL:
                        if(strcasecmp(str, "true"))
                            intV = true;
                        else if(strcasecmp(str, "false"))
                            intV = false;
                        else
                            return false;
                    default:
                        return false;
                }
                break;
            default:
                return false;
        }
        type = to;
        return true;
    }
    void set(JSON_POBJ obj, JSON_TYPE t) {
        type = t;
        objV = obj;
        switch(type) {
            case JT_INT:
                intV = JG_INT(obj);
                break;
            case JT_DOUBLE:
                fltV = JG_DOUBLE(obj);
                break;
            case JT_STR:
                strV = JG_STR(obj);
                break;
            case JT_BOOL:
                intV = JG_BOOL(obj);
                break;
            case JT_ARRAY:
            case JT_OBJ:
            case JT_NULL:
                break;
        }
    }
};

struct JsonProcFromJson : public JsonProc {
    std::map<std::string, JSON_TYPE> allow; // Allowed type
    std::map<std::string, JsonVal> found;   // Actual values
    std::stack<std::map<std::string, JsonVal>> history;

    JsonProcFromJson() {
        // Map main part
        allow["sequenceId"] = JT_INT;
        allow["sdoId"] = JT_INT;
        allow["domainNumber"] = JT_INT;
        allow["versionPTP"] = JT_INT;
        allow["minorVersionPTP"] = JT_INT;
        allow["PTPProfileSpecific"] = JT_INT;
        allow["unicastFlag"] = JT_BOOL;
        allow["messageType"] = JT_STR;
        allow["actionField"] = JT_STR;
        allow["tlvType"] = JT_STR;
        allow["managementId"] = JT_STR;
        allow["sourcePortIdentity"] = JT_OBJ;
        allow["targetPortIdentity"] = JT_OBJ;
        allow["dataField"] = JT_OBJ;
    }
    bool convertType(const char *key, JSON_TYPE to) {
        if(found.count(key) != 1)
            return false;
        return found[key].convType(to);
    }
    bool isType(const char *key, JSON_TYPE type) {
        if(found.count(key) == 1 && found[key].type == type)
            return true;
        return false;
    }
    void push() {
        history.push(found);
    }
    void pop() {
        found = history.top();
        history.pop();
    }
    bool jloop(JSON_POBJ jobj, bool withAllow = true) {
        found.clear();
        auto end = JI_END(jobj);
        for(auto it = JI_BEGIN(jobj); !JI_EQ(it, end); JI_NEXT(it)) {
            const char *key = JI_NAME(it);
            JSON_POBJ val = JI_VAL(it);
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
            JSON_TYPE type = JG_TYPE(val);
            found[key].set(val, type);
            if(withAllow && !found[key].convType(allow[key])) {
                // Ignore dataField with null
                if(strcmp("dataField", key) || type != JT_NULL) {
                    PMC_ERRORA("Key '%s' use wrong type '%s' instead of '%s'",
                        key, JG_TNAME(type), JG_TNAME(allow[key]));
                    return false;
                }
            }
        }
        return true;
    }
    bool mainProc(JSON_POBJ jobj) {
        if(!jloop(jobj))
            return false;
        // Optional, if value present verify it
#define testOpt(key, val, emsg)\
    if(isType(#key, JT_STR) && found[#key].strV.compare(#val)) {\
        PMC_ERRORA("Message must " emsg", not '%s'", found[#key].strV.c_str());\
        return false;\
    }
        testOpt(messageType, Management, "be management")
        testOpt(tlvType, MANAGEMENT, "use management tlv")
        // Mandatory
#define testMand(key, emsg)\
    if(!isType(#key, JT_STR)) {\
        PMC_ERROR("message must have " emsg);\
        return false;\
    }
        testMand(actionField, "action field")
        testMand(managementId, "management ID")
        return true;
    }
    bool procMng(mng_vals_e &id, const char *&str) {
        str = found["managementId"].strV.c_str();
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
        if(!convertType(key, JT_INT))\
            return false;\
        val = found[key].intV;\
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
        if(!convertType(key, JT_DOUBLE))\
            return false;\
        val = found[key].fltV;\
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
        auto val = found[key];
        auto type = val.type;
        if(type == JT_INT) {
            auto v = val.intV;
            if(v == Accurate_Unknown || (v <= Accurate_more_10s && v >= 0))
                d = (clockAccuracy_e)v;
            else
                return false;
        } else if(type == JT_DOUBLE) {
            auto v = val.fltV;
            if(fmod(v, 1) == 0 && (v == Accurate_Unknown ||
                    (v <= Accurate_more_10s && v >= 0)))
                d = (clockAccuracy_e)v;
            else
                return false;
        } else if(type != JT_STR)
            return false;
        const char *str = val.strV.c_str();
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
#define procObj(type)\
    bool procValue(const char *key, type &d) {\
        if(!isType(key, JT_OBJ))\
            return false;\
        JSON_POBJ obj = found[key].objV;\
        push();\
        bool ret = procValue(obj, d);\
        pop();\
        return ret;\
    }
    bool procValue(const char *key, TimeInterval_t &d) {
        if(!convertType(key, JT_INT))
            return false;
        d.scaledNanoseconds = found[key].intV;
        return true;
    }
    bool procValue(const char *key, Timestamp_t &d) {
        if(!convertType(key, JT_DOUBLE))
            return false;
        double v = found[key].fltV;
        d.secondsField = trunc(v);
        v -= d.secondsField;
        d.nanosecondsField = trunc(v * NSEC_PER_SEC);
        return true;
    }
    bool procClock(const std::string &val, ClockIdentity_t &d) {
        Binary b;
        if(!b.fromHex(val) || b.size() != d.size())
            return false;
        b.copy(d.v);
        return true;
    }
    bool procValue(const char *key, ClockIdentity_t &d) {
        return isType(key, JT_STR) && procClock(found[key].strV, d);
    }
    bool procValue(const char *key, PortIdentity_t &d) {
        if(!isType(key, JT_OBJ))
            return false;
        allow.clear();
        allow["clockIdentity"] = JT_STR;
        allow["portNumber"] = JT_INT;
        auto o = found[key].objV;
        push();
        bool ret = jloop(o) &&
            PROC_VAL(clockIdentity) &&
            PROC_VAL(portNumber);
        pop();
        return ret;
    }
    bool procValue(JSON_POBJ obj, PortAddress_t &d) {
        allow.clear();
        allow["networkProtocol"] = JT_STR;
        allow["addressField"] = JT_STR;
        return jloop(obj) &&
            PROC_VAL(networkProtocol) &&
            procBinary("addressField", d.addressField, d.addressLength);
    }
    procObj(PortAddress_t)
    bool procValue(const char *key, ClockQuality_t &d) {
        if(!isType(key, JT_OBJ))
            return false;
        allow.clear();
        allow["clockClass"] = JT_INT;
        allow["clockAccuracy"] = JT_STR;
        allow["offsetScaledLogVariance"] = JT_INT;
        auto o = found[key].objV;
        push();
        bool ret = jloop(o) &&
            PROC_VAL(clockClass) &&
            PROC_VAL(clockAccuracy) &&
            PROC_VAL(offsetScaledLogVariance);
        pop();
        return ret;
    }
    bool procValue(const char *key, PTPText_t &d) {
        if(!convertType(key, JT_STR))
            return false;
        d.textField = found[key].strV;
        return true;
    }
    bool procValue(JSON_POBJ obj, FaultRecord_t &d) {
        allow.clear();
        allow["faultRecordLength"] = JT_INT;
        allow["faultTime"] = JT_DOUBLE;
        allow["severityCode"] = JT_STR;
        allow["faultName"] = JT_STR;
        allow["faultValue"] = JT_STR;
        allow["faultDescription"] = JT_STR;
        return jloop(obj) &&
            PROC_VAL(faultRecordLength) &&
            PROC_VAL(faultTime) &&
            PROC_VAL(severityCode) &&
            PROC_VAL(faultName) &&
            PROC_VAL(faultValue) &&
            PROC_VAL(faultDescription);
    }
    procObj(FaultRecord_t)
    bool procValue(JSON_POBJ obj, AcceptableMaster_t &d) {
        allow.clear();
        allow["alternatePriority1"] = JT_INT;
        allow["acceptablePortIdentity"] = JT_OBJ;
        return jloop(obj) &&
            PROC_VAL(alternatePriority1) &&
            PROC_VAL(acceptablePortIdentity);
    }
    procObj(AcceptableMaster_t)
    bool procBinary(const char *key, Binary &d, uint16_t &len) {
        if(!isType(key, JT_STR) || !d.fromId(found[key].strV) ||
            d.size() == 0)
            return false;
        len = d.size();
        return true;
    }
    bool procBinary(const char *key, uint8_t *d, size_t len) {
        Binary b;
        if(!isType(key, JT_STR) || !b.fromId(found[key].strV) ||
            b.size() != len)
            return false;
        b.copy(d);
        return true;
    }
    bool procFlag(const char *key, uint8_t &flags, int mask) {
        if(!convertType(key, JT_BOOL))
            return false;
        if(found[key].blV())
            flags |= mask;
        return true;
    }
    bool procArray(const char *key, std::vector<ClockIdentity_t> &d) {
        if(!isType(key, JT_ARRAY))
            return false;
        auto arr = found[key].objV;
        int size = JA_LEN(arr);
        bool ret = true;
        if(size > 0) {
            for(int i = 0; i < size; i++) {
                ClockIdentity_t rec;
                JSON_POBJ obj = JA_GET(arr, i);
                if(JG_TYPE(obj) != JT_STR) {
                    ret = false;
                    break;
                }
                ret = procClock(JG_STR(obj), rec);
                if(!ret)
                    break;
                d.push_back(rec);
            }
        }
        return ret;
    }
#undef procVector
#define procVector(type)\
    bool procArray(const char *key, std::vector<type> &d) {\
        if(!isType(key, JT_ARRAY))\
            return false;\
        auto arr = found[key].objV;\
        int size = JA_LEN(arr);\
        bool ret = true;\
        if(size > 0) {\
            push();\
            for(int i = 0; i < size; i++) {\
                type rec;\
                JSON_POBJ obj = JA_GET(arr, i);\
                if(JG_TYPE(obj) != JT_OBJ) {\
                    ret = false;\
                    break;\
                }\
                ret = procValue(obj, rec);\
                if(!ret)\
                    break;\
                d.push_back(rec);\
            }\
            pop();\
        }\
        return ret;\
    }
    procVector(PortAddress_t)
    procVector(FaultRecord_t)
    procVector(AcceptableMaster_t)
#undef procType
#undef procVector
};

bool Json2msg::fromJson(const std::string json)
{
    JSON_POBJ jobj = JSON_PARSE(json.c_str());
    if(jobj == nullptr) {
        PMC_ERROR("JSON parse fail");
        return false;
    }
    bool ret = fromJsonObj(jobj);
    JSON_OBJ_FREE(jobj);
    return ret;
}
bool Json2msg::fromJsonObj(const void *jobj)
{
    JsonProcFromJson proc;
    if(!proc.mainProc((JSON_POBJ)jobj))
        return false;
    const char *str = proc.found["actionField"].strV.c_str();
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
#define optProc(key)\
    if(proc.isType(#key, JT_INT)) {\
        m_have[have_##key] = true;\
        m_##key = proc.found[#key].intV;\
    }
    optProc(sequenceId)
    optProc(sdoId)
    optProc(domainNumber)
    optProc(versionPTP)
    optProc(minorVersionPTP)
    optProc(PTPProfileSpecific)
    if(proc.isType("unicastFlag", JT_BOOL)) {
        m_have[have_unicastFlag] = true;
        m_unicastFlag = proc.found["unicastFlag"].blV();
    }
#define portProc(key, var)\
    if(proc.isType(#key, JT_OBJ)) {\
        if(!proc.procValue(#key, m_##var)) {\
            PMC_ERROR("Fail parsing " #key);\
            return false;\
        }\
        m_have[have_##var] = true;\
    }
    portProc(sourcePortIdentity, srcPort)
    portProc(targetPortIdentity, dstPort)
    bool have_data = proc.isType("dataField", JT_OBJ);
    if(m_action == GET) {
        if(have_data) {
            PMC_ERROR("GET use dataField with zero values only, "
                "do not send dataField over JSON");
            return false;
        }
    } else if(Message::isEmpty(m_managementId)) {
        if(have_data) {
            PMC_ERRORA("%s do use dataField", mngStrID);
            return false;
        }
    } else {
        if(!have_data) {
            PMC_ERRORA("%s must use dataField", mngStrID);
            return false;
        }
        JSON_POBJ dataField = proc.found["dataField"].objV;
        if(!proc.jloop(dataField, false))
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
bool Json2msg::fromJsonObj(const void *jobj)
{
    PMC_ERROR("fromJsonObj need JSON-C library support");
    return false;
}
#endif /*PMC_USE_CJSON*/
