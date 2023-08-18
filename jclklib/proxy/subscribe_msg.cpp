/*! \file subscribe_msg.cpp
    \brief Proxy subscribe message implementation. Implements proxy specific subscribe message function.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <proxy/subscribe_msg.hpp>
#include <proxy/clock_config.hpp>

using namespace JClkLibProxy;

bool ProxySubscribeMessage::processMessage(ClockConfiguration &config)
{
	//config.setEvent(subscription.getEvent());
	//config.setValue(subscription.getValue());

	return true;
}
bool ProxySubscribeMessage::generateResponse(uint8_t *msgBuffer, size_t &length,
					const ClockStatus &status)
{
	return false;
}


