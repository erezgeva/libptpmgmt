/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy connect ptp4l class.
 *
 * @author Lai Peter Jun Ann <peter.jun.ann.lai@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "proxy/client.hpp"
#include "proxy/connect_srv.hpp"
#include "common/print.hpp"

// libptpmgmt
#include "sock.h"
#include "msgCall.h"

#include <cmath>

__CLKMGR_NAMESPACE_USE;

using namespace std;
using namespace ptpmgmt;

static const uint64_t timeout_ms = 1000;
static const string baseAddr = "/var/run/pmc.";
static const size_t bufSize = 2000;
static SUBSCRIBE_EVENTS_NP_t eventsTlv; // Threads only read it!

// Callbacks of MessageDispatcher
#define callback_declare(n)\
    void n##_h(const ptpmgmt::Message &, const n##_t &tlv, const char *) override
#define callback_define(n)\
    void ptpSet::n##_h(const ptpmgmt::Message &, const n##_t &tlv, const char *)
// One set per a thread
class ptpSet : public Thread4TimeBase, MessageDispatcher
{
  private:
    const TimeBaseCfg &param; // time base configuration
    ptpEvent event;
    const string &udsAddr;
    ptpmgmt::Message msg;
    SockUnix sku;
    bool do_notify = false;
    char buf[bufSize];
    int seq = 0; // PTP message sequance
    ClockIdentity_t gmIdentity; // Grandmaster clock ID
    bool need_set_action = false; // Request action(PORT_DATA_SET)
  public:
    // These methods are used during initializing, before we create the thread.
    ptpSet(size_t timeBaseIndex, const TimeBaseCfg &p, const string &uds) :
        Thread4TimeBase(timeBaseIndex), param(p), event(timeBaseIndex),
        udsAddr(uds) { }
    bool init() override final;
    void close() override final { sku.close(); }
    // This is the thread method
    void thread_loop() override final;
  private:
    void portDataReset();
    void portReset();
    callback_declare(TIME_STATUS_NP);
    callback_declare(PORT_DATA_SET);
    callback_declare(CMLDS_INFO_NP);
    callback_declare(LOG_SYNC_INTERVAL);
    void event_handle();
    bool msg_send() {
        MNG_PARSE_ERROR_e err = msg.build(buf, bufSize, seq);
        if(err != MNG_PARSE_ERROR_OK) {
            PrintError(string("build error ") + msg.err2str_c(err));
            return false;
        }
        if(!sku.send(buf, msg.getMsgLen())) {
            PrintDebug("send failed");
            return false;
        }
        seq++;
        return true;
    }
    bool msg_set_action(mng_vals_e id) {
        if(!msg.setAction(GET, id)) {
            PrintError(string("Fail get ") + msg.mng2str_c(id));
            return false;
        }
        return msg_send();
    }
    /**
     * Subscribes to a set of PTP (Precision Time Protocol) events.
     *
     * This function configures a subscription to various PTP events
     * by setting the appropriate bitmask in a subscription data
     * structure. It then sends an action to subscribe to these
     * events using a message handling system.
     * The function clears the message data after sending the action
     * to avoid referencing local data that may go out of scope.
     *
     * @param handle A double pointer to a clkmgr_handle structure
     *               representing the handle to be used for the
     *               subscription.
     *               The actual usage of this parameter is not shown in
     *               the provided code snippet, so it may need to be
     *               implemented or removed.
     * @return A boolean value indicating the success of the subscription
     *         action.
     *         Returns true if the subscription action is successfully
     *         sent, false otherwise.
     *
     */
    bool event_subscription() {
        if(!msg.setAction(SET, SUBSCRIBE_EVENTS_NP, &eventsTlv)) {
            PrintError("Fail set SUBSCRIBE_EVENTS_NP");
            return false;
        }
        return msg_send();
    }
};

callback_define(TIME_STATUS_NP)
{
    event.event.clockOffset = tlv.master_offset;
    event.event.gmClockUUID = 0;
    for(int i = 0; i < 8; ++i)
        event.event.gmClockUUID |=
            static_cast<uint64_t>(tlv.gmIdentity.v[i]) << (8 * (7 - i));
    gmIdentity = tlv.gmIdentity;
    do_notify = true;
    PrintDebug("clockOffset = " + to_string(event.event.clockOffset) +
        ", syncedWithGm = " + to_string(event.event.syncedWithGm));
    char buf[100];
    snprintf(buf, sizeof buf, "gmClockUUID = %016lx", event.event.gmClockUUID);
    PrintDebug(buf);
}
void ptpSet::portDataReset()
{
    event.portDataClear();
    need_set_action = true;
}
void ptpSet::portReset()
{
    event.clear();
    need_set_action = true;
}
callback_define(LOG_SYNC_INTERVAL)
{
    if(static_cast<Integer8_t>(event.event.syncInterval) != tlv.logSyncInterval) {
        event.event.syncInterval =
            pow(2.0, tlv.logSyncInterval) * USEC_PER_SEC;
        do_notify = true;
    }
}
callback_define(PORT_DATA_SET)
{
    switch(tlv.portState) {
        case INITIALIZING:
        case FAULTY:
        case DISABLED:
        case LISTENING:
        case PRE_MASTER:
        case PASSIVE:
        case UNCALIBRATED:
            // Reset port data
            portDataReset();
            break;
        case MASTER:
            // Set own clock identity as GM identity
            portDataReset();
            event.event.gmClockUUID = 0;
            for(int i = 0; i < 8; ++i)
                event.event.gmClockUUID |=
                    static_cast<uint64_t>(tlv.portIdentity.clockIdentity.v[i])
                    << (8 * (7 - i));
            gmIdentity = tlv.portIdentity.clockIdentity;
            if(need_set_action) {
                msg_set_action(LOG_SYNC_INTERVAL);
                need_set_action = false;
            }
            break;
        case SLAVE:
            event.event.syncedWithGm = true;
            if(need_set_action) {
                msg_set_action(LOG_SYNC_INTERVAL);
                need_set_action = false;
            }
            break;
        default:
            PrintInfo("Unsupported portState: " + to_string(tlv.portState));
            break;
    }
    do_notify = true;
}
callback_define(CMLDS_INFO_NP)
{
    if(msg.getPeer().portNumber == 0)
        return;
    bool asCapable = tlv.as_capable > 0;
    if(event.event.asCapable != asCapable) {
        event.event.asCapable = asCapable;
        do_notify = true;
    } else
        // Skip client notification if no event changes
        PrintDebug("Ignore unchanged asCapable");
}
void ptpSet::event_handle()
{
    if(stopThread)
        return;
    do_notify = false;
    // Call the callbacks of the last message
    callHadler(msg);
    if(stopThread)
        return;
    if(do_notify) {
        event.copy();
        Client::NotifyClients(timeBaseIndex, PTPClock);
    }
}

/**
 * Runs the main event loop for handling PTP (Precision Time Protocol)
 *        events.
 *
 * This function enters an infinite loop, where it sends a GET request
 * with the intention of receiving a reply from the local PTP daemon.
 * If the GET request is successful, it waits for incoming messages
 * with a timeout and processes them if received. The loop terminates
 * if a message is successfully handled. After breaking out of the first
 * loop, the function sends a GET request to all destinations and
 * enters a second infinite loop to handle asynchronous events using epoll.
 *
 * @param arg A void pointer to an argument that can be passed to the function.
 *            The actual type and content of this argument should be
 *            defined by the user and cast appropriately within the function.
 * @return This function does not return a value. If a return value is needed,
 *         the function signature and implementation should be modified accordingly.
 *
 */
void ptpSet::thread_loop()
{
    bool lost_connection = false;
    if(stopThread)
        return;
    if(!event_subscription()) {
        PrintError("Failed to connect to ptp4l at " + udsAddr);
        lost_connection = true;
    } else {
        PrintInfo("Connected to ptp4l at " + udsAddr);
        msg_set_action(TIME_STATUS_NP);
        msg_set_action(PORT_DATA_SET);
        msg_set_action(LOG_SYNC_INTERVAL);
    }
    for(;;) {
        if(stopThread)
            return;
        bool ret = sku.poll(timeout_ms);
        if(stopThread)
            return;
        if(ret) {
            const auto cnt = sku.rcv(buf, bufSize);
            if(cnt > 0) {
                MNG_PARSE_ERROR_e err = msg.parse(buf, cnt);
                if(err == MNG_PARSE_ERROR_OK)
                    event_handle();
            }
        } else {
            for(;;) {
                if(stopThread)
                    return;
                if(event_subscription())
                    break;
                if(!lost_connection) {
                    PrintError("Lost connection to ptp4l at " + udsAddr);
                    portReset();
                    lost_connection = true;
                    if(stopThread)
                        return;
                    event.copy();
                    Client::NotifyClients(timeBaseIndex, PTPClock);
                }
                PrintInfo("Attempting to reconnect to ptp4l at " + udsAddr);
                // Wait 5 seconds before retrying
                for(int i = 0; i < 50 && !stopThread; i++)
                    this_thread::sleep_for(chrono::milliseconds(100));
            }
            if(lost_connection) {
                PrintInfo("Reconnected to ptp4l at " + udsAddr);
                lost_connection = false;
            }
        }
    }
}

bool ptpSet::init()
{
    string addr = baseAddr + to_string(param.domainNumber);
    if(!sku.setDefSelfAddress(addr) || !sku.init() ||
        !sku.setPeerAddress(udsAddr))
        return false;
    MsgParams prms = msg.getParams();
    prms.transportSpecific = param.transportSpecific;
    prms.domainNumber = param.domainNumber;
    return msg.updateParams(prms);
}

class Ptp4l : public ConnectSrv
{
  protected:
    bool isValid(const TimeBaseCfgFull &cfg) override final {
        return !cfg.udsAddrPtp4l.empty();
    }
    Thread4TimeBase *alloc(size_t timeBaseIndex,
        const TimeBaseCfgFull &cfg) override final {
        return new ptpSet(timeBaseIndex, cfg.base, cfg.udsAddrPtp4l);
    }

  public:
    Ptp4l() = default;
};
static Ptp4l instance;

/**
 * Establishes a connection to the local PTP (Precision Time Protocol)
 *        daemon.
 *
 * This method initializes a Unix socket connection to the local PTP daemon.
 * It sets up the message parameters, including the boundary hops and the
 * process ID as part of the clock identity.
 * It also configures the epoll instance to monitor the socket for incoming
 * data or errors.
 *
 * @return An integer indicating the status of the connection attempt.
 *         Returns 0 on success, or -1 if an error occurs during socket
 *         initialization, address setting, or epoll configuration.
 */
bool Client::connect_ptp4l()
{
    // Threads only read the events setting TLV
    // and send it to ptp4l without any change in the TLV itself
    eventsTlv.duration = UINT16_MAX;
    eventsTlv.setEvent(NOTIFY_TIME_SYNC);
    eventsTlv.setEvent(NOTIFY_PORT_STATE);
    eventsTlv.setEvent(NOTIFY_CMLDS);
    return instance.registerSrv();
}
