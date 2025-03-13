/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client utilities to setup and cleanup the library.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "pub/clockmanager.h"
#include "client/client_state.hpp"
#include "client/connect_msg.hpp"
#include "client/msgq_tport.hpp"
#include "client/notification_msg.hpp"
#include "client/subscribe_msg.hpp"
#include "client/timebase_configs.hpp"
#include "client/timebase_state.hpp"
#include "common/print.hpp"
#include "common/sighandler.hpp"

#include <chrono>
#include <cstring>
#include <rtpi/condition_variable.hpp>
#include <rtpi/mutex.hpp>

#define DEFAULT_LIVENESS_TIMEOUT_IN_MS 50  //50 ms
#define DEFAULT_CONNECT_TIME_OUT 5  //5 sec
#define DEFAULT_SUBSCRIBE_TIME_OUT 5  //5 sec

__CLKMGR_NAMESPACE_USE;

using namespace std;
using namespace std::chrono;

rtpi::mutex ClientConnectMessage::cv_mtx;
rtpi::condition_variable ClientConnectMessage::cv;
rtpi::mutex ClientSubscribeMessage::cv_mtx;
rtpi::condition_variable ClientSubscribeMessage::cv;

ClockManager::ClockManager() : implClientState(new ClientState()) {}

ClockManager::~ClockManager() = default;

ClockManager &ClockManager::FetchSingle()
{
    static ClockManager m_single;
    return m_single;
}

bool ClockManager::init()
{
    return true;
}

bool ClockManager::clkmgr_connect()
{
    // Send a connect message to Proxy Daemon
    Message0 connectMsg(new ClientConnectMessage());
    ClientConnectMessage *cmsg = dynamic_cast<decltype(cmsg)>(connectMsg.get());
    cmsg->setClientState(implClientState.get());
    if(!ClientMessage::init()) {
        PrintDebug("[CONNECT] Failed to initialize Client message.");
        return false;
    }
    if(!ClientTransport::init()) {
        PrintDebug("[CONNECT] Failed to initialize Client transportation.");
        return false;
    }
    ClientMessageQueue::writeTransportClientId(connectMsg.get());
    ClientMessageQueue::sendMessage(connectMsg.get());
    // Wait DEFAULT_CONNECT_TIME_OUT seconds for response from Proxy Daemon
    unsigned int timeout_sec = (unsigned int)DEFAULT_CONNECT_TIME_OUT;
    auto endTime = system_clock::now() + seconds(timeout_sec);
    unique_lock<rtpi::mutex> lck(ClientConnectMessage::cv_mtx);
    while(!implClientState->get_connected()) {
        auto res = ClientConnectMessage::cv.wait_until(lck, endTime);
        if(res == cv_status::timeout) {
            if(!implClientState->get_connected()) {
                PrintDebug("[CONNECT] Timeout waiting reply from Proxy.");
                return false;
            }
        } else
            PrintDebug("[CONNECT] Received reply from Proxy.");
    }
    // Store Client ID in Client State
    if((cmsg != nullptr) && !(cmsg->getClientId().empty())) {
        TransportClientId newClientID;
        strcpy((char *)newClientID.data(), (char *)cmsg->getClientId().data());
        implClientState->set_clientID(newClientID);
    }
    return true;
}

std::vector<TimeBaseCfg> ClockManager::clkmgr_get_timebase_cfgs() const
{
    return TimeBaseConfigurations::getInstance().getTimeBaseCfgs();
}

bool ClockManager::clkmgr_subscribe_by_name(const ClkMgrSubscription &newSub,
    char timeBaseName[STRING_SIZE_MAX], Event_state &currentState)
{
    int timeBaseIndex = -1;
    for(const auto &cfg : ClockManager::clkmgr_get_timebase_cfgs()) {
        if(strcmp(cfg.timeBaseName, timeBaseName) == 0)
            timeBaseIndex = cfg.timeBaseIndex;
    }
    if(timeBaseIndex == -1) {
        PrintDebug("[SUBSCRIBE] Invalid timeBaseName.");
        return false;
    }
    return clkmgr_subscribe(newSub, timeBaseIndex, currentState);
}

bool ClockManager::clkmgr_subscribe(const ClkMgrSubscription &newSub,
    int timeBaseIndex, Event_state &currentState)
{
    // Check whether connection between Proxy and Client is established or not
    if(!implClientState->get_connected()) {
        PrintDebug("[SUBSCRIBE] Client is not connected to Proxy.");
        return false;
    }
    // Check whether requested timeBaseIndex is valid or not
    auto &timeBaseConfigs = TimeBaseConfigurations::getInstance();
    if(!timeBaseConfigs.isTimeBaseIndexPresent(timeBaseIndex)) {
        PrintDebug("[SUBSCRIBE] Invalid timeBaseIndex.");
        return false;
    }
    // Store the event subscription in TimeBaseStates
    auto &states = TimeBaseStates::getInstance();
    states.setEventSubscription(timeBaseIndex, newSub);
    // Send a subscribe message to Proxy Daemon
    MessageX subscribeMsg(new ClientSubscribeMessage());
    ClientSubscribeMessage *cmsg = dynamic_cast<decltype(cmsg)>(subscribeMsg.get());
    if(cmsg == nullptr) {
        PrintDebug("[SUBSCRIBE] Failed to create subscribe message.");
        return false;
    }
    cmsg->setClientState(implClientState.get());
    cmsg->set_timeBaseIndex(timeBaseIndex);
    strcpy((char *)cmsg->getClientId().data(),
        (char *)implClientState->get_clientID().data());
    cmsg->set_sessionId(implClientState->get_sessionId());
    ClientMessageQueue::writeTransportClientId(subscribeMsg.get());
    ClientMessageQueue::sendMessage(subscribeMsg.get());
    // Wait DEFAULT_SUBSCRIBE_TIME_OUT seconds for response from Proxy Daemon
    unsigned int timeout_sec = (unsigned int) DEFAULT_SUBSCRIBE_TIME_OUT;
    auto endTime = system_clock::now() + seconds(timeout_sec);
    unique_lock<rtpi::mutex> lck(ClientSubscribeMessage::cv_mtx);
    while(!states.getSubscribed(timeBaseIndex)) {
        auto res = ClientSubscribeMessage::cv.wait_until(lck, endTime);
        if(res == cv_status::timeout) {
            if(!states.getSubscribed(timeBaseIndex)) {
                PrintDebug("[SUBSCRIBE] Timeout waiting reply from Proxy.");
                return false;
            }
        } else
            PrintDebug("[SUBSCRIBE] Received reply from Proxy.");
    }
    // Get the current state of the timebase
    TimeBaseState state;
    if(!states.getTimeBaseState(timeBaseIndex, state)) {
        PrintDebug("[SUBSCRIBE] Failed to get specific timebase state.");
        return false;
    }
    currentState = state.get_eventState();
    return true;
}

bool ClockManager::clkmgr_disconnect()
{
    // Send a disconnect message
    if(!ClientTransport::stop()) {
        PrintDebug("Client Stop Failed");
        return false;
    }
    if(!ClientTransport::finalize()) {
        PrintDebug("Client Finalize Failed");
        return false;
    }
    return true;
}

int64_t timespec_delta(const timespec &last_notification_time,
    const timespec &current_time)
{
    int64_t delta_in_sec = current_time.tv_sec - last_notification_time.tv_sec;
    int64_t delta_in_nsec = current_time.tv_nsec - last_notification_time.tv_nsec;
    int64_t delta_in_ms = delta_in_sec * 1000LL + delta_in_nsec / 1000000LL;
    return delta_in_ms;
}

bool check_proxy_liveness(ClientState &implClientState, int timeBaseIndex)
{
    auto &states = TimeBaseStates::getInstance();
    timespec lastNotificationTime;
    timespec currentTime;
    int64_t timeout = 0;
    if(clock_gettime(CLOCK_REALTIME, &currentTime) == -1) {
        PrintDebug("[WAIT] Failed to get currentTime.");
        goto send_connect;
    }
    if(!states.getLastNotificationTime(timeBaseIndex, lastNotificationTime)) {
        PrintDebug("[WAIT] Failed to get lastNotificationTime.");
        goto send_connect;
    }
    timeout = timespec_delta(lastNotificationTime, currentTime);
    if(timeout < DEFAULT_LIVENESS_TIMEOUT_IN_MS)
        return true;
send_connect:
    Message0 connectMsg(new ClientConnectMessage());
    ClientConnectMessage *cmsg = dynamic_cast<decltype(cmsg)>(connectMsg.get());
    if(cmsg == nullptr) {
        PrintDebug("[WAIT] Failed to cast to ClientConnectMessage");
        return false;
    }
    implClientState.set_connected(false);
    cmsg->setClientState(&implClientState);
    cmsg->set_sessionId(implClientState.get_sessionId());
    ClientMessageQueue::writeTransportClientId(connectMsg.get());
    ClientMessageQueue::sendMessage(connectMsg.get());
    /* Wait for connection result */
    auto endTime = system_clock::now() +
        milliseconds(DEFAULT_LIVENESS_TIMEOUT_IN_MS);
    unique_lock<rtpi::mutex> lck(ClientConnectMessage::cv_mtx);
    while(!implClientState.get_connected()) {
        auto res = ClientConnectMessage::cv.wait_until(lck, endTime);
        if(res == cv_status::timeout) {
            if(!implClientState.get_connected()) {
                PrintDebug("[CONNECT] Timeout waiting reply from Proxy.");
                return false;
            }
        } else
            PrintDebug("[CONNECT] Received reply from Proxy.");
    }
    return true;
}

int ClockManager::clkmgr_status_wait_by_name(int timeout,
    char timeBaseName[STRING_SIZE_MAX],
    Event_state &currentState, Event_count &currentCount)
{
    int timeBaseIndex = -1;
    for(const auto &cfg : ClockManager::clkmgr_get_timebase_cfgs()) {
        if(strcmp(cfg.timeBaseName, timeBaseName) == 0)
            timeBaseIndex = cfg.timeBaseIndex;
    }
    if(timeBaseIndex == -1) {
        PrintDebug("[SUBSCRIBE] Invalid timeBaseName.");
        return -1;
    }
    return clkmgr_status_wait(timeout, timeBaseIndex, currentState, currentCount);
}

int ClockManager::clkmgr_status_wait(int timeout, int timeBaseIndex,
    Event_state &currentState, Event_count &currentCount)
{
    // Check whether connection between Proxy and Client is established or not
    if(!implClientState->get_connected()) {
        PrintDebug("[WAIT] Client is not connected to Proxy.");
        return false;
    }
    // Check whether requested timeBaseIndex is subscribed or not
    auto &states = TimeBaseStates::getInstance();
    if(!states.getSubscribed(timeBaseIndex)) {
        PrintDebug("[WAIT] Invalid timeBaseIndex.");
        return false;
    }
    auto start = high_resolution_clock::now();
    auto end = (timeout == -1) ?  time_point<high_resolution_clock>::max() :
        start + seconds(timeout);
    bool event_changes_detected = false;
    TimeBaseState state;
    do {
        // Check liveness of the Proxy Daemon
        if(!check_proxy_liveness(*implClientState, timeBaseIndex)) {
            PrintDebug("[WAIT] Proxy Daemon is not alive.");
            return -1;
        }
        // Get the current state of the timebase
        if(!states.getTimeBaseState(timeBaseIndex, state)) {
            PrintDebug("[WAIT] Failed to get specific timebase state.");
            return -1;
        }
        // Check whether there is any changes on the event state
        if(state.is_event_changed()) {
            event_changes_detected = true;
            break;
        }
        // Sleep for a short duration before the next iteration
        this_thread::sleep_for(milliseconds(10));
    } while(high_resolution_clock::now() < end);
    /* Copy out the current state */
    currentCount = state.get_eventStateCount();
    currentState = state.get_eventState();
    if(!event_changes_detected)
        return 0;
    return 1;
}

bool ClockManager::clkmgr_gettime(timespec &ts)
{
    return clock_gettime(CLOCK_REALTIME, &ts) == 0;
}
