/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client subscribe message class.
 * Implements client specific functionality.
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "client/notification_msg.hpp"
#include "client/subscribe_msg.hpp"
#include "client/timebase_state.hpp"
#include "common/print.hpp"
#include "common/serialize.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

ClientState *ClientSubscribeMessage::currentClientState = nullptr;

/**
 * Create the ClientSubscribeMessage object
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

/**
 * @brief Add client's SUBSCRIBE_MSG type and its builder to transport layer.
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

void ClientSubscribeMessage::setClientState(ClientState *newClientState)
{
    currentClientState = newClientState;
}

BUILD_TXBUFFER_TYPE(ClientSubscribeMessage::makeBuffer) const
{
    PrintDebug("[ProxySubscribeMessage]::makeBuffer");
    if(!CommonSubscribeMessage::makeBuffer(TxContext))
        return false;
    if(!WRITE_TX(FIELD, timeBaseIndex, TxContext))
        return false;
    return true;
}

PARSE_RXBUFFER_TYPE(ClientSubscribeMessage::parseBuffer)
{
    ptp_event data = {};
    PrintDebug("[ClientSubscribeMessage]::parseBuffer ");
    if(!CommonSubscribeMessage::parseBuffer(LxContext))
        return false;
    if(!PARSE_RX(FIELD, timeBaseIndex, LxContext))
        return false;
    if(!PARSE_RX(FIELD, data, LxContext))
        return false;
    TimeBaseStates::getInstance().setTimeBaseState(timeBaseIndex, data);
    return true;
}

/**
 * @brief process the reply for notification msg from proxy.
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
    unique_lock<rtpi::mutex> lock(cv_mtx);
    TimeBaseStates::getInstance().setSubscribed(timeBaseIndex, true);
    this->set_msgAck(ACK_NONE);
    cv.notify_one(lock);
    return true;
}
