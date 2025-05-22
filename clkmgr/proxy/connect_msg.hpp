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

#include "proxy/message.hpp"
#include "common/connect_msg.hpp"

__CLKMGR_NAMESPACE_BEGIN

class ProxyConnectMessage : public ConnectMessage
{
  public:
    /**
     * process the connect msg from client-runtime
     * @param rxContext proxy listener
     * @param txContext proxy transmitter
     * @return true
     */
    bool processMessage(Listener &rxContext, Transmitter *&txContext) override;
    bool makeBuffer(Transmitter &txContext) const override;
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_CONNECT_MSG_HPP */
