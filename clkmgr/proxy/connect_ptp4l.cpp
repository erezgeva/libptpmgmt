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
static ptpmgmt::SUBSCRIBE_EVENTS_NP_t eventsTlv;

std::vector<std::unique_ptr<ptpmgmt::Message>> msgList;
std::vector<std::unique_ptr<SockUnix>> sockets;
std::vector<SockBase *> socketList;
std::vector<ptpmgmt::Message *> msgs;
std::vector<std::tuple<SockBase *, ptpmgmt::Message *, int>> combinedList;
std::map<int, ptp_event> ptp4lEvents;
std::string baseAddr = "/var/run/pmc.";
std::map<int, std::vector<sessionId_t>> subscribedClients;

void notify_client(int timeBaseIndex)
{
    PrintDebug("[clkmgr]::notify_client");
    for(const auto &sessionId : subscribedClients[timeBaseIndex]) {
        unique_ptr<ProxyMessage> notifyMsg(new ProxyNotificationMessage());
        ProxyNotificationMessage *pmsg =
            dynamic_cast<decltype(pmsg)>(notifyMsg.get());
        if(pmsg == nullptr) {
            PrintErrorCode("[clkmgr::notify_client] notifyMsg is nullptr !!");
            return;
        }
        PrintDebug("[clkmgr::notify_client] notifyMsg creation is OK !!");
        /* Send data for multiple sessions */
        pmsg->setTimeBaseIndex(timeBaseIndex);
        PrintDebug("Get client session ID: " + to_string(sessionId));
        auto TxContext = Client::GetClientSession(
                sessionId).get()->get_transmitContext();
        if(!pmsg->transmitMessage(*TxContext))
            Client::RemoveClientSession(sessionId);
    }
}

void event_handle(ptpmgmt::Message *msg, int timeBaseIndex)
{
    const BaseMngTlv *data = msg->getData();
    switch(msg->getTlvId()) {
        case TIME_STATUS_NP: {
            const auto *timeStatus = static_cast<const TIME_STATUS_NP_t *>(data);
            ptp4lEvents[timeBaseIndex].master_offset =
                timeStatus->master_offset;
            memcpy(ptp4lEvents[timeBaseIndex].gm_identity,
                timeStatus->gmIdentity.v,
                sizeof(ptp4lEvents[timeBaseIndex].gm_identity));
            #if 0
            PrintDebug("master_offset = " +
                to_string(ptp4lEvents[timeBaseIndex].master_offset) +
                ", synced_to_primary_clock = " +
                to_string(
                    ptp4lEvents[timeBaseIndex].synced_to_primary_clock));
            char buf[100];
            snprintf(buf, sizeof buf,
                "gm_identity = %02x%02x%02x.%02x%02x.%02x%02x%02x",
                ptp4lEvents[timeBaseIndex].gm_identity[0],
                ptp4lEvents[timeBaseIndex].gm_identity[1],
                ptp4lEvents[timeBaseIndex].gm_identity[2],
                ptp4lEvents[timeBaseIndex].gm_identity[3],
                ptp4lEvents[timeBaseIndex].gm_identity[4],
                ptp4lEvents[timeBaseIndex].gm_identity[5],
                ptp4lEvents[timeBaseIndex].gm_identity[6],
                ptp4lEvents[timeBaseIndex].gm_identity[7]);
            PrintDebug(buf);
            #endif
            break;
        }
        case PORT_PROPERTIES_NP: /* Get initial port state when Proxy starts */
        case PORT_DATA_SET: {
            const auto *portDataSet = static_cast<const PORT_DATA_SET_t *>(data);
            portState_e portState = portDataSet->portState;
            ptp4lEvents[timeBaseIndex].synced_to_primary_clock = false;
            if(portState == SLAVE)
                ptp4lEvents[timeBaseIndex].synced_to_primary_clock = true;
            else if(portState == MASTER) {
                /* Set own clock identity as GM identity */
                ptp4lEvents[timeBaseIndex].master_offset = 0;
                memcpy(ptp4lEvents[timeBaseIndex].gm_identity,
                    portDataSet->portIdentity.clockIdentity.v,
                    sizeof(ptp4lEvents[timeBaseIndex].gm_identity));
                break;
            } else if(portState <= PASSIVE) {
                /* Reset master offset and GM identity */
                ptp4lEvents[timeBaseIndex].master_offset = 0;
                memset(ptp4lEvents[timeBaseIndex].gm_identity, 0,
                    sizeof(ptp4lEvents[timeBaseIndex].gm_identity));
            }
            break;
        }
        case CMLDS_INFO_NP: {
            const auto *cmldsInfo = static_cast<const CMLDS_INFO_NP_t *>(data);
            bool asCapable = cmldsInfo->as_capable > 0 ? true : false;
            /* Skip client notification if no event changes */
            if(ptp4lEvents[timeBaseIndex].as_capable == asCapable) {
                PrintDebug("Ignore unchanged as_capable");
                return;
            }
            ptp4lEvents[timeBaseIndex].as_capable = asCapable;
            break;
        }
        default:
            return;
    }
    notify_client(timeBaseIndex);
}

static inline bool msg_send(bool local)
{
    for(const auto &pair : combinedList) {
        SockBase *sk = std::get<0>(pair);
        ptpmgmt::Message *msg = std::get<1>(pair);
        static int seq = 0;
        ptpmgmt::Message *m;
        MNG_PARSE_ERROR_e err;
        if(local) {
            m = &msgu;
            err = m->build(buf, bufSize, seq);
        } else
            err = msg->build(buf, bufSize, seq);
        if(err != MNG_PARSE_ERROR_OK) {
            PrintError(string("build error ") + msg->err2str_c(err));
            return false;
        }
        bool ret;
        ret = sk->send(buf, msg->getMsgLen());
        if(!ret) {
            #if 0
            PrintError("send failed");
            #endif
            return false;
        }
        seq++;
    }
    return true;
}

static inline bool msg_set_action(bool local, mng_vals_e id)
{
    bool ret;
    for(const auto &pair : combinedList) {
        ptpmgmt::Message *msg = std::get<1>(pair);
        if(local)
            ret = msgu.setAction(GET, id);
        else
            ret = msg->setAction(GET, id);
        if(!ret) {
            PrintError(string("Fail get ") + msg->mng2str_c(id));
            return false;
        }
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
    bool ret = false;
    memset(eventsTlv.bitmask, 0, sizeof eventsTlv.bitmask);
    eventsTlv.duration = UINT16_MAX;
    eventsTlv.setEvent(NOTIFY_TIME_SYNC);
    eventsTlv.setEvent(NOTIFY_PORT_STATE);
    eventsTlv.setEvent(NOTIFY_CMLDS);
    for(const auto &pair : combinedList) {
        ptpmgmt::Message *msg = std::get<1>(pair);
        MsgParams prms = msg->getParams();
        if(!msg->setAction(SET, SUBSCRIBE_EVENTS_NP, &eventsTlv)) {
            PrintError("Fail set SUBSCRIBE_EVENTS_NP");
            return false;
        }
        ret = msg_send(false);
        /* Remove referance to local SUBSCRIBE_EVENTS_NP_t */
        msg->clearData();
    }
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
    if(!msg_set_action(true, PORT_PROPERTIES_NP))
        PrintDebug("Failed to get port properties\n");
    ssize_t cnt;
    for(const auto &pair : combinedList) {
        SockBase *sk = std::get<0>(pair);
        ptpmgmt::Message *msg = std::get<1>(pair);
        int timeBaseIndex = std::get<2>(pair);
        sk->poll(timeout_ms);
        cnt = sk->rcv(buf, bufSize);
        if(cnt > 0) {
            MNG_PARSE_ERROR_e err = msg->parse(buf, cnt);
            if(err == MNG_PARSE_ERROR_OK)
                event_handle(msg, timeBaseIndex);
        }
    }
    msg_set_action(false, PORT_PROPERTIES_NP);
    event_subscription(nullptr);
    for(;;) {
        for(const auto &pair : combinedList) {
            SockBase *sk = std::get<0>(pair);
            ptpmgmt::Message *msg = std::get<1>(pair);
            int timeBaseIndex = std::get<2>(pair);
            MsgParams prms;
            prms = msg->getParams();
            if(sk->poll(timeout_ms) > 0) {
                const auto cnt = sk->rcv(buf, bufSize);
                if(cnt > 0) {
                    MNG_PARSE_ERROR_e err = msg->parse(buf, cnt);
                    if(err == MNG_PARSE_ERROR_OK)
                        event_handle(msg, timeBaseIndex);
                }
            } else {
                for(;;) {
                    if(event_subscription(nullptr))
                        break;
                    if(!lost_connection) {
                        PrintError("Lost connection to ptp4l.");
                        PrintInfo("Resetting clkmgr's ptp4l data.");
                        ptp4lEvents[timeBaseIndex].master_offset = 0;
                        memset(ptp4lEvents[timeBaseIndex].gm_identity, 0,
                            sizeof(ptp4lEvents[timeBaseIndex].gm_identity));
                        ptp4lEvents[timeBaseIndex].synced_to_primary_clock = false;
                        ptp4lEvents[timeBaseIndex].as_capable = 0;
                        notify_client(prms.domainNumber);
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
}

int ConnectPtp4l::subscribe_ptp4l(int timeBaseIndex, sessionId_t sessionId)
{
    auto it = ptp4lEvents.find(timeBaseIndex);
    if(it != ptp4lEvents.end()) {
        /* timeBaseIndex exists in the map */
        subscribedClients[timeBaseIndex].push_back(sessionId);
    } else {
        /* timeBaseIndex does not exist in the map */
        PrintDebug("timeBaseIndex does not exist in the map");
    }
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
int ConnectPtp4l::connect_ptp4l(const std::vector<TimeBaseCfg> &params)
{
    std::vector<int> timeBaseIndexs;
    for(const auto &param : params) {
        /* skip if ptp4l UDS address is empty */
        if(param.udsAddrPtp4l.empty())
            continue;
        SockUnix *sku = new SockUnix;
        std::unique_ptr<ptpmgmt::Message> msg_ptr(new ptpmgmt::Message());
        msgList.push_back(std::move(msg_ptr));
        ptpmgmt::Message &msg = *msgList.back();
        if(sku == nullptr)
            return -1;
        std::unique_ptr<SockUnix> sku_ptr(sku);
        std::string addr = baseAddr + std::to_string(param.domainNumber);
        if(!sku->setDefSelfAddress(addr) || !sku->init() ||
            !sku->setPeerAddress(param.udsAddrPtp4l.c_str()))
            return -1;
        sockets.push_back(std::move(sku_ptr));
        MsgParams prms = msg.getParams();
        prms.transportSpecific = param.transportSpecific;
        prms.domainNumber = param.domainNumber;
        ptp4lEvents[param.timeBaseIndex] = ptp_event();
        timeBaseIndexs.push_back(param.timeBaseIndex);
        msg.updateParams(prms);
        msgs.push_back(&msg);
        SockBase &sk = *sockets.back();
        socketList.push_back(&sk);
    }
    for(size_t i = 0; i < socketList.size(); ++i)
        combinedList.push_back(std::make_tuple(socketList[i], std::move(msgs[i]),
                timeBaseIndexs[i]));
    handle_connect();
    return 0;
}

void ConnectPtp4l::disconnect_ptp4l()
{
    for(auto &sk : socketList)
        sk->close();
}
