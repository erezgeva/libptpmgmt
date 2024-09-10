/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file
 * @brief Client subscribe message class.
 * Implements client specific functionality.
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLIENT_SUBSCRIBE_MSG_HPP
#define CLIENT_SUBSCRIBE_MSG_HPP

#include <client/message.hpp>
#include <clkmgr/client_state.h>
#include <common/subscribe_msg.hpp>
#include <rtpi/condition_variable.hpp>

__CLKMGR_NAMESPACE_BEGIN

class ClientSubscribeMessage : virtual public
    CommonSubscribeMessage,
    virtual public ClientMessage
{
  private:
    inline static clkmgr_event_state *clkmgrCurrentState;
    inline static ClientState *currentClientState;
    inline static std::map <sessionId_t,
           std::array<client_ptp_event *, 2>> client_ptp_event_map;

  public:
    ClientSubscribeMessage() : MESSAGE_SUBSCRIBE() {};

    static rtpi::mutex cv_mtx;
    static rtpi::condition_variable cv;
    /**
     * @brief process the reply for subscribe msg from proxy.
     * @param LxContext client run-time transport listener context
     * @param TxContext client run-time transport transmitter context
     * @return true
     */
    virtual PROCESS_MESSAGE_TYPE(processMessage);

    /**
     * @brief Create the ClientSubscribeMessage object
     * @param msg msg structure to be fill up
     * @param LxContext client run-time transport listener context
     * @return true
     */
    static MAKE_RXBUFFER_TYPE(buildMessage);

    /**
     * @brief Add client's SUBSCRIBE_MSG type and its builder to transport layer.
     * @return true
     */
    static bool initMessage();

    virtual PARSE_RXBUFFER_TYPE(parseBuffer);

    void setClientState(ClientState *newClientState);

    /* Delete the corresponding map pair item */
    static void deleteClientPtpEventStruct(sessionId_t sID);

    /* Get the corresponding map pair item */
    static client_ptp_event *getClientPtpEventStruct(sessionId_t sID);

    static client_ptp_event *getClientPtpEventCompositeStruct(sessionId_t sID);

    /* Reduce the corresponding eventCount */
    static void resetClientPtpEventStruct(sessionId_t sID,
        clkmgr_event_count &eventCount);
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_SUBSCRIBE_MSG_HPP */
