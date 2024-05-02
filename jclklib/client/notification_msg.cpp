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
JClkLibCommon::ptp_event proxy_data;

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
		if ((client_ptp_data.master_offset > client_ptp_data.master_offset_low) && (client_ptp_data.master_offset < client_ptp_data.master_offset_high)) {
			client_ptp_data.master_offset_within_boundary = true;
			client_ptp_data.offset_event_count.fetch_add(1, std::memory_order_relaxed);
		}
	}

	if (proxy_data.servo_state != client_ptp_data.servo_state) {
		client_ptp_data.servo_state = proxy_data.servo_state;
		client_ptp_data.servo_state_event_count.fetch_add(1, std::memory_order_relaxed);
	}

	if (memcmp(client_ptp_data.gmIdentity, proxy_data.gmIdentity, sizeof(proxy_data.gmIdentity)) != 0) {
		memcpy(client_ptp_data.gmIdentity, proxy_data.gmIdentity, sizeof(proxy_data.gmIdentity));
		client_ptp_data.gmIdentity_event_count.fetch_add(1, std::memory_order_relaxed);
	}

	if (proxy_data.asCapable != client_ptp_data.asCapable) {
		client_ptp_data.asCapable = proxy_data.asCapable;
		client_ptp_data.asCapable_event_count.fetch_add(1, std::memory_order_relaxed);
	}

	// Load the value of event_count atomically for TESTING only, will be remove later
	int offset_event_count = client_ptp_data.offset_event_count;
	int gmIdentity_event_count = client_ptp_data.gmIdentity_event_count;
	int asCapable_event_count = client_ptp_data.asCapable_event_count;
	int servo_state_event_count = client_ptp_data.servo_state_event_count;

	printf("CLIENT master_offset = %ld, servo_state = %d ", client_ptp_data.master_offset, client_ptp_data.servo_state);
	printf("gmIdentity = %02x%02x%02x.%02x%02x.%02x%02x%02x ",
		client_ptp_data.gmIdentity[0], client_ptp_data.gmIdentity[1],client_ptp_data.gmIdentity[2],
		client_ptp_data.gmIdentity[3], client_ptp_data.gmIdentity[4],
		client_ptp_data.gmIdentity[5], client_ptp_data.gmIdentity[6],client_ptp_data.gmIdentity[7]);
	printf("asCapable = %d\n\n", client_ptp_data.asCapable);
	printf("CLIENT offset_event_count = %d, gmIdentity_event_count = %d, asCapable_event_count = %d, servo_state_event_count = %d\n\n", offset_event_count, gmIdentity_event_count, asCapable_event_count, servo_state_event_count);

	return true;
}

PARSE_RXBUFFER_TYPE(ClientNotificationMessage::parseBuffer)
{
	PrintDebug("[ClientNotificationMessage]::parseBuffer ");
	if(!Message::parseBuffer(LxContext))
		return false;

	if (!PARSE_RX(FIELD, proxy_data, LxContext))
		return false;

	printf("master_offset = %ld, servo_state = %d ", proxy_data.master_offset, proxy_data.servo_state);
	printf("gmIdentity = %02x%02x%02x.%02x%02x.%02x%02x%02x ",
		proxy_data.gmIdentity[0], proxy_data.gmIdentity[1],proxy_data.gmIdentity[2],
		proxy_data.gmIdentity[3], proxy_data.gmIdentity[4],
		proxy_data.gmIdentity[5], proxy_data.gmIdentity[6],proxy_data.gmIdentity[7]);
	printf("asCapable = %d\n\n", proxy_data.asCapable);


	return true;
}
