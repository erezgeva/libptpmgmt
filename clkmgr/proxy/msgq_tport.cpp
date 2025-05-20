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
#include "proxy/client.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

static Listener rxContext;

TransportTransmitterContext *Client::CreateTransmitterContext(TransportClientId
    &clientId)
{
    string id((const char *)clientId.data());
    Transmitter *nCtx = new Transmitter();
    if(nCtx != nullptr) {
        if(!nCtx->open(id, false)) {
            PrintErrorCode("Failed to open message queue " + id);
            delete nCtx;
            return nullptr;
        }
        PrintDebug("Successfully connected to client " + id);
    } else
        PrintError("Failed to allocate new message queue " + id);
    return nCtx;
}

bool ProxyMessageQueue::initTransport()
{
    PrintDebug("Initializing Message Queue Proxy Transport...");
    if(!rxContext.init(mqProxyName, MAX_CLIENT_COUNT)) {
        PrintError("Initializing failed");
        return false;
    }
    PrintDebug("Proxy Message queue opened");
    return true;
}

bool ProxyMessageQueue::stopTransport()
{
    return rxContext.stopTransport();
}

bool ProxyMessageQueue::finalizeTransport()
{
    return rxContext.finalize();
}

bool ProxyMessageQueue::stop()
{
    rxContext.stopSignal();
    return true;
}
