/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file notification_msg.hpp
 * @brief Client notification message class.
 * Implements client specific notification message function.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#include <algorithm>
#include <client/notification_msg.hpp>
#include <client/subscribe_msg.hpp>
#include <common/message.hpp>
#include <common/print.hpp>
#include <common/serialize.hpp>
#include "jclk_client_state.hpp"

using namespace JClkLibCommon;
using namespace JClkLibClient;
using namespace std;

/** @brief Create the ClientNotificationMessage object
 *
 * @param msg msg structure to be fill up
 * @param LxContext proxy transport listener context
 * @return true
 */
MAKE_RXBUFFER_TYPE(ClientNotificationMessage::buildMessage)
{
    msg = new ClientNotificationMessage();
    return true;
}

/** @brief Add proxy's NOTIFY_MESSAGE type and its builder to transport layer.
 *
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
    if(eventSub & (1 << eventFlag)) {
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
    if(eventSub & (1 << eventFlag)) {
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
        std::uint32_t eventSub[1];
        std::uint32_t composite_eventSub[1];
        ClientState *currentClientState = *it;
        struct timespec last_notification_time = {};
        if(clock_gettime(CLOCK_MONOTONIC, &last_notification_time) == -1)
            PrintDebug("ClientNotificationMessage::processMessage \
                clock_gettime failed.\n");
        else
            currentClientState->set_last_notification_time(last_notification_time);
        double seconds = last_notification_time.tv_sec;
        double nanoseconds = last_notification_time.tv_nsec / 1e9;
        jcl_state &jclCurrentState =
            currentClientState->get_eventState();
        jcl_state_event_count &jclCurrentEventCount =
            currentClientState->get_eventStateCount();
        currentClientState->get_eventSub().get_event().readEvent(eventSub,
            (std::size_t)sizeof(eventSub));
        currentClientState->get_eventSub().get_composite_event().readEvent(
            composite_eventSub,
            (std::size_t)sizeof(composite_eventSub));
        /* Get the correct client_ptp_data according to our current sessionID */
        JClkLibCommon::client_ptp_event *client_ptp_data = nullptr;
        client_ptp_data = ClientSubscribeMessage::getClientPtpEventStruct(
                currentClientState->get_sessionId());
        JClkLibCommon::client_ptp_event *composite_client_ptp_data = nullptr;
        composite_client_ptp_data =
            ClientSubscribeMessage::getClientPtpEventCompositeStruct(
                currentClientState->get_sessionId());
        int64_t lower_master_offset =
            currentClientState->get_eventSub().get_value().getLower(gmOffsetValue);
        int64_t upper_master_offset =
            currentClientState->get_eventSub().get_value().getUpper(gmOffsetValue);
        if(client_ptp_data == nullptr) {
            PrintDebug("ClientNotificationMessage::processMessage \
                ERROR in obtaining client_ptp_data.\n");
            return false;
        }
        if((eventSub[0] & 1 << gmOffsetEvent) &&
            (proxy_data.master_offset != client_ptp_data->master_offset)) {
            client_ptp_data->master_offset = proxy_data.master_offset;
            if((client_ptp_data->master_offset > lower_master_offset) &&
                (client_ptp_data->master_offset < upper_master_offset)) {
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
        if((eventSub[0] & 1 << syncedToPrimaryClockEvent) &&
            (proxy_data.synced_to_primary_clock !=
                client_ptp_data->synced_to_primary_clock)) {
            client_ptp_data->synced_to_primary_clock =
                proxy_data.synced_to_primary_clock;
            client_ptp_data->synced_to_primary_clock_event_count.fetch_add(1,
                std::memory_order_relaxed);
        }
        if((eventSub[0] & 1 << gmChangedEvent) &&
            (memcmp(client_ptp_data->gm_identity, proxy_data.gm_identity,
                    sizeof(proxy_data.gm_identity)) != 0)) {
            memcpy(client_ptp_data->gm_identity, proxy_data.gm_identity,
                sizeof(proxy_data.gm_identity));
            client_ptp_data->gm_changed_event_count.fetch_add(1,
                std::memory_order_relaxed);
            jclCurrentState.gm_changed = true;
        } else
            jclCurrentState.gm_changed = false;
        if((eventSub[0] & 1 << asCapableEvent) &&
            (proxy_data.as_capable != client_ptp_data->as_capable)) {
            client_ptp_data->as_capable = proxy_data.as_capable;
            client_ptp_data->as_capable_event_count.fetch_add(1,
                std::memory_order_relaxed);
        }
        if(composite_eventSub[0]) {
            old_composite_event = composite_client_ptp_data->composite_event;
            composite_client_ptp_data->composite_event = true;
        }
        if(composite_eventSub[0] & 1 << gmOffsetEvent) {
            composite_client_ptp_data->master_offset = proxy_data.master_offset;
            if((composite_client_ptp_data->master_offset > lower_master_offset) &&
                (composite_client_ptp_data->master_offset < upper_master_offset))
                composite_client_ptp_data->composite_event = true;
            else
                composite_client_ptp_data->composite_event = false;
        }
        if(composite_eventSub[0] & 1 << syncedToPrimaryClockEvent)
            composite_client_ptp_data->composite_event &=
                proxy_data.synced_to_primary_clock;
        if(composite_eventSub[0] & 1 << asCapableEvent)
            composite_client_ptp_data->composite_event &= proxy_data.as_capable;
        if(composite_eventSub[0] &&
            (old_composite_event != composite_client_ptp_data->composite_event))
            client_ptp_data->composite_event_count.fetch_add(1,
                std::memory_order_relaxed);
        jclCurrentState.as_capable = client_ptp_data->as_capable;
        jclCurrentState.offset_in_range = client_ptp_data->master_offset_in_range;
        jclCurrentState.clock_offset = client_ptp_data->master_offset;
        jclCurrentState.notification_timestamp = seconds + nanoseconds;
        jclCurrentState.synced_to_primary_clock =
            client_ptp_data->synced_to_primary_clock;
        jclCurrentState.composite_event =
            composite_client_ptp_data->composite_event;
        memcpy(jclCurrentState.gm_identity, client_ptp_data->gm_identity,
            sizeof(client_ptp_data->gm_identity));
        jclCurrentEventCount.offset_in_range_event_count =
            client_ptp_data->offset_in_range_event_count;
        jclCurrentEventCount.as_capable_event_count =
            client_ptp_data->as_capable_event_count;
        jclCurrentEventCount.synced_to_primary_clock_event_count =
            client_ptp_data->synced_to_primary_clock_event_count;
        jclCurrentEventCount.gm_changed_event_count =
            client_ptp_data->gm_changed_event_count;
        jclCurrentEventCount.composite_event_count =
            client_ptp_data->composite_event_count;
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
