/*! \file init.cpp
    \brief Client utilities to setup and cleanup the library.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <client/init.hpp>
#include <client/msgq_tport.hpp>
#include <client/connect_msg.hpp>
#include <common/sighandler.hpp>
#include <common/print.hpp>

using namespace JClkLibClient;
using namespace JClkLibCommon;
using namespace std;

bool JClkLibClient::connect()
{
	Message0 connectMsg(new ClientConnectMessage());

	BlockStopSignal();
	if(!ClientMessage::init()) {
		PrintError("Client Message Init Failed");
		return false;
	}
	if(!ClientTransport::init()) {
		PrintError("Client Transport Init Failed");
		return false;
	}

	ClientMessageQueue::writeTransportClientId(connectMsg.get());
	ClientMessageQueue::sendMessage(connectMsg.get());

	// Wait for connection result
	

	return true;
}

bool JClkLibClient::disconnect()
{
	bool retVal = false;

	// Send a disconnect message
	if(!ClientTransport::stop()) {
		PrintDebug("Client Stop Failed");
		goto done;
	}
	if(!ClientTransport::finalize()) {
		PrintDebug("Client Finalize Failed");
		goto done;
	}
	retVal = true;

 done:
	if (!retVal)
		PrintError("Client Error Occured");
	return retVal;
}
