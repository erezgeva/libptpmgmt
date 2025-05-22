/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client notification message class.
 * Implements client specific notification message function.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "client/notification_msg.hpp"
#include "client/timebase_state.hpp"
#include "common/serialize.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

bool ClientNotificationMessage::processMessage(Listener &rxContext,
    Transmitter *&txContext)
{
    PrintDebug("[ClientNotificationMessage]::processMessage ");
    return true;
}

bool ClientNotificationMessage::parseBuffer(Listener &rxContext)
{
    PrintDebug("[ClientNotificationMessage]::parseBuffer ");
    int timeBaseIndex = 0;
    ptp_event data = {};
    if(!Message::parseBuffer(rxContext))
        return false;
    if(!PARSE_RX(FIELD, timeBaseIndex, rxContext))
        return false;
    if(!PARSE_RX(FIELD, data, rxContext))
        return false;
    TimeBaseStates::getInstance().setTimeBaseState(timeBaseIndex, data);
    return true;
}
