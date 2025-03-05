/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy connect ptp4l message class.
 *
 * @author Lai Peter Jun Ann <peter.jun.ann.lai@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "common/print.hpp"
#include "proxy/client.hpp"
#include "proxy/config_parser.hpp"
#include "proxy/connect_ptp4l.hpp"
#include "proxy/notification_msg.hpp"

// libptpmgmt
#include "msg.h"
#include "sock.h"
#include "msgCall.h"

#include <stdio.h>
#include <string>
#include <unistd.h>
#include <atomic>

__CLKMGR_NAMESPACE_USE;

using namespace std;
using namespace ptpmgmt;

static const uint64_t timeout_ms = 1000;
static const string baseAddr = "/var/run/pmc.";
static const size_t bufSize = 2000;
static SUBSCRIBE_EVENTS_NP_t eventsTlv; // Threads only read it!
/*
 * Modern CPUs are allow to run the threads before
 * the main thread end the initializing,
 * regardless of the code flow of the main thread.
 * The atomic guarantee it should not happens.
 * Threads will wait till initializing is done.
 */
static atomic<bool> all_init(false);

// Globals
map<int, ptp_event> ptp4lEvents;

// Callbacks of MessageDispatcher
#define callback_declare(n)\
    void n##_h(const ptpmgmt::Message &, const n##_t &tlv, const char *) override
#define callback_define(n)\
    void ptpSet::n##_h(const ptpmgmt::Message &, const n##_t &tlv, const char *)
// One set per a thread
class ptpSet : MessageDispatcher
{
  private:
    int index; // Index of the time base (timeBaseIndex)
    uint8_t domainNumber; // PTP Domain number
    const TimeBaseCfg &param; // time base configuration
    ptp_event &event;
    string udsAddr;
    ptpmgmt::Message msg;
    SockUnix sku;
    bool do_notify = false;
    char buf[bufSize];
    int seq = 0; // PTP message sequance
    // Used during initializing, before we create the thread
  public:
    ptpSet(const TimeBaseCfg &p) : index(p.timeBaseIndex),
        domainNumber(p.domainNumber), param(p),
        event(ptp4lEvents[p.timeBaseIndex]) {}
    bool init();
    void close() { sku.close(); }
    // Used inside the thread
    void thread_loop();
  private:
    void portDataSet(portState_e state, const ClockIdentity_t &id);
    callback_declare(TIME_STATUS_NP);
    callback_declare(PORT_PROPERTIES_NP) {
        portDataSet(tlv.portState, tlv.portIdentity.clockIdentity);
    }
    callback_declare(PORT_DATA_SET) {
        portDataSet(tlv.portState, tlv.portIdentity.clockIdentity);
    }
    callback_declare(CMLDS_INFO_NP);
    void event_handle();
    bool msg_send() {
        MNG_PARSE_ERROR_e err = msg.build(buf, bufSize, seq);
        if(err != MNG_PARSE_ERROR_OK) {
            PrintError(string("build error ") + msg.err2str_c(err));
            return false;
        }
        if(!sku.send(buf, msg.getMsgLen())) {
            #if 0
            PrintError("send failed");
            #endif
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

static vector<unique_ptr<ptpSet>> ptpSets;
static map<int, vector<sessionId_t>> subscribedClients;

void notify_client(int timeBaseIndex)
{
    PrintDebug("[clkmgr]::notify_client");
    for(const auto &sessionId : subscribedClients[timeBaseIndex]) {
        ProxyNotificationMessage *pmsg = new ProxyNotificationMessage();
        if(pmsg == nullptr) {
            PrintErrorCode("[clkmgr::notify_client] notifyMsg is nullptr !!");
            return;
        }
        unique_ptr<ProxyNotificationMessage> notifyMsg(pmsg);
        PrintDebug("[clkmgr::notify_client] notifyMsg creation is OK !!");
        // Send data for multiple sessions
        pmsg->setTimeBaseIndex(timeBaseIndex);
        PrintDebug("Get client session ID: " + to_string(sessionId));
        auto TxContext =
            Client::GetClientSession(sessionId).get()->get_transmitContext();
        if(!pmsg->transmitMessage(*TxContext))
            Client::RemoveClientSession(sessionId);
    }
}

callback_define(TIME_STATUS_NP)
{
    event.master_offset = tlv.master_offset;
    memcpy(event.gm_identity, tlv.gmIdentity.v, sizeof(event.gm_identity));
    do_notify = true;
    #if 0
    PrintDebug("master_offset = " + to_string(event.master_offset) +
        ", synced_to_primary_clock = " + to_string(event.synced_to_primary_clock));
    char buf[100];
    snprintf(buf, sizeof buf, "gm_identity = %02x%02x%02x.%02x%02x.%02x%02x%02x",
        event.gm_identity[0], event.gm_identity[1],
        event.gm_identity[2], event.gm_identity[3],
        event.gm_identity[4], event.gm_identity[5],
        event.gm_identity[6], event.gm_identity[7]);
    PrintDebug(buf);
    #endif
}
void ptpSet::portDataSet(portState_e state, const ClockIdentity_t &id)
{
    if(state == SLAVE)
        event.synced_to_primary_clock = true;
    else if(state == MASTER) {
        event.synced_to_primary_clock = false;
        // Set own clock identity as GM identity
        event.master_offset = 0;
        memcpy(event.gm_identity, id.v, sizeof(event.gm_identity));
    } else if(state <= PASSIVE) {
        event.synced_to_primary_clock = false;
        // Reset master offset and GM identity
        event.master_offset = 0;
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
    do_notify = false;
    // Call the callbacks of the last message
    callHadler(msg);
    if(do_notify)
        notify_client(index);
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
    if(!msg_set_action(PORT_PROPERTIES_NP))
        PrintDebug("Failed to get port properties\n");
    sku.poll(timeout_ms);
    ssize_t cnt = sku.rcv(buf, bufSize);
    if(cnt > 0) {
        MNG_PARSE_ERROR_e err = msg.parse(buf, cnt);
        if(err == MNG_PARSE_ERROR_OK)
            event_handle();
    }
    msg_set_action(PORT_PROPERTIES_NP);
    event_subscription();
    for(;;) {
        if(sku.poll(timeout_ms) > 0) {
            const auto cnt = sku.rcv(buf, bufSize);
            if(cnt > 0) {
                MNG_PARSE_ERROR_e err = msg.parse(buf, cnt);
                if(err == MNG_PARSE_ERROR_OK)
                    event_handle();
            }
        } else {
            for(;;) {
                if(event_subscription())
                    break;
                if(!lost_connection) {
                    PrintError("Lost connection to ptp4l at address: " + udsAddr);
                    PrintInfo("Resetting clkmgr's ptp4l data.");
                    event.master_offset = 0;
                    memset(event.gm_identity, 0, sizeof(event.gm_identity));
                    event.synced_to_primary_clock = false;
                    event.as_capable = 0;
                    lost_connection = true;
                    notify_client(index);
                }
                PrintInfo("Attempting to reconnect to ptp4l at address: " +
                    udsAddr);
                sleep(2);
            }
            if(lost_connection) {
                PrintInfo("Successful connected to ptp4l at address: " + udsAddr);
                lost_connection = false;
            }
        }
    }
}

void ptp4l_event_loop(ptpSet *set)
{
    // Ensure we start after initializing ends
    while(!all_init.load())
        sleep(1);
    set->thread_loop();
}

bool ptpSet::init()
{
    string addr = baseAddr + to_string(domainNumber);
    if(!sku.setDefSelfAddress(addr) || !sku.init() ||
        !sku.setPeerAddress(param.udsAddrPtp4l))
        return false;
    udsAddr = param.udsAddrPtp4l;
    MsgParams prms = msg.getParams();
    prms.transportSpecific = param.transportSpecific;
    prms.domainNumber = domainNumber;
    return msg.updateParams(prms);
}

int ConnectPtp4l::subscribe_ptp4l(int timeBaseIndex, sessionId_t sessionId)
{
    auto it = ptp4lEvents.find(timeBaseIndex);
    if(it != ptp4lEvents.end())
        // timeBaseIndex exists in the map
        subscribedClients[timeBaseIndex].push_back(sessionId);
    else
        // timeBaseIndex does not exist in the map
        PrintDebug("timeBaseIndex does not exist in the map");
    return 0;
}

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
int ConnectPtp4l::connect_ptp4l()
{
    // Create all sets required for the threads
    for(const auto &param : timeBaseCfgs) {
        // skip if ptp4l UDS address is empty
        if(*param.udsAddrPtp4l == 0)
            continue;
        ptpSet *set = new ptpSet(param);
        if(set == nullptr)
            return -1;
        ptpSets.push_back(unique_ptr<ptpSet>(set));
    }
    // initializing before creating the threads
    for(auto &set : ptpSets) {
        if(!set->init()) {
            // We need to close all other sockets, which succeed
            disconnect_ptp4l();
            return -1;
        }
    }
    /* Threads only read the events setting TLV
       and send it to ptp4l without any change in the TLV itself */
    eventsTlv.duration = UINT16_MAX;
    eventsTlv.setEvent(NOTIFY_TIME_SYNC);
    eventsTlv.setEvent(NOTIFY_PORT_STATE);
    eventsTlv.setEvent(NOTIFY_CMLDS);
    // Ensure threads start after we initializing
    all_init.store(true);
    for(auto &set : ptpSets)
        // Create a thread for each set
        thread(ptp4l_event_loop, set.get()).detach();
    // TODO why do we detach threads? Should we join them when we disconnect?
    return 0;
}

void ConnectPtp4l::disconnect_ptp4l()
{
    /* TODO Should we mark the threads to stop and
       join them before we close the sockets? */
    for(auto &set : ptpSets)
        set->close();
}
