/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file
 * @brief Proxy notification message class.
 * Implements proxy specific notification message function.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef PROXY_NOTIFICATION_MSG_HPP
#define PROXY_NOTIFICATION_MSG_HPP

#include <cstdint>

#include <common/ptp_event.hpp>
#include <common/notification_msg.hpp>
#include <proxy/message.hpp>

__CLKMGR_NAMESPACE_BEGIN

class ProxyNotificationMessage : virtual public ProxyMessage,
    virtual public NotificationMessage
{
  public:
    ProxyNotificationMessage() : MESSAGE_NOTIFY() {}
    virtual PROCESS_MESSAGE_TYPE(processMessage);
    virtual BUILD_TXBUFFER_TYPE(makeBuffer) const;

    /**
     * @brief Create the ProxyNotificationMessage object
     * @param msg msg structure to be fill up
     * @param LxContext proxy transport listener context
     * @return true
     */
    static MAKE_RXBUFFER_TYPE(buildMessage);

    /**
     * @brief Add proxy's NOTIFY_MESSAGE type and its builder to transport layer.
     * @return true
     */
    static bool initMessage();
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_NOTIFICATION_MSG_HPP */
