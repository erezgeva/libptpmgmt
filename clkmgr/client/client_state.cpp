/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Set and get the client subscribe event state
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "client/client_state.hpp"
#include "client/connect_msg.hpp"
#include "client/notification_msg.hpp"
#include "client/subscribe_msg.hpp"
#include "client/disconnect_msg.hpp"
#include "common/termin.hpp"
#include "common/print.hpp"

#include <unistd.h>
#include <rtpi/condition_variable.hpp>

__CLKMGR_NAMESPACE_USE;

using namespace std;
using namespace std::chrono;

DECLARE_STATIC(ClientState::m_clientID);
DECLARE_STATIC(ClientState::m_sessionId, InvalidSessionId);
DECLARE_STATIC(ClientState::m_connected, false);

static thread_local Buffer txBuff;
static Transmitter txContext;
static rtpi::mutex connect_cv_mtx;
static rtpi::condition_variable connect_cv;

bool ClientState::init()
{
    PrintDebug("Initializing Client Message");
    reg_message_type<ClientConnectMessage, ClientSubscribeMessage,
                     ClientNotificationMessage, ClientDisconnectMessage>();
    Listener &rx = Listener::getSingleListenerInstance();
    /* Two outstanding messages per client */
    PrintDebug("Initializing Client Queue ...");
    string mqListenerName(mqProxyName);
    mqListenerName += "." + to_string(getpid());
    if(!txContext.open(mqProxyName)) {
        PrintErrorCode("Failed to open transmitter queue: " + mqProxyName);
        return false;
    }
    if(!rx.init(mqListenerName, 2)) {
        PrintError("Failed to open listener queue");
        return false;
    }
    PrintDebug("Client Message queue opened");
    m_clientID = rx.getClientId();
    return true;
}

bool ClientState::connect(uint32_t timeOut, timespec *lastConnectTime)
{
    ClientConnectMessage *cmsg = new ClientConnectMessage();
    if(cmsg == nullptr) {
        PrintDebug("[CONNECT] Failed to allocate ClientConnectMessage");
        return false;
    }
    unique_ptr<Message> connectMsg(cmsg);
    set_connected(false);
    cmsg->setClientId(m_clientID);
    cmsg->set_sessionId(get_sessionId());
    sendMessage(*cmsg);
    auto endTime = system_clock::now() + milliseconds(timeOut);
    unique_lock<rtpi::mutex> lock(connect_cv_mtx);
    while(!get_connected()) {
        auto res = connect_cv.wait_until(lock, endTime);
        if(res == cv_status::timeout) {
            if(!get_connected()) {
                PrintDebug("[CONNECT] Timeout waiting reply from Proxy.");
                return false;
            }
        } else {
            // Store the last connect time
            if(lastConnectTime != nullptr &&
                clock_gettime(CLOCK_REALTIME, lastConnectTime) == -1)
                PrintDebug("[CONNECT] Failed to get lastConnectTime.");
            PrintDebug("[CONNECT] Received reply from Proxy: " +
                to_string(get_connected()));
        }
    }
    return true;
}

bool ClientState::notifyDisconnect()
{
    ClientDisconnectMessage *cmsg = new ClientDisconnectMessage();
    if(cmsg == nullptr) {
        PrintDebug("[DISCONNECT] Failed to allocate ClientDisconnectMessage");
        return false;
    }
    unique_ptr<Message> disconnectMsg(cmsg);
    cmsg->set_sessionId(get_sessionId());
    if(!sendMessage(*cmsg))
        return false;
    set_connected(false);
    PrintDebug("[DISCONNECT] Disconnect message sent successfully to Proxy.");
    return true;
}

bool ClientState::connectReply(sessionId_t sessionId)
{
    PrintDebug("Processing client connect message (reply)");
    PrintDebug("Connected with session ID: " + to_string(sessionId));
    PrintDebug("Current state.sessionId: " + to_string(get_sessionId()));
    unique_lock<rtpi::mutex> lock(connect_cv_mtx);
    set_connected(true);
    m_sessionId = sessionId;
    connect_cv.notify_one(lock);
    return true;
}

bool ClientState::sendMessage(Message &msg)
{
    if(!msg.makeBuffer(txBuff) || !txContext.sendBuffer(txBuff))
        return false;
    PrintDebug("[ClientState]::sendMessage successful");
    return true;
}

Transmitter *Transmitter::getTransmitterInstance(sessionId_t sessionId)
{
    return &txContext;
}

class ClientStateEnd : public End
{
    bool stop() override final { return true; }
    bool finalize() override final {
        PrintDebug("Transmitter Queue = " + txContext.getQueueName());
        return txContext.finalize();
    }
};
static ClientStateEnd endClient;
