/*! \file jclklib_import.cpp
    \brief C API import.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <common/jclklib_import.hpp>
#include <common/serialize.hpp>
#include <common/print.hpp>

#include <cstring>

using namespace JClkLibCommon;
using namespace std;


jcl_value::value_t::value_t()
{
	upper = 0;
	lower = 0;
}

jcl_value::value_t::value_t(uint32_t limit)
{
	upper = limit;
	lower = limit;
}

bool jcl_value::value_t::equal(const value_t &v)
{
	if (this->upper == v.upper && this->lower == v.lower)
		return true;
	return false;
}

uint8_t *jcl_value::parse(uint8_t *buf, std::size_t &length)
{
	for(size_t i = 0; i < sizeof(value)/sizeof(value[0]) && buf != NULL; ++i)
		//buf = PARSE(value[i],buf,length);
	for(size_t i = 0; i < sizeof(reserved)/sizeof(reserved[0]) && buf != NULL; ++i) {
		//buf = PARSE(reserved[i],buf,length);
		if (!CHECK_RSV(reserved[i]))
			return (uint8_t *)-1;
	}

	return buf;
}

uint8_t *jcl_value::write(uint8_t *buf, std::size_t &length)
{
	for(size_t i = 0; i < sizeof(value)/sizeof(value[0]) && buf != NULL; ++i)
		buf += WRITE_FIELD(value[i],buf,length);
	for(size_t i = 0; i < sizeof(reserved)/sizeof(reserved[0]) && buf != NULL; ++i)
		reserved[i].zero();

	return buf;
}

bool jcl_value::equal( const jcl_value &c)
{
	if (memcmp(this->value, c.value, sizeof(this->value)) == 0)
		return true;
	return false;
}


uint8_t *jcl_subscription::parse(uint8_t *buf, std::size_t &length)
{
	buf = event.parse(buf, length);
	if (buf != NULL)
		buf = value.parse(buf, length);

	return buf;
}

uint8_t *jcl_subscription::write(uint8_t *buf, std::size_t &length)
{
	buf = event.write(buf, length);
	if (buf != NULL)
		buf = value.write(buf, length);

	return buf;
}

int8_t jcl_event::writeEvent(uint32_t *newEvent, std::size_t newEventlength)
{
	if (newEvent == NULL || sizeof(event_mask) < newEventlength) {
		PrintDebug("jcl_event::writeEvent failure");
		return -1;
	}

	memcpy(this->event_mask, newEvent , newEventlength);

	return 0;
}

int8_t jcl_event::readEvent(uint32_t *readEvnt, std::size_t eventLength)
{
	if (readEvnt == NULL || sizeof(event_mask) > eventLength) {
		PrintDebug("jcl_event::readEvent failure");
		return -1;
	}

	memcpy(readEvnt, this->event_mask, eventLength);

	return 0;
}

int8_t jcl_event::copyEventMask(jcl_event &newEvent)
{
	std::size_t newEventLength = sizeof(newEvent.getEventMask());

	memcpy(event_mask, newEvent.event_mask , newEventLength);
	printf ("[jcl_event]::copyEventMask - newEventLength = %ld , incoming event mask = 0x%x\n", newEventLength, newEvent.getEventMask()[0]);

	return 0;
}

uint8_t *jcl_event::parse(uint8_t *buf, std::size_t &length)
{
	for(size_t i = 0; i < sizeof(event_mask)/sizeof(event_mask[0]) && buf != NULL; ++i)
		//buf = PARSE(event_mask[i],buf,length);
	for(size_t i = 0; i < sizeof(reserved)/sizeof(reserved[0]) && buf != NULL; ++i) {
		//buf = PARSE(reserved[i],buf,length);
		if (!CHECK_RSV(reserved[i]))
			return (uint8_t *)-1;
	}

	return buf;
}

uint8_t *jcl_event::write(uint8_t *buf, std::size_t &length)
{
	for(size_t i = 0; i < sizeof(event_mask)/sizeof(event_mask[0]) && buf != NULL; ++i)
		buf += WRITE_FIELD(event_mask[i],buf,length);
	memset(reserved, 0, sizeof(reserved));

	return buf;
}

void jcl_event::zero()
{
	for(size_t i = 0; i < sizeof(event_mask)/sizeof(event_mask[0]); ++i)
		event_mask[i] = 0;
	memset(reserved, 0, sizeof(reserved));
}

bool jcl_event::equal(const jcl_event &c)
{
	if (memcmp(this->event_mask, c.event_mask, sizeof(event_mask)) == 0)
		return true;
	return false;
}

uint8_t *jcl_eventcount::parse(uint8_t *buf, std::size_t &length)
{
	for(size_t i = 0; i < sizeof(count)/sizeof(count[0]) && buf != NULL; ++i)
		//buf = PARSE(count[i],buf,length);
	for(size_t i = 0; i < sizeof(reserved)/sizeof(reserved[0]) && buf != NULL; ++i) {
		//buf = PARSE(reserved[i],buf,length);
		if (!CHECK_RSV(reserved[i]))
			return (uint8_t *)-1;
	}

	return buf;
}

uint8_t *jcl_eventcount::write(uint8_t *buf, std::size_t &length)
{
	for(size_t i = 0; i < sizeof(count)/sizeof(count[0]) && buf != NULL; ++i)
		buf += WRITE_FIELD(count[i],buf,length);
	memset(reserved, 0, sizeof(reserved));

	return buf;
}

void jcl_eventcount::zero()
{
	for(size_t i = 0; i < sizeof(count)/sizeof(count[0]); ++i)
		count[i] = 0;
	memset(reserved, 0, sizeof(reserved));
}

bool jcl_eventcount::equal( const jcl_eventcount &ec)
{
	if (memcmp(this->count, ec.count, sizeof(this->count)) == 0)
		return true;
	return false;
}

std::string jcl_value::toString()
{
    std::string name = "jcl_value : upper " + to_string (this->value->upper) + " lower : " +  to_string (this->value->lower) + "\n";
	return name;
}

std::string jcl_event::toString()
{
    std::string name = "jcl_event : event[0] = " + to_string (this->event_mask[0]) + "\n";
	return name;
}
