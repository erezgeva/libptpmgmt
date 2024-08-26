/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file notification_msg.hpp
 * @brief Client notification message class.
 * Implements client specific notification message function.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#ifndef CLIENT_NOTIFICATION_MSG_HPP
#define CLIENT_NOTIFICATION_MSG_HPP

#include <cstdint>

#include <client/message.hpp>
#include <common/ptp_event.hpp>
#include <common/notification_msg.hpp>

__CLKMGR_NAMESPACE_BEGIN

class ClientNotificationMessage : virtual public ClientMessage,
    virtual public NotificationMessage
{
  public:
    virtual PROCESS_MESSAGE_TYPE(processMessage);
    virtual BUILD_TXBUFFER_TYPE(makeBuffer) const;

    /**
     * @brief Create the ClientNotificationMessage object
     * @param msg msg structure to be fill up
     * @param LxContext client transport listener context
     * @return true
     */
    static MAKE_RXBUFFER_TYPE(buildMessage);

    /**
     * @brief Add client's NOTIFY_MESSAGE type and its builder to transport layer.
     * @return true
     */
    static bool initMessage();

    virtual PARSE_RXBUFFER_TYPE(parseBuffer);

    static void addClientState(ClientState *newClientState);
    static void deleteClientState(ClientState *newClientState);
    void handleEventUpdate(uint32_t eventSub, uint32_t eventFlag,
        bool &currentState, const bool &newState, std::atomic<int> &eventCount,
        bool &compositeEvent);
    void handleGmOffsetEvent(uint32_t eventSub, uint32_t eventFlag,
        int64_t &masterOffset, const uint32_t &newMasterOffset,
        std::atomic<int> &eventCount, bool &withinBoundary, uint32_t lowerBound,
        uint32_t upperBound);

  protected:
    ClientNotificationMessage() : MESSAGE_NOTIFY() {}

  private:
    inline static std::vector<ClientState *> ClientStateArray;

    ptp_event proxy_data = {};
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_NOTIFICATION_MSG_HPP */
