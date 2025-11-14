/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Client disconnect message implementation.
 * Implements client specific disconnect message function.
 *
 * @author Goh Wei Sheng <wei.sheng.goh@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include "client/disconnect_msg.hpp"
#include "client/client_state.hpp"
#include "common/serialize.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

bool ClientDisconnectMessage::parseBufferTail()
{
    PrintDebug("[ClientDisconnectMessage]::parseBufferTail - "
        "Received disconnect message from Proxy");
    ClientState::set_connected(false);
    return true;
}
