/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file clkmgr_subscription.cpp
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
#include "clkmgr_subscription.hpp"

__CLKMGR_NAMESPACE_USE

using namespace std;

clkmgr_value::value_t::value_t()
{
    upper = 0;
    lower = 0;
}

clkmgr_value::value_t::value_t(uint32_t limit)
{
    upper = limit;
    lower = limit;
}

bool clkmgr_value::value_t::equal(const value_t &v)
{
    if(this->upper == v.upper && this->lower == v.lower)
        return true;
    return false;
}

uint8_t *clkmgr_value::parse(uint8_t *buf, std::size_t &length)
{
    return buf;
}

uint8_t *clkmgr_value::write(uint8_t *buf, std::size_t &length)
{
    for(size_t i = 0; i < sizeof(value) / sizeof(value[0]) && buf != nullptr; ++i)
        buf += WRITE_FIELD(value[i], buf, length);
    for(size_t i = 0; i < sizeof(reserved) / sizeof(reserved[0]) &&
        buf != nullptr; ++i)
        reserved[i].zero();
    return buf;
}

bool clkmgr_value::equal(const clkmgr_value &c)
{
    if(memcmp(this->value, c.value, sizeof(this->value)) == 0)
        return true;
    return false;
}

int8_t clkmgr_event::writeEvent(uint32_t *newEvent, std::size_t newEventlength)
{
    if(newEvent == nullptr || sizeof(event_mask) < newEventlength) {
        PrintDebug("clkmgr_event::writeEvent failure");
        return -1;
    }
    memcpy(this->event_mask, newEvent, newEventlength);
    return 0;
}

int8_t clkmgr_event::readEvent(uint32_t *readEvnt, std::size_t eventLength)
{
    if(readEvnt == nullptr || sizeof(event_mask) > eventLength) {
        PrintDebug("clkmgr_event::readEvent failure");
        return -1;
    }
    memcpy(readEvnt, this->event_mask, eventLength);
    return 0;
}

int8_t clkmgr_event::copyEventMask(clkmgr_event &newEvent)
{
    std::size_t newEventLength = sizeof(newEvent.getEventMask());
    if(sizeof(event_mask) < newEventLength) {
        PrintDebug("clkmgr_event::copyEventMask failure");
        return -1;
    }
    memcpy(event_mask, newEvent.event_mask, newEventLength);
    return 0;
}

uint8_t *clkmgr_event::parse(uint8_t *buf, std::size_t &length)
{
    return buf;
}

uint8_t *clkmgr_event::write(uint8_t *buf, std::size_t &length)
{
    for(size_t i = 0; i < sizeof(event_mask) / sizeof(event_mask[0]) &&
        buf != nullptr; ++i)
        buf += WRITE_FIELD(event_mask[i], buf, length);
    memset(reserved, 0, sizeof(reserved));
    return buf;
}

void clkmgr_event::zero()
{
    for(size_t i = 0; i < sizeof(event_mask) / sizeof(event_mask[0]); ++i)
        event_mask[i] = 0;
    memset(reserved, 0, sizeof(reserved));
}

bool clkmgr_event::equal(const clkmgr_event &c)
{
    if(memcmp(this->event_mask, c.event_mask, sizeof(event_mask)) == 0)
        return true;
    return false;
}

uint8_t *clkmgr_eventcount::parse(uint8_t *buf, std::size_t &length)
{
    return buf;
}

uint8_t *clkmgr_eventcount::write(uint8_t *buf, std::size_t &length)
{
    for(size_t i = 0; i < sizeof(count) / sizeof(count[0]) && buf != nullptr; ++i)
        buf += WRITE_FIELD(count[i], buf, length);
    memset(reserved, 0, sizeof(reserved));
    return buf;
}

void clkmgr_eventcount::zero()
{
    for(size_t i = 0; i < sizeof(count) / sizeof(count[0]); ++i)
        count[i] = 0;
    memset(reserved, 0, sizeof(reserved));
}

bool clkmgr_eventcount::equal(const clkmgr_eventcount &ec)
{
    if(memcmp(this->count, ec.count, sizeof(this->count)) == 0)
        return true;
    return false;
}

std::string clkmgr_value::toString()
{
    std::string name = "clkmgr_value : upper " + to_string(this->value->upper) +
        " lower : " +  to_string(this->value->lower) + "\n";
    return name;
}

std::string clkmgr_event::toString()
{
    std::string name = "clkmgr_event : event[0] = " +
        to_string(this->event_mask[0]) + "\n";
    return name;
}
