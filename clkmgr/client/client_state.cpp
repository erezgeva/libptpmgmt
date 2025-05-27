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
#include "client/msgq_tport.hpp"
#include "common/print.hpp"

#include <rtpi/condition_variable.hpp>

__CLKMGR_NAMESPACE_USE;

using namespace std;
using namespace std::chrono;

static rtpi::mutex connect_cv_mtx;
static rtpi::condition_variable connect_cv;

ClientState &ClientState::getSingleInstance()
{
    static ClientState instance;
    return instance;
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
    cmsg->setClientId(clientID);
    cmsg->set_sessionId(get_sessionId());
    ClientQueue::sendMessage(cmsg);
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
            PrintDebug("[CONNECT] Received reply from Proxy.");
        }
    }
    return true;
}

bool ClientState::connectReply(sessionId_t sessionId)
{
    PrintDebug("Processing client connect message (reply)");
    PrintDebug("Connected with session ID: " + to_string(sessionId));
    PrintDebug("Current state.sessionId: " + to_string(get_sessionId()));
    unique_lock<rtpi::mutex> lock(connect_cv_mtx);
    set_connected(true);
    set_sessionId(sessionId);
    connect_cv.notify_one(lock);
    return true;
}
