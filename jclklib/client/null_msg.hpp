/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file null_msg.hpp
 * @brief Non-functional message class for debug
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#ifndef CLIENT_NULL_MSG_HPP
#define CLIENT_NULL_MSG_HPP

#include <client/message.hpp>
#include <common/null_msg.hpp>

namespace JClkLibClient
{
    class ClientNullMessage : public JClkLibClient::ClientMessage,
				  public JClkLibCommon::CommonNullMessage
    {
    protected:
    public:
        static bool initMessage() { return true; }
        ClientNullMessage() {}
    };
}

#endif /* CLIENT_NULL_MSG_HPP */
