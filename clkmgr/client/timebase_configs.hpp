/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Class to get and set the configuration of time base
 *
 * @author Song Yoong Siang <yoong.siang.song@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#ifndef CLIENT_TIMEBASE_CONFIGS_HPP
#define CLIENT_TIMEBASE_CONFIGS_HPP

#include "pub/clkmgr/utility.h"
#include "pub/clkmgr/types.h"
#include <vector>

__CLKMGR_NAMESPACE_BEGIN

class TimeBaseConfigurations
{
  private:
    /**
     * Vector to hold all time base configurations.
     */
    std::vector<TimeBaseCfg> timeBaseCfgs;

    /**
     * Private constructor to prevent instantiation.
     */
    TimeBaseConfigurations() = default;

  public:
    /**
     * Get the single instance of the TimeBaseConfigurations.
     * @return Reference to the single instance.
     */
    static TimeBaseConfigurations &getInstance();

    /**
     * Get the time base configurations.
     * @return Vector of TimeBaseCfg.
     */
    const std::vector<TimeBaseCfg> &getTimeBaseCfgs() const;

    /**
     * Add a time base configuration.
     * @param[in] cfg TimeBaseCfg to add.
     */
    void addTimeBaseCfg(const TimeBaseCfg &cfg);
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_TIMEBASE_CONFIGS_HPP */
