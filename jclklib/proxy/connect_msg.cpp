/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file connect_msg.cpp
 * @brief Proxy connect message implementation. Implements proxy specific connect message function.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#include <common/message.hpp>
#include <common/print.hpp>
#include <common/serialize.hpp>
#include <proxy/connect_msg.hpp>
#include <proxy/client.hpp>
#include <proxy/subscribe_msg.hpp>

using namespace JClkLibCommon;
using namespace JClkLibProxy;
using namespace std;

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
    sessionId_t newSessionId = this->getc_sessionId();

    PrintDebug("Processing proxy connect message");

    /* Check whether there is ptp4l available */
    if (!pe.ptp4l_id) {
        PrintError("ptp4l_id is not available.");
        return false;
    }

    if (newSessionId != InvalidSessionId) {
        auto clientSession = Client::GetClientSession(newSessionId);
        if (clientSession)
                TxContext = clientSession.get()->get_transmitContext();

        if (TxContext) {
            PrintDebug("Receive Connect msg as liveness check.");
            set_msgAck(ACK_SUCCESS);
            return true;
        }

        PrintError("Session ID not exists: " + to_string(newSessionId));
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
