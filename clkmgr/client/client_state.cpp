/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Set and get the client subscribe event state
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "pub/clkmgr/client_state.h"
#include "client/msgq_tport.hpp"
#include "common/clkmgrtypes.hpp"
#include "common/message.hpp"
#include "common/transport.hpp"

#include <cstring>
#include <string>

__CLKMGR_NAMESPACE_USE

using namespace std;

ClientState::ClientState()
{
    connected = false;
    subscribed = false;
    m_sessionId = InvalidSessionId;
    fill(begin(clientID), end(clientID), 0);
    eventState = {};
    eventStateCount = {};
    eventSub = {};
    last_notification_time = {};
}

ClientState::ClientState(ClientState &newState)
{
    connected = newState.get_connected();
    subscribed = newState.get_subscribed();
    m_sessionId = newState.get_sessionId();
    strcpy((char *)clientID.data(), (char *)newState.get_clientID().data());
    eventState = newState.get_eventState();
    eventStateCount = newState.get_eventStateCount();
    eventSub = newState.get_eventSub();
    last_notification_time = newState.get_last_notification_time();
}

void ClientState::set_clientState(ClientState &newState)
{
    connected = newState.get_connected();
    subscribed = newState.get_subscribed();
    m_sessionId = newState.get_sessionId();
    strcpy((char *)clientID.data(), (char *)newState.get_clientID().data());
    eventState = newState.get_eventState();
    eventStateCount = newState.get_eventStateCount();
    eventSub = newState.get_eventSub();
    last_notification_time = newState.get_last_notification_time();
}

bool ClientState::get_connected()
{
    return connected;
}

void ClientState::set_connected(bool new_state)
{
    connected = new_state;
}

bool ClientState::get_subscribed()
{
    return subscribed;
}

void ClientState::set_subscribed(bool subscriptionState)
{
    subscribed = subscriptionState;
}

TransportClientId ClientState::get_clientID()
{
    return clientID;
}

void ClientState::set_clientID(TransportClientId &new_cID)
{
    strcpy((char *)clientID.data(), (char *)new_cID.data());
}

clkmgr_event_count &ClientState::get_eventStateCount()
{
    return eventStateCount;
}

clkmgr_event_state &ClientState::get_eventState()
{
    return eventState;
}

void ClientState::set_eventStateCount(clkmgr_event_count newCount)
{
    eventStateCount = newCount;
}

void ClientState::set_eventState(clkmgr_event_state newState)
{
    eventState = newState;
}

string ClientState::toString()
{
    string name = "[ClientState::eventState]";
    name += " as_capable = " + to_string(this->get_eventState().as_capable);
    name += " gm_changed = " + to_string(this->get_eventState().gm_changed);
    name += " offset_in_range = " + to_string(
            this->get_eventState().offset_in_range);
    name += " synced_to_primary_clock = " + to_string(
            this->get_eventState().synced_to_primary_clock);
    name += "\n";
    return name;
}

ClkMgrSubscription &ClientState::get_eventSub()
{
    return eventSub;
}

void ClientState::set_last_notification_time(struct timespec newTime)
{
    last_notification_time = newTime;
}

struct timespec ClientState::get_last_notification_time()
{
    return last_notification_time;
}
