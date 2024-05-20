/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file notification_msg.hpp
 * @brief Client notification message class. Implements client specific notification message function.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#include <client/notification_msg.hpp>
#include <client/subscribe_msg.hpp>

#include <common/serialize.hpp>
#include <common/print.hpp>
#include <common/message.hpp>

using namespace std;
using namespace JClkLibCommon;
using namespace JClkLibClient;

/*
JClkLibCommon::client_ptp_event client_ptp_data;
JClkLibCommon::client_ptp_event composite_client_ptp_data;
JClkLibCommon::ptp_event proxy_data = {};
JClkLibCommon::jcl_state jclCurrentState = {};
JClkLibCommon::jcl_state_event_count jclCurrentEventCount = {};
*/

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

void ClientNotificationMessage::addClientState(ClientState *newClientState){
	ClientStateArray.push_back(newClientState);
}

void ClientNotificationMessage::deleteClientState(ClientState *newClientState) {
	ClientStateArray.erase(std::remove(ClientStateArray.begin(), \
	ClientStateArray.end(), newClientState), ClientStateArray.end());
}

PROCESS_MESSAGE_TYPE(ClientNotificationMessage::processMessage)
{
	PrintDebug("[ClientNotificationMessage]::processMessage ");

	//std::uint32_t eventSub[1];
	//state.get_eventSub().get_event().readEvent(eventSub, (std::size_t)sizeof(eventSub));
	//std::uint32_t composite_eventSub[1];
	//state.get_eventSub().get_composite_event().readEvent(composite_eventSub, (std::size_t)sizeof(composite_eventSub));
	
	bool old_composite_event;
	/* Need to walk thru the whole vector*/
	std::vector <ClientState *>::iterator it ;

	for (it=ClientStateArray.begin(); it<ClientStateArray.end();it++)
	{
	std::uint32_t eventSub[1];
	std::uint32_t composite_eventSub[1];

	ClientState *currentClientState = *it;

	JClkLibCommon::jcl_state &jclCurrentState = currentClientState->get_eventState();
	JClkLibCommon::jcl_state_event_count &jclCurrentEventCount = currentClientState->get_eventStateCount();

	currentClientState->get_eventSub().get_event().readEvent(eventSub, (std::size_t)sizeof(eventSub));
	currentClientState->get_eventSub().get_composite_event().readEvent(composite_eventSub, (std::size_t)sizeof(composite_eventSub));

	/* get the correct client_ptp_data according to our current sessionID */
	JClkLibCommon::client_ptp_event* client_ptp_data = NULL;
	client_ptp_data = ClientSubscribeMessage::getClientPtpEventStruct(currentClientState->get_sessionId());

	JClkLibCommon::client_ptp_event* composite_client_ptp_data = NULL;
	composite_client_ptp_data = ClientSubscribeMessage::getClientPtpEventCompositeStruct(currentClientState->get_sessionId());

	if ( client_ptp_data == NULL ) {
		printf ("ClientNotificationMessage::processMessage ERROR in obtaining client_ptp_data.\n");
		return false;
	}

	if ((eventSub[0] & 1<<gmOffsetEvent) && (proxy_data.master_offset != client_ptp_data->master_offset)) {
		client_ptp_data->master_offset = proxy_data.master_offset;
		if ((client_ptp_data->master_offset > currentClientState->get_eventSub().get_value().getLower(gmOffsetValue)) &&
		    (client_ptp_data->master_offset < currentClientState->get_eventSub().get_value().getUpper(gmOffsetValue))) {
			if (!(client_ptp_data->master_offset_within_boundary))
			{
				client_ptp_data->master_offset_within_boundary = true;
				client_ptp_data->offset_event_count.fetch_add(1, std::memory_order_relaxed);
			}
		}
		else
		{
			if ((client_ptp_data->master_offset_within_boundary))
			{
				client_ptp_data->master_offset_within_boundary = false;
				client_ptp_data->offset_event_count.fetch_add(1, std::memory_order_relaxed);
			}
		}
	}

	if ((eventSub[0] & 1<<servoLockedEvent) && (proxy_data.servo_state != client_ptp_data->servo_state)) {
		client_ptp_data->servo_state = proxy_data.servo_state;
		client_ptp_data->servo_state_event_count.fetch_add(1, std::memory_order_relaxed);
	}

	if ((eventSub[0] & 1<<gmChangedEvent) && (memcmp(client_ptp_data->gmIdentity, proxy_data.gmIdentity, sizeof(proxy_data.gmIdentity)) != 0)) {
		memcpy(client_ptp_data->gmIdentity, proxy_data.gmIdentity, sizeof(proxy_data.gmIdentity));
		client_ptp_data->gmChanged_event_count.fetch_add(1, std::memory_order_relaxed);
		jclCurrentState.gm_changed = true;
	}
	else {
		jclCurrentState.gm_changed = false;
	}

	if ((eventSub[0] & 1<<asCapableEvent) && (proxy_data.asCapable != client_ptp_data->asCapable)) {
		client_ptp_data->asCapable = proxy_data.asCapable;
		client_ptp_data->asCapable_event_count.fetch_add(1, std::memory_order_relaxed);
	}

	if (composite_eventSub[0]) {
		old_composite_event = composite_client_ptp_data->composite_event;
		composite_client_ptp_data->composite_event = true;
	}

	if (composite_eventSub[0] & 1<<gmOffsetEvent) {
		composite_client_ptp_data->master_offset = proxy_data.master_offset;
		if ((composite_client_ptp_data->master_offset > currentClientState->get_eventSub().get_value().getLower(gmOffsetValue)) &&
		    (composite_client_ptp_data->master_offset < currentClientState->get_eventSub().get_value().getUpper(gmOffsetValue))) {
			composite_client_ptp_data->composite_event = true;
		}
		else {
			composite_client_ptp_data->composite_event = false;
		}
	}

	if (composite_eventSub[0] & 1<<servoLockedEvent) {
		composite_client_ptp_data->composite_event &= proxy_data.servo_state >= SERVO_LOCKED ? true:false;
	}

	if (composite_eventSub[0] & 1<<asCapableEvent) {
		composite_client_ptp_data->composite_event &= proxy_data.asCapable > 0 ? true:false;
	}

	if (composite_eventSub[0] && (old_composite_event != composite_client_ptp_data->composite_event)) {
		client_ptp_data->composite_event_count.fetch_add(1, std::memory_order_relaxed);
	}

/*
	jclCurrentState.as_Capable = client_ptp_data.asCapable > 0 ? true:false;
	jclCurrentState.offset_in_range = client_ptp_data.master_offset_within_boundary;
	jclCurrentState.servo_locked = client_ptp_data.servo_state >= SERVO_LOCKED ? true:false;
	jclCurrentState.composite_event = composite_client_ptp_data.composite_event;
	memcpy(jclCurrentState.gmIdentity, client_ptp_data.gmIdentity, sizeof(client_ptp_data.gmIdentity));
*/
	/* TODO : checked for jclCurrentState.gm_changed based on GM_identity previously stored */

/*
	jclCurrentEventCount.offset_in_range_event_count = client_ptp_data.offset_event_count;
	jclCurrentEventCount.asCapable_event_count = client_ptp_data.asCapable_event_count;
	jclCurrentEventCount.servo_locked_event_count = client_ptp_data.servo_state_event_count;
	jclCurrentEventCount.gm_changed_event_count = client_ptp_data.gmChanged_event_count;
	jclCurrentEventCount.composite_event_count = client_ptp_data.composite_event_count;
*/

	//state.set_eventState (jclCurrentState);
	//state.set_eventStateCount (jclCurrentEventCount);

	jclCurrentState.as_Capable = client_ptp_data->asCapable > 0 ? true:false;
	jclCurrentState.offset_in_range = client_ptp_data->master_offset_within_boundary;
	jclCurrentState.servo_locked = client_ptp_data->servo_state >= SERVO_LOCKED ? true:false;
	jclCurrentState.composite_event = composite_client_ptp_data->composite_event;
	memcpy(jclCurrentState.gmIdentity, client_ptp_data->gmIdentity, sizeof(client_ptp_data->gmIdentity));

	/* TODO : checked for jclCurrentState.gm_changed based on GM_identity previously stored */

	jclCurrentEventCount.offset_in_range_event_count = client_ptp_data->offset_event_count;
	jclCurrentEventCount.asCapable_event_count = client_ptp_data->asCapable_event_count;
	jclCurrentEventCount.servo_locked_event_count = client_ptp_data->servo_state_event_count;
	jclCurrentEventCount.gm_changed_event_count = client_ptp_data->gmChanged_event_count;
	jclCurrentEventCount.composite_event_count = client_ptp_data->composite_event_count;

	}
	
	return true;
}

PARSE_RXBUFFER_TYPE(ClientNotificationMessage::parseBuffer)
{
	PrintDebug("[ClientNotificationMessage]::parseBuffer ");
	if(!Message::parseBuffer(LxContext))
		return false;

	if (!PARSE_RX(FIELD, proxy_data, LxContext))
		return false;

	return true;
}
