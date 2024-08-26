/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file jclk_subscription.cpp
 * @brief PTP event subscription.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#include <cstring>

#include <common/print.hpp>
#include <common/serialize.hpp>
#include "jclk_subscription.hpp"

__CLKMGR_NAMESPACE_USE

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
    if(this->upper == v.upper && this->lower == v.lower)
        return true;
    return false;
}

uint8_t *jcl_value::parse(uint8_t *buf, std::size_t &length)
{
    return buf;
}

uint8_t *jcl_value::write(uint8_t *buf, std::size_t &length)
{
    for(size_t i = 0; i < sizeof(value) / sizeof(value[0]) && buf != nullptr; ++i)
        buf += WRITE_FIELD(value[i], buf, length);
    for(size_t i = 0; i < sizeof(reserved) / sizeof(reserved[0]) &&
        buf != nullptr; ++i)
        reserved[i].zero();
    return buf;
}

bool jcl_value::equal(const jcl_value &c)
{
    if(memcmp(this->value, c.value, sizeof(this->value)) == 0)
        return true;
    return false;
}

int8_t jcl_event::writeEvent(uint32_t *newEvent, std::size_t newEventlength)
{
    if(newEvent == nullptr || sizeof(event_mask) < newEventlength) {
        PrintDebug("jcl_event::writeEvent failure");
        return -1;
    }
    memcpy(this->event_mask, newEvent, newEventlength);
    return 0;
}

int8_t jcl_event::readEvent(uint32_t *readEvnt, std::size_t eventLength)
{
    if(readEvnt == nullptr || sizeof(event_mask) > eventLength) {
        PrintDebug("jcl_event::readEvent failure");
        return -1;
    }
    memcpy(readEvnt, this->event_mask, eventLength);
    return 0;
}

int8_t jcl_event::copyEventMask(jcl_event &newEvent)
{
    std::size_t newEventLength = sizeof(newEvent.getEventMask());
    if(sizeof(event_mask) < newEventLength) {
        PrintDebug("jcl_event::copyEventMask failure");
        return -1;
    }
    memcpy(event_mask, newEvent.event_mask, newEventLength);
    return 0;
}

uint8_t *jcl_event::parse(uint8_t *buf, std::size_t &length)
{
    return buf;
}

uint8_t *jcl_event::write(uint8_t *buf, std::size_t &length)
{
    for(size_t i = 0; i < sizeof(event_mask) / sizeof(event_mask[0]) &&
        buf != nullptr; ++i)
        buf += WRITE_FIELD(event_mask[i], buf, length);
    memset(reserved, 0, sizeof(reserved));
    return buf;
}

void jcl_event::zero()
{
    for(size_t i = 0; i < sizeof(event_mask) / sizeof(event_mask[0]); ++i)
        event_mask[i] = 0;
    memset(reserved, 0, sizeof(reserved));
}

bool jcl_event::equal(const jcl_event &c)
{
    if(memcmp(this->event_mask, c.event_mask, sizeof(event_mask)) == 0)
        return true;
    return false;
}

uint8_t *jcl_eventcount::parse(uint8_t *buf, std::size_t &length)
{
    return buf;
}

uint8_t *jcl_eventcount::write(uint8_t *buf, std::size_t &length)
{
    for(size_t i = 0; i < sizeof(count) / sizeof(count[0]) && buf != nullptr; ++i)
        buf += WRITE_FIELD(count[i], buf, length);
    memset(reserved, 0, sizeof(reserved));
    return buf;
}

void jcl_eventcount::zero()
{
    for(size_t i = 0; i < sizeof(count) / sizeof(count[0]); ++i)
        count[i] = 0;
    memset(reserved, 0, sizeof(reserved));
}

bool jcl_eventcount::equal(const jcl_eventcount &ec)
{
    if(memcmp(this->count, ec.count, sizeof(this->count)) == 0)
        return true;
    return false;
}

std::string jcl_value::toString()
{
    std::string name = "jcl_value : upper " + to_string(this->value->upper) +
        " lower : " +  to_string(this->value->lower) + "\n";
    return name;
}

std::string jcl_event::toString()
{
    std::string name = "jcl_event : event[0] = " +
        to_string(this->event_mask[0]) + "\n";
    return name;
}
