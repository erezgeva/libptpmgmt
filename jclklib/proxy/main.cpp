/*! \file main.cpp
    \brief Proxy application implementation

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <proxy/transport.hpp>
#include <proxy/message.hpp>
#include <proxy/connect.hpp>
#include <common/sighandler.hpp>
#include <common/print.hpp>

#include <iostream>

using namespace JClkLibProxy;
using namespace JClkLibCommon;
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
