/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file jclk_client_state.cpp
 * @brief Set and get the client subscribe event state
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#include <cstring>
#include <iostream>
#include <string>

#include <client/jclk_client_state.hpp>
#include <client/msgq_tport.hpp>
#include <common/jcltypes.hpp>
#include <common/message.hpp>
#include <common/transport.hpp>

using namespace JClkLibClient;
using namespace JClkLibCommon;
using namespace std;

ClientState::ClientState()
{
    connected = false;
    subscribed = false;
    sessionId = JClkLibCommon::InvalidSessionId;
    fill(begin(clientID ), end( clientID ), 0 );
    eventState = {};
    eventStateCount = {};
    eventSub = {};
}

ClientState::ClientState(ClientState &newState)
{
    connected = newState.get_connected();
    subscribed = newState.get_subscribed();
    sessionId = newState.get_sessionId();
    strcpy((char *)clientID.data(), (char *)newState.get_clientID().data());
    eventState = newState.get_eventState();
    eventStateCount = newState.get_eventStateCount();
    eventSub = newState.get_eventSub();
}

void ClientState::set_clientState(ClientState &newState)
{
    connected = newState.get_connected();
    subscribed = newState.get_subscribed();
    sessionId = newState.get_sessionId();
    strcpy((char *)clientID.data(), (char *)newState.get_clientID().data());
    eventState = newState.get_eventState();
    eventStateCount = newState.get_eventStateCount();
    eventSub = newState.get_eventSub();
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

jcl_state_event_count &ClientState::get_eventStateCount()
{
    return eventStateCount;
}

jcl_state &ClientState::get_eventState()
{
    return eventState;
}

void ClientState::set_eventStateCount(jcl_state_event_count newCount)
{
    eventStateCount = newCount;
}

void ClientState::set_eventState(jcl_state newState)
{
    eventState = newState;
}

string ClientState::toString()
{
    string name = "[ClientState::eventState]";
    name += " as_capable = " + to_string(this->get_eventState().as_capable);
    name += " gm_changed = " + to_string(this->get_eventState().gm_changed);
    name += " offset_in_range = " + to_string(this->get_eventState().offset_in_range);
    name += " servo_locked = " + to_string(this->get_eventState().servo_locked);
    name += "\n";
    return name;
}

JClkLibCommon::jcl_subscription &ClientState::get_eventSub()
{
    return eventSub;
}
