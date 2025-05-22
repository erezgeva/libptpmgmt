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

bool ClientSubscribeMessage::makeBuffer(Transmitter &txContext) const
{
    PrintDebug("[ProxySubscribeMessage]::makeBuffer");
    if(!SubscribeMessage::makeBuffer(txContext))
        return false;
    if(!WRITE_TX(FIELD, timeBaseIndex, txContext))
        return false;
    return true;
}

bool ClientSubscribeMessage::parseBuffer(Listener &rxContext)
{
    ptp_event data = {};
    PrintDebug("[ClientSubscribeMessage]::parseBuffer ");
    if(!SubscribeMessage::parseBuffer(rxContext))
        return false;
    if(!PARSE_RX(FIELD, timeBaseIndex, rxContext))
        return false;
    if(!PARSE_RX(FIELD, data, rxContext))
        return false;
    TimeBaseStates::getInstance().setTimeBaseState(timeBaseIndex, data);
    return true;
}

bool ClientSubscribeMessage::writeClientId(Listener &)
{
    PrintDebug("[ClientQueue] [SUBSCRIBE] : subscription->event Mask : " +
        to_string(getSubscription().get_event_mask()));
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
 * @param rxContext client run-time listener
 * @param txContext client run-time transmitter
 * @return true
 */
bool ClientSubscribeMessage::processMessage(Listener &rxContext,
    Transmitter *&txContext)
{
    PrintDebug("[ClientSubscribeMessage]::processMessage (reply)");
    unique_lock<rtpi::mutex> lock(cv_mtx);
    TimeBaseStates::getInstance().setSubscribed(timeBaseIndex, true);
    set_msgAck(ACK_NONE);
    cv.notify_one(lock);
    return true;
}
