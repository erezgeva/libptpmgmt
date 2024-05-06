/*! \file init.cpp
    \brief Client utilities to setup and cleanup the library.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <client/init.hpp>
#include <client/msgq_tport.hpp>
#include <client/connect_msg.hpp>
#include <client/subscribe_msg.hpp>
#include <common/sighandler.hpp>
#include <common/print.hpp>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <iostream>
#include <cstring>

#define DEFAULT_CONNECT_TIME_OUT 5  //5 sec
#define DEFAULT_SUBSCRIBE_TIME_OUT 5  //5 sec

using namespace JClkLibClient;
using namespace JClkLibCommon;
using namespace std;

std::mutex ClientConnectMessage::cv_mtx;
std::condition_variable ClientConnectMessage::cv;
std::mutex ClientSubscribeMessage::cv_mtx;
std::condition_variable ClientSubscribeMessage::cv;

TransportClientId globalClientID;

bool JClkLibClient::connect()
{
	unsigned int timeout_sec = (unsigned int) DEFAULT_CONNECT_TIME_OUT;
	Message0 connectMsg(new ClientConnectMessage());

	//BlockStopSignal();
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
	auto endTime = std::chrono::system_clock::now() + std::chrono::seconds(timeout_sec);
	std::unique_lock<std::mutex> lck(ClientConnectMessage::cv_mtx);
	while (state.get_connected() == false )
	{
		auto res = ClientConnectMessage::cv.wait_until(lck, endTime);
		if (res == std::cv_status::timeout) {
			if (state.get_connected() == false) {
				PrintDebug("[CONNECT] Connect reply from proxy - timeout failure!!");
				return false;
				}
		}
		else {
			PrintDebug("[CONNECT] Connect reply received.");
		}
	}

	ClientConnectMessage *cmsg = dynamic_cast<decltype(cmsg)>(connectMsg.get());
	strcpy((char *)globalClientID.data(), (char *)cmsg->getClientId().data());
	state.set_clientID(globalClientID);

	return true;
}


bool JClkLibClient::subscribe(JClkLibCommon::jcl_subscription &newSub, JClkLibCommon::jcl_state &currentState)
{
	unsigned int timeout_sec = (unsigned int) DEFAULT_SUBSCRIBE_TIME_OUT;

	PrintDebug("[JClkLibClient]::subscribe");
	MessageX subscribeMsg(new ClientSubscribeMessage());

	ClientSubscribeMessage *cmsg = dynamic_cast<decltype(cmsg)>(subscribeMsg.get());
	if (cmsg == NULL) {
		PrintErrorCode("[JClkLibClient::subscribe] subscribeMsg is NULL !!\n");
		return false;
	}
	else
		PrintDebug("[JClkLibClient::subscribe] subscribeMsgcreation is OK !!\n");

	/* Write the current event subscription */
	cmsg->getSubscription().get_event().copyEventMask(newSub.get_event());

	ClientMessageQueue::writeTransportClientId(subscribeMsg.get());
	ClientMessageQueue::sendMessage(subscribeMsg.get());

	// Wait for subscription result
	auto endTime = std::chrono::system_clock::now() + std::chrono::seconds(timeout_sec);
	std::unique_lock<std::mutex> lck(ClientSubscribeMessage::cv_mtx);
	while (state.get_subscribed() == false )
	{
		auto res = ClientSubscribeMessage::cv.wait_until(lck, endTime);
		if (res == std::cv_status::timeout) {
			if (state.get_subscribed() == false) {
				PrintDebug("[SUBSCRIBE] No reply from proxy - timeout failure!!");
				return false;
				}
		}
		else {
			PrintDebug("[SUBSCRIBE] SUBSCRIBE reply received.");
		}
	}

	JClkLibCommon::jcl_state jclCurrentState = state.get_eventState();
	printf("[JClkLibClient]::subscribe : state -  \n");
	printf ("offset_in_range = %d, servo_locked = %d gmPresent = %d as_Capable = %d gm_Changed = %d\n", \
	jclCurrentState.offset_in_range, jclCurrentState.servo_locked,\
	jclCurrentState.gm_present, jclCurrentState.as_Capable, jclCurrentState.gm_changed);

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
