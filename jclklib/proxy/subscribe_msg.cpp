/*! \file subscribe_msg.cpp
    \brief Proxy subscribe message implementation. Implements proxy specific subscribe message function.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <proxy/subscribe_msg.hpp>
#include <proxy/clock_config.hpp>
#include <proxy/client.hpp>
#include <common/print.hpp>
#include <common/serialize.hpp>
#include <common/message.hpp>

using namespace std;
using namespace JClkLibProxy;
using namespace JClkLibCommon;

extern JClkLibCommon::ptp_event pe;

/** @brief Create the ProxySubscribeMessage object
 *
 * @param msg msg structure to be fill up
 * @param LxContext proxy transport listener context
 * @return true
 */
MAKE_RXBUFFER_TYPE(ProxySubscribeMessage::buildMessage)
{
	msg = new ProxySubscribeMessage();
	return true;
}

/** @brief Add proxy's SUBSCRIBE_MSG type and its builder to transport layer.
 *
 * This function will be called during init to add a map of SUBSCRIBE_MSG
 * type and its corresponding buildMessage function.
 *
 * @return true
 */
bool ProxySubscribeMessage::initMessage()
{
	addMessageType(parseMsgMapElement_t(SUBSCRIBE_MSG, buildMessage));
	return true;
}

BUILD_TXBUFFER_TYPE(ProxySubscribeMessage::makeBuffer) const
{
	PrintDebug("[ProxySubscribeMessage]::makeBuffer");
	if(!CommonSubscribeMessage::makeBuffer(TxContext))
		return false;

	/* Add ptp data here */
	if (!WRITE_TX(FIELD, pe, TxContext))
		return false;

	return true;
}

/*
This is to process the subscription from the jclklib client runtime via POSIX msg queue
[Question] The client list of real-time subscription is hold inside jcklib_proxy?
*/
PROCESS_MESSAGE_TYPE(ProxySubscribeMessage::processMessage)
{
	//config.setEvent(subscription.getEvent());
	//config.setValue(subscription.getValue());

	sessionId_t sID;
	sID = this->getc_sessionId();
	PrintDebug("[ProxySubscribeMessage]::processMessage - Use current client session ID: " + to_string(sID));
	
	if(sID == InvalidSessionId) {
		PrintError("Session ID *should be* invalid for received proxy connect message");
		return false;
	}
	TxContext = Client::GetClientSession(sID).get()->get_transmitContext();
	set_msgAck(ACK_SUCCESS);
	return true;
}

/*
[NOTE] This is to response towards ProxySubscribeMessage - wont be using this for the time being.
*/
bool ProxySubscribeMessage::generateResponse(uint8_t *msgBuffer, size_t &length,
					const ClockStatus &status)
{
	return false;
}


