/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file test.cpp
 * @brief Test client code
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#include "init.hpp"

#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <common/jclklib_import.hpp>
#include <client/client_state.hpp>
#include <client/connect_msg.hpp>
#include <signal.h>

using namespace JClkLibClient;
using namespace JClkLibCommon;
using namespace std;

volatile sig_atomic_t signal_flag = 0;

void signal_handler(int sig)
{
    signal_flag = 1;
}

int main()
{
    int ret = EXIT_SUCCESS;
    JClkLibCommon::jcl_subscription sub = {};
    JClkLibCommon::jcl_state currentState = {};
    JClkLibCommon::jcl_state jcl_state = {};
    JClkLibCommon::jcl_state_event_count eventCount = {};
    int timeout = 10;

    std::uint32_t event2Sub1[1] = {((1<<gmPresentEvent)|(1<<gmChangedEvent)|(1<<servoLockedEvent)|(1<<gmOffsetEvent))};

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGHUP, signal_handler);

    std::cout << "[CLIENT] Before connect : Session ID : " << state.get_sessionId() << "\n";

    if (jcl_connect() == false) {
        std::cout << "[CLIENT] Failure in connecting !!!\n";
        ret = EXIT_FAILURE;
        goto do_exit;
    }
    else {
        std::cout << "[CLIENT] Connected. Session ID : " << state.get_sessionId() << "\n";
    }
    sleep(5);

    sub.get_event().writeEvent(event2Sub1, (std::size_t)sizeof(event2Sub1));
    sub.get_value().setValue(gmOffsetValue, 100000, -100000);
    std::cout << "[CLIENT] set subscribe event : " + sub.c_get_val_event().toString() << "\n";
    jcl_subscribe(sub, currentState);
    std::cout << "[CLIENT] " + state.toString();

    while (!signal_flag) {
        if (!jcl_status_wait(timeout, jcl_state , eventCount)) {
            printf("No event status changes identified in %d seconds.\n\n", timeout);
            sleep(1);
            continue;
        }

        printf("+------------------+--------------+-------------+\n");
        printf("| %-16s | %-12s | %-11s |\n", "Event", "Event Status", "Event Count");
        printf("+------------------+--------------+-------------+\n");
        printf("| %-16s | %-12d | %-11ld |\n", "offset_in_range",
            jcl_state.offset_in_range, eventCount.offset_in_range_event_count);
        printf("| %-16s | %-12d | %-11ld |\n", "servo_locked",
            jcl_state.servo_locked, eventCount.servo_locked_event_count);
        printf("| %-16s | %-12d | %-11ld |\n", "gmPresent",
            jcl_state.gm_present, eventCount.gmPresent_event_count);
        printf("| %-16s | %-12d | %-11ld |\n", "as_Capable",
            jcl_state.as_Capable, eventCount.asCapable_event_count);
        printf("| %-16s | %-12d | %-11ld |\n", "gm_Changed",
            jcl_state.gm_changed, eventCount.gm_changed_event_count);
        printf("+------------------+--------------+-------------+\n");
        printf("| %-16s |     %02x%02x%02x.%02x%02x.%02x%02x%02x     |\n", "UUID",
            jcl_state.gmIdentity[0], jcl_state.gmIdentity[1], jcl_state.gmIdentity[2],
            jcl_state.gmIdentity[3], jcl_state.gmIdentity[4],
            jcl_state.gmIdentity[5], jcl_state.gmIdentity[6], jcl_state.gmIdentity[7]);
        printf("+------------------+----------------------------+\n\n");
        sleep(1);
    }

 do_exit:
	jcl_disconnect();

    return ret;
}
