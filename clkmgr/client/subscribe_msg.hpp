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
    bool parseBufferTail() override final;

  public:
    static rtpi::mutex cv_mtx;
    static rtpi::condition_variable cv;
    /**
     * process the reply for subscribe msg from proxy.
     * @return true
     */
    bool writeClientId() override final;

    void setClientState(ClientState &newClientState) {
        currentClientState = &newClientState;
    }
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_SUBSCRIBE_MSG_HPP */
