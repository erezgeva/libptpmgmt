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
	return true;
}

PARSE_RXBUFFER_TYPE(ClientNotificationMessage::parseBuffer)
{
	ptp_event data;

	PrintDebug("[ClientNotificationMessage]::parseBuffer ");
	if(!Message::parseBuffer(LxContext))
		return false;

	if (!PARSE_RX(FIELD, data, LxContext))
		return false;

	printf("master_offset = %ld, servo_state = %d ", data.master_offset, data.servo_state);
	printf("gmIdentity = %02x%02x%02x.%02x%02x.%02x%02x%02x ",
		data.gmIdentity[0], data.gmIdentity[1],data.gmIdentity[2],
		data.gmIdentity[3], data.gmIdentity[4],
		data.gmIdentity[5], data.gmIdentity[6],data.gmIdentity[7]);
	printf("asCapable = %d\n\n", data.asCapable);
	return true;
}
