/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy queue implementation.
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

bool clkmgr::proxyQueueInit()
{
    PrintDebug("Initializing Proxy Queue ...");
    if(!rxContext.init(mqProxyName, MAX_CLIENT_COUNT)) {
        PrintError("Initializing failed");
        return false;
    }
    PrintDebug("Proxy queue opened");
    return true;
}
