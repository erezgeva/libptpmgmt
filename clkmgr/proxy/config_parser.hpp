/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Proxy JSON parser
 *
 * @author by Goh Wei Sheng <wei.sheng.goh@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#ifndef PROXY_CONFIG_PARSER_HPP
#define PROXY_CONFIG_PARSER_HPP

#include "pub/clkmgr/utility.h"
#include "pub/clkmgr/types.h"

#include <cstdint>
#include <string>
#include <vector>

__CLKMGR_NAMESPACE_BEGIN

/* Global vector to hold all time base configurations */
extern std::vector<TimeBaseCfg> timeBaseCfgs;

__CLKMGR_NAMESPACE_END

#endif /* PROXY_CONFIG_PARSER_HPP */
