/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy connect message class.
 * Implements proxy specific connect message function.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef PROXY_CONNECT_MSG_HPP
#define PROXY_CONNECT_MSG_HPP

#include "common/connect_msg.hpp"

__CLKMGR_NAMESPACE_BEGIN

class ProxyConnectMessage : public ConnectMessage
{
  private:
    bool makeBufferTail(Buffer &buff) const override final;
    bool parseBufferTail() override final;
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_CONNECT_MSG_HPP */
