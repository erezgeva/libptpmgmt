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

/* Global vector to hold all time base configurations */
std::vector<TimeBaseCfg> timeBaseCfgs;

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
    unsigned int timeout_sec = (unsigned int) DEFAULT_CONNECT_TIME_OUT;
    Message0 connectMsg(new ClientConnectMessage());
    TransportClientId newClientID;
    ClientConnectMessage *cmsg = dynamic_cast<decltype(cmsg)>(connectMsg.get());
    cmsg->setClientState(implClientState.get());
    if(!ClientMessage::init()) {
        PrintError("Client Message Init Failed");
        return false;
    }
    if(!ClientTransport::init()) {
        PrintError("Client Transport Init Failed");
        return false;
    }
    ClientMessageQueue::writeTransportClientId(connectMsg.get());
    ClientMessageQueue::sendMessage(connectMsg.get());
    /* Wait for connection result */
    auto endTime = system_clock::now() + seconds(timeout_sec);
    unique_lock<rtpi::mutex> lck(ClientConnectMessage::cv_mtx);
    while(implClientState->get_connected() == false) {
        auto res = ClientConnectMessage::cv.wait_until(lck, endTime);
        if(res == cv_status::timeout) {
            if(implClientState->get_connected() == false) {
                PrintDebug("[CONNECT] Connect reply from proxy - "
                    "timeout failure!!");
                return false;
            }
        } else
            PrintDebug("[CONNECT] Connect reply received.");
    }
    if((cmsg != nullptr) && !(cmsg->getClientId().empty())) {
        strcpy((char *)newClientID.data(), (char *)cmsg->getClientId().data());
        implClientState->set_clientID(newClientID);
    }
    return true;
}

std::vector<TimeBaseCfg> ClockManager::clkmgr_get_timebase_cfgs() const
{
    return timeBaseCfgs;
}

bool ClockManager::clkmgr_subscribe(const ClkMgrSubscription &newSub,
    int timeBaseIndex, Event_state &currentState)
{
    unsigned int timeout_sec = (unsigned int) DEFAULT_SUBSCRIBE_TIME_OUT;
    PrintDebug("[clkmgr]::subscribe");
    MessageX subscribeMsg(new ClientSubscribeMessage());
    ClientSubscribeMessage *cmsg = dynamic_cast<decltype(cmsg)>(subscribeMsg.get());
    if(cmsg == nullptr) {
        PrintErrorCode("[clkmgr::subscribe] subscribeMsg is nullptr !!");
        return false;
    } else
        PrintDebug("[clkmgr::subscribe] subscribeMsgcreation is OK !!");
    cmsg->setClientState(implClientState.get());
    cmsg->set_timeBaseIndex(timeBaseIndex);
    /* Write the current event subscription */
    implClientState->set_eventSub(newSub);
    /* Copy the event Mask */
    cmsg->getSubscription().set_event_mask(newSub.get_event_mask());
    strcpy((char *)cmsg->getClientId().data(),
        (char *)implClientState->get_clientID().data());
    cmsg->set_sessionId(implClientState->get_sessionId());
    ClientMessageQueue::writeTransportClientId(subscribeMsg.get());
    ClientMessageQueue::sendMessage(subscribeMsg.get());
    /* Wait for subscription result */
    auto endTime = system_clock::now() + seconds(timeout_sec);
    unique_lock<rtpi::mutex> lck(ClientSubscribeMessage::cv_mtx);
    while(implClientState->get_subscribed() == false) {
        auto res = ClientSubscribeMessage::cv.wait_until(lck, endTime);
        if(res == cv_status::timeout) {
            if(implClientState->get_subscribed() == false) {
                PrintDebug("[SUBSCRIBE] No reply from proxy - timeout failure!!");
                return false;
            }
        } else
            PrintDebug("[SUBSCRIBE] SUBSCRIBE reply received.");
    }
    Event_state clkmgrCurrentState = implClientState->get_eventState();
    currentState = clkmgrCurrentState;
    return true;
}

bool ClockManager::clkmgr_disconnect()
{
    bool retVal = false;
    /* Send a disconnect message */
    if(!ClientTransport::stop()) {
        PrintDebug("Client Stop Failed");
        goto done;
    }
    if(!ClientTransport::finalize()) {
        PrintDebug("Client Finalize Failed");
        goto done;
    }
    /* Delete the ClientPtpEvent inside Subscription */
    ClientSubscribeMessage::deleteClientPtpEventStruct(
        implClientState->get_sessionId());
    /* Delete the ClientState reference inside ClientNotificationMessage class */
    ClientNotificationMessage::deleteClientState(implClientState.get());
    retVal = true;
done:
    if(!retVal)
        PrintError("Client Error Occured");
    return retVal;
}

int64_t timespec_delta(const timespec &last_notification_time,
    const timespec &current_time)
{
    int64_t delta_in_sec = current_time.tv_sec - last_notification_time.tv_sec;
    int64_t delta_in_nsec = current_time.tv_nsec - last_notification_time.tv_nsec;
    int64_t delta_in_ms = delta_in_sec * 1000LL + delta_in_nsec / 1000000LL;
    return delta_in_ms;
}

bool check_proxy_liveness(ClientState &implClientState)
{
    timespec current_time;
    if(clock_gettime(CLOCK_REALTIME, &current_time) == -1)
        PrintDebug("[CONNECT] Failed to get current time.");
    else {
        int64_t timeout =
            timespec_delta(implClientState.get_last_notification_time(),
                current_time);
        if(timeout < DEFAULT_LIVENESS_TIMEOUT_IN_MS)
            return true;
    }
    Message0 connectMsg(new ClientConnectMessage());
    ClientConnectMessage *cmsg = dynamic_cast<decltype(cmsg)>(connectMsg.get());
    if(cmsg == nullptr) {
        PrintDebug("[CONNECT] Failed to cast to ClientConnectMessage");
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
    while(implClientState.get_connected() == false) {
        auto res = ClientConnectMessage::cv.wait_until(lck, endTime);
        if(res == cv_status::timeout) {
            if(implClientState.get_connected() == false) {
                PrintDebug("[CONNECT] Connect reply timeout!!");
                return false;
            }
        } else
            PrintDebug("[CONNECT] Connect reply received.");
    }
    return true;
}

int ClockManager::clkmgr_status_wait(int timeout,
    Event_state &currentState, Event_count &currentCount)
{
    auto start = high_resolution_clock::now();
    auto end = (timeout == -1) ?  time_point<high_resolution_clock>::max() :
        start + seconds(timeout);
    bool event_changes_detected = false;
    Event_count eventCount;
    Event_state eventState;
    do {
        /* Check the liveness of the Proxy's message queue */
        if(!check_proxy_liveness(*implClientState))
            return -1;
        /* Get the event state and event count from the API */
        eventCount = implClientState->get_eventStateCount();
        eventState = implClientState->get_eventState();
        /* Check if any member of eventCount is non-zero */
        if(eventCount.offset_in_range_event_count ||
            eventCount.as_capable_event_count ||
            eventCount.synced_to_gm_event_count ||
            eventCount.composite_event_count ||
            eventCount.gm_changed_event_count ||
            eventCount.chrony_offset_in_range_event_count) {
            event_changes_detected = true;
            break;
        }
        /* Sleep for a short duration before the next iteration */
        this_thread::sleep_for(milliseconds(10));
    } while(high_resolution_clock::now() < end);
    /* Copy out the current state */
    currentCount = eventCount;
    currentState = eventState;
    if(!event_changes_detected)
        return 0;
    /* Reset the atomic count by reducing the corresponding eventCount */
    ClientSubscribeMessage::resetClientPtpEventStruct(
        implClientState->get_sessionId(), eventCount);
    implClientState->set_eventStateCount(eventCount);
    return 1;
}

int ClockManager::clkmgr_gettime(timespec *ts)
{
    clockid_t clk_id = CLOCK_REALTIME;
    return clock_gettime(clk_id, ts);
}
