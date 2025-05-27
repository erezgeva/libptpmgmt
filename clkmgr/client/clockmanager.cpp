/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client utilities to setup and cleanup the library.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "pub/clockmanager.h"
#include "pub/clkmgr/timebase_configs.h"
#include "client/msgq_tport.hpp"
#include "client/subscribe_msg.hpp"
#include "client/timebase_state.hpp"
#include "common/print.hpp"

#include <chrono>
#include <thread>
#include <rtpi/condition_variable.hpp>

// Number of microsecond in a second
static const int32_t USEC_PER_SEC = 1000000;

// in miliseconds
static const uint32_t DEFAULT_LIVENESS_TIMEOUT_IN_MS = 200;
static const uint32_t DEFAULT_CONNECT_TIME_OUT = USEC_PER_SEC * 5;
#define DEFAULT_SUBSCRIBE_TIME_OUT 5  //5 sec

__CLKMGR_NAMESPACE_USE;

using namespace std;
using namespace std::chrono;

static atomic_bool doInit(false);

rtpi::mutex ClientSubscribeMessage::cv_mtx;
rtpi::condition_variable ClientSubscribeMessage::cv;

ClockManager &ClockManager::fetchSingleInstance()
{
    static ClockManager m_single;
    return m_single;
}

bool ClockManager::connect()
{
    if(!doInit.load()) {
        // Send a connect message to Proxy Daemon
        if(!clientMessageRegister()) {
            PrintDebug("[CONNECT] Failed to initialize Client message.");
            return false;
        }
        if(!ClientQueue::init()) {
            PrintDebug("[CONNECT] Failed to initialize Client queue.");
            return false;
        }
        doInit.store(true);
    }
    return ClientState::getSingleInstance().connect(DEFAULT_CONNECT_TIME_OUT);
}

const TimeBaseConfigurations &ClockManager::getTimebaseCfgs()
{
    return TimeBaseConfigurations::getInstance();
}

bool ClockManager::subscribeByName(const ClkMgrSubscription &newSub,
    const string &timeBaseName, ClockSyncData &clockSyncData)
{
    size_t timeBaseIndex = 0;
    if(!TimeBaseConfigurations::BaseNameToBaseIndex(timeBaseName, timeBaseIndex)) {
        PrintDebug("[SUBSCRIBE] Invalid timeBaseName.");
        return false;
    }
    return subscribe(newSub, timeBaseIndex, clockSyncData);
}
bool ClockManager::subscribe(const ClkMgrSubscription &newSub,
    size_t timeBaseIndex, ClockSyncData &clockSyncData)
{
    ClientState &implClientState = ClientState::getSingleInstance();
    // Check whether connection between Proxy and Client is established or not
    if(!implClientState.get_connected()) {
        PrintDebug("[SUBSCRIBE] Client is not connected to Proxy.");
        return false;
    }
    // Check whether requested timeBaseIndex is valid or not
    if(!TimeBaseConfigurations::isTimeBaseIndexPresent(timeBaseIndex)) {
        PrintDebug("[SUBSCRIBE] Invalid timeBaseIndex.");
        return false;
    }
    // Store the event subscription in TimeBaseStates
    auto &states = TimeBaseStates::getInstance();
    states.setEventSubscription(timeBaseIndex, newSub);
    // Send a subscribe message to Proxy Daemon
    ClientSubscribeMessage *cmsg = new ClientSubscribeMessage();
    if(cmsg == nullptr) {
        PrintDebug("[SUBSCRIBE] Failed to create subscribe message.");
        return false;
    }
    unique_ptr<Message> subscribeMsg(cmsg);
    cmsg->set_timeBaseIndex(timeBaseIndex);
    cmsg->set_sessionId(implClientState.get_sessionId());
    ClientQueue::sendMessage(cmsg);
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
    ClockSyncBaseHandler handler(clockSyncData);
    // TODO: check ptp4l and chrony data is received
    PTPClockEvent &ptpData = state.get_ptp4lEventState();
    handler.setPTPAvailability(true);
    handler.updatePTPClock(ptpData);
    SysClockEvent &chronydata = state.get_chronyEventState();
    handler.setSysAvailability(true);
    handler.updateSysClock(chronydata);
    return true;
}

bool ClockManager::disconnect()
{
    // Send a disconnect message - TODO do we want to send a message here?
    if(doInit.load()) {
        if(!End::stopAll()) {
            PrintDebug("Client disconnect Failed");
            return false;
        }
        doInit.store(true);
    }
    return true;
}

// Calculate delta in miliseconds
static inline int64_t timespec_delta(const timespec &last, const timespec &cur)
{
    return (cur.tv_sec - last.tv_sec) * 1000LL +
        (cur.tv_nsec - last.tv_nsec) / 1000000LL;
}

static inline bool check_proxy_liveness(size_t timeBaseIndex)
{
    auto &states = TimeBaseStates::getInstance();
    timespec currentTime, lastNotificationTime;
    static timespec lastConnectTime = {0};
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
    timeout = timespec_delta(lastConnectTime, currentTime);
    if(timeout < DEFAULT_LIVENESS_TIMEOUT_IN_MS)
        return true;
send_connect:
    return ClientState::getSingleInstance().
        connect(DEFAULT_LIVENESS_TIMEOUT_IN_MS, &lastConnectTime);
}

int ClockManager::statusWaitByName(int timeout, const string &timeBaseName,
    ClockSyncData &clockSyncData)
{
    size_t timeBaseIndex = 0;
    if(!TimeBaseConfigurations::BaseNameToBaseIndex(timeBaseName, timeBaseIndex)) {
        PrintDebug("[SUBSCRIBE] Invalid timeBaseName.");
        return -1;
    }
    return statusWait(timeout, timeBaseIndex, clockSyncData);
}

int ClockManager::statusWait(int timeout, size_t timeBaseIndex,
    ClockSyncData &clockSyncData)
{
    // Check whether connection between Proxy and Client is established or not
    ClientState &implClientState = ClientState::getSingleInstance();
    if(!implClientState.get_connected()) {
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
        // Check liveness of the Proxy Daemon
        if(!check_proxy_liveness(timeBaseIndex)) {
            PrintDebug("[WAIT] Proxy Daemon is not alive.");
            return -1;
        }
        // Sleep for a short duration before the next iteration
        this_thread::sleep_for(milliseconds(10));
    } while(high_resolution_clock::now() < end);
    ClockSyncBaseHandler handler(clockSyncData);
    // TODO: check ptp4l and chrony data is received
    PTPClockEvent &ptpData = state.get_ptp4lEventState();
    handler.setPTPAvailability(true);
    handler.updatePTPClock(ptpData);
    SysClockEvent &chronydata = state.get_chronyEventState();
    handler.setSysAvailability(true);
    handler.updateSysClock(chronydata);
    if(!event_changes_detected)
        return 0;
    return 1;
}

bool ClockManager::getTime(timespec &ts)
{
    return clock_gettime(CLOCK_REALTIME, &ts) == 0;
}
