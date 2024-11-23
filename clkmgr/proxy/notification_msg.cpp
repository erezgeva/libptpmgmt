/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy notification message implementation.
 * Implements proxy specific notification message function.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "common/message.hpp"
#include "common/print.hpp"
#include "common/serialize.hpp"
#include "proxy/notification_msg.hpp"

__CLKMGR_NAMESPACE_USE

using namespace std;

extern ptp_event pe;

/**
 * Create the ProxyNotificationMessage object
 * @param msg msg structure to be fill up
 * @param LxContext proxy transport listener context
 * @return true
 */
MAKE_RXBUFFER_TYPE(ProxyNotificationMessage::buildMessage)
{
    msg = new ProxyNotificationMessage();
    return true;
}

/**
 * @brief Add proxy's NOTIFY_MESSAGE type and its builder to transport layer.
 *
 * This function will be called during init to add a map of NOTIFY_MESSAGE
 * type and its corresponding buildMessage function.
 *
 * @return true
 */
bool ProxyNotificationMessage::initMessage()
{
    addMessageType(parseMsgMapElement_t(NOTIFY_MESSAGE, buildMessage));
    return true;
}

BUILD_TXBUFFER_TYPE(ProxyNotificationMessage::makeBuffer) const
{
    PrintDebug("[ProxyNotificationMessage]::makeBuffer");
    if(!Message::makeBuffer(TxContext))
        return false;
    /* Add ptp data here */
    if(!WRITE_TX(FIELD, pe, TxContext))
        return false;
    return true;
}

PROCESS_MESSAGE_TYPE(ProxyNotificationMessage::processMessage)
{
    return true;
}
