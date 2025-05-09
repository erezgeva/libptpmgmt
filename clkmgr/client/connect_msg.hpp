/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client connect message class. Implements client specific functionality.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLIENT_CONNECT_MSG_HPP
#define CLIENT_CONNECT_MSG_HPP

#include "client/message.hpp"
#include "common/connect_msg.hpp"

#include <rtpi/condition_variable.hpp>

__CLKMGR_NAMESPACE_BEGIN

class ClientConnectMessage : virtual public CommonConnectMessage,
    virtual public ClientMessage
{
  private:
    static ClientState *currentClientState;
  public:
    ClientConnectMessage() : MESSAGE_CONNECT() {};
    static rtpi::mutex cv_mtx;
    static rtpi::condition_variable cv;

    /**
     * process the reply for connect msg from proxy.
     * @param LxContext client run-time transport listener context
     * @param TxContext client run-time transport transmitter context
     * @return true
     */
    virtual PROCESS_MESSAGE_TYPE(processMessage);
    virtual PARSE_RXBUFFER_TYPE(parseBuffer);

    /**
     * Create the ClientConnectMessage object
     * @param msg msg structure to be fill up
     * @param LxContext client run-time transport listener context
     * @return true
     */
    static MAKE_RXBUFFER_TYPE(buildMessage);

    /**
     * Add client's CONNECT_MSG type and its builder to transport layer.
     * @return true
     */
    static bool initMessage();

    void setClientState(ClientState &newClientState);
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_CONNECT_MSG_HPP */
