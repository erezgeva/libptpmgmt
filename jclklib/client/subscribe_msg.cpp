/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file subscribe_msg.cpp
 * @brief Client subscribe message class. Implements client specific functionality.
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#include <client/subscribe_msg.hpp>
#include <common/serialize.hpp>
#include <common/print.hpp>

using namespace JClkLibClient;
using namespace JClkLibCommon;
using namespace std;

JClkLibCommon::client_ptp_event client_data = {};
JClkLibCommon::client_ptp_event composite_client_data = {};

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

PARSE_RXBUFFER_TYPE(ClientSubscribeMessage::parseBuffer) {
	JClkLibCommon::ptp_event data = {};
	JClkLibCommon::jcl_state jclCurrentState = {};

	std::uint32_t eventSub[1];
	state.get_eventSub().get_event().readEvent(eventSub, (std::size_t)sizeof(eventSub));

	std::uint32_t composite_eventSub[1];
	state.get_eventSub().get_composite_event().readEvent(composite_eventSub, (std::size_t)sizeof(composite_eventSub));

	PrintDebug("[ClientSubscribeMessage]::parseBuffer ");
	if(!CommonSubscribeMessage::parseBuffer(LxContext))
		return false;

	if (!PARSE_RX(FIELD, data, LxContext))
		return false;

	printf("master_offset = %ld, servo_state = %d gmPresent = %d\n", data.master_offset, data.servo_state, data.gmPresent);
	printf("gmIdentity = %02x%02x%02x.%02x%02x.%02x%02x%02x ",
		data.gmIdentity[0], data.gmIdentity[1],data.gmIdentity[2],
		data.gmIdentity[3], data.gmIdentity[4],
		data.gmIdentity[5], data.gmIdentity[6],data.gmIdentity[7]);
	printf("asCapable = %d, ptp4l_id = %d\n\n", data.asCapable, data.ptp4l_id);

	/* TODO : set the client_data per client instead of global */
	if ((eventSub[0] & 1<<gmOffsetEvent) && (data.master_offset != client_data.master_offset)) {
		client_data.master_offset = data.master_offset;
		if ((client_data.master_offset > state.get_eventSub().get_value().getLower(gmOffsetValue)) &&
		    (client_data.master_offset < state.get_eventSub().get_value().getUpper(gmOffsetValue))) {
			client_data.master_offset_within_boundary = true;
		}
	}

	if ((eventSub[0] & 1<<servoLockedEvent) && (data.servo_state != client_data.servo_state)) {
		client_data.servo_state = data.servo_state;
	}

	if ((eventSub[0] & 1<<gmChangedEvent) && (memcmp(client_data.gmIdentity, data.gmIdentity, sizeof(data.gmIdentity))) != 0) {
		memcpy(client_data.gmIdentity, data.gmIdentity, sizeof(data.gmIdentity));
		jclCurrentState.gm_changed = true;
	}

	if ((eventSub[0] & 1<<asCapableEvent) && (data.asCapable != client_data.asCapable)) {
		client_data.asCapable = data.asCapable;
	}

	if ((eventSub[0] & 1<<gmPresentEvent) && (data.gmPresent != client_data.gmPresent)) {
		client_data.gmPresent = data.gmPresent;
	}

	if (composite_eventSub[0]) {
		composite_client_data.composite_event = true;
	}

	if ((composite_eventSub[0] & 1<<gmOffsetEvent) && (data.master_offset != composite_client_data.master_offset)) {
		composite_client_data.master_offset = data.master_offset;
		if ((composite_client_data.master_offset > state.get_eventSub().get_value().getLower(gmOffsetValue)) &&
		    (composite_client_data.master_offset < state.get_eventSub().get_value().getUpper(gmOffsetValue))) {
			composite_client_data.composite_event = true;
		} else {
			composite_client_data.composite_event = false;
		}
	}

	if (composite_eventSub[0] & 1<<servoLockedEvent) {
		composite_client_data.composite_event &= data.servo_state >= SERVO_LOCKED ? true:false;
	}

	if (composite_eventSub[0] & 1<<asCapableEvent) {
		composite_client_data.composite_event &= data.asCapable > 0 ? true:false;
	}

	printf("CLIENT master_offset = %ld, servo_state = %d gmPresent = %d\n", client_data.master_offset, client_data.servo_state, client_data.gmPresent);
	printf("gmIdentity = %02x%02x%02x.%02x%02x.%02x%02x%02x ",
		client_data.gmIdentity[0], client_data.gmIdentity[1],client_data.gmIdentity[2],
		client_data.gmIdentity[3], client_data.gmIdentity[4],
		client_data.gmIdentity[5], client_data.gmIdentity[6],client_data.gmIdentity[7]);
	printf("asCapable = %d\n\n", client_data.asCapable);

	jclCurrentState.gm_present = client_data.gmPresent > 0 ? true:false;
	jclCurrentState.as_Capable = client_data.asCapable > 0 ? true:false;
	jclCurrentState.offset_in_range = client_data.master_offset_within_boundary;
	jclCurrentState.servo_locked = client_data.servo_state >= SERVO_LOCKED ? true:false;
	jclCurrentState.composite_event = composite_client_data.composite_event;
	memcpy(jclCurrentState.gmIdentity, client_data.gmIdentity, sizeof(client_data.gmIdentity));
	/* TODO : checked for jclCurrentState.gm_changed based on GM_identity previously stored */

	this->setClientState (jclCurrentState);
	
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

		state.set_eventState(this->getClientState());
		state.set_subscribed(true);

		this->set_msgAck(ACK_NONE);

		JClkLibCommon::jcl_state jclCurrentState = state.get_eventState();
		printf("[ClientSubscribeMessage]::processMessage : state -  offset in range = %d, servo_locked = %d gmPresent = %d as_Capable = %d\n", \
	 	jclCurrentState.offset_in_range, jclCurrentState.servo_locked, jclCurrentState.gm_present, jclCurrentState.as_Capable);
	
		cv.notify_one();
        return true;
}
