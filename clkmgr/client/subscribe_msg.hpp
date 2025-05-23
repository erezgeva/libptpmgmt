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

class ClientSubscribeMessage : public SubscribeMessage
{
  private:
    static ClientState *currentClientState;
    int timeBaseIndex = 0; /**< Timebase index */

  public:
    static rtpi::mutex cv_mtx;
    static rtpi::condition_variable cv;
    /**
     * process the reply for subscribe msg from proxy.
     * @param rxContext client run-time listener
     * @param txContext client run-time transmitter
     * @return true
     */
    bool processMessage(Listener &rxContext, Transmitter *&txContext) override;
    bool parseBuffer(Listener &rxContext) override;
    bool writeClientId(Listener &rxContext) override;
    bool makeBuffer(Transmitter &txContext) const override;

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
