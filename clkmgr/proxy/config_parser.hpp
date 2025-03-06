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

class JsonConfigParser
{
  private:
    std::vector<TimeBaseCfg> timeBaseCfgs;
    JsonConfigParser() = default;
  public:
    static JsonConfigParser &getInstance();
    bool process_json(const char *file);
    bool get_Int_Val(jsonObject *obj, const char *key, uint8_t *res);
    bool get_Str_Val(jsonObject *obj, const char *key, char *res);
    void print_config();
    const std::vector<TimeBaseCfg> &getTimeBaseCfgs() const {
        return timeBaseCfgs;
    }
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_CONFIG_PARSER_HPP */
