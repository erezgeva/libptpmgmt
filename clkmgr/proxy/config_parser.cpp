/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Proxy JSON parser
 *
 * @author Goh Wei Sheng <wei.sheng.goh@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */
#include <cstring>

#include "common/print.hpp"
#include "proxy/config_parser.hpp"

__CLKMGR_NAMESPACE_USE;

JsonConfigParser &JsonConfigParser::getInstance()
{
    static JsonConfigParser instance;
    return instance;
}
void JsonConfigParser::print_config()
{
    for(const auto &row : timeBaseCfgs) {
        const TimeBaseCfg &config = row.base;
        PrintInfo(" [Index: " + std::to_string(config.timeBaseIndex) +
            "] Name: " + std::string(config.timeBaseName));
        if(!row.udsAddrPtp4l.empty()) {
            PrintInfo(" [Index: " + std::to_string(config.timeBaseIndex) +
                "] Interface Name: " + std::string(config.interfaceName));
            PrintInfo(" [Index: " + std::to_string(config.timeBaseIndex) +
                "] UDS Address PTP4L: " + row.udsAddrPtp4l);
            PrintInfo(" [Index: " + std::to_string(config.timeBaseIndex) +
                "] Domain Number: " + std::to_string(config.domainNumber));
            PrintInfo(" [Index: " + std::to_string(config.timeBaseIndex) +
                "] Transport Specific: " +
                std::to_string(config.transportSpecific));
        }
        if(!row.udsAddrChrony.empty())
            PrintInfo(" [Index: " + std::to_string(config.timeBaseIndex) +
                "] UDS Address Chrony: " + row.udsAddrChrony);
    }
}

bool JsonConfigParser::get_Int_Val(jsonObject *obj, const std::string &key,
    uint8_t &res)
{
    int64_t i;
    if(!obj->getVal(key))
        return true;
    if(!obj->getVal(key)->getInt64(i) || obj->getType(key) != t_number ||
        i  < 0 || i > UINT8_MAX) {
        PrintError("Invalid " + key);
        return false;
    }
    res = static_cast<uint8_t>(i);
    return true;
}

bool JsonConfigParser::get_Str_Val(jsonObject *obj, const std::string &key,
    char *res)
{
    std::string r;
    if(get_Str_Val(obj, key, r)) {
        if(r.size() >= STRING_SIZE_MAX) {
            PrintError("Invalid " + key);
            return false;
        }
        strncpy(res, r.c_str(), STRING_SIZE_MAX - 1);
    }
    return false;
}
bool JsonConfigParser::get_Str_Val(jsonObject *obj, const std::string &key,
    std::string &res)
{
    jsonValue *val = obj->getVal(key);
    if(val == nullptr)
        return true;
    res = val->getStr();
    if(val->getType() != t_string || res.empty()) {
        PrintError("Invalid " + key);
        return false;
    }
    return true;
}

bool JsonConfigParser::process_json(const char *file)
{
    jsonMain main;
    jsonArray *timeBaseArray;
    int currentIndex = 1;
    if(!main.parseFile(file, true))
        return false;
    timeBaseArray = main.getObj()->getArr("timeBases");
    if(!timeBaseArray)
        return false;
    for(size_t idx = 0; idx < timeBaseArray->size(); ++idx) {
        TimeBaseCfgFull row = {};
        TimeBaseCfg &config = row.base;
        jsonObject *timeBaseObj = timeBaseArray->getObj(idx);
        if(timeBaseObj == nullptr)
            return false;
        jsonObject *ptp4lObj = timeBaseObj->getObj("ptp4l");
        jsonObject *chronyObj = timeBaseObj->getObj("chrony");
        if(!get_Str_Val(timeBaseObj, "timeBaseName", config.timeBaseName))
            return false;
        if(ptp4lObj != nullptr) {
            row.udsAddrPtp4l = "/var/run/ptp4l";
            config.domainNumber = 0;
            config.transportSpecific = 1;
            if(!get_Str_Val(ptp4lObj, "interfaceName", config.interfaceName) ||
                !get_Str_Val(ptp4lObj, "udsAddr", row.udsAddrPtp4l) ||
                !get_Int_Val(ptp4lObj, "domainNumber", config.domainNumber) ||
                !get_Int_Val(ptp4lObj, "transportSpecific",
                    config.transportSpecific))
                return false;
        }
        if(chronyObj != nullptr) {
            row.udsAddrChrony = "/var/run/chrony/chronyd.sock";
            if(!get_Str_Val(chronyObj, "udsAddr", row.udsAddrChrony))
                return false;
        }
        config.timeBaseIndex = currentIndex;
        currentIndex++;
        timeBaseCfgs.push_back(row);
    }
    print_config();
    return true;
}
