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
#include "common/shared_mutex.hpp" // Replace C++17 <shared_mutex>
#include "common/termin.hpp"
#include "common/print.hpp"

#include <map>
#include <cstring>
#include <rtpi/mutex.hpp>

__CLKMGR_NAMESPACE_USE;

using namespace std;

// Define the static clockType member variable
ClockEventHandler::ClockType Client::clockType = ClockEventHandler::PTPClock;

#ifdef HAVE_LIBCHRONY
#define CHRONY_INIT && connect_chrony()
#else
#define CHRONY_INIT
#endif

static thread_local Buffer notifyBuff;
static sessionId_t nextSession = 0;
static rtpi::mutex sessionMapLock;
static map<sessionId_t, unique_ptr<Client>> sessionMap;
struct perTimeBase {
    map<sessionId_t, bool> clients;
    shared_mutex clLock;
};
// Map of all subscriped clients using a timeBaseIndex of PTP and chrony
static map<size_t, perTimeBase> timeBaseClients;
static map<size_t, ptp_event> ptp4lEvents;
static map<size_t, chrony_event> chronyEvents;
// Lock for ptp4lEvents
static map<size_t, rtpi::mutex> timeBaseLock;

static inline Transmitter *CreateTransmitter(const string &clientId)
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

void Client::getPTPEvent(size_t timeBaseIndex, ptp_event &event)
{
    if(ptp4lEvents.count(timeBaseIndex) > 0) {
        unique_lock<rtpi::mutex> eventLock(timeBaseLock[timeBaseIndex]);
        event = ptp4lEvents[timeBaseIndex];
    } else {
        static ptp_event dummy = { 0 };
        event = dummy;
    }
}

void Client::getChronyEvent(size_t timeBaseIndex, chrony_event &event)
{
    if(chronyEvents.count(timeBaseIndex) > 0) {
        unique_lock<rtpi::mutex> eventLock(timeBaseLock[timeBaseIndex]);
        event = chronyEvents[timeBaseIndex];
    } else {
        static chrony_event dummy = { 0 };
        event = dummy;
    }
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
    Transmitter *tx = CreateTransmitter(id);
    if(tx == nullptr) {
        delete client;
        return InvalidSessionId;
    }
    sessionId_t cur = nextSession;
    client->m_sessionId = cur;
    client->m_transmitter.reset(tx);
    sessionMap[cur].reset(client);
    ++nextSession;
    nextSession &= ValidMaskSessionId;
    return cur;
}

sessionId_t Client::connect(sessionId_t sessionId, const string &id)
{
    unique_lock<rtpi::mutex> mapLock(sessionMapLock);
    if(sessionId != InvalidSessionId) {
        if(existClient(sessionId))
            return sessionId;
        mapLock.unlock(); // Explicitly unlock the mutex
        PrintError("Session ID does not exists: " + to_string(sessionId));
        return InvalidSessionId;
    }
    sessionId = CreateClientSession(id);
    mapLock.unlock(); // Explicitly unlock the mutex
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
    unique_lock<rtpi::mutex> mapLock(sessionMapLock);
    if(!existClient(sessionId)) {
        mapLock.unlock(); // Explicitly unlock the mutex
        PrintError("Session ID " + to_string(sessionId) + " is not registered");
        return false;
    }
    mapLock.unlock(); // Explicitly unlock the mutex
    PrintDebug("[ProxySubscribeMessage]::parseBufferTail - "
        "Use current client session ID: " + to_string(sessionId));
    unique_lock<shared_mutex> clLock(timeBaseClients[timeBaseIndex].clLock);
    bool exist = timeBaseClients[timeBaseIndex].clients.count(sessionId) > 0;
    if(!exist)
        timeBaseClients[timeBaseIndex].clients[sessionId] = true;
    clLock.unlock(); // Explicitly unlock the mutex
    if(exist)
        PrintDebug("sessionId " + to_string(sessionId) + " is already subscribe");
    return true;
}

void Client::RemoveClient(sessionId_t sessionId)
{
    unique_lock<rtpi::mutex> mapLock(sessionMapLock);
    Client *client = getClient(sessionId);
    if(client != nullptr) {
        Transmitter *tx = client->getTransmitter();
        if(tx != nullptr)
            tx->finalize();
    }
    sessionMap.erase(sessionId);
    mapLock.unlock(); // Explicitly unlock the mutex
    for(auto &base : timeBaseClients) {
        auto &rec = base.second;
        unique_lock<shared_mutex> clLock(rec.clLock);
        rec.clients.erase(sessionId);
    }
}

Transmitter *Client::getTransmitter(sessionId_t sessionId)
{
    if(sessionId == InvalidSessionId)
        return nullptr;
    unique_lock<rtpi::mutex> mapLock(sessionMapLock);
    Client *client = getClient(sessionId);
    return client != nullptr ? client->getTransmitter() : nullptr;
}

Client *Client::getClient(sessionId_t sessionId)
{
    return sessionMap.count(sessionId) > 0 ? sessionMap[sessionId].get() : nullptr;
}

bool Client::init(bool useMsgQAllAccess)
{
    // Register messages we recieve from client side
    reg_message_type<ProxyConnectMessage, ProxySubscribeMessage>();
    // ProxyNotificationMessage - Proxy send it only, never send from client
    Listener &rx = Listener::getSingleListenerInstance();
    PrintDebug("Initializing Proxy listener Queue ...");
    if(!rx.init(mqProxyName, MAX_CLIENT_COUNT, useMsgQAllAccess)) {
        PrintError("Initializing Proxy listener queue failed");
        return false;
    }
    PrintDebug("Proxy listener queue opened");
    return connect_ptp4l() CHRONY_INIT;
}

void Client::NotifyClients(size_t timeBaseIndex)
{
    PrintDebug("Client::NotifyClients");
    if(timeBaseClients.count(timeBaseIndex) == 0) {
        PrintError("[Client::NotifyClients] call with non exist timeBaseIndex " +
            to_string(timeBaseIndex));
        return;
    }
    ProxyNotificationMessage *pmsg = new ProxyNotificationMessage();
    if(pmsg == nullptr) {
        PrintErrorCode("[Client::NotifyClients] notifyMsg is nullptr !!");
        return;
    }
    // Release message on function ends
    unique_ptr<ProxyNotificationMessage> notifyMsg(pmsg);
    // Send data for multiple sessions
    pmsg->setTimeBaseIndex(timeBaseIndex);
    if(!pmsg->makeBuffer(notifyBuff)) {
        PrintError("[Client::NotifyClients] Failed to create message");
        return;
    }
    vector<sessionId_t> sessionIdToRemove;
    {
        // Use share lock, allow all readers to share
        shared_lock<shared_mutex> clLock(timeBaseClients[timeBaseIndex].clLock);
        for(auto &c : timeBaseClients[timeBaseIndex].clients) {
            const sessionId_t sessionId = c.first;
            PrintDebug("[Client::NotifyClients] Get client session ID: " +
                to_string(sessionId));
            Transmitter *ptx = getTransmitter(sessionId);
            if(ptx == nullptr || !ptx->sendBuffer(notifyBuff))
                // Add sessionId into the list to remove
                sessionIdToRemove.push_back(sessionId);
        }
    }
    for(const sessionId_t sessionId : sessionIdToRemove)
        Client::RemoveClient(sessionId);
}

ptpEvent::ptpEvent(size_t index) : timeBaseIndex(index)
{
    timeBaseClients[index]; // Ensure we have subscribed clients list
    portClear();
    copy();
}

void ptpEvent::copy()
{
    unique_lock<rtpi::mutex> eventLock(timeBaseLock[timeBaseIndex]);
    ptp_event &to = ptp4lEvents[timeBaseIndex];
    to.master_offset = master_offset;
    to.as_capable = as_capable;
    to.ptp4l_sync_interval = ptp4l_sync_interval;
    to.synced_to_primary_clock = synced_to_primary_clock;
    memcpy(to.gm_identity, gm_identity, sizeof(gm_identity));
}

chronyEvent::chronyEvent(size_t index) : timeBaseIndex(index)
{
    timeBaseClients[index]; // Ensure we have subscribed clients list
    clear();
}

void chronyEvent::copy()
{
    unique_lock<rtpi::mutex> eventLock(timeBaseLock[timeBaseIndex]);
    chrony_event &to = chronyEvents[timeBaseIndex];
    to.chrony_offset = chrony_offset;
    to.chrony_reference_id = chrony_reference_id;
    to.polling_interval = polling_interval;
}

Transmitter *Transmitter::getTransmitterInstance(sessionId_t sessionId)
{
    return Client::getTransmitter(sessionId);
}

__CLKMGR_NAMESPACE_BEGIN

class ClientRemoveAll : public End
{
  public:
    // TODO : do we need to send a disconnect message to the clients?
    // Here is our opportunity :-)
    bool stop() override final { return true; }
    bool finalize() override final {
        unique_lock<rtpi::mutex> mapLock(sessionMapLock);
        for(auto &it : sessionMap)
            it.second.get()->getTransmitter()->finalize();
        sessionMap.clear();
        return true;
    }
};
static ClientRemoveAll endClients;

__CLKMGR_NAMESPACE_END
