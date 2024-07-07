/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Convert management messages to json
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#include <stack>
#include <cmath>
#include <mutex>
#include "comp.h"
#include "c/json.h"

__PTPMGMT_NAMESPACE_BEGIN

#define JS(n) static inline bool proc_##n(JsonProc &proc, n##_t &d)
#define PROC_VAL(name) proc.procValue(#name, d.name)
#define PROC_BIN(name, len) proc.procBinary(#name, d.name, d.len)
#define PROC_OCT(name) proc.procBinary(#name, d.name, sizeof d.name)
#define PROC_ZFLG proc.procZeroFlag(d.flags)
#define PROC_FLG(name, mask) proc.procFlag(#name, d.flags, mask)
#define PROC_FLG1(name) proc.procFlag(#name, d.flags, 1)
#define PROC_FLGB(name, flag, bit) proc.procFlag(#name, d.flag, (1 << bit))
#define PROC_ARR(name) proc.procArray(#name, d.name)
#define PROC_FLD(dir, name, macro)\
    proc.procValue(#dir "_" #name, d.dir##MsgType[STAT_##macro])

#define DIV_EVENT(n)\
    static auto n##_L = SUBSCRIBE_EVENTS_NP_t::div_event(n)
DIV_EVENT(NOTIFY_PORT_STATE);
DIV_EVENT(NOTIFY_TIME_SYNC);
DIV_EVENT(NOTIFY_PARENT_DATA_SET);
DIV_EVENT(NOTIFY_CMLDS);
#define PROC_EVENT_FLG(name)\
    proc.procFlag(#name, d.bitmask[name##_L.quot], name##_L.rem)

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
    PROC_ZFLG;
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
    PROC_ZFLG;
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
    PROC_ZFLG;
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
    PROC_ZFLG;
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
    PROC_ZFLG;
    return
        PROC_VAL(currentUtcOffset) &&
        PROC_FLG(leap61, F_LI_61) &&
        PROC_FLG(leap59, F_LI_59) &&
        PROC_FLG(currentUtcOffsetValid, F_UTCV);
}
JS(TRACEABILITY_PROPERTIES)
{
    PROC_ZFLG;
    return
        PROC_FLG(timeTraceable, F_TTRA) &&
        PROC_FLG(frequencyTraceable, F_FTRA);
}
JS(TIMESCALE_PROPERTIES)
{
    PROC_ZFLG;
    return
        PROC_FLG(ptpTimescale, F_PTP) &&
        PROC_VAL(timeSource);
}
JS(UNICAST_NEGOTIATION_ENABLE)
{
    PROC_ZFLG;
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
    PROC_ZFLG;
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
    PROC_ZFLG;
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
    PROC_ZFLG;
    return
        PROC_FLG1(transmitAlternateMulticastSync) &&
        PROC_VAL(logAlternateMulticastSyncInterval) &&
        PROC_VAL(numberOfAlternateMasters);
}
JS(ALTERNATE_TIME_OFFSET_ENABLE)
{
    PROC_ZFLG;
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
    PROC_ZFLG;
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
    PROC_ZFLG;
    return
        PROC_FLG1(externalPortConfiguration);
}
JS(MASTER_ONLY)
{
    PROC_ZFLG;
    return
        PROC_FLG1(masterOnly);
}
JS(HOLDOVER_UPGRADE_ENABLE)
{
    PROC_ZFLG;
    return
        PROC_FLG1(holdoverUpgradeDS);
}
JS(EXT_PORT_CONFIG_PORT_DATA_SET)
{
    PROC_ZFLG;
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
    PROC_ZFLG;
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
        PROC_EVENT_FLG(NOTIFY_PORT_STATE) &&
        PROC_EVENT_FLG(NOTIFY_TIME_SYNC) &&
        PROC_EVENT_FLG(NOTIFY_PARENT_DATA_SET) &&
        PROC_EVENT_FLG(NOTIFY_CMLDS);
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
JS(POWER_PROFILE_SETTINGS_NP)
{
    return
        PROC_VAL(version) &&
        PROC_VAL(grandmasterID) &&
        PROC_VAL(grandmasterTimeInaccuracy) &&
        PROC_VAL(networkTimeInaccuracy) &&
        PROC_VAL(totalTimeInaccuracy);
}
JS(CMLDS_INFO_NP)
{
    return
        PROC_VAL(meanLinkDelay) &&
        PROC_VAL(scaledNeighborRateRatio) &&
        PROC_VAL(as_capable);
}

/* Finish Process functions of JsonProc */
#undef JS

#define procProperty(name)\
    procValue(#name, val.name)
#define procType(type) \
    void procValue(const char *name, const type &val) {\
        startName(name, " ");\
        m_result += to_string(val);\
    }\
    bool procValue(const char *name, type &val) override {\
        startName(name, " ");\
        m_result += to_string(val);\
        return true;\
    }
#define procTypeEnum(type, func)\
    bool procValue(const char *name, type val) {\
        procString(name, Message::func(val));\
        return true;\
    }
#define procTypeEnumR(type, func)\
    bool procValue(const char *name, type &val) override {\
        procString(name, Message::func(val));\
        return true;\
    }
#define procVector(type) \
    bool procArray(const char *name, vector<type> &val) {\
        procArray(name);\
        for(type &rec : val) {\
            close();\
            procValue(rec);\
        }\
        closeArray();\
        return true;\
    }
#define procVectorA(type) \
    bool procArray(const char *name, vector<type> &val) override {\
        procArray(name);\
        for(type &rec : val) {\
            close();\
            procValue(rec);\
        }\
        closeArray();\
        return true;\
    }

bool JsonProc::procData(mng_vals_e managementId, const BaseMngTlv *&data)
{
#define _ptpmCaseUF(n) case n: {\
            n##_t *d;\
            if(data == nullptr) {\
                d = new n##_t;\
                data = d;\
            } else\
                d = dynamic_cast<n##_t *>(const_cast<BaseMngTlv *>(data));\
            return d == nullptr ? false : proc_##n(*this, *d); }
    switch(managementId) {
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
#include "ids.h"
        default:
            return false;
    }
}

struct JsonProcToJson : public JsonProc {
    string m_result;
    stack<bool> m_first_vals;
    int m_base_indent;
    bool m_first;
    JsonProcToJson(const Message &msg, int indent);
    JsonProcToJson(mng_vals_e managementId, const BaseMngTlv *data, int indent);
    bool data2json(mng_vals_e managementId, const BaseMngTlv *data,
        bool header = true);
    bool smpte2json(SMPTE_ORGANIZATION_EXTENSION_t *data);
    void sig2json(tlvType_e tlvType, const BaseSigTlv *tlv);
    void close() {
        if(!m_first)
            m_result += ',';
        m_result += '\n';
        m_first = false;
    }
    void indent() {
        m_result += string(m_first_vals.size() * 2 + m_base_indent, ' ');
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
    void procString(const char *name, const string &val) {
        startName(name, " \"");
        m_result += val;
        m_result += '"';
    }
    void procValue(const char *name, const string &val) {
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
        // Linuxptp bool value
        bool selected = val.selected != 0;
        procBool("selected", selected);
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
    procTypeEnum(managementErrorId_e, errId2str_c)
    procTypeEnum(SMPTEmasterLockingStatus_e, smpteLck2str_c)
    procTypeEnumR(networkProtocol_e, netProt2str_c)
    bool procValue(const char *name, mng_vals_e val) {
        if(val != SMPTE_MNG_ID)
            procString(name, Message::mng2str_c(val));
        else
            procString(name, "SMPTE_MNG_ID");
        return true;
    }
    bool procValue(const char *name, clockAccuracy_e &val) override {
        if(val < Accurate_within_1ps) {
            char buf[10];
            snprintf(buf, sizeof buf, "0x%x", val);
            procString(name, buf);
        } else
            procString(name, Message::clockAcc2str_c(val));
        return true;
    }
    procTypeEnumR(faultRecord_e, faultRec2str_c)
    procTypeEnumR(timeSource_e, timeSrc2str_c)
    procTypeEnumR(portState_e, portState2str_c)
    procTypeEnumR(delayMechanism_e, delayMech2str_c)
    procTypeEnumR(linuxptpTimeStamp_e, ts2str_c)
    procTypeEnumR(linuxptpPowerProfileVersion_e, pwr2str_c)
    procTypeEnumR(linuxptpUnicastState_e, us2str_c)
    bool procValue(const char *name, TimeInterval_t &val) override {
        procValue(name, val.scaledNanoseconds);
        return true;
    }
    bool procValue(const char *name, Timestamp_t &val) override {
        procValue(name, val.string());
        return true;
    }
    bool procValue(const char *name, ClockIdentity_t &val) override {
        procString(name, val.string());
        return true;
    }
    bool procValue(const char *name, PortIdentity_t &val) override {
        procObject(name);
        procString("clockIdentity", val.clockIdentity.string());
        procProperty(portNumber);
        closeObject();
        return true;
    }
    bool procValue(const char *name, PortAddress_t &val) override {
        startName(name, "\n");
        procValue(val);
        return true;
    }
    bool procValue(const char *name, ClockQuality_t &val) override {
        procObject(name);
        procProperty(clockClass);
        procProperty(clockAccuracy);
        procProperty(offsetScaledLogVariance);
        closeObject();
        return true;
    }
    bool procValue(const char *name, PTPText_t &val) override {
        procString(name, val.textField);
        return true;
    }
    bool procValue(const char *name, FaultRecord_t &val) override {
        startName(name, "\n");
        procValue(val);
        return true;
    }
    bool procValue(const char *name, AcceptableMaster_t &val) override {
        startName(name, "\n");
        procValue(val);
        return true;
    }
    bool procValue(const char *name, LinuxptpUnicastMaster_t &val) override {
        startName(name, "\n");
        procValue(val);
        return true;
    }
    bool procBinary(const char *name, Binary &val, uint16_t &) override {
        procString(name, val.toId());
        return true;
    }
    bool procBinary(const char *name, uint8_t *val, size_t len) override {
        procString(name, Binary::bufToId(val, len));
        return true;
    }
    bool procFlag(const char *name, uint8_t &flags, int mask) override {
        procBool(name, (flags & mask) > 0);
        return true;
    }
    void procZeroFlag(uint8_t &flags) override {}
    procVectorA(ClockIdentity_t)
    procVectorA(PortAddress_t)
    procVectorA(FaultRecord_t)
    procVectorA(AcceptableMaster_t)
    procVectorA(LinuxptpUnicastMaster_t)
    procVector(SLAVE_RX_SYNC_TIMING_DATA_rec_t)
    procVector(SLAVE_RX_SYNC_COMPUTED_DATA_rec_t)
    procVector(SLAVE_TX_EVENT_TIMESTAMPS_rec_t)
    procVector(SLAVE_DELAY_TIMING_DATA_NP_rec_t)
};

bool JsonProcToJson::data2json(mng_vals_e managementId, const BaseMngTlv *data,
    bool header)
{
    if(data != nullptr) {
        if(header)
            procObject("dataField");
        else // header can be false only if data is NOT null!
            startObject();
        procData(managementId, data);
        closeObject();
    }
    return true;
}

/* Signalling functions */
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
JS(SMPTE_ORGANIZATION_EXTENSION)
{
    PROC_OCT(organizationId);
    PROC_OCT(organizationSubType);
    PROC_VAL(defaultSystemFrameRate_numerator);
    PROC_VAL(defaultSystemFrameRate_denominator);
    PROC_VAL(masterLockingStatus);
    PROC_VAL(timeAddressFlags);
    PROC_VAL(currentLocalOffset);
    PROC_VAL(jumpSeconds);
    PROC_VAL(timeOfNextJump);
    PROC_VAL(timeOfNextJam);
    PROC_VAL(timeOfPreviousJam);
    PROC_VAL(previousJamLocalOffset);
    PROC_VAL(daylightSaving);
    PROC_VAL(leapSecondJump);
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
            FALLTHROUGH;
        case ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE:
            FALLTHROUGH;
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

bool JsonProcToJson::smpte2json(SMPTE_ORGANIZATION_EXTENSION_t *data)
{
    if(data != nullptr)
        parse_SMPTE_ORGANIZATION_EXTENSION(*this, *data);
    return true;
}

JsonProcToJson::JsonProcToJson(const Message &msg, int indent) :
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
                case ORGANIZATION_EXTENSION:
                    if(managementId == SMPTE_MNG_ID)
                        smpte2json((SMPTE_ORGANIZATION_EXTENSION_t *)msg.getData());
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

JsonProcToJson::JsonProcToJson(mng_vals_e managementId, const BaseMngTlv *tlv,
    int indent) : m_base_indent(indent), m_first(false)
{
    data2json(managementId, tlv, false);
}

string msg2json(const Message &msg, int indent)
{
    JsonProcToJson proc(msg, indent);
    return proc.m_result;
}

string tlv2json(mng_vals_e managementId, const BaseMngTlv *tlv, int indent)
{
    if(tlv == nullptr || Message::isEmpty(managementId))
        return "{}"; // empty JSON
    JsonProcToJson proc(managementId, tlv, indent);
    return proc.m_result;
}

__PTPMGMT_NAMESPACE_END

__PTPMGMT_NAMESPACE_USE;

extern "C" {
    // C interfaces
    char *ptpmgmt_json_msg2json(const_ptpmgmt_msg m, int indent)
    {
        if(m != nullptr && m->_this != nullptr) {
            string ret = msg2json(*(Message *)m->_this, indent);
            if(!ret.empty())
                return strdup(ret.c_str());
        }
        return nullptr;
    }
    char *ptpmgmt_json_tlv2json(ptpmgmt_mng_vals_e managementId, const void *tlv,
        int indent)
    {
        if(tlv != nullptr) {
            mng_vals_e id = (mng_vals_e)managementId;
            BaseMngTlv *t = c2cppMngTlv(id, tlv);
            if(t != nullptr) {
                string ret = tlv2json(id, t, indent);
                delete t;
                if(!ret.empty())
                    return strdup(ret.c_str());
            }
        }
        return nullptr;
    }
}
