/*! \file clock_config.cpp
    \brief Proxy configuration implementation. One configuration object per session is instatiated

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <proxy/clock_config.hpp>

using namespace JClkLibProxy;
using namespace JClkLibCommon;
using namespace std;

bool ClockConfiguration::readConsume()
{
	lock_guard<decltype(update_lock)> update_guard(update_lock);
	if (update)
		readShadow = config;
	update = false;

	return update;
}

ClockConfiguration::ClockConfiguration()
{
	// Initialize configuration
}

void ClockConfiguration::speculateWrite()
{
	lock_guard<decltype(update_lock)> update_guard(update_lock);
	writeShadow = config;
}

void ClockConfiguration::setEvent( const jcl_event &sEvent )
{
	if (writeShadow.event != sEvent) {
		writeShadow.event = sEvent;
		writeUpdate = true;
	}
}

void ClockConfiguration::setValue( const jcl_value &sValue )
{
	if (writeShadow.value != sValue) {
		writeShadow.value = sValue;
		writeUpdate = true;
	}
}

void ClockConfiguration::commitWrite()
{
	lock_guard<decltype(update_lock)> update_guard(update_lock);
	if (writeUpdate) {
		config = writeShadow;
		update = writeUpdate;
	}
	writeUpdate = false;
}

sessionId_t ClockConfiguration::getSessionId()
{
	return InvalidSessionId;
}
