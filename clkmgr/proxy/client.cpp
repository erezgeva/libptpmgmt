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
#include "common/termin.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

static sessionId_t nextSession = 0;
static map<sessionId_t, unique_ptr<Client>> sessionMap;

static inline Transmitter *CreateTransmitterContext(const ClientId &clientId)
{
    string id((const char *)clientId.data());
    Transmitter *nCtx = new Transmitter();
    if(nCtx != nullptr) {
        if(!nCtx->open(id, false)) {
            PrintErrorCode("Failed to open message queue " + id);
            delete nCtx;
            return nullptr;
        }
        PrintDebug("Successfully connected to client " + id);
    } else
        PrintError("Failed to allocate new message queue " + id);
    return nCtx;
}

bool Client::existClient(sessionId_t sessionId)
{
    return sessionMap.count(sessionId) > 0;
}

Client *Client::getClient(sessionId_t sessionId)
{
    return sessionMap.count(sessionId) > 0 ? sessionMap[sessionId].get() : nullptr;
}

Transmitter *Client::getTxContext(sessionId_t sessionId)
{
    Client *client = getClient(sessionId);
    return client != nullptr ?  client->getTxContext() : nullptr;
}

sessionId_t Client::CreateClientSession(const ClientId &id)
{
    for(; sessionMap.count(nextSession) > 0 ||
        nextSession == InvalidSessionId; nextSession++);
    Client *client = new Client;
    if(client == nullptr)
        return InvalidSessionId;
    Transmitter *tx = CreateTransmitterContext(id);
    if(tx == nullptr) {
        delete client;
        return InvalidSessionId;
    }
    client->m_sessionId = nextSession;
    client->m_transmitContext.reset(tx);
    sessionMap[nextSession].reset(client);
    return nextSession++;
}

void Client::RemoveClientSession(sessionId_t sessionId)
{
    Transmitter *tx = getTxContext(sessionId);
    if(tx != nullptr) {
        tx->finalize();
        sessionMap.erase(sessionId);
    }
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
