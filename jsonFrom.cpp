/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief library to parse JSON
 *
 * The library depends on c-json or fastjason libraries
 * And uses the libptpm library!
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#include <stack>
#include <cmath>
#include <memory>
#include "jsonDef.h"
#include "timeCvrt.h"
#include "err.h"

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
    // If it is not false, it is true :-)
    bool blV() { return intV != false; }
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
                        intV = intV != 0;
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
                        if(!std::isnormal(fltV) || fmod(fltV, 1) != 0)
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
                        if(strcasecmp(str, "true") == 0 ||
                            strcasecmp(str, "enable") == 0 ||
                            strcasecmp(str, "on") == 0)
                            intV = true;
                        else if(strcasecmp(str, "false") == 0 ||
                            strcasecmp(str, "disable") == 0 ||
                            strcasecmp(str, "off") == 0)
                            intV = false;
                        else {
                            auto i = strtoll(str, &end, 0);
                            if(end == str || *end != 0)
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
                // Use JSON false!
                intV = JG_BOOL(obj) != false;
                break;
            case JT_ARRAY:
            case JT_OBJ:
            case JT_NULL:
                break;
        }
    }
};

using namespace ptpmgmt;

struct JsonProcFromJson : public JsonProcFrom {
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
                PTPMGMT_ERRORA("Key '%s' in not allowed", key);
                return false;
            }
            if(val == nullptr) {
                PTPMGMT_ERRORA("Key '%s' do not have value", key);
                return false;
            }
            if(found.count(key) != 0) {
                PTPMGMT_ERRORA("Key '%s' apear twice", key);
                return false;
            }
            JSON_TYPE type = JG_TYPE(val);
            found[key].set(val, type);
            if(withAllow && !found[key].convType(allow[key])) {
                // Ignore dataField with null
                if(strcmp("dataField", key) || type != JT_NULL) {
                    PTPMGMT_ERRORA("Key '%s' use wrong type '%s' instead of '%s'",
                        key, JG_TNAME(type), JG_TNAME(allow[key]));
                    return false;
                }
            }
        }
        return true;
    }
    bool mainProc(const void *_jobj) {
        JSON_POBJ jobj = (JSON_POBJ)_jobj;
        if(!jloop(jobj))
            return false;
        // Optional, if value present verify it
#define testOpt(key, val, emsg)\
    if(isType(#key, JT_STR) && found[#key].strV.compare(#val)) {\
        PTPMGMT_ERRORA("Message must " emsg", not '%s'", found[#key].strV.c_str());\
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
    bool procMng(mng_vals_e &id, const char *&str) {
        bool ret = Message::findMngID(found["managementId"].strV, id);
        if(!ret)
            PTPMGMT_ERRORA("No such managementId '%s'", str);
        return ret;
    }
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
#define procTypeDb(type) \
    bool procValue(const char *key, type &val) {\
        if(!convertType(key, JT_DOUBLE))\
            return false;\
        val = found[key].fltV;\
        return true;\
    }
    procTypeDb(float)
    procTypeDb(double)
    procTypeDb(long double)
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
        auto &val = found[key];
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
        // Check enumerator values
        if(PROC_STR(Unknown)) {
            d = Accurate_Unknown;
            return true;
        }
        for(int i = Accurate_within_1ps; i <= Accurate_more_10s; i++) {
            clockAccuracy_e v = (clockAccuracy_e)i;
            if(strcmp(str, Message::clockAcc2str_c(v)) == 0) {
                d = v;
                return true;
            }
        }
        // Fall to number value
        char *end;
        long a = strtol(str, &end, 0);
        if(end == str || *end != 0 || a < 0 ||
            (a > Accurate_more_10s && a != Accurate_Unknown))
            return false;
        d = (clockAccuracy_e)a;
        return true;
    }
    bool procValue(const char *key, faultRecord_e &d) {
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
    bool procValue(const char *key, timeSource_e &d) {
        if(!isType(key, JT_STR))
            return false;
        return Message::findTimeSrc(found[key].strV, d);
    }
    bool procValue(const char *key, portState_e &d) {
        if(!isType(key, JT_STR))
            return false;
        return Message::findPortState(found[key].strV, d);
    }
    bool procValue(const char *key, linuxptpTimeStamp_e &d) {
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
    bool procValue(const char *key, linuxptpPowerProfileVersion_e &d) {
        GET_STR
        for(int i = IEEE_C37_238_VERSION_NONE; i <= IEEE_C37_238_VERSION_2017;
            i++) {
            linuxptpPowerProfileVersion_e v = (linuxptpPowerProfileVersion_e)i;
            if(strcmp(str, Message::pwr2str_c(v)) == 0) {
                d = v;
                return true;
            }
        }
        return false;
    }
    bool procValue(const char *key, linuxptpUnicastState_e &d) {
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
    bool procValue(JSON_POBJ obj, LinuxptpUnicastMaster_t &d) {
        allow.clear();
        allow["portIdentity"] = JT_OBJ;
        allow["clockQuality"] = JT_OBJ;
        allow["selected"] = JT_BOOL;
        allow["portState"] = JT_STR;
        allow["priority1"] = JT_INT;
        allow["priority2"] = JT_INT;
        allow["portAddress"] = JT_OBJ;
        return jloop(obj) &&
            PROC_VAL(portIdentity) &&
            PROC_VAL(clockQuality) &&
            PROC_VAL(selected) &&
            PROC_VAL(portState) &&
            PROC_VAL(priority1) &&
            PROC_VAL(priority2) &&
            PROC_VAL(portAddress);
    }
    procObj(LinuxptpUnicastMaster_t)
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
    procVector(LinuxptpUnicastMaster_t)
    const std::string &getActionField() {
        return found["actionField"].strV;
    }
    bool getUnicastFlag(bool &unicastFlag) {
        if(!isType("unicastFlag", JT_BOOL))
            return false;
        unicastFlag = found["unicastFlag"].blV();
        return true;
    }
    bool getIntVal(const char *key, int64_t &val) {
        if(!isType(key, JT_INT))
            return false;
        val = found[key].intV;
        return true;
    }
    bool parsePort(const char *key, bool &have, PortIdentity_t &port) {
        if(isType(key, JT_OBJ)) {
            if(!procValue(key, port)) {
                PTPMGMT_ERRORA("Fail parsing %s", key);
                return false; // Error
            }
            have = true;
        } else
            have = false;
        return true; // No error
    }
    bool haveData() { return isType("dataField", JT_OBJ); }
    bool parseData() {
        JSON_POBJ dataField = found["dataField"].objV;
        return jloop(dataField, false);
    }
};

// Library binding functions use C, so we can find them easily with dlsym()
extern "C" {
#define _n(n) ptpm_json_##n
    void *_n(parse)(const char *json) { return JSON_PARSE(json); }
    void _n(free)(void *jobj) { JSON_OBJ_FREE((JSON_POBJ)jobj); }
    JsonProcFrom *_n(alloc_proc)() { return new JsonProcFromJson; }
}
