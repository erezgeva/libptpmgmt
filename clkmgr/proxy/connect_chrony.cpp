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
#include "proxy/client.hpp"
#include "proxy/config_parser.hpp"
#include "proxy/connect_ptp4l.hpp"
#include "proxy/notification_msg.hpp"
#include "common/ptp_event.hpp"
#include "common/print.hpp"

#include <chrony.h>
#include <poll.h>
#include <unistd.h>
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
    int timeBaseIndex;
    string udsAddrChrony;
    thread self;
    ptp_event &ptp4lEvent;
    vector<sessionId_t> subscribedClients;
    chrony_session *m_s = nullptr;

    // Internal methods
    chrony_err subscribe_to_chronyd();
    chrony_err process_chronyd_data();
    void notify_client();

  public:
    ChronyThreadSet(int timeBaseIndex, const string &udsAddrChrony);
    void monitor_chronyd(); // The actual thread
    // notification subscribe
    bool subscribe(sessionId_t sessionId);
    // notification unsubscribe
    bool unsubscribe(sessionId_t sessionId);
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
    PrintDebug("[clkmgr]::notify_client");
    vector<sessionId_t> sessionIdToRemove;
    unique_lock<rtpi::mutex> local(subscribedLock[timeBaseIndex]);
    for(auto it = subscribedClients.begin(); it != subscribedClients.end();) {
        sessionId_t sessionId = *it;
        unique_ptr<ProxyMessage> notifyMsg(new ProxyNotificationMessage());
        ProxyNotificationMessage *pmsg =
            dynamic_cast<decltype(pmsg)>(notifyMsg.get());
        if(pmsg == nullptr) {
            PrintErrorCode("[clkmgr::notify_client] notifyMsg is nullptr !!");
            return;
        }
        PrintDebug("[clkmgr::notify_client] notifyMsg creation is OK !!");
        // Send data for multiple sessions
        pmsg->setTimeBaseIndex(timeBaseIndex);
        PrintDebug("Get client session ID: " + to_string(sessionId));
        auto TxContext = Client::GetClientSession(
                sessionId).get()->get_transmitContext();
        if(!pmsg->transmitMessage(*TxContext)) {
            it = subscribedClients.erase(it);
            /* Add sessionId into the list to remove */
            sessionIdToRemove.push_back(sessionId);
        } else
            ++it;
    }
    local.unlock(); // Explicitly unlock the mutex
    for(const sessionId_t sessionId : sessionIdToRemove) {
        ConnectPtp4l::remove_ptp4l_subscriber(sessionId);
        Client::RemoveClientSession(sessionId);
    }
}

chrony_err ChronyThreadSet::process_chronyd_data()
{
    pollfd pfd = { .fd = chrony_get_fd(m_s), .events = POLLIN };
    int timeout = 1000;
    while(chrony_needs_response(m_s)) {
        int n = poll(&pfd, 1, timeout);
        if(n < 0)
            PrintErrorCode("poll");
        else if(n == 0)
            PrintError("No valid response received");
        chrony_err r = chrony_process_response(m_s);
        if(r != CHRONY_OK)
            return r;
    }
    return CHRONY_OK;
}

chrony_err ChronyThreadSet::subscribe_to_chronyd()
{
    int record_index = 0;
    chrony_err r = chrony_request_record(m_s, "sources", record_index);
    if(r != CHRONY_OK)
        return r;
    r = process_chronyd_data();
    if(r != CHRONY_OK)
        return r;
    int field_index = chrony_get_field_index(m_s, "reference ID");
    ptp4lEvent.chrony_reference_id =
        chrony_get_field_uinteger(m_s, field_index);
    field_index = chrony_get_field_index(m_s, "poll");
    int32_t interval = static_cast<int32_t>
        (static_cast<int16_t>(chrony_get_field_integer(m_s, field_index)));
    ptp4lEvent.polling_interval =
        pow(2.0, interval) * 1000000;
    PrintDebug("CHRONY polling_interval = " +
        to_string(ptp4lEvent.polling_interval) + " us");
    field_index = chrony_get_field_index(m_s, "original last sample offset");
    float second = (chrony_get_field_float(m_s, field_index) * 1e9);
    ptp4lEvent.chrony_offset = (int)second;
    PrintDebug("CHRONY master_offset = " +
        to_string(ptp4lEvent.chrony_offset));
    notify_client();
    return CHRONY_OK;
}

void ChronyThreadSet::monitor_chronyd()
{
    // connect to chronyd unix socket using udsAddrChrony
    int fd = chrony_open_socket(udsAddrChrony.c_str());
    if(chrony_init_session(&m_s, fd) == CHRONY_OK && fd > 0)
        PrintInfo("Connected to Chrony at " + udsAddrChrony);
    for(;;) {
        if(subscribe_to_chronyd() != CHRONY_OK) {
            chrony_deinit_session(m_s);
            ptp4lEvent.chrony_reference_id = 0;
            ptp4lEvent.polling_interval = 0;
            ptp4lEvent.chrony_offset = 0;
            notify_client();
            PrintError("Failed to connect to Chrony at " + udsAddrChrony);
            // Reconnection loop
            for(;;) {
                PrintInfo("Attempting to reconnect to Chrony at " +
                    udsAddrChrony);
                fd = chrony_open_socket(udsAddrChrony.c_str());
                if(fd < 0) {
                    sleep(5); // Wait before retrying
                    continue;
                }
                if(chrony_init_session(&m_s, fd) == CHRONY_OK) {
                    PrintInfo("Reconnected to Chrony at " + udsAddrChrony);
                    break;
                }
            }
        }
        // Sleep duration is based on chronyd polling interval
        usleep(ptp4lEvent.polling_interval);
    }
}

static void thread_start(ChronyThreadSet *me)
{
    // Ensure we start after initializing ends
    while(!all_init.load())
        sleep(1);
    me->monitor_chronyd();
}

ChronyThreadSet::ChronyThreadSet(int l_timeBaseIndex,
    const string &l_udsAddrChrony) :
    timeBaseIndex(l_timeBaseIndex), udsAddrChrony(l_udsAddrChrony),
    ptp4lEvent(ptp4lEvents[l_timeBaseIndex])
{
    subscribedLock[l_timeBaseIndex]; // Make dure mutex exist before thread start
    self = thread(thread_start, this);
}

int ConnectChrony::subscribe_chrony(int timeBaseIndex, sessionId_t sessionId)
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
