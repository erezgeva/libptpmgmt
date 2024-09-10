/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file
 * @brief Non-functional message class for debug
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLIENT_NULL_MSG_HPP
#define CLIENT_NULL_MSG_HPP

#include <client/message.hpp>
#include <common/null_msg.hpp>

__CLKMGR_NAMESPACE_BEGIN

class ClientNullMessage : public ClientMessage, public CommonNullMessage
{
  protected:
  public:
    static bool initMessage() { return true; }
    ClientNullMessage() {}
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_NULL_MSG_HPP */
