/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file subscribe_msg.hpp
 * @brief Client subscribe message class. Implements client specific functionality.
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#ifndef CLIENT_SUBSCRIBE_MSG_HPP
#define CLIENT_SUBSCRIBE_MSG_HPP

#include <client/message.hpp>
#include <common/subscribe_msg.hpp>

namespace JClkLibClient
{
    class ClientSubscribeMessage : virtual public JClkLibCommon::CommonSubscribeMessage,
        virtual public ClientMessage
    {
    private:
        inline static JClkLibCommon::jcl_state *jclCurrentState;
        inline static ClientState *currentClientState;
        inline static std::map <JClkLibCommon::sessionId_t, 
            std::array<JClkLibCommon::client_ptp_event*, 2>> client_ptp_event_map;

    public:
        ClientSubscribeMessage() : MESSAGE_SUBSCRIBE() {};

        static std::mutex cv_mtx;
        static std::condition_variable cv;
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
        static void deleteClientPtpEventStruct(JClkLibCommon::sessionId_t sID);

        /* Get the corresponding map pair item */
        static JClkLibCommon::client_ptp_event* getClientPtpEventStruct(JClkLibCommon::sessionId_t sID);

        static JClkLibCommon::client_ptp_event* getClientPtpEventCompositeStruct(JClkLibCommon::sessionId_t sID);

        /* Reduce the corresponding eventCount */
        static void resetClientPtpEventStruct(JClkLibCommon::sessionId_t sID, JClkLibCommon::jcl_state_event_count &eventCount);
    };
}

#endif /* CLIENT_SUBSCRIBE_MSG_HPP */
