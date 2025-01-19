/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client notification message class.
 * Implements client specific notification message function.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "pub/clkmgr/client_state.h"
#include "client/notification_msg.hpp"
#include "client/subscribe_msg.hpp"
#include "common/message.hpp"
#include "common/print.hpp"
#include "common/serialize.hpp"

#include <algorithm>

__CLKMGR_NAMESPACE_USE;

using namespace std;

std::vector<ClientState *> ClientNotificationMessage::ClientStateArray;

/**
 * Create the ClientNotificationMessage object
 * @param msg msg structure to be fill up
 * @param LxContext proxy transport listener context
 * @return true
 */
MAKE_RXBUFFER_TYPE(ClientNotificationMessage::buildMessage)
{
    msg = new ClientNotificationMessage();
    return true;
}

/**
 * @brief Add proxy's NOTIFY_MESSAGE type and its builder to transport layer.
 * This function will be called during init to add a map of NOTIFY_MESSAGE
 * type and its corresponding buildMessage function.
 *
 * @return true
 */
bool ClientNotificationMessage::initMessage()
{
    addMessageType(parseMsgMapElement_t(NOTIFY_MESSAGE, buildMessage));
    return true;
}

BUILD_TXBUFFER_TYPE(ClientNotificationMessage::makeBuffer) const
{
    PrintDebug("[ClientNotificationMessage]::makeBuffer");
    return true;
}

void ClientNotificationMessage::addClientState(ClientState *newClientState)
{
    ClientStateArray.push_back(newClientState);
}

void ClientNotificationMessage::deleteClientState(ClientState *newClientState)
{
    ClientStateArray.erase(std::remove(ClientStateArray.begin(),
            ClientStateArray.end(), newClientState), ClientStateArray.end());
}

void ClientNotificationMessage::handleEventUpdate(uint32_t eventSub,
    uint32_t eventFlag, bool &currentState, const bool &newState,
    std::atomic<int> &eventCount, bool &compositeEvent)
{
    if(eventSub & eventFlag) {
        if(currentState != newState) {
            currentState = newState;
            eventCount.fetch_add(1, std::memory_order_relaxed);
        }
        compositeEvent &= newState;
    }
}

void ClientNotificationMessage::handleGmOffsetEvent(uint32_t eventSub,
    uint32_t eventFlag, int64_t &masterOffset, const uint32_t &newMasterOffset,
    std::atomic<int> &eventCount, bool &withinBoundary, uint32_t lowerBound,
    uint32_t upperBound)
{
    if(eventSub & eventFlag) {
        if(masterOffset != newMasterOffset) {
            masterOffset = newMasterOffset;
            bool isWithinBoundary = (masterOffset > lowerBound) &&
                (masterOffset < upperBound);
            if(withinBoundary != isWithinBoundary) {
                withinBoundary = isWithinBoundary;
                eventCount.fetch_add(1, std::memory_order_relaxed);
            }
        }
    }
}

PROCESS_MESSAGE_TYPE(ClientNotificationMessage::processMessage)
{
    PrintDebug("[ClientNotificationMessage]::processMessage ");
    bool old_composite_event;
    /* Need to walk thru the whole vector */
    std::vector <ClientState *>::iterator it ;
    for(it = ClientStateArray.begin(); it < ClientStateArray.end(); it++) {
        uint32_t eventSub;
        uint32_t composite_eventSub;
        ClientState *currentClientState = *it;
        timespec last_notification_time = {};
        if(clock_gettime(CLOCK_REALTIME, &last_notification_time) == -1)
            PrintDebug("ClientNotificationMessage::processMessage \
                clock_gettime failed.\n");
        else
            currentClientState->set_last_notification_time(last_notification_time);
        Event_state &clkmgrCurrentState = currentClientState->get_eventState();
        eventSub = currentClientState->get_eventSub().get_event_mask();
        composite_eventSub =
            currentClientState->get_eventSub().get_composite_event_mask();
        /* Get the correct client_ptp_data according to our current sessionID */
        client_ptp_event *client_ptp_data = nullptr;
        client_ptp_data = ClientSubscribeMessage::getClientPtpEventStruct(
                currentClientState->get_sessionId());
        client_ptp_event *composite_client_ptp_data = nullptr;
        composite_client_ptp_data =
            ClientSubscribeMessage::getClientPtpEventCompositeStruct(
                currentClientState->get_sessionId());
        if(client_ptp_data == nullptr) {
            PrintDebug("ClientNotificationMessage::processMessage \
                ERROR in obtaining client_ptp_data.\n");
            return false;
        }
        if((eventSub & eventGMOffset) &&
            (proxy_data.master_offset != client_ptp_data->master_offset)) {
            client_ptp_data->master_offset = proxy_data.master_offset;
            if(currentClientState->get_eventSub().in_range(thresholdGMOffset,
                    client_ptp_data->master_offset)) {
                if(!(client_ptp_data->master_offset_in_range)) {
                    client_ptp_data->master_offset_in_range = true;
                    client_ptp_data->offset_in_range_event_count.fetch_add(1,
                        std::memory_order_relaxed);
                }
            } else {
                if((client_ptp_data->master_offset_in_range)) {
                    client_ptp_data->master_offset_in_range = false;
                    client_ptp_data->offset_in_range_event_count.fetch_add(1,
                        std::memory_order_relaxed);
                }
            }
        }
        if((eventSub & eventSyncedToGM) &&
            (proxy_data.synced_to_primary_clock !=
                client_ptp_data->synced_to_primary_clock)) {
            client_ptp_data->synced_to_primary_clock =
                proxy_data.synced_to_primary_clock;
            client_ptp_data->synced_to_gm_event_count.fetch_add(1,
                std::memory_order_relaxed);
        }
        if((eventSub & eventGMChanged) &&
            (memcmp(client_ptp_data->gm_identity, proxy_data.gm_identity,
                    sizeof(proxy_data.gm_identity)) != 0)) {
            memcpy(client_ptp_data->gm_identity, proxy_data.gm_identity,
                sizeof(proxy_data.gm_identity));
            client_ptp_data->gm_changed_event_count.fetch_add(1,
                std::memory_order_relaxed);
            clkmgrCurrentState.gm_changed = true;
        }
        if((eventSub & eventASCapable) &&
            (proxy_data.as_capable != client_ptp_data->as_capable)) {
            client_ptp_data->as_capable = proxy_data.as_capable;
            client_ptp_data->as_capable_event_count.fetch_add(1,
                std::memory_order_relaxed);
        }
        if(composite_eventSub) {
            old_composite_event = composite_client_ptp_data->composite_event;
            composite_client_ptp_data->composite_event = true;
        }
        if(composite_eventSub & eventGMOffset) {
            composite_client_ptp_data->master_offset = proxy_data.master_offset;
            if(currentClientState->get_eventSub().in_range(thresholdGMOffset,
                    client_ptp_data->master_offset))
                composite_client_ptp_data->composite_event = true;
            else
                composite_client_ptp_data->composite_event = false;
        }
        if(composite_eventSub & eventSyncedToGM)
            composite_client_ptp_data->composite_event &=
                proxy_data.synced_to_primary_clock;
        if(composite_eventSub & eventASCapable)
            composite_client_ptp_data->composite_event &= proxy_data.as_capable;
        if(composite_eventSub &&
            (old_composite_event != composite_client_ptp_data->composite_event))
            client_ptp_data->composite_event_count.fetch_add(1,
                std::memory_order_relaxed);
        clkmgrCurrentState.as_capable = client_ptp_data->as_capable;
        clkmgrCurrentState.offset_in_range =
            client_ptp_data->master_offset_in_range;
        clkmgrCurrentState.clock_offset = client_ptp_data->master_offset;
        clkmgrCurrentState.notification_timestamp = last_notification_time.tv_sec;
        clkmgrCurrentState.notification_timestamp *= NSEC_PER_SEC;
        clkmgrCurrentState.notification_timestamp += last_notification_time.tv_nsec;
        clkmgrCurrentState.synced_to_primary_clock =
            client_ptp_data->synced_to_primary_clock;
        clkmgrCurrentState.composite_event =
            composite_client_ptp_data->composite_event;
        memcpy(clkmgrCurrentState.gm_identity, client_ptp_data->gm_identity,
            sizeof(client_ptp_data->gm_identity));
        if(proxy_data.chrony_offset != client_ptp_data->chrony_offset) {
            client_ptp_data->chrony_offset = proxy_data.chrony_offset;
            if(currentClientState->get_eventSub().in_range(thresholdChronyOffset,
                    client_ptp_data->chrony_offset)) {
                if(!(client_ptp_data->chrony_offset_in_range)) {
                    client_ptp_data->chrony_offset_in_range = true;
                    client_ptp_data->chrony_offset_in_range_event_count.fetch_add(1,
                        std::memory_order_relaxed);
                }
            } else {
                if((client_ptp_data->chrony_offset_in_range)) {
                    client_ptp_data->chrony_offset_in_range = false;
                    client_ptp_data->chrony_offset_in_range_event_count.fetch_add(1,
                        std::memory_order_relaxed);
                }
            }
        }
        clkmgrCurrentState.chrony_clock_offset = client_ptp_data->chrony_offset;
        clkmgrCurrentState.chrony_offset_in_range =
            client_ptp_data->chrony_offset_in_range;
        client_ptp_data->chrony_reference_id = proxy_data.chrony_reference_id;
        clkmgrCurrentState.chrony_reference_id =
            client_ptp_data->chrony_reference_id;
        client_ptp_data->polling_interval = proxy_data.polling_interval;
        clkmgrCurrentState.polling_interval =
            client_ptp_data->polling_interval;
        // Update Event_count
        Event_count clkmgrCurrentEventCount =
            currentClientState->get_eventStateCount();
        clkmgrCurrentEventCount.offset_in_range_event_count =
            client_ptp_data->offset_in_range_event_count;
        clkmgrCurrentEventCount.as_capable_event_count =
            client_ptp_data->as_capable_event_count;
        clkmgrCurrentEventCount.synced_to_gm_event_count =
            client_ptp_data->synced_to_gm_event_count;
        clkmgrCurrentEventCount.gm_changed_event_count =
            client_ptp_data->gm_changed_event_count;
        clkmgrCurrentEventCount.composite_event_count =
            client_ptp_data->composite_event_count;
        clkmgrCurrentEventCount.chrony_offset_in_range_event_count =
            client_ptp_data->chrony_offset_in_range_event_count;
        currentClientState->set_eventStateCount(clkmgrCurrentEventCount);
    }
    return true;
}

PARSE_RXBUFFER_TYPE(ClientNotificationMessage::parseBuffer)
{
    PrintDebug("[ClientNotificationMessage]::parseBuffer ");
    if(!Message::parseBuffer(LxContext))
        return false;
    if(!PARSE_RX(FIELD, proxy_data, LxContext))
        return false;
    return true;
}
