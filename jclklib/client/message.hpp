/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file message.hpp
 * @brief Client message base class.
 * Extended for specific messages such as connect and subscribe.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#ifndef CLIENT_MESSAGE_HPP
#define CLIENT_MESSAGE_HPP

#include <cstdint>
#include <map>
#include <memory>

#include <client/jclk_client_state.hpp>
#include <common/message.hpp>

namespace JClkLibClient
{
class ClientMessage;
typedef std::shared_ptr<ClientMessage> MessageX;
typedef std::unique_ptr<ClientMessage> Message0;

class ClientMessage : virtual public JClkLibCommon::Message
{
  protected:
    ClientMessage() : JClkLibCommon::Message() {}
  public:
    static bool init();
};
}

#endif /* CLIENT_MESSAGE_HPP */
