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

class ClientConnectMessage : public ConnectMessage
{
  private:
    static ClientState *currentClientState;
    bool parseBufferTail() override final;

  public:
    static rtpi::mutex cv_mtx;
    static rtpi::condition_variable cv;

    /**
     * process the reply for connect msg from proxy.
     * @return true
     */
    bool processMessage() override final;
    bool writeClientId() override final;
    void setClientState(ClientState &newClientState);
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_CONNECT_MSG_HPP */
