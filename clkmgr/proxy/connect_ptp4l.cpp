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
    callback_declare(TIME_STATUS_NP);
    callback_declare(PORT_DATA_SET);
    callback_declare(CMLDS_INFO_NP);
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
     * @brief Subscribes to a set of PTP (Precision Time Protocol) events.
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
    event.master_offset = tlv.master_offset;
    memcpy(event.gm_identity, tlv.gmIdentity.v, sizeof(event.gm_identity));
    gmIdentity = tlv.gmIdentity;
    do_notify = true;
    PrintDebug("master_offset = " + to_string(event.master_offset) +
        ", synced_to_primary_clock = " + to_string(event.synced_to_primary_clock));
    char buf[100];
    snprintf(buf, sizeof buf, "gm_identity = %02x%02x%02x.%02x%02x.%02x%02x%02x",
        event.gm_identity[0], event.gm_identity[1],
        event.gm_identity[2], event.gm_identity[3],
        event.gm_identity[4], event.gm_identity[5],
        event.gm_identity[6], event.gm_identity[7]);
    PrintDebug(buf);
}
void ptpSet::portDataReset()
{
    event.portClear();
    need_set_action = true;
}
callback_define(PORT_DATA_SET)
{
    if(gmIdentity == tlv.portIdentity.clockIdentity) {
        if(tlv.portState == MASTER)
            event.ptp4l_sync_interval =
                pow(2.0, tlv.logSyncInterval) * USEC_PER_SEC;
        return;
    }
    if(tlv.portState == SLAVE) {
        event.synced_to_primary_clock = true;
        if(need_set_action) {
            msg_set_action(PORT_DATA_SET);
            need_set_action = false;
        }
    } else if(tlv.portState == MASTER) {
        // Set own clock identity as GM identity
        portDataReset();
        memcpy(event.gm_identity, tlv.portIdentity.clockIdentity.v,
            sizeof(event.gm_identity));
    } else if(tlv.portState <= UNCALIBRATED) {
        // Reset master offset and GM identity
        portDataReset();
        memset(event.gm_identity, 0, sizeof(event.gm_identity));
    }
    do_notify = true;
}
callback_define(CMLDS_INFO_NP)
{
    bool asCapable = tlv.as_capable > 0;
    if(event.as_capable != asCapable) {
        event.as_capable = asCapable;
        do_notify = true;
    } else
        // Skip client notification if no event changes
        PrintDebug("Ignore unchanged as_capable");
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
        Client::NotifyClients(timeBaseIndex);
    }
}

/**
 * @brief Runs the main event loop for handling PTP (Precision Time Protocol)
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
                    portDataReset();
                    memset(event.gm_identity, 0, sizeof(event.gm_identity));
                    event.as_capable = false;
                    lost_connection = true;
                    if(stopThread)
                        return;
                    event.copy();
                    Client::NotifyClients(timeBaseIndex);
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
 * @brief Establishes a connection to the local PTP (Precision Time Protocol)
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
