/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file client.hpp
 * @brief Common client infomation implementation
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#ifndef PROXY_CLIENT
#define PROXY_CLIENT

#include <map>

#include <client/message.hpp>

namespace JClkLibProxy {
    class Client;
    typedef std::shared_ptr<JClkLibProxy::Client> ClientX;

    class Client
    {
    public:
        typedef std::pair<JClkLibCommon::sessionId_t,ClientX> SessionMapping_t;
    private:
        static JClkLibCommon::sessionId_t nextSession;
        static std::map<SessionMapping_t::first_type,SessionMapping_t::second_type> SessionMap;
    public:
        static JClkLibCommon::sessionId_t CreateClientSession();
        static size_t GetSessionCount() { return SessionMap.size(); }
        static JClkLibCommon::sessionId_t GetSessionIdAt(size_t index);
        static ClientX GetClientSession(JClkLibCommon::sessionId_t sessionId);
    private:
        std::unique_ptr<JClkLibCommon::TransportTransmitterContext> transmitContext;
    public:
        void set_transmitContext(decltype(transmitContext)::pointer context)
        { this->transmitContext.reset(context); }
        auto get_transmitContext() { return transmitContext.get(); }
    };
}

#endif /* PROXY_CLIENT */
