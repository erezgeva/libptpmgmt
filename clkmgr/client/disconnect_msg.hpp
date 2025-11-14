/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Client disconnect message class.
 * Implements client specific disconnect message function.
 *
 * @author Goh Wei Sheng <wei.sheng.goh@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#ifndef CLIENT_DISCONNECT_MSG_HPP
#define CLIENT_DISCONNECT_MSG_HPP

#include "common/disconnect_msg.hpp"

__CLKMGR_NAMESPACE_BEGIN

class ClientDisconnectMessage : public DisconnectMessage
{
  private:
    bool parseBufferTail() override final;
  public:
    ClientDisconnectMessage() = default;
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_DISCONNECT_MSG_HPP */
