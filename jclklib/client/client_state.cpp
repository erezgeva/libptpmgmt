/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file client_state.cpp
 * @brief Set and get the client subscribe event state
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#include <client/msgq_tport.hpp>
#include <client/client_state.hpp>
#include <common/jcltypes.hpp>
#include <common/transport.hpp>
#include <common/message.hpp>
#include <iostream>
#include <cstring>
#include <string>

using namespace std;
using namespace JClkLibClient;
using namespace JClkLibCommon;

ClientState::ClientState()
{
	connected = false;
	subscribed = false;
	sessionId = JClkLibCommon::InvalidSessionId;
	fill(begin(clientID ), end( clientID ), 0 );
}

bool ClientState::get_connected() {return connected;}

void ClientState::set_connected(bool new_state) {connected = new_state;}

bool ClientState::get_subscribed() {return subscribed;}

void ClientState::set_subscribed(bool subscriptionState) {subscribed = subscriptionState;}

TransportClientId ClientState::get_clientID() {
	return clientID;
}

void ClientState::set_clientID(TransportClientId new_cID) {
	strcpy((char *)clientID.data(), (char *)new_cID.data());
}

JClkLibCommon::jcl_state_event_count &ClientState::get_eventStateCount() {
	return eventStateCount;
}
JClkLibCommon::jcl_state &ClientState::get_eventState() {
	return eventState;
}

void ClientState::set_eventStateCount(JClkLibCommon::jcl_state_event_count newCount) {
	eventStateCount = newCount;
}

void ClientState::set_eventState(JClkLibCommon::jcl_state newState) {
	eventState = newState;
}

string ClientState::toString() {
    string name = "[ClientState::eventState]";
	name += " as_Capable = " + to_string(this->get_eventState().as_Capable);
	name += " gm_changed = " + to_string(this->get_eventState().gm_changed);
	name += " gm_present = " + to_string(this->get_eventState().gm_present);
	name += " offset_in_range = " + to_string(this->get_eventState().offset_in_range);
	name += " servo_locked = " + to_string(this->get_eventState().servo_locked);
	name += "\n";
	return name;
}

JClkLibCommon::jcl_subscription &ClientState::get_eventSub() {
	return eventSub;
}

ClientState JClkLibClient::state{};
