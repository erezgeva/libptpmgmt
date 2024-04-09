/*! \file subscribe_msg.cpp
    \brief Client subscribe message class. Implements client specific functionality.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    @author: Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@intel.com>
*/

#include <client/subscribe_msg.hpp>
#include <common/print.hpp>

using namespace JClkLibClient;
using namespace JClkLibCommon;
using namespace std;

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
