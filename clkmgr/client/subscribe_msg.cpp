/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client subscribe message class.
 * Implements client specific functionality.
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "client/notification_msg.hpp"
#include "client/subscribe_msg.hpp"
#include "common/print.hpp"
#include "common/serialize.hpp"

__CLKMGR_NAMESPACE_USE

using namespace std;

clkmgr_event_state *ClientSubscribeMessage::clkmgrCurrentState = nullptr;
ClientState *ClientSubscribeMessage::currentClientState = nullptr;
std::map<sessionId_t, std::array<client_ptp_event *, 2>>
    ClientSubscribeMessage::client_ptp_event_map;

/** @brief Create the ClientSubscribeMessage object
 *
 * @param msg msg structure to be fill up
 * @param LxContext client run-time transport listener context
 * @return true
 */
MAKE_RXBUFFER_TYPE(ClientSubscribeMessage::buildMessage)
{
    PrintDebug("[ClientSubscribeMessage]::buildMessage()");
    msg = new ClientSubscribeMessage();
    return true;
}

/** @brief Add client's SUBSCRIBE_MSG type and its builder to transport layer.
 *
 * This function will be called during init to add a map of SUBSCRIBE_MSG
 * type and its corresponding buildMessage function.
 *
 * @return true
 */
bool ClientSubscribeMessage::initMessage()
{
    PrintDebug("[ClientSubscribeMessage]::initMessage()");
    addMessageType(parseMsgMapElement_t(SUBSCRIBE_MSG, buildMessage));
    return true;
}

void ClientSubscribeMessage::setClientState(ClientState *newClientState)
{
    currentClientState = newClientState;
    clkmgrCurrentState = &(newClientState->get_eventState());
}

PARSE_RXBUFFER_TYPE(ClientSubscribeMessage::parseBuffer)
{
    ptp_event data = {};
    std::uint32_t eventSub;
    struct timespec last_notification_time = {};
    if(clock_gettime(CLOCK_REALTIME, &last_notification_time) == -1)
        PrintDebug("ClientNotificationMessage::processMessage \
            clock_gettime failed.\n");
    else
        currentClientState->set_last_notification_time(last_notification_time);
    eventSub = currentClientState->get_eventSub().get_event_mask();
    std::uint32_t composite_eventSub;
    composite_eventSub =
        currentClientState->get_eventSub().get_composite_event_mask();
    PrintDebug("[ClientSubscribeMessage]::parseBuffer ");
    if(!CommonSubscribeMessage::parseBuffer(LxContext))
        return false;
    if(!PARSE_RX(FIELD, data, LxContext))
        return false;
    /* TODO :
    1. Remove the pair if the sessionID is terminated (disconnect)
    2. to move some/all processing inside the processMessage instead of here.
    */
    sessionId_t currentSessionID = currentClientState->get_sessionId();
    std::map <sessionId_t, std::array<client_ptp_event *, 2>>::iterator it;
    client_ptp_event *client_data, *composite_client_data;
    it = client_ptp_event_map.find(currentSessionID);
    if(it == client_ptp_event_map.end()) {
        /* Creation of a new map item for this new sessionID */
        client_data = new client_ptp_event();
        composite_client_data = new client_ptp_event();
        client_ptp_event_map.insert(\
            std::pair<sessionId_t, \
            std::array<client_ptp_event *, 2>>\
            (currentSessionID, {client_data, composite_client_data}));
    } else {
        /* Reuse the current client data */
        client_data = it->second[0];
        composite_client_data = it->second[1];
    }
    if((eventSub & eventGMOffset) &&
        (data.master_offset != client_data->master_offset)) {
        client_data->master_offset = data.master_offset;
        if(currentClientState->get_eventSub().in_range(thresholdGMOffset,
                client_data->master_offset))
            client_data->master_offset_in_range = true;
    }
    if((eventSub & eventSyncedToGM) &&
        (data.synced_to_primary_clock != client_data->synced_to_primary_clock))
        client_data->synced_to_primary_clock = data.synced_to_primary_clock;
    if((eventSub & eventGMChanged) &&
        (memcmp(client_data->gm_identity, data.gm_identity,
                sizeof(data.gm_identity))) != 0) {
        memcpy(client_data->gm_identity, data.gm_identity,
            sizeof(data.gm_identity));
        clkmgrCurrentState->gm_changed = true;
    }
    if((eventSub & eventASCapable) &&
        (data.as_capable != client_data->as_capable))
        client_data->as_capable = data.as_capable;
    if(composite_eventSub)
        composite_client_data->composite_event = true;
    if((composite_eventSub & eventGMOffset) &&
        (data.master_offset != composite_client_data->master_offset)) {
        composite_client_data->master_offset = data.master_offset;
        if(currentClientState->get_eventSub().in_range(thresholdGMOffset,
                client_data->master_offset))
            composite_client_data->composite_event = true;
        else
            composite_client_data->composite_event = false;
    }
    if(composite_eventSub & eventSyncedToGM)
        composite_client_data->composite_event &= data.synced_to_primary_clock;
    if(composite_eventSub & eventASCapable)
        composite_client_data->composite_event &= data.as_capable;
    clkmgrCurrentState->as_capable = client_data->as_capable;
    clkmgrCurrentState->offset_in_range = client_data->master_offset_in_range;
    clkmgrCurrentState->synced_to_primary_clock =
        client_data->synced_to_primary_clock;
    clkmgrCurrentState->composite_event = composite_client_data->composite_event;
    clkmgrCurrentState->clock_offset = client_data->master_offset;
    clkmgrCurrentState->notification_timestamp = last_notification_time.tv_sec;
    clkmgrCurrentState->notification_timestamp *= NSEC_PER_SEC;
    clkmgrCurrentState->notification_timestamp += last_notification_time.tv_nsec;
    memcpy(clkmgrCurrentState->gm_identity, client_data->gm_identity,
        sizeof(client_data->gm_identity));
    return true;
}

/** @brief process the reply for notification msg from proxy.
 *
 * This function will be called when the transport layer
 * in client runtime received a SUBSCRIBE_MSG type (an echo reply from
 * proxy)
 * In this case, transport layer will rx a buffer in the client associated
 * listening message queue (listening to proxy) and call this function when
 * the enum ID corresponding to the SUBSCRIBE_MSG is received.
 *
 * @param LxContext client run-time transport listener context
 * @param TxContext client run-time transport transmitter context
 * @return true
 */
PROCESS_MESSAGE_TYPE(ClientSubscribeMessage::processMessage)
{
    PrintDebug("[ClientSubscribeMessage]::processMessage (reply)");
    std::unique_lock<rtpi::mutex> lock(cv_mtx);
    currentClientState->set_subscribed(true);
    /* Add the current ClientState to the notification class */
    ClientNotificationMessage::addClientState(currentClientState);
    this->set_msgAck(ACK_NONE);
    cv.notify_one(lock);
    return true;
}

/* delete the client ptp event based on session ID given */
void ClientSubscribeMessage::deleteClientPtpEventStruct(sessionId_t sID)
{
    std::map <sessionId_t, std::array<client_ptp_event *, 2>>::iterator it;
    client_ptp_event *client_data, *composite_data;
    it = client_ptp_event_map.find(sID);
    if(it != client_ptp_event_map.end()) {
        client_data = it->second[0];
        composite_data = it->second[1];
        delete client_data;
        delete composite_data;
        client_ptp_event_map.erase(it);
    } else
        PrintDebug("The item is not found in client_ptp_event_map");
}

/* get the corresponding ClientPtpEvent */
client_ptp_event
*ClientSubscribeMessage::getClientPtpEventStruct(sessionId_t sID)
{
    std::map <sessionId_t, std::array<client_ptp_event *, 2>>::iterator it;
    client_ptp_event *client_data = nullptr;
    it = client_ptp_event_map.find(sID);
    if(it != client_ptp_event_map.end())
        client_data = it->second[0];
    return client_data;
}

/* get the corresponding ClientPtpEvent for composite  */
client_ptp_event *ClientSubscribeMessage::getClientPtpEventCompositeStruct(
    sessionId_t sID)
{
    std::map <sessionId_t, std::array<client_ptp_event *, 2>>::iterator it;
    client_ptp_event *client_data = nullptr;
    it = client_ptp_event_map.find(sID);
    if(it != client_ptp_event_map.end())
        client_data = it->second[1];
    return client_data;
}

/* reduce the corresponding eventCount */
void ClientSubscribeMessage::resetClientPtpEventStruct(sessionId_t sID,
    clkmgr_event_count &eventCount)
{
    std::map <sessionId_t, std::array<client_ptp_event *, 2>>::iterator it;
    client_ptp_event *client_ptp_data = nullptr;
    it = client_ptp_event_map.find(sID);
    if(it != client_ptp_event_map.end())
        client_ptp_data = it->second[0];
    else {
        PrintError("resetClientPtpEventStruct Failed.");
        return;
    }
    client_ptp_data->offset_in_range_event_count.fetch_sub(
        eventCount.offset_in_range_event_count,
        std::memory_order_relaxed);
    client_ptp_data->as_capable_event_count.fetch_sub(
        eventCount.as_capable_event_count,
        std::memory_order_relaxed);
    client_ptp_data->synced_to_gm_event_count.fetch_sub(
        eventCount.synced_to_gm_event_count,
        std::memory_order_relaxed);
    client_ptp_data->gm_changed_event_count.fetch_sub(
        eventCount.gm_changed_event_count,
        std::memory_order_relaxed);
    client_ptp_data->composite_event_count.fetch_sub(
        eventCount.composite_event_count,
        std::memory_order_relaxed);
    eventCount.offset_in_range_event_count =
        client_ptp_data->offset_in_range_event_count;
    eventCount.as_capable_event_count = client_ptp_data->as_capable_event_count;
    eventCount.synced_to_gm_event_count =
        client_ptp_data->synced_to_gm_event_count;
    eventCount.gm_changed_event_count = client_ptp_data->gm_changed_event_count;
    eventCount.composite_event_count = client_ptp_data->composite_event_count;
}
