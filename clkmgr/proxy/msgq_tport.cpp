/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy POSIX message queue transport implementation.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "proxy/msgq_tport.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

static ProxyMessageQueueListenerContext listenerQueue;

TransportTransmitterContext
*ProxyMessageQueueListenerContext::CreateTransmitterContext(
    TransportClientId &clientId)
{
    string id((const char *)clientId.data());
    PosixMessageQueue txd;
    if(!txd.TxOpen(id, false)) {
        PrintErrorCode("Failed to open message queue " + id);
        return nullptr;
    }
    PrintDebug("Successfully connected to client " + id);
    return new ProxyMessageQueueTransmitterContext(std::move(txd));
}

bool ProxyMessageQueue::initTransport()
{
    PrintDebug("Initializing Message Queue Proxy Transport...");
    if(!listenerQueue.init(mqProxyName, MAX_CLIENT_COUNT)) {
        PrintError("Initializing failed");
        return false;
    }
    PrintDebug("Proxy Message queue opened");
    return true;
}

bool ProxyMessageQueue::stopTransport()
{
    return listenerQueue.stopTransport();
}

bool ProxyMessageQueue::finalizeTransport()
{
    return listenerQueue.finalize();
}

bool ProxyMessageQueue::stop()
{
    listenerQueue.stopSignal();
    return true;
}
