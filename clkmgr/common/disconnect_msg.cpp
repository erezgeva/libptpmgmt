/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Common disconnect message implementation.
 * Implements common functions and (de-)serialization
 *
 * @author Goh Wei Sheng <wei.sheng.goh@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include "common/disconnect_msg.hpp"
#include "common/serialize.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

bool DisconnectMessage::parseBufferComm()
{
    PrintDebug("[DisconnectMessage]::parseBufferComm");
    sessionId_t sessionId;
    if(!PARSE_RX(sessionId, rxBuf))
        return false;
    set_sessionId(sessionId);
    return true;
}

bool DisconnectMessage::makeBufferComm(Buffer &buff) const
{
    PrintDebug("[DisconnectMessage]::makeBufferComm");
    return WRITE_TX(get_sessionId(), buff);
}
