/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

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

#include "client/client_state.hpp"
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
    ClientSubscribeMessage() : MESSAGE_SUBSCRIBE() {};

    static rtpi::mutex cv_mtx;
    static rtpi::condition_variable cv;
    /**
     * process the reply for subscribe msg from proxy.
     * @param LxContext client run-time transport listener context
     * @param TxContext client run-time transport transmitter context
     * @return true
     */
    virtual PROCESS_MESSAGE_TYPE(processMessage);

    /**
     * Create the ClientSubscribeMessage object
     * @param msg msg structure to be fill up
     * @param LxContext client run-time transport listener context
     * @return true
     */
    static MAKE_RXBUFFER_TYPE(buildMessage);

    /**
     * Add client's SUBSCRIBE_MSG type and its builder to transport layer.
     * @return true
     */
    static bool initMessage();

    virtual PARSE_RXBUFFER_TYPE(parseBuffer);
    virtual BUILD_TXBUFFER_TYPE(makeBuffer) const;

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
