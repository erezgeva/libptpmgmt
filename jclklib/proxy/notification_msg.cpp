/*! \file notification_msg.cpp
    \brief Proxy connect message implementation. Implements proxy specific connect message function.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <proxy/notification_msg.hpp>
#include <common/serialize.hpp>
#include <proxy/clock_config.hpp>

using namespace JClkLibProxy;

bool ProxyNotificationMessage::processMessage(ClockConfiguration &config)
{
	//config.setWait( waitEnable );

	return true;
}
bool ProxyNotificationMessage::generateResponse(uint8_t *msgBuffer, size_t &length,
					   const ClockStatus &status)
{
	return false;
}


