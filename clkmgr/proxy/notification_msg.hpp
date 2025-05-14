/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy notification message class.
 * Implements proxy specific notification message function.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef PROXY_NOTIFICATION_MSG_HPP
#define PROXY_NOTIFICATION_MSG_HPP

#include "proxy/message.hpp"
#include "common/notification_msg.hpp"

__CLKMGR_NAMESPACE_BEGIN

class ProxyNotificationMessage : virtual public ProxyMessage,
    virtual public NotificationMessage
{
  private:
    int timeBaseIndex = 0;
  public:
    ProxyNotificationMessage() : Message(NOTIFY_MESSAGE) {}
    virtual bool processMessage(TransportListenerContext &LxContext,
        TransportTransmitterContext *&TxContext);
    virtual bool makeBuffer(TransportTransmitterContext &TxContext) const;

    /**
     * Create the ProxyNotificationMessage object
     * @param msg msg structure to be fill up
     * @param LxContext proxy transport listener context
     * @return true
     */
    static bool buildMessage(Message *&msg, TransportListenerContext &LxContext);

    /**
     * Add proxy's NOTIFY_MESSAGE type and its builder to transport layer.
     * @return true
     */
    static bool initMessage();
    void setTimeBaseIndex(int newTimeBaseIndex) {
        timeBaseIndex = newTimeBaseIndex;
    }
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_NOTIFICATION_MSG_HPP */
