/*! \file notification_msg.cpp
    \brief Proxy connect message implementation. Implements proxy specific connect message function.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <proxy/notification_msg.hpp>
#include <common/serialize.hpp>
#include <proxy/clock_config.hpp>

using namespace JClkLibProxy;


/*
* [Azu] This is to send notification from Proxy to subscribed client upon state change
* This shd be in the RX buffer - so this shd be inside the client to process
* notification from proxy.
*/
/* REMOVE */
bool ProxyNotificationMessage::processMessage(ClockConfiguration &config)
{
	//config.setWait( waitEnable );

	return true;
}

/*
TO BE REMOVED
*/
bool ProxyNotificationMessage::generateResponse(uint8_t *msgBuffer, size_t &length,
					   const ClockStatus &status)
{
	return false;
}


