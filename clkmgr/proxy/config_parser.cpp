/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Proxy JSON parser
 *
 * @author Goh Wei Sheng <wei.sheng.goh@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include "proxy/config_parser.hpp"
#include "common/print.hpp"

#include <cstring>

__CLKMGR_NAMESPACE_USE;
using namespace std;

JsonConfigParser &JsonConfigParser::getInstance()
{
    static JsonConfigParser instance;
    return instance;
}
void JsonConfigParser::print_config()
{
    for(const auto &row : timeBaseCfgs) {
        const TimeBaseCfg &config = row.base;
        PrintInfo("[Index: " + to_string(config.timeBaseIndex) +
            "] Name: " + config.timeBaseName);
        if(!row.udsAddrPtp4l.empty()) {
            PrintInfo("[Index: " + to_string(config.timeBaseIndex) +
                "] Interface Name: " + config.interfaceName);
            PrintInfo("[Index: " + to_string(config.timeBaseIndex) +
                "] UDS Address PTP4L: " + row.udsAddrPtp4l);
            PrintInfo("[Index: " + to_string(config.timeBaseIndex) +
                "] Domain Number: " + to_string(config.domainNumber));
            PrintInfo("[Index: " + to_string(config.timeBaseIndex) +
                "] Transport Specific: " + to_string(config.transportSpecific));
        }
        if(!row.udsAddrChrony.empty())
            PrintInfo("[Index: " + to_string(config.timeBaseIndex) +
                "] UDS Address Chrony: " + row.udsAddrChrony);
    }
}

bool JsonConfigParser::get_Int_Val(jsonObject *obj, const string &key,
    const uint8_t *defaultVal, uint8_t &res)
{
    int64_t i;
    jsonValue *val = obj->getVal(key);
    if(val == nullptr) {
        if(defaultVal != nullptr) {
            res = *defaultVal;
            return true;
        }
        PrintError("Invalid " + key);
        return false;
    }
    if(!val->getInt64(i) || val->getType() != t_number ||
        i  < 0 || i > UINT8_MAX) {
        PrintError("Invalid " + key);
        return false;
    }
    res = static_cast<uint8_t>(i);
    return true;
}

bool JsonConfigParser::get_Str_Val(jsonObject *obj, const string &key,
    const char *defaultVal, char *res)
{
    string r;
    if(get_Str_Val(obj, key, defaultVal, r)) {
        if(r.size() >= STRING_SIZE_MAX) {
            PrintError("Invalid " + key);
            return false;
        }
        strncpy(res, r.c_str(), STRING_SIZE_MAX - 1);
        return true;
    }
    return false;
}
bool JsonConfigParser::get_Str_Val(jsonObject *obj, const string &key,
    const char *defaultVal, string &res)
{
    jsonValue *val = obj->getVal(key);
    if(val == nullptr) {
        if(defaultVal != nullptr) {
            res = defaultVal;
            return true;
        }
        PrintError("Invalid " + key);
        return false;
    }
    res = val->getStr();
    if(val->getType() != t_string || res.empty()) {
        PrintError("Invalid " + key);
        return false;
    }
    return true;
}

bool JsonConfigParser::process_json(const string &file)
{
    jsonMain main;
    jsonArray *timeBaseArray;
    size_t currentIndex = 1;
    const uint8_t defaultDomainPtp4l = 0;
    const uint8_t defaultTransportPtp4l = 0;
    const char *defaultUdsPtp4l = "/var/run/ptp/ptp4l";
    const char *defaultUdsChrony = "/var/run/chrony/chronyd.sock";
    timeBaseCfgs.clear();
    if(!main.parseFile(file, true))
        return false;
    timeBaseArray = main.getObj()->getArr("timeBases");
    if(!timeBaseArray)
        return false;
    for(auto *it : *timeBaseArray) {
        TimeBaseCfgFull row = {};
        TimeBaseCfg &config = row.base;
        jsonObject *timeBaseObj = it->getObj();
        if(timeBaseObj == nullptr)
            return false;
        jsonObject *ptp4lObj = timeBaseObj->getObj("ptp4l");
        jsonObject *chronyObj = timeBaseObj->getObj("chrony");
        if(!get_Str_Val(timeBaseObj, "timeBaseName", nullptr,
                config.timeBaseName))
            return false;
        if(ptp4lObj != nullptr) {
            if(!get_Str_Val(ptp4lObj, "interfaceName", nullptr,
                    config.interfaceName) ||
                !get_Str_Val(ptp4lObj, "udsAddr", defaultUdsPtp4l,
                    row.udsAddrPtp4l) ||
                !get_Int_Val(ptp4lObj, "domainNumber", &defaultDomainPtp4l,
                    config.domainNumber) ||
                !get_Int_Val(ptp4lObj, "transportSpecific",
                    &defaultTransportPtp4l, config.transportSpecific))
                return false;
            config.havePtp = true;
        }
        if(chronyObj != nullptr) {
            if(!get_Str_Val(chronyObj, "udsAddr", defaultUdsChrony,
                    row.udsAddrChrony))
                return false;
            config.haveSys = true;
        }
        config.timeBaseIndex = currentIndex;
        currentIndex++;
        timeBaseCfgs.push_back(std::move(row));
    }
    print_config();
    return true;
}
