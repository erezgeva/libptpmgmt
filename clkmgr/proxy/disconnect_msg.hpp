/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Proxy disconnect message class.
 * Implements proxy specific disconnect message function.
 *
 * @author Goh Wei Sheng <wei.sheng.goh@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#ifndef PROXY_DISCONNECT_MSG_HPP
#define PROXY_DISCONNECT_MSG_HPP

#include "common/disconnect_msg.hpp"

__CLKMGR_NAMESPACE_BEGIN

class ProxyDisconnectMessage : public DisconnectMessage
{
  private:
    bool parseBufferTail() override final;
  public:
    ProxyDisconnectMessage() = default;
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_DISCONNECT_MSG_HPP */
