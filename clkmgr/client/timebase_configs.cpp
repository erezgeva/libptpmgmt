/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Set and get the configuration of time base
 *
 * @author Song Yoong Siang <yoong.siang.song@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include "client/timebase_configs.hpp"

__CLKMGR_NAMESPACE_USE;

TimeBaseConfigurations &TimeBaseConfigurations::getInstance()
{
    static TimeBaseConfigurations instance;
    return instance;
}

const std::vector<TimeBaseCfg> &TimeBaseConfigurations::getTimeBaseCfgs() const
{
    return timeBaseCfgs;
}

void TimeBaseConfigurations::addTimeBaseCfg(const TimeBaseCfg &cfg)
{
    timeBaseCfgs.push_back(cfg);
}
