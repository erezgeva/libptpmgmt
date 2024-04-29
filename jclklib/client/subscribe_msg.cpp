/*! \file subscribe_msg.cpp
    \brief Client subscribe message class. Implements client specific functionality.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    @author: Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@intel.com>
*/

#include <client/subscribe_msg.hpp>
#include <common/serialize.hpp>
#include <common/print.hpp>

using namespace JClkLibClient;
using namespace JClkLibCommon;
using namespace std;

JClkLibCommon::client_ptp_event client_data;

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
	JClkLibCommon::ptp_event data;

	PrintDebug("[ClientSubscribeMessage]::parseBuffer ");
	if(!CommonSubscribeMessage::parseBuffer(LxContext))
		return false;

	if (!PARSE_RX(FIELD, data, LxContext))
		return false;

	printf("master_offset = %ld, servo_state = %d ", data.master_offset, data.servo_state);
	printf("gmIdentity = %02x%02x%02x.%02x%02x.%02x%02x%02x ",
		data.gmIdentity[0], data.gmIdentity[1],data.gmIdentity[2],
		data.gmIdentity[3], data.gmIdentity[4],
		data.gmIdentity[5], data.gmIdentity[6],data.gmIdentity[7]);
	printf("asCapable = %d, ptp4l_id = %d\n\n", data.asCapable, data.ptp4l_id);

	if (data.master_offset != client_data.master_offset) {
		client_data.master_offset = data.master_offset;
		if ((client_data.master_offset > client_data.master_offset_low) && (client_data.master_offset < client_data.master_offset_high)) {
			client_data.master_offset_within_boundary = true;
			client_data.offset_event_count.fetch_add(1, std::memory_order_relaxed) + 1; // Atomic increment
		}
	}

	if (data.servo_state != client_data.servo_state) {
		client_data.servo_state = data.servo_state;
		client_data.servo_state_event_count.fetch_add(1, std::memory_order_relaxed); // Atomic increment
	}

	if (memcmp(client_data.gmIdentity, data.gmIdentity, sizeof(data.gmIdentity)) != 0) {
		memcpy(client_data.gmIdentity, data.gmIdentity, sizeof(data.gmIdentity));
		client_data.gmIdentity_event_count.fetch_add(1, std::memory_order_relaxed); // Atomic increment
	}

	if (data.asCapable != client_data.asCapable) {
		client_data.asCapable = data.asCapable;
		client_data.asCapable_event_count.fetch_add(1, std::memory_order_relaxed); // Atomic increment
	}

	printf("CLIENT master_offset = %ld, servo_state = %d ", data.master_offset, data.servo_state);
	printf("gmIdentity = %02x%02x%02x.%02x%02x.%02x%02x%02x ",
		client_data.gmIdentity[0], client_data.gmIdentity[1],client_data.gmIdentity[2],
		client_data.gmIdentity[3], client_data.gmIdentity[4],
		client_data.gmIdentity[5], client_data.gmIdentity[6],client_data.gmIdentity[7]);
	printf("asCapable = %d\n\n", data.asCapable);

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

	this->set_msgAck(ACK_NONE);
        return true;
}
