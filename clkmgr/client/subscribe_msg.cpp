/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client subscribe message class.
 * Implements client specific functionality.
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "client/subscribe_msg.hpp"
#include "client/timebase_state.hpp"
#include "common/serialize.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

ClientState *ClientSubscribeMessage::currentClientState = nullptr;

void ClientSubscribeMessage::setClientState(ClientState &newClientState)
{
    currentClientState = &newClientState;
}

bool ClientSubscribeMessage::makeBuffer(Transmitter &TxContext) const
{
    PrintDebug("[ProxySubscribeMessage]::makeBuffer");
    if(!SubscribeMessage::makeBuffer(TxContext))
        return false;
    if(!WRITE_TX(FIELD, timeBaseIndex, TxContext))
        return false;
    return true;
}

bool ClientSubscribeMessage::parseBuffer(Listener &LxContext)
{
    ptp_event data = {};
    PrintDebug("[ClientSubscribeMessage]::parseBuffer ");
    if(!SubscribeMessage::parseBuffer(LxContext))
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
 * @param LxContext client run-time listener
 * @param TxContext client run-time transmitter
 * @return true
 */
bool ClientSubscribeMessage::processMessage(Listener &LxContext,
    Transmitter *&TxContext)
{
    PrintDebug("[ClientSubscribeMessage]::processMessage (reply)");
    unique_lock<rtpi::mutex> lock(cv_mtx);
    TimeBaseStates::getInstance().setSubscribed(timeBaseIndex, true);
    this->set_msgAck(ACK_NONE);
    cv.notify_one(lock);
    return true;
}
