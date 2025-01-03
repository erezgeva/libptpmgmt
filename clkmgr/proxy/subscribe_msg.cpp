/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy subscribe message implementation.
 * Implements proxy specific subscribe message function.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "common/message.hpp"
#include "common/print.hpp"
#include "common/serialize.hpp"
#include "proxy/client.hpp"
#include "proxy/subscribe_msg.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

extern ptp_event pe;

/**
 * Create the ProxySubscribeMessage object
 * @param msg msg structure to be fill up
 * @param LxContext proxy transport listener context
 * @return true
 */
MAKE_RXBUFFER_TYPE(ProxySubscribeMessage::buildMessage)
{
    msg = new ProxySubscribeMessage();
    return true;
}

/**
 * @brief Add proxy's SUBSCRIBE_MSG type and its builder to transport layer.
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
    if(!WRITE_TX(FIELD, pe, TxContext))
        return false;
    return true;
}

/*
This is to process the subscription from the clkmgr client runtime
via POSIX msg queue.
*/
PROCESS_MESSAGE_TYPE(ProxySubscribeMessage::processMessage)
{
    sessionId_t sID;
    sID = this->getc_sessionId();
    PrintDebug("[ProxySubscribeMessage]::processMessage - \
        Use current client session ID: "
        + to_string(sID));
    if(sID == InvalidSessionId) {
        PrintError("Session ID *should be* invalid for received \
            proxy connect message");
        return false;
    }
    TxContext = Client::GetClientSession(sID).get()->get_transmitContext();
    set_msgAck(ACK_SUCCESS);
    return true;
}
