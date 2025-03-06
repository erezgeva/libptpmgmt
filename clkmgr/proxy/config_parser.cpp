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
    for(const auto &config : timeBaseCfgs) {
        PrintInfo(" [Index: " + std::to_string(config.timeBaseIndex) +
            "] Name: " + std::string(config.timeBaseName));
        if(strlen(config.udsAddrPtp4l) > 0) {
            PrintInfo(" [Index: " + std::to_string(config.timeBaseIndex) +
                "] Interface Name: " + std::string(config.interfaceName));
            PrintInfo(" [Index: " + std::to_string(config.timeBaseIndex) +
                "] UDS Address PTP4L: " + std::string(config.udsAddrPtp4l));
            PrintInfo(" [Index: " + std::to_string(config.timeBaseIndex) +
                "] Domain Number: " + std::to_string(config.domainNumber));
            PrintInfo(" [Index: " + std::to_string(config.timeBaseIndex) +
                "] Transport Specific: " +
                std::to_string(config.transportSpecific));
        }
        if(strlen(config.udsAddrChrony) > 0)
            PrintInfo(" [Index: " + std::to_string(config.timeBaseIndex) +
                "] UDS Address Chrony: " + std::string(config.udsAddrChrony));
    }
}

bool JsonConfigParser::get_Int_Val(jsonObject *obj, const char *key,
    uint8_t *res)
{
    int64_t i;
    if(!obj->getVal(key))
        return true;
    if(!obj->getVal(key)->getInt64(i) || obj->getType(key) != t_number ||
        i  < 0 || i > UINT8_MAX) {
        PrintError("Invalid " + std::string(key));
        return false;
    }
    *res = static_cast<uint8_t>(i);
    return true;
}

bool JsonConfigParser::get_Str_Val(jsonObject *obj, const char *key,
    char *res)
{
    if(!obj->getVal(key))
        return true;
    const char *val = obj->getVal(key)->getCStr();
    if(obj->getType(key) != t_string || strlen(val) <= 0 ||
        strlen(val) >= STRING_SIZE_MAX) {
        PrintError("Invalid " + std::string(key));
        return false;
    }
    strncpy(res, val, STRING_SIZE_MAX - 1);
    return true;
}

bool JsonConfigParser::process_json(const char *file)
{
    jsonMain main;
    jsonArray *timeBaseArray;
    jsonObject *timeBaseObj, *ptp4lObj, *chronyObj;
    int currentIndex = 1;
    if(!main.parseFile(file, true))
        return false;
    timeBaseArray = main.getObj()->getArr("timeBases");
    if(!timeBaseArray)
        return false;
    for(size_t idx = 0; idx < timeBaseArray->size(); ++idx) {
        TimeBaseCfg config = {};
        timeBaseObj = timeBaseArray->getObj(idx);
        if(!timeBaseObj)
            return false;
        ptp4lObj = timeBaseObj->getObj("ptp4l");
        chronyObj = timeBaseObj->getObj("chrony");
        if(!get_Str_Val(timeBaseObj, "timeBaseName", config.timeBaseName))
            return false;
        if(ptp4lObj) {
            strncpy(config.udsAddrPtp4l, "/var/run/ptp4l",
                sizeof(config.udsAddrPtp4l) - 1);
            config.domainNumber = 0;
            config.transportSpecific = 1;
            if(!get_Str_Val(ptp4lObj, "interfaceName", config.interfaceName))
                return false;
            if(!get_Str_Val(ptp4lObj, "udsAddr", config.udsAddrPtp4l))
                return false;
            if(!get_Int_Val(ptp4lObj, "domainNumber", &config.domainNumber))
                return false;
            if(!get_Int_Val(ptp4lObj, "transportSpecific",
                    &config.transportSpecific))
                return false;
        }
        if(chronyObj) {
            strncpy(config.udsAddrChrony, "/var/run/chrony/chronyd.sock",
                sizeof(config.udsAddrChrony) - 1);
            if(!get_Str_Val(chronyObj, "udsAddr", config.udsAddrChrony))
                return false;
        }
        config.timeBaseIndex = currentIndex;
        currentIndex++;
        timeBaseCfgs.push_back(config);
    }
    print_config();
    return true;
}
