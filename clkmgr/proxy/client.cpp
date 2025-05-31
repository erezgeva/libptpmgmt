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
#include "proxy/config_parser.hpp"
#include "proxy/connect_msg.hpp"
#include "proxy/notification_msg.hpp"
#include "proxy/subscribe_msg.hpp"
#include "common/termin.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

static thread_local Buffer notifyBuff;
static sessionId_t nextSession = 0;
static rtpi::mutex sessionMapLock;
static map<sessionId_t, unique_ptr<Client>> sessionMap;
struct perTimeBase {
    map<sessionId_t, bool> clients;
    rtpi::mutex lock;
};
// Map of all subscriped clients to a timeBaseIndex of and PTP or chrony
static map<size_t, perTimeBase> timeBaseClients;
static map<size_t, ptp_event> ptp4lEvents;
// Lock for ptp4lEvents
static map<size_t, rtpi::mutex> timeBaseLock;

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

rtpi::mutex &Client::getTimeBaseLock(size_t timeBaseIndex)
{
    if(timeBaseLock.count(timeBaseIndex) > 0)
        return timeBaseLock[timeBaseIndex];
    static rtpi::mutex dummy;
    return dummy;
}

ptp_event &Client::getPTPEvent(size_t timeBaseIndex)
{
    if(ptp4lEvents.count(timeBaseIndex) > 0)
        return ptp4lEvents[timeBaseIndex];
    static ptp_event dummy;
    return dummy;
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
    unique_lock<rtpi::mutex> local(sessionMapLock);
    if(sessionId != InvalidSessionId) {
        if(existClient(sessionId))
            return sessionId;
        local.unlock(); // Explicitly unlock the mutex
        PrintError("Session ID does not exists: " + to_string(sessionId));
        return InvalidSessionId;
    }
    sessionId = CreateClientSession(id);
    local.unlock(); // Explicitly unlock the mutex
    if(sessionId == InvalidSessionId)
        PrintError("Fail to allocate new session");
    else
        PrintDebug("Created new client session ID: " + to_string(sessionId));
    return sessionId;
}

bool Client::subscribe(size_t timeBaseIndex, sessionId_t sessionId)
{
    if(sessionId == InvalidSessionId ||
        timeBaseClients.count(timeBaseIndex) == 0)
        return false;
    unique_lock<rtpi::mutex> local(sessionMapLock);
    if(!existClient(sessionId)) {
        local.unlock(); // Explicitly unlock the mutex
        PrintError("Session ID " + to_string(sessionId) + " is not registered");
        return false;
    }
    local.unlock(); // Explicitly unlock the mutex
    PrintDebug("[ProxySubscribeMessage]::parseBufferTail - "
        "Use current client session ID: " + to_string(sessionId));
    unique_lock<rtpi::mutex> localSub(timeBaseClients[timeBaseIndex].lock);
    bool exist = timeBaseClients[timeBaseIndex].clients.count(sessionId) > 0;
    if(!exist)
        timeBaseClients[timeBaseIndex].clients[sessionId] = true;
    localSub.unlock(); // Explicitly unlock the mutex
    if(exist)
        PrintDebug("sessionId " + to_string(sessionId) + " is already subscribe");
    return true;
}

void Client::RemoveClient(sessionId_t sessionId)
{
    unique_lock<rtpi::mutex> local(sessionMapLock);
    Client *client = getClient(sessionId);
    if(client == nullptr)
        return;
    Transmitter *tx = client->getTxContext();
    if(tx != nullptr)
        tx->finalize();
    sessionMap.erase(sessionId);
    local.unlock(); // Explicitly unlock the mutex
    for(auto &base : timeBaseClients) {
        auto &rec = base.second;
        unique_lock<rtpi::mutex> localSub(rec.lock);
        rec.clients.erase(sessionId);
    }
}

Transmitter *Client::getTxContext(sessionId_t sessionId)
{
    if(sessionId == InvalidSessionId)
        return nullptr;
    unique_lock<rtpi::mutex> local(sessionMapLock);
    Client *client = getClient(sessionId);
    return client != nullptr ? client->getTxContext() : nullptr;
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
    // Ensure timeBase subscribed clients have all existing timeBaseIndexes
    for(const auto &param : JsonConfigParser::getInstance()) {
        timeBaseClients[param.base.timeBaseIndex];
        timeBaseLock[param.base.timeBaseIndex];
        ptp4lEvents[param.base.timeBaseIndex];
    }
    return connect_ptp4l()
        #ifdef HAVE_LIBCHRONY
        && connect_chrony()
        #endif
        ;
}

void Client::NotifyClients(size_t timeBaseIndex)
{
    PrintDebug("Client::NotifyClients");
    if(timeBaseClients.count(timeBaseIndex) == 0) {
        PrintError("[Client::NotifyClients] call with non exist timeBaseIndex " +
            to_string(timeBaseIndex));
        return;
    }
    vector<sessionId_t> sessionIdToRemove;
    unique_lock<rtpi::mutex> local(timeBaseClients[timeBaseIndex].lock);
    ProxyNotificationMessage *pmsg = new ProxyNotificationMessage();
    if(pmsg == nullptr) {
        local.unlock(); // Explicitly unlock the mutex
        PrintErrorCode("[Client::NotifyClients] notifyMsg is nullptr !!");
        return;
    }
    // Release message on function ends
    unique_ptr<ProxyNotificationMessage> notifyMsg(pmsg);
    // Send data for multiple sessions
    pmsg->setTimeBaseIndex(timeBaseIndex);
    if(!pmsg->makeBuffer(notifyBuff)) {
        local.unlock(); // Explicitly unlock the mutex
        PrintError("[Client::NotifyClients] Failed to create message");
        return;
    }
    for(auto &c : timeBaseClients[timeBaseIndex].clients) {
        const sessionId_t sessionId = c.first;
        PrintDebug("[Client::NotifyClients] Get client session ID: " +
            to_string(sessionId));
        Transmitter *ptxContext = getTxContext(sessionId);
        if(ptxContext == nullptr || !ptxContext->sendBuffer(notifyBuff))
            // Add sessionId into the list to remove
            sessionIdToRemove.push_back(sessionId);
    }
    local.unlock(); // Explicitly unlock the mutex
    for(const sessionId_t sessionId : sessionIdToRemove)
        Client::RemoveClient(sessionId);
}

Transmitter *Transmitter::getTransmitterInstance(sessionId_t sessionId)
{
    return Client::getTxContext(sessionId);
}

__CLKMGR_NAMESPACE_BEGIN

class ClientRemoveAll : public End
{
  public:
    // TODO : do we need to send a disconnect message to the clients?
    // Here is our opportunity :-)
    bool stop() override final { return true; }
    bool finalize() override final {
        unique_lock<rtpi::mutex> local(sessionMapLock);
        for(auto &it : sessionMap)
            it.second.get()->getTxContext()->finalize();
        sessionMap.clear();
        return true;
    }
};
static ClientRemoveAll endClients;

__CLKMGR_NAMESPACE_END
