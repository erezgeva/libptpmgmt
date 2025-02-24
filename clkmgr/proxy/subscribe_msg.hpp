/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy subscribe message class.
 * Implements proxy specific subscribe message function.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef PROXY_SUBSCRIBE_MSG_HPP
#define PROXY_SUBSCRIBE_MSG_HPP

#include "common/subscribe_msg.hpp"
#include "proxy/message.hpp"

__CLKMGR_NAMESPACE_BEGIN

class ProxySubscribeMessage : virtual public ProxyMessage,
    virtual public CommonSubscribeMessage
{
  private:
    int timeBaseIndex;
  protected:
    ProxySubscribeMessage() : MESSAGE_SUBSCRIBE() {};
  public:
    virtual PROCESS_MESSAGE_TYPE(processMessage);
    virtual BUILD_TXBUFFER_TYPE(makeBuffer) const;
    virtual PARSE_RXBUFFER_TYPE(parseBuffer);

    /**
     * Create the ProxyConnectMessage object
     * @param msg msg structure to be fill up
     * @param LxContext proxy transport listener context
     * @return true
     */
    static MAKE_RXBUFFER_TYPE(buildMessage);

    /**
     * Add proxy's CONNECT_MSG type and its builder to transport layer.
     * @return true
     */
    static bool initMessage();
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_SUBSCRIBE_MSG_HPP */
