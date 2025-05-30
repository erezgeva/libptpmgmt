/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy client infomation implementation
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef PROXY_CLIENT_HPP
#define PROXY_CLIENT_HPP

#include "common/msgq_tport.hpp"
#include "common/message.hpp"

#include <memory>
#include <map>

__CLKMGR_NAMESPACE_BEGIN

static const size_t MAX_CLIENT_COUNT = 8;

class Client
{
  private:
    sessionId_t m_sessionId = InvalidSessionId;
    std::unique_ptr<Transmitter> m_transmitContext;
    static sessionId_t CreateClientSession(const std::string &id);
    static void RemoveClient(sessionId_t sessionId);

  public:
    static bool init();
    static sessionId_t connect(sessionId_t sessionId, const std::string &id);
    static bool subscribe(size_t timeBaseIndex, sessionId_t sessionId);
    static void RemoveClients(const std::vector<sessionId_t> &sessionIdToRemove);
    static void NotifyClients(size_t timeBaseIndex,
        std::vector<sessionId_t> &subscribedClients,
        std::vector<sessionId_t> &sessionIdToRemove);
    static Client *getClient(sessionId_t sessionId);
    static Transmitter *getTxContext(sessionId_t sessionId);
    sessionId_t getSessionId() const { return m_sessionId; }
    Transmitter *getTxContext() { return m_transmitContext.get(); }
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_CLIENT_HPP */
