/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file client.cpp
 * @brief Common client infomation implementation
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#include <common/print.hpp>
#include <proxy/client.hpp>

using namespace JClkLibCommon;
using namespace JClkLibProxy;
using namespace std;

DECLARE_STATIC(Client::nextSession,sessionId_t(InvalidSessionId+1));
DECLARE_STATIC(Client::SessionMap);

sessionId_t Client::CreateClientSession()
{
    for (auto iter = SessionMap.find(nextSession);
        nextSession != InvalidSessionId && iter != SessionMap.cend();
        iter = SessionMap.find(++nextSession));
    SessionMap.emplace(SessionMapping_t(nextSession,new Client()));
    return nextSession;
}

sessionId_t Client::GetSessionIdAt(size_t index)
{
    if (index < SessionMap.size()) {
        auto iter = SessionMap.begin();
        std::advance(iter, index);
        return iter->first;
    }
    return InvalidSessionId;
}

ClientX Client::GetClientSession(sessionId_t sessionId)
{
    auto iter = SessionMap.find(sessionId);
    if (iter == SessionMap.cend()) {
        PrintError("Session ID " + to_string(sessionId) + " not found");
        return ClientX(NULL);
    }

    return iter->second;
}
