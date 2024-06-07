/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file transport.cpp
 * @brief Proxy transport base class implementation for JClkLib. It provides common functionality for derived transport classes.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#include <common/print.hpp>
#include <common/sighandler.hpp>
#include <common/util.hpp>
#include <proxy/msgq_tport.hpp>
#include <proxy/null_tport.hpp>
#include <proxy/transport.hpp>

#define EXIT_TIMEOUT	(200 /*ms*/)
#define START_TIMEOUT	(20 /*ms*/)

using namespace JClkLibProxy;
using namespace JClkLibCommon;
using namespace std;

bool ProxyTransport::init()
{
    if (!Transport::init())
        return false;
    PrintDebug("Finished common init");
	
    return JClkLibCommon::_initTransport<NullProxyTransport,ProxyMessageQueue>();
}

bool ProxyTransport::stop()
{
    if (!Transport::stop())
        return false;

    /* Do any transport specific stop */
    return JClkLibCommon::_stopTransport<NullProxyTransport,ProxyMessageQueue>();
}

bool ProxyTransport::finalize()
{
    if (!Transport::finalize())
        return false;

    return JClkLibCommon::_finalizeTransport<NullProxyTransport,ProxyMessageQueue>();
}
