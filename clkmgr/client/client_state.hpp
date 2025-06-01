/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Class to get and set the state of client-runtime
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLIENT_CLIENT_STATE_HPP
#define CLIENT_CLIENT_STATE_HPP

#include "common/msgq_tport.hpp"

#include <string>
#include <atomic>

__CLKMGR_NAMESPACE_BEGIN

class Message;

/**
 * Class to keep the current state of client-runtime
 */
class ClientState
{
  private:
    static std::string m_clientID; /**< Client ID */
    static std::atomic<sessionId_t> m_sessionId;
    static std::atomic_bool m_connected; /**< Connection status */

  public:
    static bool init();
    static bool sendMessage(Message &msg);
    static Transmitter *getTransmitter();
    static bool connect(uint32_t timeOut, timespec *lastConnectTime = nullptr);
    static bool connectReply(sessionId_t sessionId);

    static const std::string &get_clientID() { return m_clientID; }
    static sessionId_t get_sessionId() { return m_sessionId; }
    static bool get_connected() { return m_connected; }
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_CLIENT_STATE_HPP */
