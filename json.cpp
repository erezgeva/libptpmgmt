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

struct JsonProc {
    void data2json(mng_vals_e id, const BaseMngTlv *data);
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
    procType(networkProtocol_e);
    procType(clockAccuracy_e);
    procType(faultRecord_e);
    procType(timeSource_e);
    procType(portState_e);
    procType(linuxptpTimeStamp_e);
    procType(TimeInterval_t);
    procType(Timestamp_t);
    procType(ClockIdentity_t);
    procType(PortIdentity_t);
    procType(PortAddress_t);
    procType(ClockQuality_t);
    procType(PTPText_t)
    procType(FaultRecord_t);
    procType(AcceptableMaster_t);
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
    std::string result;
    std::stack<bool> m_first_vals;
    bool m_first;
    JsonProcToJson() : m_first(0) {}
    void close() {
        if(!m_first)
            result += ',';
        result += '\n';
        m_first = false;
    }
    void indent() {
        for(size_t i = 0; i < m_first_vals.size(); i++)
            result += "  ";
    }
    void startName(const char *name, const char *end) {
        close();
        indent();
        result += '"';
        result += name;
        result += "\" :";
        result += end;
    }
    void startObject() {
        indent();
        result += "{";
        m_first_vals.push(m_first);
        m_first = true;
    }
    void closeObject() {
        result += '\n';
        m_first = m_first_vals.top();
        m_first_vals.pop();
        indent();
        result += "}";
    }
    void procObject(const char *name) {
        startName(name, "\n");
        startObject();
    }
    void startArray() {
        indent();
        result += "[";
        m_first_vals.push(m_first);
        m_first = true;
    }
    void closeArray() {
        result += '\n';
        m_first = m_first_vals.top();
        m_first_vals.pop();
        indent();
        result += "]";
    }
    void procArray(const char *name) {
        startName(name, "\n");
        startArray();
    }
    void procString(const char *name, const std::string &val) {
        startName(name, " \"");
        result += val;
        result += '"';
    }
    void procValue(const char *name, const std::string &val) {
        startName(name, " ");
        result += val;
    }
    void procValue(const char *name, const Binary &val) {
        procString(name, val.toId());
    }
    void procBool(const char *name, const bool &val) {
        startName(name, " ");
        result += val ? "true" : "false";
    }
#define procProperty(name)\
    procValue(#name, val.name)
    void procValue(ClockIdentity_t &val) {
        close();
        indent();
        result += '"';
        result += val.string();
        result += '"';
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
#define procType(type) \
    void procValue(const char *name, const type &val) {\
        startName(name, " ");\
        result += std::to_string(val);\
    }\
    bool procValue(const char *name, type &val) {\
        startName(name, " ");\
        result += std::to_string(val);\
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
    bool procValue(const char *name, type &val) {\
        procString(name, Message::func(val));\
        return true;\
    }
#define procTypeEnumVal(type, vtype)\
    bool procValue(const char *name, clockAccuracy_e &val) {\
        uint8_t v = val;\
        return procValue(name, v);\
    }
    procTypeEnum(networkProtocol_e, netProt2str_c)
    procTypeEnumVal(clockAccuracy_e, uint8_t)
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
        closeArray();\
        return true;\
    }
    procVector(ClockIdentity_t)
    procVector(PortAddress_t)
    procVector(FaultRecord_t)
    procVector(AcceptableMaster_t)
#undef procType
#undef procVector
};

#define JS(id) \
    static inline bool proc_##id(JsonProc &proc, const BaseMngTlv *data)\
    {\
        id##_t &d = *(id##_t *)data;
#define JEND\
    }
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
    JEND;
}
JS(USER_DESCRIPTION)
{
    return
        PROC_VAL(userDescription);
    JEND;
}
JS(INITIALIZE)
{
    return
        PROC_VAL(initializationKey);
    JEND;
}
JS(FAULT_LOG)
{
    return
        PROC_VAL(numberOfFaultRecords) &&
        PROC_ARR(faultRecords);
    JEND;
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
    JEND;
}
JS(CURRENT_DATA_SET)
{
    return
        PROC_VAL(stepsRemoved) &&
        PROC_VAL(offsetFromMaster) &&
        PROC_VAL(meanPathDelay);
    JEND;
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
    JEND;
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
    JEND;
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
    JEND;
}
JS(PRIORITY1)
{
    return
        PROC_VAL(priority1);
    JEND;
}
JS(PRIORITY2)
{
    return
        PROC_VAL(priority2);
    JEND;
}
JS(DOMAIN)
{
    return
        PROC_VAL(domainNumber);
    JEND;
}
JS(SLAVE_ONLY)
{
    return
        PROC_FLG1(slaveOnly);
    JEND;
}
JS(LOG_ANNOUNCE_INTERVAL)
{
    return
        PROC_VAL(logAnnounceInterval);
    JEND;
}
JS(ANNOUNCE_RECEIPT_TIMEOUT)
{
    return
        PROC_VAL(announceReceiptTimeout);
    JEND;
}
JS(LOG_SYNC_INTERVAL)
{
    return
        PROC_VAL(logSyncInterval);
    JEND;
}
JS(VERSION_NUMBER)
{
    return
        PROC_VAL(versionNumber);
    JEND;
}
JS(TIME)
{
    return
        PROC_VAL(currentTime);
    JEND;
}
JS(CLOCK_ACCURACY)
{
    return
        PROC_VAL(clockAccuracy);
    JEND;
}
JS(UTC_PROPERTIES)
{
    return
        PROC_VAL(currentUtcOffset) &&
        PROC_FLG(leap61, F_LI_61) &&
        PROC_FLG(leap59, F_LI_59) &&
        PROC_FLG(currentUtcOffsetValid, F_UTCV);
    JEND;
}
JS(TRACEABILITY_PROPERTIES)
{
    return
        PROC_FLG(timeTraceable, F_TTRA) &&
        PROC_FLG(frequencyTraceable, F_FTRA);
    JEND;
}
JS(TIMESCALE_PROPERTIES)
{
    return
        PROC_FLG(ptpTimescale, F_PTP) &&
        PROC_VAL(timeSource);
    JEND;
}
JS(UNICAST_NEGOTIATION_ENABLE)
{
    return
        PROC_FLG1(unicastNegotiationPortDS);
    JEND;
}
JS(PATH_TRACE_LIST)
{
    return
        PROC_ARR(pathSequence);
    JEND;
}
JS(PATH_TRACE_ENABLE)
{
    return
        PROC_FLG1(pathTraceDS);
    JEND;
}
JS(GRANDMASTER_CLUSTER_TABLE)
{
    return
        PROC_VAL(logQueryInterval) &&
        PROC_VAL(actualTableSize) &&
        PROC_ARR(PortAddress);
    JEND;
}
JS(UNICAST_MASTER_TABLE)
{
    return
        PROC_VAL(logQueryInterval) &&
        PROC_VAL(actualTableSize) &&
        PROC_ARR(PortAddress);
    JEND;
}
JS(UNICAST_MASTER_MAX_TABLE_SIZE)
{
    return
        PROC_VAL(maxTableSize);
    JEND;
}
JS(ACCEPTABLE_MASTER_TABLE)
{
    return
        PROC_VAL(actualTableSize) &&
        PROC_ARR(list);
    JEND;
}
JS(ACCEPTABLE_MASTER_TABLE_ENABLED)
{
    return
        PROC_FLG1(acceptableMasterPortDS);
    JEND;
}
JS(ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
{
    return
        PROC_VAL(maxTableSize);
    JEND;
}
JS(ALTERNATE_MASTER)
{
    return
        PROC_FLG1(transmitAlternateMulticastSync) &&
        PROC_VAL(logAlternateMulticastSyncInterval) &&
        PROC_VAL(numberOfAlternateMasters);
    JEND;
}
JS(ALTERNATE_TIME_OFFSET_ENABLE)
{
    return
        PROC_VAL(keyField) &&
        PROC_FLG1(alternateTimescaleOffsetsDS);
    JEND;
}
JS(ALTERNATE_TIME_OFFSET_NAME)
{
    return
        PROC_VAL(keyField) &&
        PROC_VAL(displayName);
    JEND;
}
JS(ALTERNATE_TIME_OFFSET_MAX_KEY)
{
    return
        PROC_VAL(maxKey);
    JEND;
}
JS(ALTERNATE_TIME_OFFSET_PROPERTIES)
{
    return
        PROC_VAL(keyField) &&
        PROC_VAL(currentOffset) &&
        PROC_VAL(jumpSeconds) &&
        PROC_VAL(timeOfNextJump);
    JEND;
}
JS(TRANSPARENT_CLOCK_PORT_DATA_SET)
{
    return
        PROC_VAL(portIdentity) &&
        PROC_FLG1(transparentClockPortDS) &&
        PROC_VAL(logMinPdelayReqInterval) &&
        PROC_VAL(peerMeanPathDelay);
    JEND;
}
JS(LOG_MIN_PDELAY_REQ_INTERVAL)
{
    return
        PROC_VAL(logMinPdelayReqInterval);
    JEND;
}
JS(TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
{
    return
        PROC_VAL(clockIdentity) &&
        PROC_VAL(numberPorts) &&
        PROC_VAL(delayMechanism) &&
        PROC_VAL(primaryDomain);
    JEND;
}
JS(PRIMARY_DOMAIN)
{
    return
        PROC_VAL(primaryDomain);
    JEND;
}
JS(DELAY_MECHANISM)
{
    return
        PROC_VAL(delayMechanism);
    JEND;
}
JS(EXTERNAL_PORT_CONFIGURATION_ENABLED)
{
    return
        PROC_FLG1(externalPortConfiguration);
    JEND;
}
JS(MASTER_ONLY)
{
    return
        PROC_FLG1(masterOnly);
    JEND;
}
JS(HOLDOVER_UPGRADE_ENABLE)
{
    return
        PROC_FLG1(holdoverUpgradeDS);
    JEND;
}
JS(EXT_PORT_CONFIG_PORT_DATA_SET)
{
    return
        PROC_FLG1(acceptableMasterPortDS) &&
        PROC_VAL(desiredState);
    JEND;
}
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
    JEND;
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
    JEND;
}
JS(PORT_DATA_SET_NP)
{
    return
        PROC_VAL(neighborPropDelayThresh) &&
        PROC_VAL(asCapable);
    JEND;
}
JS(SUBSCRIBE_EVENTS_NP)
{
    return
        PROC_VAL(duration);
    //  uint8_t bitmask[EVENT_BITMASK_CNT];
    JEND;
}
JS(PORT_PROPERTIES_NP)
{
    return
        PROC_VAL(portIdentity) &&
        PROC_VAL(portState) &&
        PROC_VAL(timestamping) &&
        PROC_VAL(interface);
    JEND;
}
JS(PORT_STATS_NP)
{
    return
        PROC_VAL(portIdentity);
    //  uint64_t rxMsgType[MAX_MESSAGE_TYPES];
    //  uint64_t txMsgType[MAX_MESSAGE_TYPES];
    JEND;
}
JS(SYNCHRONIZATION_UNCERTAIN_NP)
{
    return
        PROC_VAL(val);
    JEND;
}

void JsonProc::data2json(mng_vals_e id, const BaseMngTlv *data)
{
#define caseUF(n) case n: proc_##n(*this, data); break;
#define A(n, v, sc, a, sz, f) case##f(n)
    switch(id) {
#include "ids.h"
        default:
            break;
    }
}

std::string msg2json(Message &msg)
{
    JsonProcToJson proc;
    proc.startObject();
    proc.procValue("sequenceId", msg.getSequence());
    proc.procValue("sdoId", msg.getSdoId());
    proc.procValue("domainNumber", msg.getDomainNumber());
    proc.procValue("versionPTP", msg.getVersionPTP());
    proc.procValue("minorVersionPTP", msg.getMinorVersionPTP());
    msgType_e type = msg.getType();
    const char *typeStr;
    switch(type) {
        case Management:
            typeStr = "Management";
            break;
        case Signaling:
            typeStr = "Signaling";
        default:
            typeStr = "Unsupported";
    }
    proc.procString("messageType", typeStr);
    proc.procValue("sourcePortIdentity", msg.getPeer());
    tlvType_e mngType;
    mng_vals_e id;
    const BaseMngTlv *data;
    switch(type) {
        case Management:
            mngType = msg.getMngType();
            id = msg.getTlvId();
            proc.procString("actionField", msg.act2str_c(msg.getReplyAction()));
            proc.procString("tlvType", msg.tlv2str_c(mngType));
            proc.procString("managementId", msg.mng2str_c(id));
            switch(mngType) {
                case MANAGEMENT:
                    data = msg.getData();
                    if(data != nullptr) {
                        proc.procObject("dataField");
                        proc.data2json(id, data);
                        proc.closeObject();
                    }
                    break;
                case MANAGEMENT_ERROR_STATUS:
                    proc.procString("managementErrorId",
                        msg.errId2str_c(msg.getErrId()));
                    proc.procString("displayData", msg.getErrDisplay());
                    break;
                default:
                    break;
            }
            break;
        case Signaling:
            // TODO
            break;
        default:
            break;
    }
    proc.closeObject();
    return proc.result;
}
