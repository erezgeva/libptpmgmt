/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file clock_status.cpp
 * @brief Proxy status implementation. One status object per session.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#include <proxy/clock_status.hpp>

__CLKMGR_NAMESPACE_USE

using namespace std;

bool ClockStatus::readConsume()
{
    lock_guard<decltype(update_lock)> update_guard(update_lock);
    if(update)
        readShadow = status;
    update = false;
    return update;
}

ClockStatus::ClockStatus()
{
    /* Initialize status */
}

void ClockStatus::speculateWrite()
{
    lock_guard<decltype(update_lock)> update_guard(update_lock);
    writeShadow = status;
}

void ClockStatus::setEvent(const clkmgr_event &sEvent)
{
    if(writeShadow.event != sEvent) {
        writeShadow.event = sEvent;
        writeUpdate = true;
    }
}

void ClockStatus::setCount(const clkmgr_eventcount &sCount)
{
    if(writeShadow.count != sCount) {
        writeShadow.count = sCount;
        writeUpdate = true;
    }
}

void ClockStatus::commitWrite()
{
    lock_guard<decltype(update_lock)> update_guard(update_lock);
    if(writeUpdate) {
        status = writeShadow;
        update = writeUpdate;
    }
    writeUpdate = false;
}
