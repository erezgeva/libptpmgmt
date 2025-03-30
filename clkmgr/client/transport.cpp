/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client transport base implementation for clkmgr.
 * It is extended for specific transports such as POSIX message queue.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "client/msgq_tport.hpp"
#include "client/null_tport.hpp"
#include "client/transport.hpp"
#include "common/print.hpp"
#include "common/sighandler.hpp"
#include "common/util.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

bool ClientTransport::init()
{
    if(!Transport::init())
        return false;
    PrintDebug("Finished common init");
    return _initTransport<ClientMessageQueue>();
}

bool ClientTransport::stop()
{
    if(!Transport::stop())
        return false;
    /* Do any transport specific stop */
    return _stopTransport<ClientMessageQueue>();
}

bool ClientTransport::finalize()
{
    if(!Transport::finalize())
        return false;
    return _finalizeTransport<ClientMessageQueue>();
}
