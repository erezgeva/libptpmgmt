/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy transport base class implementation for Clkmgr.
 * It provides common functionality for derived transport classes.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "proxy/transport.hpp"
#include "proxy/msgq_tport.hpp"
#include "proxy/null_tport.hpp"
#include "common/print.hpp"

#if 0
#define EXIT_TIMEOUT    (200 /*ms*/)
#define START_TIMEOUT   (20 /*ms*/)
#endif

__CLKMGR_NAMESPACE_USE;

using namespace std;

bool ProxyTransport::init()
{
    if(!Transport::init())
        return false;
    PrintDebug("Finished common init");
    return _initTransport<NullProxyTransport, ProxyMessageQueue>();
}

bool ProxyTransport::stop()
{
    if(!Transport::stop())
        return false;
    /* Do any transport specific stop */
    return _stopTransport<NullProxyTransport,
        ProxyMessageQueue>();
}

bool ProxyTransport::finalize()
{
    if(!Transport::finalize())
        return false;
    return _finalizeTransport<NullProxyTransport,
        ProxyMessageQueue>();
}
