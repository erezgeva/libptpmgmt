/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Proxy JSON parser
 *
 * @author Goh Wei Sheng <wei.sheng.goh@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#ifndef PROXY_CONFIG_PARSER_HPP
#define PROXY_CONFIG_PARSER_HPP

#include "jsonParser.h"
#include "pub/clkmgr/utility.h"
#include "pub/clkmgr/types.h"

#include <cstdint>
#include <vector>

__CLKMGR_NAMESPACE_BEGIN

struct TimeBaseCfgFull {
    TimeBaseCfg base;
    std::string udsAddrChrony;
    std::string udsAddrPtp4l;
};

typedef std::vector<TimeBaseCfgFull>::iterator cfgItr;

class JsonConfigParser
{
  private:
    std::vector<TimeBaseCfgFull> timeBaseCfgs;
    JsonConfigParser() = default;
    bool get_Int_Val(jsonObject *obj, const std::string &key, uint8_t &res);
    bool get_Str_Val(jsonObject *obj, const std::string &key, char *res);
    bool get_Str_Val(jsonObject *obj, const std::string &key, std::string &res);
    void print_config();
  public:
    static JsonConfigParser &getInstance();
    bool process_json(const char *file);
    cfgItr begin() { return timeBaseCfgs.begin(); }
    cfgItr end() { return timeBaseCfgs.end(); }
    size_t size() const { return timeBaseCfgs.size(); }
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_CONFIG_PARSER_HPP */
