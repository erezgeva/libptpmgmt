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
#include "proxy/connect_msg.hpp"
#include "proxy/notification_msg.hpp"
#include "proxy/subscribe_msg.hpp"
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

bool Client::init()
{
    // Register messages we recieve from client side
    reg_message_type<ProxyConnectMessage, ProxySubscribeMessage>();
    // ProxyNotificationMessage - Proxy send it only, never send from client
    Listener &rxContext = Listener::getSingleListenerInstance();
    PrintDebug("Initializing Proxy listener Queue ...");
    if(!rxContext.init(mqProxyName, MAX_CLIENT_COUNT)) {
        PrintError("Initializing Proxy listener queue failed");
        return false;
    }
    PrintDebug("Proxy listener queue opened");
    ConnectPtp4l::connect_ptp4l();
    #ifdef HAVE_LIBCHRONY
    ConnectChrony::connect_chrony();
    #endif
    return true;
}

void Client::NotifyClients(size_t timeBaseIndex,
    vector<sessionId_t> &subscribedClients,
    vector<sessionId_t> &sessionIdToRemove)
{
    ProxyNotificationMessage *pmsg = new ProxyNotificationMessage();
    if(pmsg == nullptr) {
        PrintErrorCode("[clkmgr::notify_client] notifyMsg is nullptr !!");
        return;
    }
    // Release message on function ends
    unique_ptr<ProxyNotificationMessage> notifyMsg(pmsg);
    PrintDebug("[clkmgr::notify_client] notifyMsg creation is OK !!");
    // Send data for multiple sessions
    pmsg->setTimeBaseIndex(timeBaseIndex);
    for(auto it = subscribedClients.begin(); it != subscribedClients.end();) {
        const sessionId_t sessionId = *it;
        PrintDebug("Get client session ID: " + to_string(sessionId));
        pmsg->set_sessionId(sessionId);
        if(!pmsg->transmitMessage()) {
            it = subscribedClients.erase(it);
            /* Add sessionId into the list to remove */
            sessionIdToRemove.push_back(sessionId);
        } else
            ++it;
    }
}

void Client::RemoveClients(const vector<sessionId_t> &sessionIdToRemove)
{
    for(const sessionId_t sessionId : sessionIdToRemove) {
        ConnectPtp4l::remove_ptp4l_subscriber(sessionId);
        #ifdef HAVE_LIBCHRONY
        ConnectChrony::remove_chrony_subscriber(sessionId);
        #endif
        Client::RemoveClient(sessionId);
    }
}

Transmitter *Transmitter::getTransmitterInstance(sessionId_t sessionId)
{
    return Client::getTxContext(sessionId);
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
