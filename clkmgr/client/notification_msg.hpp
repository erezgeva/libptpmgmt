/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client notification message class.
 * Implements client specific notification message function.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLIENT_NOTIFICATION_MSG_HPP
#define CLIENT_NOTIFICATION_MSG_HPP

#include "client/message.hpp"
#include "common/notification_msg.hpp"

__CLKMGR_NAMESPACE_BEGIN

class ClientNotificationMessage : virtual public ClientMessage,
    virtual public NotificationMessage
{
  public:
    virtual bool processMessage(TransportListenerContext &LxContext,
        TransportTransmitterContext *&TxContext);
    virtual bool makeBuffer(TransportTransmitterContext &TxContext) const;

    /**
     * Create the ClientNotificationMessage object
     * @param msg msg structure to be fill up
     * @param LxContext client transport listener context
     * @return true
     */
    static bool buildMessage(Message *&msg, TransportListenerContext &LxContext);

    /**
     * Add client's NOTIFY_MESSAGE type and its builder to transport layer.
     * @return true
     */
    static bool initMessage();

    virtual bool parseBuffer(TransportListenerContext &LxContext);

  protected:
    ClientNotificationMessage() : Message(NOTIFY_MESSAGE) {}
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_NOTIFICATION_MSG_HPP */
