/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Proxy disconnect message implementation.
 * Implements proxy specific disconnect message function.
 *
 * @author Goh Wei Sheng <wei.sheng.goh@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include "proxy/disconnect_msg.hpp"
#include "proxy/client.hpp"
#include "common/serialize.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

bool ProxyDisconnectMessage::parseBufferTail()
{
    PrintDebug("[ProxyDisconnectMessage]::parseBufferTail - "
        "Received disconnect message from session ID: " +
        to_string(get_sessionId()));
    Client::removeClient(get_sessionId());
    return true;
}
