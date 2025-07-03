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

bool ClientNotificationMessage::parseBufferTail()
{
    PrintDebug("[ClientNotificationMessage]::parseBufferTail");
    uint8_t clockType = 0;
    if(!PARSE_RX(clockType, rxBuf))
        return false;
    if(clockType == PTPClock) {
        ptp_event ptpData = {};
        if(!PARSE_RX(ptpData, rxBuf))
            return false;
        TimeBaseStates::getInstance().setTimeBaseStatePtp(timeBaseIndex, ptpData);
    }
    if(clockType == SysClock) {
        chrony_event chronyData = {};
        if(!PARSE_RX(chronyData, rxBuf))
            return false;
        TimeBaseStates::getInstance().setTimeBaseStateSys(timeBaseIndex,
            chronyData);
    }
    return true;
}
