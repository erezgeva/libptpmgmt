/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file clock_config.cpp
 * @brief Proxy configuration implementation. One configuration object per session is instatiated
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
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
