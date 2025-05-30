/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Common client infomation implementation
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "proxy/client.hpp"
#include "proxy/connect_ptp4l.hpp"
#ifdef HAVE_LIBCHRONY
#include "proxy/connect_chrony.hpp"
#endif
#include "common/termin.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

static sessionId_t nextSession = 0;
static map<sessionId_t, unique_ptr<Client>> sessionMap;

static inline Transmitter *CreateTransmitterContext(const string &clientId)
{
    Transmitter *nCtx = new Transmitter();
    if(nCtx != nullptr) {
        if(!nCtx->open(clientId, false)) {
            PrintErrorCode("Failed to open message queue " + clientId);
            delete nCtx;
            return nullptr;
        }
        PrintDebug("Successfully connected to client " + clientId);
    } else
        PrintError("Failed to allocate new message queue " + clientId);
    return nCtx;
}

static inline bool existClient(sessionId_t sessionId)
{
    return sessionMap.count(sessionId) > 0;
}

sessionId_t Client::CreateClientSession(const string &id)
{
    while(sessionMap.count(nextSession) > 0) {
        ++nextSession;
        nextSession &= ValidMaskSessionId;
    }
    Client *client = new Client;
    if(client == nullptr)
        return InvalidSessionId;
    Transmitter *tx = CreateTransmitterContext(id);
    if(tx == nullptr) {
        delete client;
        return InvalidSessionId;
    }
    sessionId_t cur = nextSession;
    client->m_sessionId = cur;
    client->m_transmitContext.reset(tx);
    sessionMap[cur].reset(client);
    ++nextSession;
    nextSession &= ValidMaskSessionId;
    return cur;
}

sessionId_t Client::connect(sessionId_t sessionId, const string &id)
{
    if(sessionId != InvalidSessionId) {
        if(existClient(sessionId))
            return sessionId;
        PrintError("Session ID does not exists: " + to_string(sessionId));
        return InvalidSessionId;
    }
    sessionId = CreateClientSession(id);
    if(sessionId == InvalidSessionId)
        PrintError("Fail to allocate new session");
    else
        PrintDebug("Created new client session ID: " + to_string(sessionId));
    return sessionId;
}

bool Client::subscribe(size_t timeBaseIndex, sessionId_t sessionId)
{
    if(!existClient(sessionId)) {
        PrintError("Session ID " + to_string(sessionId) + " is not registered");
        return false;
    }
    ConnectPtp4l::subscribe_ptp4l(timeBaseIndex, sessionId);
    #ifdef HAVE_LIBCHRONY
    ConnectChrony::subscribe_chrony(timeBaseIndex, sessionId);
    #endif
    PrintDebug("[ProxySubscribeMessage]::parseBufferTail - "
        "Use current client session ID: " + to_string(sessionId));
    return true;
}

void Client::RemoveClient(sessionId_t sessionId)
{
    Transmitter *tx = getTxContext(sessionId);
    if(tx != nullptr) {
        tx->finalize();
        sessionMap.erase(sessionId);
    }
}

Transmitter *Client::getTxContext(sessionId_t sessionId)
{
    Client *client = getClient(sessionId);
    return client != nullptr ?  client->getTxContext() : nullptr;
}

Client *Client::getClient(sessionId_t sessionId)
{
    return sessionMap.count(sessionId) > 0 ? sessionMap[sessionId].get() : nullptr;
}

class ClientRemoveAll : public End
{
  public:
    // TODO : do we need to send a disconnect message to the clients?
    // Here is our opportunity :-)
    bool stop() override final { return true; }
    bool finalize() override final {
        for(auto &it : sessionMap)
            it.second.get()->getTxContext()->finalize();
        sessionMap.clear();
        return true;
    }
};
static ClientRemoveAll endClients;
