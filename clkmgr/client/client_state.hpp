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

#include "common/util.hpp"

#include <ctime>
#include <string>
#include <atomic>

__CLKMGR_NAMESPACE_BEGIN

/**
 * Class to keep the current state of client-runtime
 */
class ClientState
{
  private:
    std::atomic_bool connected{false}; /**< Connection status */
    sessionId_t m_sessionId = InvalidSessionId; /**< Session ID */
    std::string clientID; /**< Client ID */

    ClientState() = default;
    void set_connected(bool state) { connected = state; }
    void set_sessionId(sessionId_t sessionId) { m_sessionId = sessionId; }

  public:
    static ClientState &getSingleInstance();
    bool connect(uint32_t timeOut, timespec *lastConnectTime = nullptr);
    bool connectReply(sessionId_t sessionId);

    bool get_connected() const { return connected; }
    const std::string &get_clientID() const { return clientID; }
    sessionId_t get_sessionId() const { return m_sessionId; }
    // Set by ClientQueue::init()
    void set_clientID(const std::string  &cID) { clientID = cID; }
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_CLIENT_STATE_HPP */
