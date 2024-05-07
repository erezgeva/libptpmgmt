/*! \file notification_msg.cpp
    \brief Client notification message implementation. Implements client specific notification message function.

    (C) Copyright Intel Corporation 2024. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <client/notification_msg.hpp>
#include <common/serialize.hpp>
#include <common/print.hpp>
#include <common/message.hpp>

using namespace std;
using namespace JClkLibCommon;
using namespace JClkLibClient;

JClkLibCommon::client_ptp_event client_ptp_data;
JClkLibCommon::ptp_event proxy_data = {};
JClkLibCommon::jcl_state jclCurrentState = {};
JClkLibCommon::jcl_state_event_count jclCurrentEventCount = {};

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

PROCESS_MESSAGE_TYPE(ClientNotificationMessage::processMessage)
{
	PrintDebug("[ClientNotificationMessage]::processMessage ");

	if (proxy_data.master_offset != client_ptp_data.master_offset) {
		client_ptp_data.master_offset = proxy_data.master_offset;
		if ((client_ptp_data.master_offset > state.get_eventSub().get_value().getLower(0)) && (client_ptp_data.master_offset < state.get_eventSub().get_value().getUpper(0))) {
			if (!(client_ptp_data.master_offset_within_boundary))
			{
				client_ptp_data.master_offset_within_boundary = true;
				client_ptp_data.offset_event_count.fetch_add(1, std::memory_order_relaxed);
			}
		}
		else
		{
			if ((client_ptp_data.master_offset_within_boundary))
			{
				client_ptp_data.master_offset_within_boundary = false;
				client_ptp_data.offset_event_count.fetch_add(1, std::memory_order_relaxed);
			}
		}
	}

	if (proxy_data.servo_state != client_ptp_data.servo_state) {
		client_ptp_data.servo_state = proxy_data.servo_state;
		client_ptp_data.servo_state_event_count.fetch_add(1, std::memory_order_relaxed);
	}

	if (memcmp(client_ptp_data.gmIdentity, proxy_data.gmIdentity, sizeof(proxy_data.gmIdentity)) != 0) {
		memcpy(client_ptp_data.gmIdentity, proxy_data.gmIdentity, sizeof(proxy_data.gmIdentity));
		client_ptp_data.gmChanged_event_count.fetch_add(1, std::memory_order_relaxed);
		jclCurrentState.gm_changed = true;
	}
	else {
		jclCurrentState.gm_changed = false;
	}

	if (proxy_data.asCapable != client_ptp_data.asCapable) {
		client_ptp_data.asCapable = proxy_data.asCapable;
		client_ptp_data.asCapable_event_count.fetch_add(1, std::memory_order_relaxed);
	}

	if (proxy_data.gmPresent != client_ptp_data.gmPresent) {
		client_ptp_data.gmPresent = proxy_data.gmPresent;
		client_ptp_data.gmPresent_event_count.fetch_add(1, std::memory_order_relaxed);
	}

	jclCurrentState.gm_present = client_ptp_data.gmPresent > 0 ? true:false;
	jclCurrentState.as_Capable = client_ptp_data.asCapable > 0 ? true:false;
	jclCurrentState.offset_in_range = client_ptp_data.master_offset_within_boundary;
	jclCurrentState.servo_locked = client_ptp_data.servo_state >= SERVO_LOCKED ? true:false;
	memcpy(jclCurrentState.gmIdentity, client_ptp_data.gmIdentity, sizeof(client_ptp_data.gmIdentity));

	/* TODO : checked for jclCurrentState.gm_changed based on GM_identity previously stored */

	jclCurrentEventCount.gmPresent_event_count = client_ptp_data.gmPresent_event_count;
	jclCurrentEventCount.offset_in_range_event_count = client_ptp_data.offset_event_count;
	jclCurrentEventCount.asCapable_event_count = client_ptp_data.asCapable_event_count;
	jclCurrentEventCount.servo_locked_event_count = client_ptp_data.servo_state_event_count;
	jclCurrentEventCount.gm_changed_event_count = client_ptp_data.gmChanged_event_count;

	state.set_eventState (jclCurrentState);
	state.set_eventStateCount (jclCurrentEventCount);

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
