/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file null_msg.hpp
 * @brief Proxy null message class. Used for debug only. Should not be used.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#ifndef PROXY_NULL_MSG_HPP
#define PROXY_NULL_MSG_HPP

#include <common/null_msg.hpp>
#include <proxy/message.hpp>

__CLKMGR_NAMESPACE_BEGIN

class ProxyNullMessage : virtual public ProxyMessage,
    virtual public CommonNullMessage
{
  protected:
    ProxyNullMessage() : MESSAGE_NULL() {};
  public:
    bool processMessage(ClockConfiguration &config) { return true; }
    bool generateResponse(uint8_t *msgBuffer, std::size_t &length,
        const ClockStatus &status)
    { return false; }
    static bool initMessage() { return true; }
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_NULL_MSG_HPP */
