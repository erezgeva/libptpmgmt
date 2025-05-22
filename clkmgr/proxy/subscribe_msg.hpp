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

class ProxySubscribeMessage : public SubscribeMessage
{
  private:
    int timeBaseIndex = 0;

  public:
    bool processMessage(Listener &rxContext, Transmitter *&txContext) override;
    bool makeBuffer(Transmitter &txContext) const override;
    bool parseBuffer(Listener &rxContext) override;

    void setTimeBaseIndex(int newTimeBaseIndex) {
        timeBaseIndex = newTimeBaseIndex;
    }
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_SUBSCRIBE_MSG_HPP */
