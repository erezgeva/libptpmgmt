/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file main.cpp
 * @brief Proxy application implementation
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#include <iostream>

#include <common/print.hpp>
#include <common/sighandler.hpp>
#include <proxy/connect.hpp>
#include <proxy/message.hpp>
#include <proxy/transport.hpp>

using namespace JClkLibCommon;
using namespace JClkLibProxy;
using namespace std;

int main()
{
    //BlockStopSignal();
    if(!ProxyTransport::init()) {
        cout << "Transport init failed" << endl;
        return -1;
    }
    if(!ProxyMessage::init()) {
        cout << "Message init failed" << endl;
        return -1;
    }
    Connect::connect();
    WaitForStopSignal();
    PrintDebug("Got stop signal");
    if(!ProxyTransport::stop()) {
        cout << "stop failed" << endl;
        return -1;
    }
    if(!ProxyTransport::finalize()) {
        cout << "finalize failed" << endl;
        return -1;
    }
	
    return 0;
}
