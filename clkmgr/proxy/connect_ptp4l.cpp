/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file
 * @brief Proxy connect ptp4l message class.
 *
 * @author Lai Peter Jun Ann <peter.jun.ann.lai@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include <libgen.h>
#include <map>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>

#include <common/print.hpp>
#include <proxy/client.hpp>
#include <proxy/connect_ptp4l.hpp>
#include <proxy/notification_msg.hpp>
#include "thread.hpp"
#include "../../pub/init.h"

__CLKMGR_NAMESPACE_USE

using namespace std;
using namespace ptpmgmt;

static const size_t bufSize = 2000;
static char buf[bufSize];
static Init obj;
static ptpmgmt::Message &msg = obj.msg();
static ptpmgmt::Message msgu;
static SockBase *sk;

static std::unique_ptr<SockBase> m_sk;

portState_e portState;
SUBSCRIBE_EVENTS_NP_t d;
ptp_event pe = { 0, {0, 0, 0, 0, 0, 0, 0, 0}, 0, 0, 0};

void notify_client()
{
    PrintDebug("[clkmgr]::notify_client");
    sessionId_t SessionId;
    std::unique_ptr<ProxyMessage> notifyMsg(new ProxyNotificationMessage());
    ProxyNotificationMessage *pmsg = dynamic_cast<decltype(pmsg)>(notifyMsg.get());
    if(pmsg == nullptr) {
        PrintErrorCode("[clkmgr::notify_client] notifyMsg is nullptr !!\n");
        return;
    }
    PrintDebug("[clkmgr::notify_client] notifyMsg creation is OK !!\n");
    /* Send data for multiple sessions */
    for(size_t i = 0; i < Client::GetSessionCount(); i++) {
        SessionId = Client::GetSessionIdAt(i);
        if(SessionId != InvalidSessionId) {
            PrintDebug("Get client session ID: " + to_string(SessionId));
            auto TxContext = Client::GetClientSession(
                    SessionId).get()->get_transmitContext();
            if(!pmsg->transmitMessage(*TxContext))
                Client::RemoveClientSession(SessionId);
        } else
            PrintError("Unable to get Session ID\n");
    }
}

void event_handle()
{
    const BaseMngTlv *data = msg.getData();
    int64_t offset;
    ClockIdentity_t gm_uuid;
    uint8_t servo;
    PORT_DATA_SET_t *pd;
    switch(msg.getTlvId()) {
        case TIME_STATUS_NP:
            /* Workaround for ptp4l continue to send even gm is not present */
            if(portState < UNCALIBRATED)
                return;
            offset = ((TIME_STATUS_NP_t *)data)->master_offset;
            gm_uuid = ((TIME_STATUS_NP_t *)data)->gmIdentity;
            pe.master_offset = offset;
            memcpy(pe.gm_identity, gm_uuid.v, sizeof(pe.gm_identity));
            /* Uncomment for debug data printing */
            //printf("master_offset = %ld, synced_to_primary_clock = %d\n", \
            //      pe.master_offset, pe.synced_to_primary_clock);
            //printf("gm_identity = %02x%02x%02x.%02x%02x.%02x%02x%02x\n\n",
            //       pe.gm_identity[0], pe.gm_identity[1],pe.gm_identity[2],
            //       pe.gm_identity[3], pe.gm_identity[4],
            //       pe.gm_identity[5], pe.gm_identity[6],pe.gm_identity[7]);
            break;
        case PORT_DATA_SET:
            pd = (PORT_DATA_SET_t *)data;
            portState = pd->portState;
            /* Reset TIME_STATUS_NP data if port_state <= PASSIVE */
            if(portState < SLAVE) {
                pe.master_offset = 0;
                memset(pe.gm_identity, 0, sizeof(pe.gm_identity));
                pe.synced_to_primary_clock = false;
            } else
                pe.synced_to_primary_clock = true;
            break;
        case PORT_PROPERTIES_NP:
            /* Retrieve current port state when proxy is started */
            pd = (PORT_DATA_SET_t *)data;
            portState = pd->portState;
            pe.synced_to_primary_clock = portState >= SLAVE ? true : false;
            break;
        case CMLDS_INFO_NP:
            if(pe.as_capable == ((CMLDS_INFO_NP_t *)data)->as_capable) {
                PrintDebug("Ignore unchanged as_capable");
                return;
            }
            pe.as_capable = \
                ((CMLDS_INFO_NP_t *)data)->as_capable > 0 ? true : false;
            //printf("as_capable = %d\n\n", pe.as_capable);
            break;
        default:
            return;
    }
    notify_client();
}

static inline bool msg_send(bool local)
{
    static int seq = 0;
    ptpmgmt::Message *m;
    if(local)
        m = &msgu;
    else
        m = &msg;
    MNG_PARSE_ERROR_e err = m->build(buf, bufSize, seq);
    if(err != MNG_PARSE_ERROR_OK) {
        fprintf(stderr, "build error %s\n", msgu.err2str_c(err));
        return false;
    }
    bool ret;
    ret = sk->send(buf, m->getMsgLen());
    if(!ret) {
        //fprintf(stderr, "send failed\n");
        return false;
    }
    seq++;
    return true;
}

static inline bool msg_set_action(bool local, mng_vals_e id)
{
    bool ret;
    if(local)
        ret = msgu.setAction(GET, id);
    else
        ret = msg.setAction(GET, id);
    if(!ret) {
        fprintf(stderr, "Fail get %s\n", msg.mng2str_c(id));
        return false;
    }
    return msg_send(local);
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
bool event_subscription(struct clkmgr_handle **handle)
{
    memset(d.bitmask, 0, sizeof d.bitmask);
    d.duration = UINT16_MAX;
    d.setEvent(NOTIFY_TIME_SYNC);
    d.setEvent(NOTIFY_PORT_STATE);
    d.setEvent(NOTIFY_CMLDS);
    if(!msg.setAction(SET, SUBSCRIBE_EVENTS_NP, &d)) {
        fprintf(stderr, "Fail set SUBSCRIBE_EVENTS_NP\n");
        return false;
    }
    bool ret = msg_send(false);
    /* Remove referance to local SUBSCRIBE_EVENTS_NP_t */
    msg.clearData();
    return ret;
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
void *ptp4l_event_loop(void *arg)
{
    const uint64_t timeout_ms = 1000;
    bool lost_connection = false;
    for(;;) {
        if(!msg_set_action(true, PORT_PROPERTIES_NP))
            break;
        ssize_t cnt;
        sk->poll(timeout_ms);
        cnt = sk->rcv(buf, bufSize);
        if(cnt > 0) {
            MNG_PARSE_ERROR_e err = msg.parse(buf, cnt);
            if(err == MNG_PARSE_ERROR_OK) {
                event_handle();
                break;
            }
        }
    }
    msg_set_action(false, PORT_PROPERTIES_NP);
    event_subscription(nullptr);
    while(1) {
        if(sk->poll(timeout_ms) > 0) {
            const auto cnt = sk->rcv(buf, bufSize);
            if(cnt > 0) {
                MNG_PARSE_ERROR_e err = msg.parse(buf, cnt);
                if(err == MNG_PARSE_ERROR_OK)
                    event_handle();
            }
        } else {
            while(1) {
                if(event_subscription(nullptr))
                    break;
                if(!lost_connection) {
                    PrintError("Lost connection to ptp4l.");
                    PrintInfo("Resetting clkmgr's ptp4l data.");
                    pe.master_offset = 0;
                    memset(pe.gm_identity, 0, sizeof(pe.gm_identity));
                    pe.synced_to_primary_clock = false;
                    pe.as_capable = 0;
                    notify_client();
                    lost_connection = true;
                }
                PrintInfo("Attempting to reconnect to ptp4l...");
                sleep(2);
            }
            if(lost_connection) {
                PrintInfo("Successful connected to ptp4l.");
                lost_connection = false;
            }
        }
    }
}

/** @brief Establishes a connection to the local PTP (Precision Time Protocol)
 *         daemon.
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
int Connect::connect(uint8_t transport_specific)
{
    std::string uds_address;
    SockUnix *sku = new SockUnix;
    if(sku == nullptr)
        return -1;
    m_sk.reset(sku);
    pe.ptp4l_id = 1;
    uds_address = "/var/run/ptp4l";
    if(!sku->setDefSelfAddress() || !sku->init() ||
        !sku->setPeerAddress(uds_address))
        return -1;
    /* Set Transport Specific */
    MsgParams prms = msg.getParams();
    prms.transportSpecific = transport_specific;
    msg.updateParams(prms);
    sk = m_sk.get();
    handle_connect();
    return 0;
}

void Connect::disconnect()
{
    obj.close();
    sk->close();
}
