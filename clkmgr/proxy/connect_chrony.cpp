/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy connect chrony message class.
 *
 * @author Lai Peter Jun Ann <peter.jun.ann.lai@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "proxy/connect_chrony.hpp"
#include "proxy/config_parser.hpp"
#include "proxy/message.hpp"
#include "common/termin.hpp"
#include "common/ptp_event.hpp"
#include "common/print.hpp"

#include <chrony.h>
#include <poll.h>
#include <cmath>
#include <thread>
#include <atomic>
#include <rtpi/mutex.hpp>

__CLKMGR_NAMESPACE_USE;

using namespace std;

extern map<int, ptp_event> ptp4lEvents;

class ChronyThreadSet
{
  private:
    size_t timeBaseIndex;
    string udsAddrChrony;
    thread self;
    ptp_event &ptp4lEvent;
    vector<sessionId_t> subscribedClients;
    int chronyFd = -1;
    chrony_session *session = nullptr;
    // Internal methods
    chrony_err subscribe_to_chronyd();
    chrony_err process_chronyd_data();
    void notify_client();

  public:
    ChronyThreadSet(size_t timeBaseIndex, const string &udsAddrChrony);
    void monitor_chronyd(); // The actual thread
    // notification subscribe
    bool subscribe(sessionId_t sessionId);
    // notification unsubscribe
    bool unsubscribe(sessionId_t sessionId);
    bool stopThread = false;
    void wait() { self.join(); }
    void close() {
        if(chronyFd >= 0) {
            chrony_close_socket(chronyFd);
            chronyFd = -1;
        }
        if(session != nullptr) {
            chrony_deinit_session(session);
            session = nullptr;
        }
    }
};

/*
 * Modern CPUs are allow to run the threads before
 * the main thread end the initializing,
 * regardless of the code flow of the main thread.
 * The atomic guarantee it should not happens.
 * Threads will wait till initializing is done.
 */
static atomic_bool all_init(false);

static map<int, unique_ptr<ChronyThreadSet>> chronyThreadList;
/* Prevent subscribe during notification per set
   We can not define the mutex inside the ChronyThreadSet class itself! */
static map<int, rtpi::mutex> subscribedLock;

bool ChronyThreadSet::subscribe(sessionId_t sessionId)
{
    unique_lock<rtpi::mutex> local(subscribedLock[timeBaseIndex]);
    for(const sessionId_t &id : subscribedClients) {
        if(id == sessionId)
            return false; // Client is already subscribed
    }
    subscribedClients.push_back(sessionId);
    return true;
}

bool ChronyThreadSet::unsubscribe(sessionId_t sessionId)
{
    unique_lock<rtpi::mutex> local(subscribedLock[timeBaseIndex]);
    for(auto it = subscribedClients.begin(); it != subscribedClients.end();) {
        if(*it == sessionId) {
            // sessionId found, remove it
            subscribedClients.erase(it);
            return true;
        }
        ++it;
    }
    return false;
}

void ChronyThreadSet::notify_client()
{
    if(stopThread)
        return;
    PrintDebug("[clkmgr]::notify_client");
    vector<sessionId_t> sessionIdToRemove;
    unique_lock<rtpi::mutex> local(subscribedLock[timeBaseIndex]);
    ProxyMessage::notify_clients(timeBaseIndex, subscribedClients,
        sessionIdToRemove);
    local.unlock(); // Explicitly unlock the mutex
    if(stopThread)
        return;
    ProxyMessage::remove_clients(sessionIdToRemove);
}

chrony_err ChronyThreadSet::process_chronyd_data()
{
    pollfd pfd = { .fd = chronyFd, .events = POLLIN };
    int timeout = 1000;
    while(!stopThread && chrony_needs_response(session)) {
        int n = poll(&pfd, 1, timeout);
        if(n < 0)
            PrintErrorCode("poll");
        else if(n == 0)
            PrintError("No valid response received");
        if(stopThread)
            return CHRONY_OK;
        chrony_err r = chrony_process_response(session);
        if(r != CHRONY_OK)
            return r;
    }
    return CHRONY_OK;
}

chrony_err ChronyThreadSet::subscribe_to_chronyd()
{
    int record_index = 0;
    if(stopThread)
        return CHRONY_OK;
    if(session == nullptr)
        return CHRONY_INVALID_ARGUMENT;
    chrony_err r = chrony_request_record(session, "sources", record_index);
    if(r != CHRONY_OK)
        return r;
    r = process_chronyd_data();
    if(r != CHRONY_OK)
        return r;
    if(stopThread)
        return CHRONY_OK;
    int field_index = chrony_get_field_index(session, "reference ID");
    if(stopThread)
        return CHRONY_OK;
    ptp4lEvent.chrony_reference_id =
        chrony_get_field_uinteger(session, field_index);
    if(stopThread)
        return CHRONY_OK;
    field_index = chrony_get_field_index(session, "poll");
    if(stopThread)
        return CHRONY_OK;
    int32_t interval = static_cast<int32_t>
        (static_cast<int16_t>(chrony_get_field_integer(session, field_index)));
    ptp4lEvent.polling_interval =
        pow(2.0, interval) * 1000000;
    PrintDebug("CHRONY polling_interval = " +
        to_string(ptp4lEvent.polling_interval) + " us");
    if(stopThread)
        return CHRONY_OK;
    field_index = chrony_get_field_index(session, "original last sample offset");
    if(stopThread)
        return CHRONY_OK;
    float second = (chrony_get_field_float(session, field_index) * 1e9);
    ptp4lEvent.chrony_offset = (int)second;
    PrintDebug("CHRONY master_offset = " +
        to_string(ptp4lEvent.chrony_offset));
    notify_client();
    return CHRONY_OK;
}

void ChronyThreadSet::monitor_chronyd()
{
    // connect to chronyd unix socket using udsAddrChrony
    chronyFd = chrony_open_socket(udsAddrChrony.c_str());
    if(chronyFd >= 0 && chrony_init_session(&session, chronyFd) == CHRONY_OK)
        PrintInfo("Connected to Chrony at " + udsAddrChrony);
    while(!stopThread) {
        if(subscribe_to_chronyd() != CHRONY_OK) {
            close();
            ptp4lEvent.chrony_reference_id = 0;
            ptp4lEvent.polling_interval = 0;
            ptp4lEvent.chrony_offset = 0;
            notify_client();
            PrintError("Failed to connect to Chrony at " + udsAddrChrony);
            // Reconnection loop
            while(!stopThread) {
                PrintInfo("Attempting to reconnect to Chrony at " +
                    udsAddrChrony);
                chronyFd = chrony_open_socket(udsAddrChrony.c_str());
                if(chronyFd < 0) {
                    // Wait 5 seconds before retrying
                    for(int i = 0; i < 50 && !stopThread; i++)
                        this_thread::sleep_for(chrono::milliseconds(100));
                    continue;
                }
                if(chrony_init_session(&session, chronyFd) == CHRONY_OK) {
                    PrintInfo("Reconnected to Chrony at " + udsAddrChrony);
                    break;
                }
            }
        }
        if(stopThread)
            break;
        // Sleep duration is based on chronyd polling interval
        this_thread::sleep_for(chrono::microseconds(ptp4lEvent.polling_interval));
    }
}

static void thread_start(ChronyThreadSet *me)
{
    // Ensure we start after initializing ends
    while(!all_init.load())
        this_thread::sleep_for(chrono::milliseconds(100));
    me->monitor_chronyd();
}

ChronyThreadSet::ChronyThreadSet(size_t l_timeBaseIndex,
    const string &l_udsAddrChrony) :
    timeBaseIndex(l_timeBaseIndex), udsAddrChrony(l_udsAddrChrony),
    ptp4lEvent(ptp4lEvents[l_timeBaseIndex])
{
    subscribedLock[l_timeBaseIndex]; // Make dure mutex exist before thread start
    self = thread(thread_start, this);
}

int ConnectChrony::subscribe_chrony(size_t timeBaseIndex, sessionId_t sessionId)
{
    if(chronyThreadList.count(timeBaseIndex) > 0) {
        // timeBaseIndex exists in the map
        if(!chronyThreadList[timeBaseIndex]->subscribe(sessionId)) {
            PrintDebug("sessionId " + to_string(sessionId) +
                " is already subscribe in chronyd");
            return -1; // We try to subscribe twice
        }
    } else
        // timeBaseIndex does not exist in the map
        PrintDebug("timeBaseIndex does not exist in the map");
    return 0;
}

int ConnectChrony::remove_chrony_subscriber(sessionId_t sessionId)
{
    // We may be subscribed in multiple time bases, we need to check all of them
    for(auto &it : chronyThreadList)
        it.second->unsubscribe(sessionId);
    return 0;
}

void ConnectChrony::connect_chrony()
{
    for(const auto &param : JsonConfigParser::getInstance()) {
        // skip if chrony UDS address is empty
        if(!param.udsAddrChrony.empty())
            chronyThreadList[param.base.timeBaseIndex].reset(new ChronyThreadSet(
                    param.base.timeBaseIndex, param.udsAddrChrony));
    }
    // Ensure threads start after finish initializing
    all_init.store(true);
}

class ChronyDisconnect : public End
{
  public:
    bool stop() override final {
        for(auto &it : chronyThreadList)
            it.second->stopThread = true;
        return true;
    }
    bool finalize() override final {
        // Wait for threads to end
        for(auto &it : chronyThreadList)
            it.second->wait();
        for(auto &it : chronyThreadList)
            it.second->close();
        chronyThreadList.clear();
        return true;
    }
};
static ChronyDisconnect endChrony;
