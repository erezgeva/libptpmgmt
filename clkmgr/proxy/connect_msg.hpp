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

class ProxyConnectMessage : virtual public ProxyMessage,
    virtual public CommonConnectMessage
{
  protected:
    ProxyConnectMessage() : Message(CONNECT_MSG) {};
  public:
    /**
     * process the connect msg from client-runtime
     * @param LxContext proxy transport listener context
     * @param TxContext proxy transport transmitter context
     * @return true
     */
    virtual bool processMessage(TransportListenerContext &LxContext,
        TransportTransmitterContext *&TxContext);
    virtual bool makeBuffer(TransportTransmitterContext &TxContext) const;

    /**
     * Create the ProxyConnectMessage object
     * @param msg msg structure to be fill up
     * @param LxContext proxy transport listener context
     * @return true
     */
    static bool buildMessage(Message *&msg, TransportListenerContext &LxContext);

    /**
     * Add proxy's CONNECT_MSG type and its builder to transport layer.
     * @return true
     */
    static bool initMessage();
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_CONNECT_MSG_HPP */
