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
#include "client/client_state.hpp"
#include "client/timebase_state.hpp"
#include "common/print.hpp"

#include <chrono>
#include <thread>

#ifdef __GNUC__
#define ON_EXIT_ATTR  __attribute__((destructor))
#endif

__CLKMGR_NAMESPACE_USE;

using namespace std;
using namespace std::chrono;

// in miliseconds
static const uint32_t DEFAULT_LIVENESS_TIMEOUT_IN_MS = 200;
static const uint32_t DEFAULT_CONNECT_TIME_OUT = USEC_PER_SEC * 5;

static atomic_bool doInit(false);

ClockManager &ClockManager::fetchSingleInstance()
{
    static ClockManager m_single;
    return m_single;
}

bool ClockManager::connect()
{
    if(!doInit.load()) {
        // Send a connect message to Proxy Daemon
        if(!ClientState::init()) {
            PrintDebug("[CONNECT] Failed to initialize Client queue.");
            return false;
        }
        #ifndef ON_EXIT_ATTR
        atexit([] { ClockManager::disconnect(); });
        #endif
        doInit.store(true);
    }
    return ClientState::connect(DEFAULT_CONNECT_TIME_OUT);
}

bool ClockManager::disconnect()
{
    // Send a disconnect message - TODO do we want to send a message here?
    if(doInit.load()) {
        if(!End::stopAll()) {
            PrintDebug("[DISCONNECT] Client disconnect Failed");
            return false;
        }
        doInit.store(false);
    }
    return true;
}

const TimeBaseConfigurations &ClockManager::getTimebaseCfgs()
{
    if(!ClientState::get_connected())
        ClockManager::connect();
    return TimeBaseConfigurations::getInstance();
}

static inline bool _subscribe(const ClockSyncSubscription &newSub,
    size_t timeBaseIndex, ClockSyncData &clockSyncData)
{
    TimeBaseStates &states = TimeBaseStates::getInstance();
    if(!states.subscribe(timeBaseIndex, newSub))
        return false;
    // Get the current state of the timebase
    TimeBaseState state;
    if(!states.getTimeBaseState(timeBaseIndex, state)) {
        PrintDebug("[SUBSCRIBE] Failed to get specific timebase state.");
        return false;
    }
    ClockSyncBaseHandler handler(clockSyncData);
    handler.updateAll(state);
    return true;
}

bool ClockManager::subscribe(const ClockSyncSubscription &newSub,
    size_t timeBaseIndex, ClockSyncData &clockSyncData)
{
    if(!ClientState::get_connected()) {
        ClockManager::connect();
        if(!ClientState::get_connected())
            return false;
    }
    return _subscribe(newSub, timeBaseIndex, clockSyncData);
}

bool ClockManager::subscribeByName(const ClockSyncSubscription &newSub,
    const string &timeBaseName, ClockSyncData &clockSyncData)
{
    if(!ClientState::get_connected()) {
        ClockManager::connect();
        if(!ClientState::get_connected())
            return false;
    }
    size_t timeBaseIndex = 0;
    if(!TimeBaseConfigurations::BaseNameToBaseIndex(timeBaseName, timeBaseIndex)) {
        PrintDebug("[SUBSCRIBE] Invalid timeBaseName.");
        return false;
    }
    return _subscribe(newSub, timeBaseIndex, clockSyncData);
}

// Calculate delta in miliseconds
static inline int64_t timespec_delta(const timespec &last, const timespec &cur)
{
    return (int64_t)(cur.tv_sec - last.tv_sec) * MSEC_PER_SEC +
        (cur.tv_nsec - last.tv_nsec) / NSEC_PER_MSEC;
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
    return ClientState::connect(DEFAULT_LIVENESS_TIMEOUT_IN_MS, &lastConnectTime);
}

static inline enum StatusWaitResult _statusWait(int timeout,
    size_t timeBaseIndex, ClockSyncData &clockSyncData)
{
    // Check whether requested timeBaseIndex is subscribed or not
    auto &states = TimeBaseStates::getInstance();
    if(!states.getSubscribed(timeBaseIndex)) {
        PrintDebug("[WAIT] Invalid timeBaseIndex.");
        return SWRInvalidArgument;
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
            return SWRInvalidArgument;
        }
        // Check whether there is any changes on the event state
        if(state.is_event_changed()) {
            event_changes_detected = true;
            break;
        }
        // Check liveness of the Proxy Daemon
        if(!check_proxy_liveness(timeBaseIndex)) {
            PrintDebug("[WAIT] Proxy Daemon is not alive.");
            return SWRLostConnection;
        }
        // Sleep for a short duration before the next iteration
        this_thread::sleep_for(milliseconds(10));
    } while(high_resolution_clock::now() < end);
    ClockSyncBaseHandler handler(clockSyncData);
    handler.updateAll(state);
    if(!event_changes_detected)
        return SWRNoEventDetected;
    return SWREventDetected;
}

enum StatusWaitResult ClockManager::statusWait(int timeout,
    size_t timeBaseIndex, ClockSyncData &clockSyncData)
{
    // Check whether connection between Proxy and Client is established or not
    if(!ClientState::get_connected()) {
        PrintDebug("[SUBSCRIBE] Client is not connected to Proxy.");
        return SWRLostConnection;
    }
    return _statusWait(timeout, timeBaseIndex, clockSyncData);
}

enum StatusWaitResult ClockManager::statusWaitByName(int timeout,
    const string &timeBaseName, ClockSyncData &clockSyncData)
{
    // Check whether connection between Proxy and Client is established or not
    if(!ClientState::get_connected()) {
        PrintDebug("[SUBSCRIBE] Client is not connected to Proxy.");
        return SWRLostConnection;
    }
    size_t timeBaseIndex = 0;
    if(!TimeBaseConfigurations::BaseNameToBaseIndex(timeBaseName, timeBaseIndex)) {
        PrintDebug("[SUBSCRIBE] Invalid timeBaseName.");
        return SWRInvalidArgument;
    }
    return _statusWait(timeout, timeBaseIndex, clockSyncData);
}

bool ClockManager::getTime(timespec &ts)
{
    return clock_gettime(CLOCK_REALTIME, &ts) == 0;
}

// Disconnect on exit
#ifdef ON_EXIT_ATTR
ON_EXIT_ATTR static void disconnect()
{
    ClockManager::disconnect();
}
#endif
