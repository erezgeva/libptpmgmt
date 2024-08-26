/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file clock_config.hpp
 * @brief Proxy configuration class.
 * One configuration object per session is instatiated
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#ifndef CLOCK_CONFIG_HPP
#define CLOCK_CONFIG_HPP

#include <cstdint>

#include <common/ptp_event.hpp>
#include <proxy/message.hpp>
#include <rtpi/mutex.hpp>
#include "jclk_subscription.hpp"

__CLKMGR_NAMESPACE_BEGIN

class ClockConfiguration
{
  private:
    enum { event = 0, value };
    bool update, writeUpdate;
    rtpi::mutex update_lock;
    struct Config {
        jcl_event event;
        jcl_value value;
    };
    Config writeShadow, readShadow;
    Config config;
  public:
    ClockConfiguration();
    void speculateWrite();
    void setEvent(const jcl_event &event);
    void setValue(const jcl_value &value);
    void commitWrite();

    bool readConsume();
    const jcl_event &getEvent() { return readShadow.event; }
    const jcl_value &getValue() { return readShadow.value; }

    static sessionId_t getSessionId();
};

__CLKMGR_NAMESPACE_END

#endif /* CLOCK_CONFIG_HPP */
