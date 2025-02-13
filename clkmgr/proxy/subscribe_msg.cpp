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

extern ptp_event clockEvent;

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
    if(!WRITE_TX(FIELD, clockEvent, TxContext))
        return false;
    return true;
}

PARSE_RXBUFFER_TYPE(ProxySubscribeMessage::parseBuffer)
{
    uint8_t ptp4lDomainNumber = 0;
    std::string chronyUDSAddr;
    std::string ptp4lUDSAddr;
    UDSAddress chronyAddr;
    UDSAddress ptp4lAddr;
    PrintDebug("[ProxySubscribeMessage]::parseBuffer ");
    if(!CommonSubscribeMessage::parseBuffer(LxContext))
        return false;
    /* Retrieve ptp4l and chrony UDS addresses */
    if(!PARSE_RX(ARRAY, chronyAddr, LxContext))
        return false;
    if(!PARSE_RX(ARRAY, ptp4lAddr, LxContext))
        return false;
    /* Retrieve ptp4l domain number */
    if(!PARSE_RX(FIELD, ptp4lDomainNumber, LxContext))
        return false;
    /* Convert array to string */
    chronyUDSAddr.assign(chronyAddr.begin(), chronyAddr.end());
    ptp4lUDSAddr.assign(ptp4lAddr.begin(), ptp4lAddr.end());
    PrintDebug("[ProxySubscribeMessage] Chrony UDS address: " + chronyUDSAddr);
    PrintDebug("[ProxySubscribeMessage] PTP4L UDS address: " + ptp4lUDSAddr);
    PrintDebug("[ProxySubscribeMessage] PTP4L Domain Number: " +
        std::to_string(ptp4lDomainNumber));
    /* ToDo: communicate to Chrony and ptp4l only after get the uds addr */
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
    PrintDebug("[ProxySubscribeMessage]::processMessage - "
        "Use current client session ID: "
        + to_string(sID));
    if(sID == InvalidSessionId) {
        PrintError("Session ID *should be* invalid for received "
            "proxy connect message");
        return false;
    }
    TxContext = Client::GetClientSession(sID).get()->get_transmitContext();
    set_msgAck(ACK_SUCCESS);
    return true;
}
