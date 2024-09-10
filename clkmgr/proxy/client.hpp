/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file
 * @brief Common client infomation implementation
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef PROXY_CLIENT
#define PROXY_CLIENT

#include <map>

#include <common/message.hpp>

__CLKMGR_NAMESPACE_BEGIN

class Client;
typedef std::shared_ptr<Client> ClientX;

class Client
{
  public:
    typedef std::pair<sessionId_t, ClientX> SessionMapping_t;
  private:
    static sessionId_t nextSession;
    static std::map<SessionMapping_t::first_type, SessionMapping_t::second_type>
    SessionMap;
  public:
    static sessionId_t CreateClientSession();
    static size_t GetSessionCount() { return SessionMap.size(); }
    static void RemoveClientSession(sessionId_t sessionId) {
        SessionMap.erase(sessionId);
    }
    static sessionId_t GetSessionIdAt(size_t index);
    static ClientX GetClientSession(sessionId_t sessionId);
  private:
    std::unique_ptr<TransportTransmitterContext> transmitContext;
  public:
    void set_transmitContext(decltype(transmitContext)::pointer context)
    { this->transmitContext.reset(context); }
    auto get_transmitContext() { return transmitContext.get(); }
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_CLIENT */
