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
#include "proxy/connect_ptp4l.hpp"
#include "proxy/notification_msg.hpp"
#include "proxy/thread.hpp"

/* libptpmgmt */
#include "msg.h"
#include "sock.h"

#include <stdio.h>
#include <string>
#include <unistd.h>

__CLKMGR_NAMESPACE_USE;

using namespace std;
using namespace ptpmgmt;

static const size_t bufSize = 2000;
static char buf[bufSize];
static ptpmgmt::Message msg;
static ptpmgmt::Message msgu;
static ptpmgmt::SockUnix sku;
static ptpmgmt::SockBase *sk = &sku;
static ptpmgmt::SUBSCRIBE_EVENTS_NP_t events_tlv;
ptp_event clockEvent = { 0 };

void notify_client()
{
    PrintDebug("[clkmgr]::notify_client");
    sessionId_t SessionId;
    unique_ptr<ProxyMessage> notifyMsg(new ProxyNotificationMessage());
    ProxyNotificationMessage *pmsg = dynamic_cast<decltype(pmsg)>(notifyMsg.get());
    if(pmsg == nullptr) {
        PrintErrorCode("[clkmgr::notify_client] notifyMsg is nullptr !!");
        return;
    }
    PrintDebug("[clkmgr::notify_client] notifyMsg creation is OK !!");
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
            PrintError("Unable to get Session ID");
    }
}

void event_handle()
{
    const BaseMngTlv *data = msg.getData();
    switch(msg.getTlvId()) {
        case TIME_STATUS_NP: {
            const auto *timeStatus = static_cast<const TIME_STATUS_NP_t *>(data);
            clockEvent.master_offset = timeStatus->master_offset;
            memcpy(clockEvent.gm_identity, timeStatus->gmIdentity.v,
                sizeof(clockEvent.gm_identity));
            #if 0
            PrintDebug("master_offset = " + to_string(clockEvent.master_offset) +
                ", synced_to_primary_clock = " +
                to_string(clockEvent.synced_to_primary_clock));
            char buf[100];
            snprintf(buf, sizeof buf,
                "gm_identity = %02x%02x%02x.%02x%02x.%02x%02x%02x",
                clockEvent.gm_identity[0], clockEvent.gm_identity[1],
                clockEvent.gm_identity[2], clockEvent.gm_identity[3],
                clockEvent.gm_identity[4], clockEvent.gm_identity[5],
                clockEvent.gm_identity[6], clockEvent.gm_identity[7]);
            PrintDebug(buf);
            #endif
            break;
        }
        case PORT_PROPERTIES_NP: /* Get initial port state when Proxy starts */
        case PORT_DATA_SET: {
            const auto *portDataSet = static_cast<const PORT_DATA_SET_t *>(data);
            portState_e portState = portDataSet->portState;
            clockEvent.synced_to_primary_clock = false;
            if(portState == SLAVE)
                clockEvent.synced_to_primary_clock = true;
            else if(portState == MASTER) {
                /* Set own clock identity as GM identity */
                clockEvent.master_offset = 0;
                memcpy(clockEvent.gm_identity,
                    portDataSet->portIdentity.clockIdentity.v,
                    sizeof(clockEvent.gm_identity));
                break;
            } else if(portState <= PASSIVE) {
                /* Reset master offset and GM identity */
                clockEvent.master_offset = 0;
                memset(clockEvent.gm_identity, 0, sizeof(clockEvent.gm_identity));
            }
            break;
        }
        case CMLDS_INFO_NP: {
            const auto *cmldsInfo = static_cast<const CMLDS_INFO_NP_t *>(data);
            bool asCapable = cmldsInfo->as_capable > 0 ? true : false;
            /* Skip client notification if no event changes */
            if(clockEvent.as_capable == asCapable) {
                PrintDebug("Ignore unchanged as_capable");
                return;
            }
            clockEvent.as_capable = asCapable;
            break;
        }
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
        PrintError(string("build error ") + msg.err2str_c(err));
        return false;
    }
    bool ret;
    ret = sk->send(buf, m->getMsgLen());
    if(!ret) {
        #if 0
        PrintError("send failed");
        #endif
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
        PrintError(string("Fail get ") + msg.mng2str_c(id));
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
bool event_subscription(clkmgr_handle **handle)
{
    memset(events_tlv.bitmask, 0, sizeof events_tlv.bitmask);
    events_tlv.duration = UINT16_MAX;
    events_tlv.setEvent(NOTIFY_TIME_SYNC);
    events_tlv.setEvent(NOTIFY_PORT_STATE);
    events_tlv.setEvent(NOTIFY_CMLDS);
    if(!msg.setAction(SET, SUBSCRIBE_EVENTS_NP, &events_tlv)) {
        PrintError("Fail set SUBSCRIBE_EVENTS_NP");
        return false;
    }
    bool ret = msg_send(false);
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
    for(;;) {
        if(sk->poll(timeout_ms) > 0) {
            const auto cnt = sk->rcv(buf, bufSize);
            if(cnt > 0) {
                MNG_PARSE_ERROR_e err = msg.parse(buf, cnt);
                if(err == MNG_PARSE_ERROR_OK)
                    event_handle();
            }
        } else {
            for(;;) {
                if(event_subscription(nullptr))
                    break;
                if(!lost_connection) {
                    PrintError("Lost connection to ptp4l.");
                    PrintInfo("Resetting clkmgr's ptp4l data.");
                    clockEvent.master_offset = 0;
                    memset(clockEvent.gm_identity, 0,
                        sizeof(clockEvent.gm_identity));
                    clockEvent.synced_to_primary_clock = false;
                    clockEvent.as_capable = 0;
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
int Connect::connect(uint8_t transport_specific)
{
    clockEvent.ptp4l_id = 1;
    const char *uds_address = "/var/run/ptp4l";
    if(!sku.setDefSelfAddress() || !sku.init() ||
        !sku.setPeerAddress(uds_address))
        return -1;
    /* Set Transport Specific */
    MsgParams prms = msg.getParams();
    prms.transportSpecific = transport_specific;
    msg.updateParams(prms);
    handle_connect();
    return 0;
}

void Connect::disconnect()
{
    sk->close();
}
