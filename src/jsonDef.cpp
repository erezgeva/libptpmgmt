/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Convert management messages to json
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#include <cmath>
#include "jsonParser.h"
#include "comp.h"
#include "timeCvrt.h"
#include "c/json.h"

__PTPMGMT_NAMESPACE_BEGIN

#define PROC_VAL(key)   procValue(#key, d.key)
#define EMPTY_KEY\
    if(key == nullptr || *key == 0)\
        return false;
#define GET_STR\
    EMPTY_KEY;\
    jsonValue *jval = m_obj->getVal(key);\
    if(jval == nullptr || jval->getType() != t_string)\
        return false;\
    const string &str = jval->getStr();
#define GET_NUM(type, func)\
    EMPTY_KEY;\
    type num;\
    jsonValue *jval = m_obj->getVal(key);\
    if(jval == nullptr || !jval->func(num))\
        return false;
#define GET_NUM_CNV(type, func)\
    EMPTY_KEY;\
    type num;\
    jsonValue *jval = m_obj->getVal(key);\
    if(jval == nullptr || !jval->func(num, true))\
        return false;
#define GET_OBJ(key)\
    jsonObject *obj = m_obj->getObj(key);\
    if(obj == nullptr)\
        return false;\
    jsonObject *keep = m_obj;\
    m_obj = obj;
#define RET_OBJ\
    m_obj = keep;\
    return ret;

struct JsonProcFrom : public JsonProc {
    jsonObject *m_obj = nullptr;
#define procUint(type) \
    bool procValue(const char *key, type &val) override {\
        GET_NUM_CNV(uint64_t, getUint64);\
        val = num;\
        return true;\
    }
    procUint(uint8_t)
    procUint(uint16_t)
    procUint(uint32_t)
    procUint(uint64_t)
#define procInt(type) \
    bool procValue(const char *key, type &val) override {\
        GET_NUM(int64_t, getInt64);\
        val = num;\
        return true;\
    }
    procInt(int8_t)
    procInt(int16_t)
    procInt(int32_t)
    procInt(int64_t)
#define procFloat(type) \
    bool procValue(const char *key, type &val) override {\
        GET_NUM(long double, getFloat);\
        val = num;\
        return true;\
    }
    procFloat(float)
    procFloat(double)
    procFloat(long double)
    bool procValue(const char *key, networkProtocol_e &d) override {
        GET_STR;
        for(int i = UDP_IPv4; i <= PROFINET; i++) {
            networkProtocol_e v = (networkProtocol_e)i;
            if(str == Message::netProt2str_c(v)) {
                d = v;
                return true;
            }
        }
        return false;
    }
    bool procValue(const char *key, clockAccuracy_e &d) override {
        jsonValue *jval = m_obj->getVal(key);
        if(jval == nullptr)
            return false;
        uint64_t u64;
        const string &str = jval->getStr();
        switch(jval->getType()) {
            case t_string:
                if(str.empty())
                    return false;
                if(str == "Unknown") {
                    d = Accurate_Unknown;
                    return true;
                }
                // Check enumerator values
                for(int i = Accurate_within_1ps; i <= Accurate_more_10s; i++) {
                    clockAccuracy_e v = (clockAccuracy_e)i;
                    if(str == Message::clockAcc2str_c(v)) {
                        d = v;
                        return true;
                    }
                }
                FALLTHROUGH;
            case t_number:
                if(jval->getUint64(u64, true)) {
                    if(u64 == Accurate_Unknown || u64 <= Accurate_more_10s) {
                        d = (clockAccuracy_e)u64;
                        return true;
                    }
                }
                break;
            default:
                break;
        }
        return false;
    }
    bool procValue(const char *key, faultRecord_e &d) override {
        GET_STR;
        for(int i = F_Emergency; i <= F_Debug; i++) {
            faultRecord_e v = (faultRecord_e)i;
            if(str == Message::faultRec2str_c(v)) {
                d = v;
                return true;
            }
        }
        return false;
    }
    bool procValue(const char *key, timeSource_e &d) override {
        GET_STR;
        return Message::findTimeSrc(str, d);
    }
    bool procValue(const char *key, portState_e &d) override {
        GET_STR;
        return Message::findPortState(str, d);
    }
    bool procValue(const char *key, delayMechanism_e &d) override {
        GET_STR;
        return Message::findDelayMech(str, d);
    }
    bool procValue(const char *key, linuxptpTimeStamp_e &d) override {
        GET_STR;
        for(int i = TS_SOFTWARE; i <= TS_P2P1STEP; i++) {
            linuxptpTimeStamp_e v = (linuxptpTimeStamp_e)i;
            if(str == Message::ts2str_c(v)) {
                d = v;
                return true;
            }
        }
        return false;
    }
    bool procValue(const char *key,
        linuxptpPowerProfileVersion_e &d) override {
        GET_STR;
        for(int i = IEEE_C37_238_VERSION_NONE;
            i <= IEEE_C37_238_VERSION_2017; i++) {
            linuxptpPowerProfileVersion_e v = (linuxptpPowerProfileVersion_e)i;
            if(str == Message::pwr2str_c(v)) {
                d = v;
                return true;
            }
        }
        return false;
    }
    bool procValue(const char *key, linuxptpUnicastState_e &d) override {
        GET_STR;
        for(int i = UC_WAIT; i <= UC_HAVE_SYDY; i++) {
            linuxptpUnicastState_e v = (linuxptpUnicastState_e)i;
            if(str == Message::us2str_c(v)) {
                d = v;
                return true;
            }
        }
        return false;
    }
    bool procValue(const char *key, TimeInterval_t &d) override {
        GET_NUM(int64_t, getInt64);
        d.scaledNanoseconds = num;
        return true;
    }
    bool procValue(const char *key, Timestamp_t &d) override {
        EMPTY_KEY;
        jsonValue *jval = m_obj->getVal(key);
        if(jval == nullptr)
            return false;
        int64_t integer;
        uint64_t fraction;
        long double num;
        if(jval->getFrac(integer, fraction, 9)) {
            if(integer > 0) {
                d.secondsField = integer;
                d.nanosecondsField = fraction;
                return true;
            }
        } else if(jval->getFloat(num)) {
            d.secondsField = trunc(num);
            num -= d.secondsField;
            d.nanosecondsField = trunc(num * NSEC_PER_SEC);
            return true;
        }
        return false;
    }
    bool procValue(const char *key, ClockIdentity_t &d) override {
        GET_STR
        Binary b;
        if(!b.fromHex(str) || b.size() != d.size())
            return false;
        b.copy(d.v);
        return true;
    }
    bool procValue(const char *key, PortIdentity_t &d) override {
        GET_OBJ(key);
        bool ret =
            PROC_VAL(clockIdentity) &&
            PROC_VAL(portNumber);
        RET_OBJ;
    }

    bool procValue(PortAddress_t &d) {
        return
            PROC_VAL(networkProtocol) &&
            procBinary("addressField", d.addressField, d.addressLength);
    }
    bool procValue(const char *key, PortAddress_t &d) override {
        GET_OBJ(key);
        bool ret = procValue(d);
        RET_OBJ;
    }
    bool procValue(const char *key, ClockQuality_t &d) override {
        GET_OBJ(key);
        bool ret =
            PROC_VAL(clockClass) &&
            PROC_VAL(clockAccuracy) &&
            PROC_VAL(offsetScaledLogVariance);
        RET_OBJ;
    }
    bool procValue(const char *key, PTPText_t &d) override {
        GET_STR;
        d.textField = str;
        return true;
    }
    bool procValue(FaultRecord_t &d) {
        return
            PROC_VAL(faultRecordLength) &&
            PROC_VAL(faultTime) &&
            PROC_VAL(severityCode) &&
            PROC_VAL(faultName) &&
            PROC_VAL(faultValue) &&
            PROC_VAL(faultDescription);
    }
    bool procValue(const char *key, FaultRecord_t &d) override {
        GET_OBJ(key);
        bool ret = procValue(d);
        RET_OBJ;
    }
    bool procValue(AcceptableMaster_t &d) {
        return
            PROC_VAL(alternatePriority1) &&
            PROC_VAL(acceptablePortIdentity);
    }
    bool procValue(const char *key, AcceptableMaster_t &d) override {
        GET_OBJ(key);
        bool ret = procValue(d);
        RET_OBJ;
    }
    bool procValue(LinuxptpUnicastMaster_t &d) {
        return
            PROC_VAL(portIdentity) &&
            PROC_VAL(clockQuality) &&
            PROC_VAL(selected) &&
            PROC_VAL(portState) &&
            PROC_VAL(priority1) &&
            PROC_VAL(priority2) &&
            PROC_VAL(portAddress);
    }
    bool procValue(const char *key, LinuxptpUnicastMaster_t &d) override {
        GET_OBJ(key);
        bool ret = procValue(d);
        RET_OBJ;
    }
    bool procBinary(const char *key, Binary &d, uint16_t &len) override {
        GET_STR;
        if(!d.fromId(str) || d.size() == 0)
            return false;
        len = d.size();
        return true;
    }
    bool procBinary(const char *key, uint8_t *d, size_t len) override {
        GET_STR;
        Binary b;
        if(!b.fromId(str) || b.size() != len)
            return false;
        b.copy(d);
        return true;
    }

    bool procFlag(const char *key, uint8_t &flags, int mask) override {
        EMPTY_KEY;
        jsonValue *jval = m_obj->getVal(key);
        if(jval == nullptr || jval->getType() != t_boolean)
            return false;
        if(jval->getBool())
            flags |= mask;
        return true;
    }
    void procZeroFlag(uint8_t &flags) override {
        flags = 0;
    }
    bool procArray(const char *key, vector<ClockIdentity_t> &d) override {
        jsonArray *arr = m_obj->getArr(key);
        if(arr == nullptr)
            return false;
        d.reserve(d.size() + arr->size());
        for(size_t i = 0; i < arr->size(); i++) {
            jsonValue *jval = arr->getVal(i);
            if(jval == nullptr || jval->getType() != t_string)
                return false;
            const string &str = jval->getStr();
            Binary b;
            ClockIdentity_t rec;
            if(!b.fromHex(str) || b.size() != rec.size())
                return false;
            b.copy(rec.v);
            d.push_back(rec);
        }
        return true;
    }
#define procVector(type)\
    bool procArray(const char *key, vector<type> &d) override {\
        jsonArray *arr = m_obj->getArr(key);\
        if(arr == nullptr)\
            return false;\
        jsonObject *keep = m_obj;\
        bool ret = true;\
        d.reserve(d.size() + arr->size());\
        for(size_t i = 0; i < arr->size(); i++) {\
            m_obj = arr->getObj(i);\
            if(m_obj == nullptr) {\
                ret = false;\
                break;\
            }\
            type rec;\
            if(!procValue(rec)) {\
                ret = false;\
                break;\
            }\
            d.push_back(std::move(rec));\
        }\
        m_obj = keep;\
        return ret;\
    }
    procVector(PortAddress_t)
    procVector(FaultRecord_t)
    procVector(AcceptableMaster_t)
    procVector(LinuxptpUnicastMaster_t)
};
// keep for ABI backward compatibility
Json2msg::~Json2msg() {}
// Obsolete as we use internal JSON parser
#ifdef PIC // Shared library code
const char *libName = "libptpmgmt_jsonc.so";
#define LIB_NAME libName
#define LIB true
#define SET_NAME\
    if(str.find("fa") == str.npos) libName = "libptpmgmt_jsonc.so";\
    else libName = "libptpmgmt_fastjson.so"
#else // PIC
#define LIB_NAME ""
#define LIB false
#define SET_NAME
#endif // PIC
bool Json2msg::selectLib(const string &str) { SET_NAME; return LIB; }
const char *Json2msg::loadLibrary() { return LIB_NAME; }
bool Json2msg::isLibShared() { return LIB; }
bool Json2msg::fromJsonObj(const void *) { return false; }
/* ************************************ */
bool Json2msg::fromJson(const string &json)
{
    jsonMain jmain;
    if(!jmain.parseBuffer(json)) {
        PTPMGMT_ERROR("Parsing failed");
        return false;
    }
    jsonObject *mobj = jmain.getObj();
    if(mobj == nullptr) {
        PTPMGMT_ERROR("Wrong JSON, must be an object not %s",
            jsonType2str(jmain.getType()));
        return false;
    }
    PTPMGMT_ERROR_CLR;
    jsonValue *val;
#define testOpt(key, result, emsg)\
    val = mobj->getVal(#key);\
    if(mobj->count(#key) > 0 && (mobj->getType(#key) != t_string ||\
            val->getStr() != #result)) {\
        PTPMGMT_ERROR("Message must " emsg);\
        return false; }
    testOpt(messageType, Management, "be management");
    testOpt(tlvType, MANAGEMENT, "use management tlv");
    // Mandatory
    string str;
#define testMand(key, emsg)\
    val = mobj->getVal(#key);\
    if(val == nullptr || val->getType() != t_string) {\
        PTPMGMT_ERROR("message must have " emsg);\
        return false; }\
    str = val->getStr();\
    if(str.empty()) {\
        PTPMGMT_ERROR("Message do not have " emsg);\
        return false; }
    testMand(actionField, "an action field");
    if(str == "GET")
        m_action = GET;
    else if(str == "SET")
        m_action = SET;
    else if(str == "COMMAND")
        m_action = COMMAND;
    else {
        PTPMGMT_ERROR("Message have wrong action field '%s'", str.c_str());
        return false;
    }
    testMand(managementId, "management ID");
    if(!Message::findMngID(str, m_managementId)) {
        PTPMGMT_ERROR("No such managementId '%s'", str.c_str());
        return false;
    }
    // Optional
    int64_t i64;
#define optProc(key)\
    if(mobj->count(#key) > 0) {\
        val = mobj->getVal(#key);\
        if(val == nullptr || !val->getInt64(i64)) {\
            PTPMGMT_ERROR("Wrong value for " #key);\
            return false;\
        }\
        m_have[have_##key] = true;\
        m_##key = i64;\
    }
    optProc(sequenceId);
    optProc(sdoId);
    optProc(domainNumber);
    optProc(versionPTP);
    optProc(minorVersionPTP);
    optProc(PTPProfileSpecific);
    if(mobj->count("unicastFlag") > 0) {
        val = mobj->getVal("unicastFlag");
        if(val == nullptr || val->getType() != t_boolean) {
            PTPMGMT_ERROR("Wrong value for unicastFlag");
            return false;
        }
        m_have[have_unicastFlag] = true;
        m_unicastFlag = val->getBool();
    }
    JsonProcFrom pproc;
    pproc.m_obj = mobj;
#define portProc(key, var)\
    if(mobj->count(#key) > 0) {\
        if(pproc.procValue(#key, m_##var))\
            m_have[have_##var] = true;\
        else {\
            PTPMGMT_ERROR("Wrong value for " #key);\
            return false;\
        }\
    }
    portProc(sourcePortIdentity, srcPort)
    portProc(targetPortIdentity, dstPort)
    bool have_data = false;
    if(mobj->count("dataField") > 0) {
        switch(mobj->getType("dataField")) {
            case t_object:
                have_data = true;
            case t_null:
                break;
            default:
                PTPMGMT_ERROR("Wrong value for dataField");
                return false;
        }
    }
    if(m_action == GET) {
        if(have_data) {
            PTPMGMT_ERROR("GET use dataField with zero values only, "
                "do not send dataField over JSON");
            return false;
        }
    } else if(Message::isEmpty(m_managementId)) {
        if(have_data) {
            PTPMGMT_ERROR("%s do use dataField",
                Message::mng2str_c(m_managementId));
            return false;
        }
    } else {
        if(!have_data) {
            PTPMGMT_ERROR("%s must use dataField",
                Message::mng2str_c(m_managementId));
            return false;
        }
        pproc.m_obj = mobj->getObj("dataField");
        const BaseMngTlv *data = nullptr;
        if(!pproc.procData(m_managementId, data)) {
            if(data != nullptr)
                delete data;
            if(!Error::isError())
                PTPMGMT_ERROR("Parsing of %s dataField failed",
                    Message::mng2str_c(m_managementId));
            return false;
        }
        m_tlvData.reset(const_cast<BaseMngTlv *>(data));
    }
    return true;
}

__PTPMGMT_NAMESPACE_END

__PTPMGMT_NAMESPACE_USE;

extern "C" {
#define C2CPP_ret(ret, func, def)\
    static ret ptpmgmt_json_##func(const_ptpmgmt_json j) {\
        if(j != nullptr && j->_this != nullptr)\
            return ((Json2msg*)j->_this)->func();\
        return def; }
#define C2CPP_cret(func, cast, def)\
    static ptpmgmt_##cast ptpmgmt_json_##func(const_ptpmgmt_json j) {\
        if(j != nullptr && j->_this != nullptr)\
            return (ptpmgmt_##cast)((Json2msg*)j->_this)->func();\
        return PTPMGMT_##def; }
#define C2CPP_port(n)\
    static const ptpmgmt_PortIdentity_t *ptpmgmt_json_##n##Port(ptpmgmt_json j) {\
        if(j != nullptr && j->_this != nullptr) {\
            if(j->_##n##Port == nullptr)\
                j->_##n##Port = (ptpmgmt_PortIdentity_t *)\
                    malloc(sizeof(ptpmgmt_PortIdentity_t));\
            if(j->_##n##Port != nullptr) {\
                const PortIdentity_t & p = ((Json2msg*)j->_this)->n##Port();\
                j->_##n##Port->portNumber = p.portNumber;\
                memcpy(j->_##n##Port->clockIdentity.v, p.clockIdentity.v,\
                    ClockIdentity_t::size());\
                return j->_##n##Port;\
            }\
        } return nullptr; }
#define C_SWP(n, m) free(j->n); j->n = m
    static void ptpmgmt_json_free(ptpmgmt_json j)
    {
        if(j != nullptr) {
            if(j->_this != nullptr) {
                delete(Json2msg *)j->_this;
                j->_this = nullptr;
            }
            C_SWP(_srcPort, nullptr);
            C_SWP(_dstPort, nullptr);
            C_SWP(data, nullptr);
            C_SWP(dataTbl, nullptr);
        }
    }
    // Obsolete as we use internal JSON parser
    bool ptpmgmt_json_selectLib(const char *libName)
    {
        if(libName != nullptr)
            return Json2msg::selectLib(libName);
        return false;
    }
    const char *ptpmgmt_json_loadLibrary() { return LIB_NAME; }
    bool ptpmgmt_json_isLibShared() { return LIB; }
    static bool ptpmgmt_json_fromJsonObj(ptpmgmt_json, const void *)
    { return false; }
    /* ************************************ */
    static bool ptpmgmt_json_fromJson(ptpmgmt_json j, const char *arg)
    {
        if(j != nullptr && j->_this != nullptr && arg != nullptr)
            return ((Json2msg *)j->_this)->fromJson(arg);
        return false;
    }
    C2CPP_cret(managementId, mng_vals_e, NULL_PTP_MANAGEMENT)
    static const void *ptpmgmt_json_dataField(ptpmgmt_json j)
    {
        if(j != nullptr && j->_this != nullptr) {
            Json2msg &me = *(Json2msg *)j->_this;
            const BaseMngTlv *t = me.dataField();
            if(t != nullptr) {
                void *x = nullptr;
                void *ret = cpp2cMngTlv(me.managementId(), t, x);
                if(ret != nullptr) {
                    C_SWP(data, ret);
                    C_SWP(dataTbl, x);
                    return ret;
                }
            }
        }
        return nullptr;
    }
    C2CPP_cret(actionField, actionField_e, GET)
    C2CPP_ret(bool, isUnicast, false)
    C2CPP_ret(bool, haveIsUnicast, false)
    C2CPP_ret(uint8_t, PTPProfileSpecific, 0)
    C2CPP_ret(bool, havePTPProfileSpecific, false)
    C2CPP_ret(uint8_t, domainNumber, 0)
    C2CPP_ret(bool, haveDomainNumber, false)
    C2CPP_ret(uint8_t, versionPTP, 0)
    C2CPP_ret(bool, haveVersionPTP, false)
    C2CPP_ret(uint8_t, minorVersionPTP, 0)
    C2CPP_ret(bool, haveMinorVersionPTP, false)
    C2CPP_ret(uint16_t, sequenceId, 0)
    C2CPP_ret(bool, haveSequenceId, false)
    C2CPP_ret(uint32_t, sdoId, 0)
    C2CPP_ret(bool, haveSdoId, false)
    C2CPP_port(src)
    C2CPP_ret(bool, haveSrcPort, false);
    C2CPP_port(dst);
    C2CPP_ret(bool, haveDstPort, false);
    static bool ptpmgmt_json_setAction(const_ptpmgmt_json j, ptpmgmt_msg m)
    {
        if(j != nullptr && j->_this != nullptr &&
            m != nullptr && m->_this != nullptr)
            return ((Json2msg *)j->_this)->setAction(*(Message *)m->_this);
        return false;
    }
    ptpmgmt_json ptpmgmt_json_alloc()
    {
        ptpmgmt_json j = (ptpmgmt_json)malloc(sizeof(ptpmgmt_json_t));
        if(j == nullptr)
            return nullptr;
        j->_this = (void *)(new Json2msg);
        if(j->_this == nullptr) {
            free(j);
            return nullptr;
        }
        j->_srcPort = nullptr;
        j->_dstPort = nullptr;
        j->data = nullptr;
        j->dataTbl = nullptr;
#define C_ASGN(n) j->n = ptpmgmt_json_##n
        C_ASGN(free);
        // Obsolete as we use internal JSON parser
        C_ASGN(selectLib);
        C_ASGN(loadLibrary);
        C_ASGN(isLibShared);
        C_ASGN(fromJsonObj);
        /* ************************************ */
        C_ASGN(fromJson);
        C_ASGN(managementId);
        C_ASGN(dataField);
        C_ASGN(actionField);
        C_ASGN(isUnicast);
        C_ASGN(haveIsUnicast);
        C_ASGN(PTPProfileSpecific);
        C_ASGN(havePTPProfileSpecific);
        C_ASGN(domainNumber);
        C_ASGN(haveDomainNumber);
        C_ASGN(versionPTP);
        C_ASGN(haveVersionPTP);
        C_ASGN(minorVersionPTP);
        C_ASGN(haveMinorVersionPTP);
        C_ASGN(sequenceId);
        C_ASGN(haveSequenceId);
        C_ASGN(sdoId);
        C_ASGN(haveSdoId);
        C_ASGN(srcPort);
        C_ASGN(haveSrcPort);
        C_ASGN(dstPort);
        C_ASGN(haveDstPort);
        C_ASGN(setAction);
        return j;
    }
}
