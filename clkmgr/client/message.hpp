/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client message base class.
 * Extended for specific messages such as connect and subscribe.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLIENT_MESSAGE_HPP
#define CLIENT_MESSAGE_HPP

#include "pub/clkmgr/client_state.h"
#include "common/message.hpp"

#include <cstdint>

__CLKMGR_NAMESPACE_BEGIN

class ClientMessage;
typedef std::shared_ptr<ClientMessage> MessageX;
typedef std::unique_ptr<ClientMessage> Message0;

class ClientMessage : virtual public Message
{
  protected:
    ClientMessage() : Message(NULL_MSG) {}
  public:
    static bool init();
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_MESSAGE_HPP */
