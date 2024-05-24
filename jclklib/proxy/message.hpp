/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file message.hpp
 * @brief Proxy message base class. Extended for specific messages such as connect and subscribe.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#ifndef PROXY_MESSAGE_HPP
#define PROXY_MESSAGE_HPP

#include <cstdint>
#include <map>
#include <memory>

#include <common/message.hpp>
#include <proxy/clock_config.hpp>
#include <proxy/clock_status.hpp>

namespace JClkLibProxy
{
    class ProxyMessage : virtual public JClkLibCommon::Message
    {
    public:
        /**
         * Generate a response, if necessary, to the received message
         * @brief generate response to received message
         *
         * @param msgBuffer (out) transport buffer to fill response
         * @param length (in/out) length of transport buffer to fill (in) and send (out)
         *
         * @return true if a response is required, false otherwise
         */
        virtual bool generateResponse(uint8_t *msgBuffer, std::size_t &length,
             const ClockStatus &status) = 0;
        static bool init();
    };
}

#endif /* PROXY_MESSAGE_HPP */
