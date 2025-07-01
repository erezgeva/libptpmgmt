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

/**
 * Process the reply for notification msg from proxy.
 *
 * This function will be called when the transport layer
 * in client runtime received a SUBSCRIBE_MSG type (an echo reply from
 * proxy)
 * In this case, transport layer will rx a buffer in the client associated
 * listening message queue (listening to proxy) and call this function when
 * the enum ID corresponding to the SUBSCRIBE_MSG is received.
 *
 * @return true
 */
bool ClientSubscribeMessage::parseBufferTail()
{
    PrintDebug("[ClientSubscribeMessage]::parseBufferTail");
    ptp_event ptpData = {};
    if(!PARSE_RX(ptpData, rxBuf) ||
        !TimeBaseStates::getInstance().subscribeReply(timeBaseIndex, ptpData))
        return false;
    set_msgAck(ACK_NONE);
    return true;
}
