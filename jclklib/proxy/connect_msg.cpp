/*! \file connect_msg.cpp
    \brief Proxy connect message implementation. Implements proxy specific connect message function.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <proxy/connect_msg.hpp>
#include <proxy/client.hpp>
#include <common/print.hpp>
#include <common/serialize.hpp>
#include <common/message.hpp>
#include <proxy/subscribe_msg.hpp>

using namespace std;
using namespace JClkLibProxy;
using namespace JClkLibCommon;

extern JClkLibCommon::ptp_event pe;

/** @brief Create the ProxyConnectMessage object
 *
 * @param msg msg structure to be fill up
 * @param LxContext proxy transport listener context
 * @return true
 */
MAKE_RXBUFFER_TYPE(ProxyConnectMessage::buildMessage)
{
	msg = new ProxyConnectMessage();
	return true;
}

/** @brief Add proxy's CONNECT_MSG type and its builder to transport layer.
 *
 * This function will be called during init to add a map of CONNECT_MSG
 * type and its corresponding buildMessage function.
 *
 * @return true
 */
bool ProxyConnectMessage::initMessage()
{
	addMessageType(parseMsgMapElement_t(CONNECT_MSG, buildMessage));
	return true;
}

/** @brief process the connect msg from client-runtime
 *
 * This function will be called when the transport layer
 * in proxy receive a CONNECT_MSG type from client-runtime.
 * In this case, proxy transport layer will rx a buffer in the
 * message queue and call this function when
 * the enum ID corresponding to the CONNECT_MSG is received.
 * A new ClientSession object and a corresponding TxContext
 * (with the transmit msq) is created in the proxy.
 *
 * @param LxContext proxy transport listener context
 * @param TxContext proxy transport transmitter context
 * @return true
 */
PROCESS_MESSAGE_TYPE(ProxyConnectMessage::processMessage)
{
	sessionId_t newSessionId;

	PrintDebug("Processing proxy connect message");

	if (this->getc_sessionId() != InvalidSessionId) {
		PrintError("Session ID *should be* invalid for received proxy connect message");
		return false;
	}

	/* check whether there is ptp4l available */
	if (!pe.ptp4l_id) {
		return false;
	}

	newSessionId = Client::CreateClientSession();
	PrintDebug("Created new client session ID: " + to_string(newSessionId));
	this->set_sessionId(newSessionId);
	TxContext = LxContext.CreateTransmitterContext(getClientId());
	Client::GetClientSession(newSessionId).get()->set_transmitContext(TxContext);
	set_msgAck(ACK_SUCCESS);

	return true;
}

BUILD_TXBUFFER_TYPE(ProxyConnectMessage::makeBuffer) const
{
	PrintDebug("[ProxyConnectMessage]::makeBuffer");
	if(!CommonConnectMessage::makeBuffer(TxContext))
		return false;

	/* Add ptp4l_id here */
	if (!WRITE_TX(FIELD, pe.ptp4l_id, TxContext))
		return false;

	return true;
}