/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file
 * @brief Test clkmgr client code
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 * @note This is a sample code, not a product! You should use it as a reference.
 *  You can compile it with: g++ -Wall clkmgr_test.cpp -o clkmgr_test -lclkmgr
 *  or use the Makefile with: make
 *
 */

#include <getopt.h>
#include <iostream>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include <clockmanager.h>

using namespace clkmgr;

volatile sig_atomic_t signal_flag = 0;

void signal_handler(int sig)
{
    signal_flag = 1;
}

double getMonotonicTime() {
    struct timespec timeSpec;

    if (clock_gettime(CLOCK_MONOTONIC, &timeSpec) == -1) {
        perror("clock_gettime failed");
        return -1;
    }

    double seconds = timeSpec.tv_sec;
    double nanoseconds = timeSpec.tv_nsec / 1e9;

    return seconds + nanoseconds;
}

int main(int argc, char *argv[])
{
    ClkMgrSubscription subscription = {};
    clkmgr_event_count eventCount = {};
    clkmgr_event_state eventState = {};
    int32_t  gmOffsetLowerLimit = -100000;
    int32_t  gmOffsetUpperLimit = 100000;
    int ret = EXIT_SUCCESS;
    uint32_t idleTime = 1;
    uint32_t timeout = 10;
    int retval;
    int option;

    std::uint32_t event2Sub = {
        (eventGMOffset | eventSyncedToGM | eventASCapable |
        eventGMChanged)
    };

    std::uint32_t composite_event = {
        (eventGMOffset | eventSyncedToGM | eventASCapable)
    };

    while ((option = getopt(argc, argv, "s:c:u:l:i:t:h")) != -1) {
        switch (option) {
        case 's':
            event2Sub = std::stoul(optarg, nullptr, 0);
            break;
        case 'c':
            composite_event = std::stoul(optarg, nullptr, 0);
            break;
        case 'u':
            gmOffsetUpperLimit = std::stoi(optarg);
            break;
        case 'l':
            gmOffsetLowerLimit = std::stoi(optarg);
            break;
        case 'i':
            idleTime = std::stoi(optarg);
            break;
        case 't':
            timeout = std::stoi(optarg);
            break;
        case 'h':
            std::cout << "Usage of " << argv[0] << " :\n"
                "Options:\n"
                "  -s subscribe_event_mask\n"
                "     Default: 0x" << std::hex << event2Sub << "\n"
                "     Bit 0: eventGMOffset\n"
                "     Bit 1: eventSyncedToGM\n"
                "     Bit 2: eventASCapable\n"
                "     Bit 3: eventGMChanged\n"
                "  -c composite_event_mask\n"
                "     Default: 0x" << composite_event << "\n"
                "     Bit 0: eventGMOffset\n"
                "     Bit 1: eventSyncedToGM\n"
                "     Bit 2: eventASCapable\n"
                "  -u gm offset upper limit (ns)\n"
                "     Default: " << std::dec << gmOffsetUpperLimit << " ns\n"
                "  -l gm offset lower limitt (ns)\n"
                "     Default: " << gmOffsetLowerLimit << " ns\n"
                "  -i idle time (s)\n"
                "     Default: " << idleTime << " s\n"
                "  -t timeout in waiting notification event (s)\n"
                "     Default: " << timeout << " s\n";
            return EXIT_SUCCESS;
        default:
            std::cerr << "Usage of " << argv[0] << " :\n"
                "Options:\n"
                "  -s subscribe_event_mask\n"
                "     Default: 0x" << std::hex << event2Sub << "\n"
                "     Bit 0: eventGMOffset\n"
                "     Bit 1: eventSyncedToGM\n"
                "     Bit 2: eventASCapable\n"
                "     Bit 3: eventGMChanged\n"
                "  -c composite_event_mask\n"
                "     Default: 0x" << composite_event << "\n"
                "     Bit 0: eventGMOffset\n"
                "     Bit 1: eventSyncedToGM\n"
                "     Bit 2: eventASCapable\n"
                "  -u gm offset upper limit (ns)\n"
                "     Default: " << std::dec << gmOffsetUpperLimit << " ns\n"
                "  -l gm offset lower limit (ns)\n"
                "     Default: " << gmOffsetLowerLimit << " ns\n"
                "  -i idle time (s)\n"
                "     Default: " << idleTime << " s\n"
                "  -t timeout in waiting notification event (s)\n"
                "     Default: " << timeout << " s\n";
            return EXIT_FAILURE;
        }
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGHUP, signal_handler);

    ClockManager *cm = new ClockManager();

    cm->init();

    ClientState &myState = cm->getClientState();

    if (cm->clkmgr_connect() == false) {
        std::cout << "[clkmgr] Failure in connecting !!!\n";
        ret = EXIT_FAILURE;
        goto do_exit;
    } else {
        std::cout << "[clkmgr] Connected. Session ID : " <<
            myState.get_sessionId() << "\n";
    }

    sleep(1);

    subscription.set_event_mask(event2Sub);
    subscription.define_threshold(thresholdGMOffset, gmOffsetUpperLimit,
        gmOffsetLowerLimit);
    subscription.set_composite_event_mask(composite_event);
    std::cout << "[clkmgr] set subscribe event : 0x"
        << std::hex << subscription.c_get_val_event_mask() <<  "\n";
    std::cout << "[clkmgr] set composite event : 0x"
        << std::hex << subscription.c_get_val_composite_event_mask() <<  "\n";
    std::cout << "GM Offset upper limit: " << std::dec << gmOffsetUpperLimit << " ns\n";
    std::cout << "GM Offset lower limit: " << std::dec << gmOffsetLowerLimit << " ns\n\n";

    if (cm->clkmgr_subscribe(subscription, eventState) == false) {
        std::cerr << "[clkmgr] Failure in subscribing to clkmgr Proxy !!!\n";
        cm->clkmgr_disconnect();
        return EXIT_FAILURE;
    }

    printf("[clkmgr][%.3f] Obtained data from Subscription Event:\n",
        getMonotonicTime());
    printf("+---------------------------+------------------------+\n");
    printf("| %-25s | %-22s |\n", "Event", "Event Status");
    if (event2Sub) {
        printf("+---------------------------+------------------------+\n");
    }
    if (event2Sub & eventGMOffset) {
        printf("| %-25s | %-22d |\n", "offset_in_range",
            eventState.offset_in_range);
    }
    if (event2Sub & eventSyncedToGM) {
        printf("| %-25s | %-22d |\n", "synced_to_primary_clock", eventState.synced_to_primary_clock);
    }
    if (event2Sub & eventASCapable) {
        printf("| %-25s | %-22d |\n", "as_capable", eventState.as_capable);
    }
    if (event2Sub & eventGMChanged) {
        printf("| %-25s | %-22d |\n", "gm_Changed", eventState.gm_changed);
    }
    printf("+---------------------------+------------------------+\n");
    printf("| %-25s | %02x%02x%02x.%02x%02x.%02x%02x%02x     |\n", "UUID",
        eventState.gm_identity[0], eventState.gm_identity[1],
        eventState.gm_identity[2], eventState.gm_identity[3],
        eventState.gm_identity[4], eventState.gm_identity[5],
        eventState.gm_identity[6], eventState.gm_identity[7]);
    printf("| %-25s | %-19ld ns |\n",
            "clock_offset", eventState.clock_offset);
    printf("| %-25s | %-19ld ns |\n",
            "notification_timestamp", eventState.notification_timestamp);
    printf("+---------------------------+------------------------+\n");
    if (composite_event) {
        printf("| %-25s | %-22d |\n", "composite_event",
            eventState.composite_event);
    }
    if (composite_event & eventGMOffset) {
        printf("| - %-23s | %-22s |\n", "offset_in_range", " ");
    }
    if (composite_event & eventSyncedToGM) {
        printf("| - %-19s | %-22s |\n", "synced_to_primary_clock", " ");
    }
    if (composite_event & eventASCapable) {
        printf("| - %-23s | %-22s |\n", "as_capable", " ");
    }
    if (composite_event) {
        printf("+---------------------------+------------------------+\n\n");
    } else {
        printf("\n");
    }

    sleep(1);

    while (!signal_flag) {
        printf("[clkmgr][%.3f] Waiting for Notification Event...\n",
            getMonotonicTime());
        retval = cm->clkmgr_status_wait(timeout, eventState , eventCount);
        if (!retval) {
            printf("[clkmgr][%.3f] No event status changes identified in %d seconds.\n\n",
                getMonotonicTime(), timeout);
            printf("[clkmgr][%.3f] sleep for %d seconds...\n\n",
                getMonotonicTime(), idleTime);
            sleep(idleTime);
            continue;
        } else if (retval < 0) {
            printf("[clkmgr][%.3f] Terminating: lost connection to clkmgr Proxy\n",
                getMonotonicTime());
            return EXIT_SUCCESS;
        }

        printf("[clkmgr][%.3f] Obtained data from Notification Event:\n",
            getMonotonicTime());
        printf("+---------------------------+--------------+-------------+\n");
        printf("| %-25s | %-12s | %-11s |\n", "Event", "Event Status",
            "Event Count");
        if (event2Sub) {
        printf("+---------------------------+--------------+-------------+\n");
        }
        if (event2Sub & eventGMOffset) {
            printf("| %-25s | %-12d | %-11d |\n", "offset_in_range",
                eventState.offset_in_range,
                eventCount.offset_in_range_event_count);
        }
        if (event2Sub & eventSyncedToGM) {
            printf("| %-25s | %-12d | %-11d |\n", "synced_to_primary_clock",
               eventState.synced_to_primary_clock, eventCount.synced_to_gm_event_count);
        }
        if (event2Sub & eventASCapable) {
            printf("| %-25s | %-12d | %-11d |\n", "as_capable",
                eventState.as_capable, eventCount.as_capable_event_count);
        }
        if (event2Sub & eventGMChanged) {
            printf("| %-25s | %-12d | %-11d |\n", "gm_Changed",
                eventState.gm_changed, eventCount.gm_changed_event_count);
        }
        printf("+---------------------------+--------------+-------------+\n");
        printf("| %-25s |     %02x%02x%02x.%02x%02x.%02x%02x%02x     |\n",
            "UUID", eventState.gm_identity[0], eventState.gm_identity[1],
            eventState.gm_identity[2], eventState.gm_identity[3],
            eventState.gm_identity[4], eventState.gm_identity[5],
            eventState.gm_identity[6], eventState.gm_identity[7]);
        printf("| %-25s |     %-19ld ns |\n",
            "clock_offset", eventState.clock_offset);
        printf("| %-25s |     %-19ld ns |\n",
            "notification_timestamp", eventState.notification_timestamp);
        printf("+---------------------------+--------------+-------------+\n");
        if (composite_event) {
            printf("| %-25s | %-12d | %-11d |\n", "composite_event",
                   eventState.composite_event, eventCount.composite_event_count);
        }
        if (composite_event & eventGMOffset) {
            printf("| - %-23s | %-12s | %-11s |\n", "offset_in_range", "", "");
        }
        if (composite_event & eventSyncedToGM) {
            printf("| - %-19s | %-12s | %-11s |\n", "synced_to_primary_clock", "", "");
        }
        if (composite_event & eventASCapable) {
            printf("| - %-23s | %-12s | %-11s |\n", "as_capable", "", "");
        }
        if (composite_event) {
            printf("+---------------------------+--------------+-------------+\n\n");
        } else {
            printf("\n");
        }

        printf("[clkmgr][%.3f] sleep for %d seconds...\n\n",
            getMonotonicTime(), idleTime);
        sleep(idleTime);
    }

do_exit:
    cm->clkmgr_disconnect();

    return ret;
}
