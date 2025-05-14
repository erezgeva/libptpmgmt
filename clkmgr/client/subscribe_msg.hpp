/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client subscribe message class.
 * Implements client specific functionality.
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLIENT_SUBSCRIBE_MSG_HPP
#define CLIENT_SUBSCRIBE_MSG_HPP

#include "client/message.hpp"
#include "common/subscribe_msg.hpp"

#include <rtpi/condition_variable.hpp>

__CLKMGR_NAMESPACE_BEGIN

class ClientSubscribeMessage : virtual public
    CommonSubscribeMessage,
    virtual public ClientMessage
{
  private:
    static ClientState *currentClientState;
    int timeBaseIndex = 0; /**< Timebase index */

  public:
    ClientSubscribeMessage() : Message(SUBSCRIBE_MSG) {};

    static rtpi::mutex cv_mtx;
    static rtpi::condition_variable cv;
    /**
     * process the reply for subscribe msg from proxy.
     * @param LxContext client run-time transport listener context
     * @param TxContext client run-time transport transmitter context
     * @return true
     */
    virtual bool processMessage(TransportListenerContext &LxContext,
        TransportTransmitterContext *&TxContext);

    /**
     * Create the ClientSubscribeMessage object
     * @param msg msg structure to be fill up
     * @param LxContext client run-time transport listener context
     * @return true
     */
    static bool buildMessage(Message *&msg, TransportListenerContext &LxContext);

    /**
     * Add client's SUBSCRIBE_MSG type and its builder to transport layer.
     * @return true
     */
    static bool initMessage();

    virtual bool parseBuffer(TransportListenerContext &LxContext);
    virtual bool makeBuffer(TransportTransmitterContext &TxContext) const;

    void setClientState(ClientState &newClientState);

    /**
     * Set the time base index.
     * @param[in] newTimeBaseIndex The new time base index to set.
     */
    void set_timeBaseIndex(int newTimeBaseIndex) {
        timeBaseIndex = newTimeBaseIndex;
    }

    /**
     * Get the value of the time base index.
     * @return The value of the time base index.
     */
    int get_timeBaseIndex() const {
        return timeBaseIndex;
    }
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_SUBSCRIBE_MSG_HPP */
