/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Common client infomation implementation
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef PROXY_CLIENT_HPP
#define PROXY_CLIENT_HPP

#include "common/msgq_tport.hpp"

#include <memory>
#include <map>

__CLKMGR_NAMESPACE_BEGIN

class Client
{
  private:
    sessionId_t m_sessionId;
    std::unique_ptr<Transmitter> m_transmitContext;
    static sessionId_t CreateClientSession(const std::string &id);

  public:
    static sessionId_t connect(sessionId_t sessionId, const std::string &id);
    static bool subscribe(size_t timeBaseIndex, sessionId_t sessionId);
    static void RemoveClient(sessionId_t sessionId);
    static Transmitter *getTxContext(sessionId_t sessionId);
    static Client *getClient(sessionId_t sessionId);
    sessionId_t getSessionId() const { return m_sessionId; }
    Transmitter *getTxContext() { return m_transmitContext.get(); }
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_CLIENT_HPP */
