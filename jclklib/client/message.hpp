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

#include <common/message.hpp>
#include "jclk_client_state.hpp"

__CLKMGR_NAMESPACE_BEGIN

class ClientMessage;
typedef std::shared_ptr<ClientMessage> MessageX;
typedef std::unique_ptr<ClientMessage> Message0;

class ClientMessage : virtual public Message
{
  protected:
    ClientMessage() : Message() {}
  public:
    static bool init();
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_MESSAGE_HPP */
