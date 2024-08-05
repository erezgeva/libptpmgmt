/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Library to parse JSON
 *
 * The library depends on c-json or fastjason libraries
 * And uses the libptpmgmt library!
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#include <stack>
#include <cmath>
#include <memory>
#include "timeCvrt.h"
#include "comp.h"

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
#define PROC_VAL(key) procValue(#key, d.key)
#define PROC_STR(val) (strcmp(str, #val) == 0)
#define EMPTY_STR(str) (str == nullptr || *str == 0)
#define GET_STR\
    if(!isType(key, JT_STR))\
        return false;\
    const char *str = valsMap[key].strV;\
    if(EMPTY_STR(str))\
        return false;

struct JsonVal {
    JSON_TYPE allow = JT_NULL;
    JSON_TYPE type = JT_NULL;
    bool found = false;
    char *strV = nullptr;
    union {
        int64_t intV; // Also boolean
        double fltV;
        JSON_POBJ objV;
    };
    ~JsonVal() { free(strV); }
    bool isAllowed() { return allow != JT_NULL; }
    JSON_TYPE &operator()() { return allow; }
    // If it is not false, it is true :-)
    bool blV() { return intV != false; }
    void setStr(const char *str) {
        free(strV);
        strV = strdup(str);
    }
    bool emptyStr() { return EMPTY_STR(strV); }
    bool convType(JSON_TYPE to) {
        if(!found)
            return false;
        if(type == to)
            return true;
        char *end, strBuf[100];
        double dTmp;
        switch(type) {
            case JT_INT:
                switch(to) {
                    case JT_DOUBLE:
                        dTmp = intV;
                        fltV = dTmp;
                        break;
                    case JT_BOOL:
                        intV = intV != 0;
                        break;
                    case JT_STR:
                        snprintf(strBuf, sizeof strBuf, "%jd", intV);
                        setStr(strBuf);
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
                        snprintf(strBuf, sizeof strBuf, "%lf", fltV);
                        setStr(strBuf);
                        break;
                    default:
                        return false;
                }
                break;
            case JT_BOOL:
                switch(to) {
                    case JT_INT:
                        break;
                    case JT_DOUBLE:
                        dTmp = intV;
                        fltV = dTmp;
                        break;
                    case JT_STR:
                        if(intV == 0)
                            setStr("false");
                        else
                            setStr("true");
                        break;
                    default:
                        return false;
                }
                break;
            case JT_STR:
                if(emptyStr())
                    return false;
                switch(to) {
                    case JT_INT:
                        intV = strtoll(strV, &end, 0);
                        if(end == strV || *end != 0)
                            return false;
                        break;
                    case JT_DOUBLE:
                        fltV = strtod(strV, &end);
                        if(end == strV || *end != 0)
                            return false;
                        break;
                    case JT_BOOL:
                        if(strcasecmp(strV, "true") == 0 ||
                            strcasecmp(strV, "enable") == 0 ||
                            strcasecmp(strV, "on") == 0)
                            intV = true;
                        else if(strcasecmp(strV, "false") == 0 ||
                            strcasecmp(strV, "disable") == 0 ||
                            strcasecmp(strV, "off") == 0)
                            intV = false;
                        else {
                            auto i = strtoll(strV, &end, 0);
                            if(end == strV || *end != 0)
                                return false;
                            intV = i != 0;
                        }
                        break;
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
        found = true;
        switch(type) {
            case JT_INT:
                intV = JG_INT(obj);
                break;
            case JT_DOUBLE:
                fltV = JG_DOUBLE(obj);
                break;
            case JT_STR:
                setStr(JG_STR(obj));
                break;
            case JT_BOOL:
                // Use JSON false!
                intV = JG_BOOL(obj) != false;
                break;
            case JT_ARRAY:
                FALLTHROUGH;
            case JT_OBJ:
                FALLTHROUGH;
            case JT_NULL:
                break;
        }
    }
    bool isType(JSON_TYPE _type) {
        return found && type == _type;
    }
};

__PTPMGMT_NAMESPACE_USE;

struct JsonProcFromJson : public JsonProcFrom {
    mapStackStr<JsonVal> valsMap;

    JsonProcFromJson() {
        // Map main part
        valsMap["sequenceId"]() = JT_INT;
        valsMap["sdoId"]() = JT_INT;
        valsMap["domainNumber"]() = JT_INT;
        valsMap["versionPTP"]() = JT_INT;
        valsMap["minorVersionPTP"]() = JT_INT;
        valsMap["PTPProfileSpecific"]() = JT_INT;
        valsMap["unicastFlag"]() = JT_BOOL;
        valsMap["messageType"]() = JT_STR;
        valsMap["actionField"]() = JT_STR;
        valsMap["tlvType"]() = JT_STR;
        valsMap["managementId"]() = JT_STR;
        valsMap["sourcePortIdentity"]() = JT_OBJ;
        valsMap["targetPortIdentity"]() = JT_OBJ;
        valsMap["dataField"]() = JT_OBJ;
    }
    bool convertType(const char *key, JSON_TYPE to) {
        if(valsMap.have(key))
            return valsMap[key].convType(to);
        return false;
    }
    bool isType(const char *key, JSON_TYPE type) {
        if(valsMap.have(key))
            return valsMap[key].isType(type);
        return false;
    }
    bool jloop(JSON_POBJ jobj, bool withAllow = true) {
        auto end = JI_END(jobj);
        for(auto it = JI_BEGIN(jobj); !JI_EQ(it, end); JI_NEXT(it)) {
            const char *key = JI_NAME(it);
            JSON_POBJ val = JI_VAL(it);
            bool have = valsMap.have(key);
            if(withAllow && have && valsMap[key].isAllowed() != 1) {
                PTPMGMT_ERROR("Key '%s' in not allowed", key);
                return false;
            }
            if(have && valsMap[key].found) {
                PTPMGMT_ERROR("Key '%s' appear twice", key);
                return false;
            }
            JSON_TYPE type = JG_TYPE(val);
            JsonVal &jVal = valsMap[key];
            jVal.set(val, type);
            if(withAllow && !jVal.convType(jVal())) {
                // Ignore dataField with null
                if(strcmp("dataField", key) || type != JT_NULL) {
                    PTPMGMT_ERROR("Key '%s' use wrong type '%s' instead of '%s'",
                        key, JG_TNAME(type), JG_TNAME(jVal()));
                    return false;
                }
            }
        }
        return true;
    }
    bool mainProc(const void *_jobj) override final {
        JSON_POBJ jobj = (JSON_POBJ)_jobj;
        if(!jloop(jobj))
            return false;
        // Optional, if value present verify it
        const char *str;
#define testOpt(key, val, emsg)\
    str = valsMap[#key].strV;\
    if(isType(#key, JT_STR) && (EMPTY_STR(str) || strcmp(str, #val) != 0)) {\
        PTPMGMT_ERROR("Message must " emsg", not '%s'", str);\
        return false;\
    }
        testOpt(messageType, Management, "be management")
        testOpt(tlvType, MANAGEMENT, "use management tlv")
        // Mandatory
#define testMand(key, emsg)\
    if(!isType(#key, JT_STR)) {\
        PTPMGMT_ERROR("message must have " emsg);\
        return false;\
    }
        testMand(actionField, "action field")
        testMand(managementId, "management ID")
        return true;
    }
    bool procMng(mng_vals_e &id) override final {
        if(!valsMap["managementId"].found) {
            PTPMGMT_ERROR("Message do not have a managementId field");
            return false;
        }
        const char *str = valsMap["managementId"].strV;
        if(EMPTY_STR(str)) {
            PTPMGMT_ERROR("Message have an empty managementId field");
            return false;
        }
        bool ret = Message::findMngID(str, id);
        if(!ret)
            PTPMGMT_ERROR("No such managementId '%s'", str);
        return ret;
    }
#define procType(type) \
    bool procValue(const char *key, type &val) override {\
        if(!convertType(key, JT_INT))\
            return false;\
        val = valsMap[key].intV;\
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
#define procTypeDb(type) \
    bool procValue(const char *key, type &val) override {\
        if(!convertType(key, JT_DOUBLE))\
            return false;\
        val = valsMap[key].fltV;\
        return true;\
    }
    procTypeDb(float)
    procTypeDb(double)
    procTypeDb(long double)
    bool procValue(const char *key, networkProtocol_e &d) override {
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
    bool procValue(const char *key, clockAccuracy_e &d) override {
        if(!valsMap.have(key))
            return false;
        JsonVal &jVal = valsMap[key];
        if(!jVal.found)
            return false;
        JSON_TYPE type = jVal.type;
        if(type == JT_INT) {
            int64_t v = jVal.intV;
            if(v == Accurate_Unknown || (v <= Accurate_more_10s && v >= 0))
                d = (clockAccuracy_e)v;
            else
                return false;
        } else if(type == JT_DOUBLE) {
            double v = jVal.fltV;
            if(fmod(v, 1) == 0 && (v == Accurate_Unknown ||
                    (v <= Accurate_more_10s && v >= 0)))
                d = (clockAccuracy_e)v;
            else
                return false;
        } else if(type != JT_STR)
            return false;
        if(jVal.emptyStr())
            return false;
        // Check enumerator values
        const char *str = jVal.strV;
        if(PROC_STR(Unknown)) {
            d = Accurate_Unknown;
            return true;
        }
        for(int i = Accurate_within_1ps; i <= Accurate_more_10s; i++) {
            clockAccuracy_e v = (clockAccuracy_e)i;
            if(strcmp(jVal.strV, Message::clockAcc2str_c(v)) == 0) {
                d = v;
                return true;
            }
        }
        // Fall to number value
        char *end;
        long a = strtol(jVal.strV, &end, 0);
        if(end == jVal.strV || *end != 0 || a < 0 ||
            (a > Accurate_more_10s && a != Accurate_Unknown))
            return false;
        d = (clockAccuracy_e)a;
        return true;
    }
    bool procValue(const char *key, faultRecord_e &d) override {
        GET_STR
        for(int i = F_Emergency; i <= F_Debug; i++) {
            faultRecord_e v = (faultRecord_e)i;
            if(strcmp(str, Message::faultRec2str_c(v)) == 0) {
                d = v;
                return true;
            }
        }
        return false;
    }
    bool procValue(const char *key, timeSource_e &d) override {
        if(!isType(key, JT_STR))
            return false;
        return Message::findTimeSrc(valsMap[key].strV, d);
    }
    bool procValue(const char *key, portState_e &d) override {
        if(!isType(key, JT_STR))
            return false;
        return Message::findPortState(valsMap[key].strV, d);
    }
    bool procValue(const char *key, delayMechanism_e &d) override {
        if(!isType(key, JT_STR))
            return false;
        return Message::findDelayMech(valsMap[key].strV, d);
    }
    bool procValue(const char *key, linuxptpTimeStamp_e &d) override {
        GET_STR
        for(int i = TS_SOFTWARE; i <= TS_P2P1STEP; i++) {
            linuxptpTimeStamp_e v = (linuxptpTimeStamp_e)i;
            if(strcmp(str, Message::ts2str_c(v)) == 0) {
                d = v;
                return true;
            }
        }
        return false;
    }
    bool procValue(const char *key, linuxptpPowerProfileVersion_e &d) override {
        GET_STR
        for(int i = IEEE_C37_238_VERSION_NONE;
            i <= IEEE_C37_238_VERSION_2017; i++) {
            linuxptpPowerProfileVersion_e v = (linuxptpPowerProfileVersion_e)i;
            if(strcmp(str, Message::pwr2str_c(v)) == 0) {
                d = v;
                return true;
            }
        }
        return false;
    }
    bool procValue(const char *key, linuxptpUnicastState_e &d) override {
        GET_STR
        for(int i = UC_WAIT; i <= UC_HAVE_SYDY; i++) {
            linuxptpUnicastState_e v = (linuxptpUnicastState_e)i;
            if(strcmp(str, Message::us2str_c(v)) == 0) {
                d = v;
                return true;
            }
        }
        return false;
    }
#define procObj(type)\
    bool procValue(const char *key, type &d) override {\
        if(!isType(key, JT_OBJ))\
            return false;\
        JSON_POBJ obj = valsMap[key].objV;\
        valsMap.push();\
        bool ret = procValue(obj, d);\
        valsMap.pop();\
        return ret;\
    }
    bool procValue(const char *key, TimeInterval_t &d) override {
        if(!convertType(key, JT_INT))
            return false;
        d.scaledNanoseconds = valsMap[key].intV;
        return true;
    }
    bool procValue(const char *key, Timestamp_t &d) override {
        if(!convertType(key, JT_DOUBLE))
            return false;
        double v = valsMap[key].fltV;
        d.secondsField = trunc(v);
        v -= d.secondsField;
        d.nanosecondsField = trunc(v * NSEC_PER_SEC);
        return true;
    }
    bool procClock(const char *str, ClockIdentity_t &d) {
        Binary b;
        if(EMPTY_STR(str) || !b.fromHex(str) || b.size() != d.size())
            return false;
        b.copy(d.v);
        return true;
    }
    bool procValue(const char *key, ClockIdentity_t &d) override {
        return isType(key, JT_STR) && procClock(valsMap[key].strV, d);
    }
    bool procValue(const char *key, PortIdentity_t &d) override {
        if(!isType(key, JT_OBJ))
            return false;
        JSON_POBJ o = valsMap[key].objV;
        valsMap.push();
        valsMap["clockIdentity"]() = JT_STR;
        valsMap["portNumber"]() = JT_INT;
        bool ret = jloop(o) &&
            PROC_VAL(clockIdentity) &&
            PROC_VAL(portNumber);
        valsMap.pop();
        return ret;
    }
    bool procValue(JSON_POBJ obj, PortAddress_t &d) {
        valsMap.push();
        valsMap["networkProtocol"]() = JT_STR;
        valsMap["addressField"]() = JT_STR;
        bool ret = jloop(obj) &&
            PROC_VAL(networkProtocol) &&
            procBinary("addressField", d.addressField, d.addressLength);
        valsMap.pop();
        return ret;
    }
    procObj(PortAddress_t)
    bool procValue(const char *key, ClockQuality_t &d) override {
        if(!isType(key, JT_OBJ))
            return false;
        JSON_POBJ o = valsMap[key].objV;
        valsMap.push();
        valsMap["clockClass"]() = JT_INT;
        valsMap["clockAccuracy"]() = JT_STR;
        valsMap["offsetScaledLogVariance"]() = JT_INT;
        bool ret = jloop(o) &&
            PROC_VAL(clockClass) &&
            PROC_VAL(clockAccuracy) &&
            PROC_VAL(offsetScaledLogVariance);
        valsMap.pop();
        return ret;
    }
    bool procValue(const char *key, PTPText_t &d) override {
        if(!convertType(key, JT_STR))
            return false;
        d.textField = valsMap[key].strV;
        return true;
    }
    bool procValue(JSON_POBJ obj, FaultRecord_t &d) {
        valsMap.push();
        valsMap["faultRecordLength"]() = JT_INT;
        valsMap["faultTime"]() = JT_DOUBLE;
        valsMap["severityCode"]() = JT_STR;
        valsMap["faultName"]() = JT_STR;
        valsMap["faultValue"]() = JT_STR;
        valsMap["faultDescription"]() = JT_STR;
        bool ret = jloop(obj) &&
            PROC_VAL(faultRecordLength) &&
            PROC_VAL(faultTime) &&
            PROC_VAL(severityCode) &&
            PROC_VAL(faultName) &&
            PROC_VAL(faultValue) &&
            PROC_VAL(faultDescription);
        valsMap.pop();
        return ret;
    }
    procObj(FaultRecord_t)
    bool procValue(JSON_POBJ obj, AcceptableMaster_t &d) {
        valsMap.push();
        valsMap["alternatePriority1"]() = JT_INT;
        valsMap["acceptablePortIdentity"]() = JT_OBJ;
        bool ret = jloop(obj) &&
            PROC_VAL(alternatePriority1) &&
            PROC_VAL(acceptablePortIdentity);
        valsMap.pop();
        return ret;
    }
    procObj(AcceptableMaster_t)
    bool procValue(JSON_POBJ obj, LinuxptpUnicastMaster_t &d) {
        valsMap.push();
        valsMap["portIdentity"]() = JT_OBJ;
        valsMap["clockQuality"]() = JT_OBJ;
        valsMap["selected"]() = JT_INT;
        valsMap["portState"]() = JT_STR;
        valsMap["priority1"]() = JT_INT;
        valsMap["priority2"]() = JT_INT;
        valsMap["portAddress"]() = JT_OBJ;
        bool ret = jloop(obj) &&
            PROC_VAL(portIdentity) &&
            PROC_VAL(clockQuality) &&
            PROC_VAL(selected) &&
            PROC_VAL(portState) &&
            PROC_VAL(priority1) &&
            PROC_VAL(priority2) &&
            PROC_VAL(portAddress);
        valsMap.pop();
        return ret;
    }
    procObj(LinuxptpUnicastMaster_t)
    bool procBinary(const char *key, Binary &d, uint16_t &len) override {
        if(!isType(key, JT_STR) || !d.fromId(valsMap[key].strV) ||
            d.size() == 0)
            return false;
        len = d.size();
        return true;
    }
    bool procBinary(const char *key, uint8_t *d, size_t len) override {
        Binary b;
        if(!isType(key, JT_STR) || !b.fromId(valsMap[key].strV) ||
            b.size() != len)
            return false;
        b.copy(d);
        return true;
    }
    bool procFlag(const char *key, uint8_t &flags, int mask) override {
        if(!convertType(key, JT_BOOL))
            return false;
        if(valsMap[key].blV())
            flags |= mask;
        return true;
    }
    void procZeroFlag(uint8_t &flags) override {
        flags = 0;
    }
    bool procArray(const char *key, vector<ClockIdentity_t> &d) override {
        if(!isType(key, JT_ARRAY))
            return false;
        JSON_POBJ arr = valsMap[key].objV;
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
#define procVector(type)\
    bool procArray(const char *key, vector<type> &d) override {\
        if(!isType(key, JT_ARRAY))\
            return false;\
        JSON_POBJ arr = valsMap[key].objV;\
        int size = JA_LEN(arr);\
        bool ret = true;\
        if(size > 0) {\
            valsMap.push();\
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
            valsMap.pop();\
        }\
        return ret;\
    }
    procVector(PortAddress_t)
    procVector(FaultRecord_t)
    procVector(AcceptableMaster_t)
    procVector(LinuxptpUnicastMaster_t)
    const char *getActionField() override final {
        return valsMap["actionField"].strV;
    }
    bool getUnicastFlag(bool &unicastFlag) override final {
        if(!isType("unicastFlag", JT_BOOL))
            return false;
        unicastFlag = valsMap["unicastFlag"].blV();
        return true;
    }
    bool getIntVal(const char *key, int64_t &val) override final {
        if(!isType(key, JT_INT))
            return false;
        val = valsMap[key].intV;
        return true;
    }
    bool parsePort(const char *key, bool &have,
        PortIdentity_t &port) override final {
        if(isType(key, JT_OBJ)) {
            if(!procValue(key, port)) {
                PTPMGMT_ERROR("Fail parsing %s", key);
                return false; // Error
            }
            have = true;
        } else
            have = false;
        return true; // No error
    }
    bool haveData() override final
    { return isType("dataField", JT_OBJ); }
    bool parseData() override final {
        JSON_POBJ dataField = valsMap["dataField"].objV;
        return jloop(dataField, false);
    }
};

// Library binding functions use C, so we can find them easily with dlsym()
Json_lib me = { [](const char *json) { return JSON_PARSE(json); },
[](void *jobj) { JSON_OBJ_FREE((JSON_POBJ)jobj); },
[]() { return new JsonProcFromJson; }, JLIB_NAME
         };
extern "C" {
    Json_lib *ptpm_json = &me; // For static library link
    Json_lib *ptpm_json_fech() { return &me; } // For shread library link
}
