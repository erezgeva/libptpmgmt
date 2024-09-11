/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file init.cpp
 * @brief Client utilities to setup and cleanup the library.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#include <chrono>
#include <cstring>
#include <iostream>

#include <client/connect_msg.hpp>
#include <client/msgq_tport.hpp>
#include <client/notification_msg.hpp>
#include <client/subscribe_msg.hpp>
#include <common/print.hpp>
#include <common/sighandler.hpp>
#include <rtpi/condition_variable.hpp>
#include <rtpi/mutex.hpp>
#include "jclk_init.hpp"

#define DEFAULT_LIVENESS_TIMEOUT_IN_MS 50  //50 ms
#define DEFAULT_CONNECT_TIME_OUT 5  //5 sec
#define DEFAULT_SUBSCRIBE_TIME_OUT 5  //5 sec

using namespace JClkLibClient;
using namespace JClkLibCommon;
using namespace std;

rtpi::mutex ClientConnectMessage::cv_mtx;
rtpi::condition_variable ClientConnectMessage::cv;
rtpi::mutex ClientSubscribeMessage::cv_mtx;
rtpi::condition_variable ClientSubscribeMessage::cv;

bool JClkLibClientApi::jcl_connect()
{
    unsigned int timeout_sec = (unsigned int) DEFAULT_CONNECT_TIME_OUT;
    Message0 connectMsg(new ClientConnectMessage());
    TransportClientId newClientID;
    ClientConnectMessage *cmsg = dynamic_cast<decltype(cmsg)>(connectMsg.get());
    cmsg->setClientState(&appClientState);
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
    auto endTime = std::chrono::system_clock::now() +
        std::chrono::seconds(timeout_sec);
    std::unique_lock<rtpi::mutex> lck(ClientConnectMessage::cv_mtx);
    while(appClientState.get_connected() == false) {
        auto res = ClientConnectMessage::cv.wait_until(lck, endTime);
        if(res == std::cv_status::timeout) {
            if(appClientState.get_connected() == false) {
                PrintDebug("[CONNECT] Connect reply from proxy - \
                    timeout failure!!");
                return false;
            }
        } else
            PrintDebug("[CONNECT] Connect reply received.");
    }
    if((cmsg != nullptr) && !(cmsg->getClientId().empty())) {
        strcpy((char *)newClientID.data(), (char *)cmsg->getClientId().data());
        appClientState.set_clientID(newClientID);
    }
    return true;
}

bool JClkLibClientApi::jcl_subscribe(JClkLibCommon::jcl_subscription &newSub,
    jcl_state &currentState)
{
    unsigned int timeout_sec = (unsigned int) DEFAULT_SUBSCRIBE_TIME_OUT;
    PrintDebug("[JClkLibClient]::subscribe");
    MessageX subscribeMsg(new ClientSubscribeMessage());
    ClientSubscribeMessage *cmsg = dynamic_cast<decltype(cmsg)>(subscribeMsg.get());
    if(cmsg == nullptr) {
        PrintErrorCode("[JClkLibClient::subscribe] subscribeMsg is nullptr !!\n");
        return false;
    } else
        PrintDebug("[JClkLibClient::subscribe] subscribeMsgcreation is OK !!\n");
    cmsg->setClientState(&appClientState);
    /* Write the current event subscription */
    appClientState.get_eventSub().set_event(newSub.getc_event());
    appClientState.get_eventSub().set_value(newSub.getc_value());
    appClientState.get_eventSub().set_composite_event(
        newSub.getc_composite_event());
    /* Copy the event Mask */
    cmsg->getSubscription().get_event().copyEventMask(newSub.get_event());
    strcpy((char *)cmsg->getClientId().data(),
        (char *)appClientState.get_clientID().data());
    cmsg->set_sessionId(appClientState.get_sessionId());
    ClientMessageQueue::writeTransportClientId(subscribeMsg.get());
    ClientMessageQueue::sendMessage(subscribeMsg.get());
    /* Wait for subscription result */
    auto endTime = std::chrono::system_clock::now() + std::chrono::seconds(
            timeout_sec);
    std::unique_lock<rtpi::mutex> lck(ClientSubscribeMessage::cv_mtx);
    while(appClientState.get_subscribed() == false) {
        auto res = ClientSubscribeMessage::cv.wait_until(lck, endTime);
        if(res == std::cv_status::timeout) {
            if(appClientState.get_subscribed() == false) {
                PrintDebug("[SUBSCRIBE] No reply from proxy - timeout failure!!");
                return false;
            }
        } else
            PrintDebug("[SUBSCRIBE] SUBSCRIBE reply received.");
    }
    jcl_state jclCurrentState = appClientState.get_eventState();
    currentState = jclCurrentState;
    return true;
}

bool JClkLibClientApi::jcl_disconnect()
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
        appClientState.get_sessionId());
    /* Delete the ClientState reference inside ClientNotificationMessage class */
    ClientNotificationMessage::deleteClientState(&appClientState);
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

bool check_proxy_liveness(ClientState &appClientState)
{
    struct timespec current_time;
    if(clock_gettime(CLOCK_MONOTONIC, &current_time) == -1)
        PrintDebug("[CONNECT] Failed to get current time.\n");
    else {
        int64_t timeout =
            timespec_delta(appClientState.get_last_notification_time(),
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
    appClientState.set_connected(false);
    cmsg->setClientState(&appClientState);
    cmsg->set_sessionId(appClientState.get_sessionId());
    ClientMessageQueue::writeTransportClientId(connectMsg.get());
    ClientMessageQueue::sendMessage(connectMsg.get());
    /* Wait for connection result */
    auto endTime = std::chrono::system_clock::now() + std::chrono::milliseconds(
            DEFAULT_LIVENESS_TIMEOUT_IN_MS);
    std::unique_lock<rtpi::mutex> lck(ClientConnectMessage::cv_mtx);
    while(appClientState.get_connected() == false) {
        auto res = ClientConnectMessage::cv.wait_until(lck, endTime);
        if(res == std::cv_status::timeout) {
            if(appClientState.get_connected() == false) {
                PrintDebug("[CONNECT] Connect reply timeout!!");
                return false;
            }
        } else
            PrintDebug("[CONNECT] Connect reply received.");
    }
    return true;
}

/**
 * @brief This function waits for a specified timeout period for any event changes.
 *
 * @param timeout The timeout period in seconds. If timeout is 0, the function
 *                will check event changes once. If timeout is -1, the function
 *                wait until there is event changes occurs.
 * @param jcl_state A reference to a jcl_state object where the current state
 *                  will be stored.
 * @param eventCount A reference to a jcl_state_event_count object where the
 *                   event counts will be stored.
 *
 * @return Returns true if there is event changes within the timeout period,
 *         and false otherwise.
 */
int JClkLibClientApi::jcl_status_wait(int timeout, jcl_state &jcl_state_ref,
    jcl_state_event_count &eventCountRef)
{
    auto start = std::chrono::high_resolution_clock::now();
    auto end = (timeout == -1) ?
        std::chrono::time_point<std::chrono::high_resolution_clock>::max() :
        start + std::chrono::seconds(timeout);
    bool event_changes_detected = false;
    jcl_state_event_count eventCount;
    jcl_state jcl_state;
    do {
        /* Check the liveness of the Proxy's message queue */
        if(!check_proxy_liveness(appClientState))
            return -1;
        /* Get the event state and event count from the API */
        eventCount = appClientState.get_eventStateCount();
        jcl_state = appClientState.get_eventState();
        /* Check if any member of eventCount is non-zero */
        if(eventCount.offset_in_range_event_count ||
            eventCount.as_capable_event_count ||
            eventCount.synced_to_primary_clock_event_count ||
            eventCount.composite_event_count ||
            eventCount.gm_changed_event_count) {
            event_changes_detected = true;
            break;
        }
        /* Sleep for a short duration before the next iteration */
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while(std::chrono::high_resolution_clock::now() < end);
    if(!event_changes_detected)
        return false;
    /* Copy out the current state */
    eventCountRef = eventCount;
    jcl_state_ref = jcl_state;
    /* Reset the atomic count by reducing the corresponding eventCount */
    ClientSubscribeMessage::resetClientPtpEventStruct(
        appClientState.get_sessionId(),
        eventCount);
    appClientState.set_eventStateCount(eventCount);
    return true;
}
