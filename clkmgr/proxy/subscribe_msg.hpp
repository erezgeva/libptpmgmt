/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy subscribe message class.
 * Implements proxy specific subscribe message function.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef PROXY_SUBSCRIBE_MSG_HPP
#define PROXY_SUBSCRIBE_MSG_HPP

#include "proxy/message.hpp"
#include "common/subscribe_msg.hpp"

__CLKMGR_NAMESPACE_BEGIN

class ProxySubscribeMessage : virtual public ProxyMessage,
    virtual public CommonSubscribeMessage
{
  private:
    int timeBaseIndex = 0;
  protected:
    ProxySubscribeMessage() : Message(SUBSCRIBE_MSG) {};
  public:
    virtual bool processMessage(TransportListenerContext &LxContext,
        TransportTransmitterContext *&TxContext);
    virtual bool makeBuffer(TransportTransmitterContext &TxContext) const;
    virtual bool parseBuffer(TransportListenerContext &LxContext);

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
    void setTimeBaseIndex(int newTimeBaseIndex) {
        timeBaseIndex = newTimeBaseIndex;
    }
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_SUBSCRIBE_MSG_HPP */
