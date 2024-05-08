/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file init.cpp
 * @brief Client utilities to setup and cleanup the library.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
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

extern JClkLibCommon::client_ptp_event client_ptp_data;

TransportClientId globalClientID;

bool JClkLibClient::jcl_connect()
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


bool JClkLibClient::jcl_subscribe(JClkLibCommon::jcl_subscription &newSub,
				  JClkLibCommon::jcl_state &currentState)
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
	state.get_eventSub().set_event(newSub.getc_event());
	state.get_eventSub().set_value(newSub.getc_value());
	state.get_eventSub().set_composite_event(newSub.getc_composite_event());

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
	printf("gmIdentity = %02x%02x%02x.%02x%02x.%02x%02x%02x \n",
		jclCurrentState.gmIdentity[0], jclCurrentState.gmIdentity[1],jclCurrentState.gmIdentity[2],
		jclCurrentState.gmIdentity[3], jclCurrentState.gmIdentity[4],
		jclCurrentState.gmIdentity[5], jclCurrentState.gmIdentity[6],jclCurrentState.gmIdentity[7]);
	return true;
}

bool JClkLibClient::jcl_disconnect()
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

/**
 * @brief This function waits for a specified timeout period for any event changes.
 *
 * @param timeout The timeout period in seconds. If timeout is 0, the function
 *                will check event changes once. If timeout is -1, the function
 *                wait until there is event changes occurs.
 * @param jcl_state A reference to a jcl_state object where the current state
 *                  will be stored.
 * @param eventCount A reference to a jcl_state_event_count object where the
 *                   event counts will be stored.
 *
 * @return Returns true if there is event changes within the timeout period,
 *         and false otherwise.
 */
int JClkLibClient::jcl_status_wait(int timeout, JClkLibCommon::jcl_state &jcl_state,
				   JClkLibCommon::jcl_state_event_count &eventCount)
{
	auto start = std::chrono::high_resolution_clock::now();
	auto end = (timeout == -1) ?
		   std::chrono::time_point<std::chrono::high_resolution_clock>::max() :
		   start + std::chrono::seconds(timeout);
	bool event_changes_detected = false;

	do {
		/* Get the event state and event count*/
		eventCount = state.get_eventStateCount();
		jcl_state = state.get_eventState();

		/* Check if any member of eventCount is non-zero */
		if (eventCount.offset_in_range_event_count ||
		    eventCount.asCapable_event_count ||
		    eventCount.servo_locked_event_count ||
		    eventCount.gmPresent_event_count ||
		    eventCount.composite_event_count ||
		    eventCount.gm_changed_event_count) {
			event_changes_detected = true;
			break;
		}

		/* Sleep for a short duration before the next iteration */
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	} while (std::chrono::high_resolution_clock::now() < end);

	if (!event_changes_detected)
		return false;

	/* Reset the atomic */
	client_ptp_data.offset_event_count.fetch_sub(eventCount.offset_in_range_event_count,
						     std::memory_order_relaxed);
	client_ptp_data.asCapable_event_count.fetch_sub(eventCount.asCapable_event_count,
							std::memory_order_relaxed);
	client_ptp_data.servo_state_event_count.fetch_sub(eventCount.servo_locked_event_count,
							  std::memory_order_relaxed);
	client_ptp_data.gmPresent_event_count.fetch_sub(eventCount.gmPresent_event_count,
							std::memory_order_relaxed);
	client_ptp_data.gmChanged_event_count.fetch_sub(eventCount.gm_changed_event_count,
							std::memory_order_relaxed);
	client_ptp_data.composite_event_count.fetch_sub(eventCount.composite_event_count,
							std::memory_order_relaxed);

	return true;
}
