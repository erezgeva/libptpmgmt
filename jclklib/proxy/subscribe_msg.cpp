/*! \file subscribe_msg.cpp
    \brief Proxy subscribe message implementation. Implements proxy specific subscribe message function.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <proxy/subscribe_msg.hpp>
#include <proxy/clock_config.hpp>
#include <proxy/client.hpp>
#include <common/print.hpp>

using namespace std;
using namespace JClkLibProxy;
using namespace JClkLibCommon;

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

/*
This is to process the subscription from the jclklib client runtime via POSIX msg queue
[Question] The client list of real-time subscription is hold inside jcklib_proxy?
*/
//bool ProxySubscribeMessage::processMessage(ClockConfiguration &config)
PROCESS_MESSAGE_TYPE(ProxySubscribeMessage::processMessage)
{
	//config.setEvent(subscription.getEvent());
	//config.setValue(subscription.getValue());

	sessionId_t sID;
	sID = this->getc_sessionId();
	PrintDebug("[AZU] ProxySubscribeMessage::processMessage - Use current client session ID: " + to_string(sID));
	
	if(sID == InvalidSessionId) {
		PrintError("Session ID *should be* invalid for received proxy connect message");
		PrintDebug("[AZU] overwrite the sID to 0");
		sID = 0;
		//return false;   //azu hack
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


