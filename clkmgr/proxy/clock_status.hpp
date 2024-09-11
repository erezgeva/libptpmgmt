/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file clock_status.hpp
 * @brief Proxy status class. One status object per session.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#ifndef CLOCK_STATUS_HPP
#define CLOCK_STATUS_HPP

#include <rtpi/mutex.hpp>

#include <clkmgr/subscription.h>
#include <common/ptp_event.hpp>

__CLKMGR_NAMESPACE_BEGIN

class ClockStatus
{
  private:
    class Status
    {
      public:
        clkmgr_event    event;
        clkmgr_eventcount   count;
    };
    bool update, writeUpdate;
    rtpi::mutex update_lock;
    Status status;
    Status writeShadow, readShadow;
  public:
    ClockStatus();
    void speculateWrite();
    void setEvent(const clkmgr_event &event);
    void setCount(const clkmgr_eventcount &count);
    void commitWrite();

    bool readConsume();
    const clkmgr_event &getEvent() { return readShadow.event; }
    const clkmgr_eventcount &getCount() { return readShadow.count; }
};

__CLKMGR_NAMESPACE_END

#endif /* CLOCK_STATUS_HPP */
