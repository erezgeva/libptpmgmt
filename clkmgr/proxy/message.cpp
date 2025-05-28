/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy message base implementation.
 * Extended for specific messages such as connect and subscribe.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "proxy/message.hpp"
#include "proxy/connect_msg.hpp"
#include "proxy/notification_msg.hpp"
#include "proxy/subscribe_msg.hpp"
#include "proxy/connect_chrony.hpp"
#include "proxy/connect_ptp4l.hpp"
#include "proxy/client.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

bool ProxyMessage::Register()
{
    reg_message_type<ProxyConnectMessage, ProxySubscribeMessage>();
    // ProxyNotificationMessage - Proxy send it only, never send from client
    return true;
}

void ProxyMessage::notify_clients(size_t timeBaseIndex,
    vector<sessionId_t> &subscribedClients,
    vector<sessionId_t> &sessionIdToRemove)
{
    ProxyNotificationMessage *pmsg = new ProxyNotificationMessage();
    if(pmsg == nullptr) {
        PrintErrorCode("[clkmgr::notify_client] notifyMsg is nullptr !!");
        return;
    }
    // Release message on function ends
    unique_ptr<ProxyNotificationMessage> notifyMsg(pmsg);
    PrintDebug("[clkmgr::notify_client] notifyMsg creation is OK !!");
    // Send data for multiple sessions
    pmsg->setTimeBaseIndex(timeBaseIndex);
    for(auto it = subscribedClients.begin(); it != subscribedClients.end();) {
        const sessionId_t sessionId = *it;
        PrintDebug("Get client session ID: " + to_string(sessionId));
        pmsg->set_sessionId(sessionId);
        if(!pmsg->transmitMessage()) {
            it = subscribedClients.erase(it);
            /* Add sessionId into the list to remove */
            sessionIdToRemove.push_back(sessionId);
        } else
            ++it;
    }
}

void ProxyMessage::remove_clients(const vector<sessionId_t> &sessionIdToRemove)
{
    for(const sessionId_t sessionId : sessionIdToRemove) {
        ConnectPtp4l::remove_ptp4l_subscriber(sessionId);
        #ifdef HAVE_LIBCHRONY
        ConnectChrony::remove_chrony_subscriber(sessionId);
        #endif
        Client::RemoveClient(sessionId);
    }
}
