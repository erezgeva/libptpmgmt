/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy connect chrony message class.
 *
 * @author Lai Peter Jun Ann <peter.jun.ann.lai@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "proxy/config_parser.hpp"
#include "proxy/client.hpp"
#include "common/termin.hpp"
#include "common/print.hpp"

#include <chrony.h>
#include <poll.h>
#include <cmath>
#include <thread>
#include <atomic>

__CLKMGR_NAMESPACE_USE;

using namespace std;

// default sleeping of 10 microsecond, until we get the value from chrony
static const uint32_t def_polling_interval = 10;

class ChronyThreadSet
{
  private:
    size_t timeBaseIndex;
    string udsAddrChrony;
    thread self;
    chronyEvent event;
    int chronyFd = -1;
    chrony_session *session = nullptr;
    // Internal methods
    chrony_err subscribe_to_chronyd();
    chrony_err process_chronyd_data();
    int64_t polling_interval = def_polling_interval;

  public:
    ChronyThreadSet(size_t timeBaseIndex, const string &udsAddrChrony);
    void monitor_chronyd(); // The actual thread
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

static map<size_t, unique_ptr<ChronyThreadSet>> chronyThreadList;

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
    event.chrony_reference_id =
        chrony_get_field_uinteger(session, field_index);
    if(stopThread)
        return CHRONY_OK;
    field_index = chrony_get_field_index(session, "poll");
    if(stopThread)
        return CHRONY_OK;
    int32_t interval = static_cast<int32_t>
        (static_cast<int16_t>(chrony_get_field_integer(session, field_index)));
    polling_interval = pow(2.0, interval) * 1000000;
    PrintDebug("CHRONY polling_interval = " + to_string(polling_interval) + " us");
    if(stopThread)
        return CHRONY_OK;
    field_index = chrony_get_field_index(session, "original last sample offset");
    if(stopThread)
        return CHRONY_OK;
    int64_t second = (int64_t)(1e9 * chrony_get_field_float(session, field_index));
    PrintDebug("CHRONY master_offset = " + to_string(second));
    if(stopThread)
        return CHRONY_OK;
    event.polling_interval = polling_interval;
    event.chrony_offset = second;
    event.copy();
    Client::NotifyClients(timeBaseIndex);
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
            if(stopThread)
                break;
            polling_interval = def_polling_interval;
            event.clear();
            Client::NotifyClients(timeBaseIndex);
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
        this_thread::sleep_for(chrono::microseconds(polling_interval));
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
    const string &l_udsAddrChrony) : timeBaseIndex(l_timeBaseIndex),
    udsAddrChrony(l_udsAddrChrony), event(l_timeBaseIndex)
{
    self = thread(thread_start, this);
}

bool Client::connect_chrony()
{
    for(const auto &param : JsonConfigParser::getInstance()) {
        // skip if chrony UDS address is empty
        if(!param.udsAddrChrony.empty())
            chronyThreadList[param.base.timeBaseIndex].reset(new ChronyThreadSet(
                    param.base.timeBaseIndex, param.udsAddrChrony));
    }
    // Ensure threads start after finish initializing
    all_init.store(true);
    return true;
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
