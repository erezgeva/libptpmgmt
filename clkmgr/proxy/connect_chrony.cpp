/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy connect chrony class.
 *
 * @author Lai Peter Jun Ann <peter.jun.ann.lai@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "proxy/client.hpp"
#include "proxy/connect_srv.hpp"
#include "common/print.hpp"
#include "client/clock_event_handler.hpp"

#include <chrony.h>
#include <poll.h>
#include <cmath>

__CLKMGR_NAMESPACE_USE;

using namespace std;

// default sleeping of 10 microsecond, until we get the value from chrony
static const uint32_t def_polling_interval = 10;

class ChronyThreadSet : public Thread4TimeBase
{
  private:
    const string &udsAddrChrony;
    chronyEvent event;
    int chronyFd = -1;
    chrony_session *session = nullptr;
    // Internal methods
    chrony_err subscribe_to_chronyd();
    chrony_err process_chronyd_data();
    int64_t polling_interval = def_polling_interval;

  public:
    ChronyThreadSet(size_t timeBaseIndex, const string &udsAddr) :
        Thread4TimeBase(timeBaseIndex), udsAddrChrony(udsAddr),
        event(timeBaseIndex) {}

    bool init() override final { return true; }
    void thread_loop() override final;
    void close() override final {
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

chrony_err ChronyThreadSet::process_chronyd_data()
{
    pollfd pfd = { .fd = chronyFd, .events = POLLIN };
    const int timeout = 1000; // milliseconds
    while(!stopThread && chrony_needs_response(session)) {
        int ret = poll(&pfd, 1, timeout);
        if(stopThread)
            return CHRONY_OK;
        if(ret == 0) {
            PrintError("No valid response received");
            // TODO why is time-out consider an error?
            // PrintDebug("timed out");
        } else if(ret > 0) {
            chrony_err r = chrony_process_response(session);
            if(r != CHRONY_OK)
                return r;
        } else
            PrintErrorCode("poll");
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
    Client::setClockType(ClockEventHandler::SysClock);
    Client::NotifyClients(timeBaseIndex);
    return CHRONY_OK;
}

void ChronyThreadSet::thread_loop()
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
            Client::setClockType(ClockEventHandler::SysClock);
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

class Chrony : public ConnectSrv
{
  protected:
    bool isValid(const TimeBaseCfgFull &cfg) override final {
        return !cfg.udsAddrChrony.empty();
    }
    Thread4TimeBase *alloc(size_t timeBaseIndex,
        const TimeBaseCfgFull &cfg) override final {
        return new ChronyThreadSet(timeBaseIndex, cfg.udsAddrChrony);
    }

  public:
    Chrony() = default;
};
static Chrony instance;

bool Client::connect_chrony()
{
    return instance.registerSrv();
}
