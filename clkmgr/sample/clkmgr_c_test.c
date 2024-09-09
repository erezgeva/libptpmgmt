/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file clkmgr_test.c
 * @brief Test clkmgr client c wrapper code
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 * @note This is a sample code, not a product! You should use it as a reference.
 *  You can compile it with: gcc -Wall clkmgr_c_test.c -o clkmgr_c_test -lclkmgr
 *  or use the Makefile with: make
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <clockmanager.h>

double getMonotonicTime() {
    struct timespec time_spec;

    if (clock_gettime(CLOCK_MONOTONIC, &time_spec) == -1) {
        printf("clock_gettime failed");
        return -1;
    }

    double seconds = time_spec.tv_sec;
    double nanoseconds = time_spec.tv_nsec / 1e9;

    return seconds + nanoseconds;
}

int main(int argc, char *argv[])
{
    struct clkmgr_c_event_count event_count = {};
    struct clkmgr_c_subscription subscription = {};
    struct clkmgr_c_event_state event_state = {};
    clkmgr_c_client_ptr client_ptr;
    int ret = EXIT_SUCCESS;
    uint32_t idle_time = 1;
    uint32_t timeout = 10;
    int retval;
    int option;

    subscription.event_mask = (clkmgr_c_event_gm_offset | clkmgr_c_event_synced2gm |
        clkmgr_c_event_as_capable | clkmgr_c_event_gm_changed);
    subscription.composite_event_mask = (clkmgr_c_event_gm_offset |
        clkmgr_c_event_synced2gm | clkmgr_c_event_as_capable);
    subscription.threshold[clkmgr_c_threshold_gm_offset].upper_limit = 100000;
    subscription.threshold[clkmgr_c_threshold_gm_offset].lower_limit = -100000;

    while ((option = getopt(argc, argv, "s:c:u:l:i:t:h")) != -1) {
        switch (option) {
        case 's':
            subscription.event_mask = strtoul(optarg, NULL, 0);;
            break;
        case 'c':
            subscription.composite_event_mask = strtoul(optarg, NULL, 0);
            break;
        case 'u':
            subscription.threshold[clkmgr_c_threshold_gm_offset].upper_limit =
                strtol(optarg, NULL, 10);
            break;
        case 'l':
            subscription.threshold[clkmgr_c_threshold_gm_offset].lower_limit =
                strtol(optarg, NULL, 10);
            break;
        case 'i':
            idle_time = strtol(optarg, NULL, 10);
            break;
        case 't':
            timeout = strtol(optarg, NULL, 10);
            break;
        case 'h':
            printf("Usage of %s :\n"
                   "Options:\n"
                   "  -s subscribe_event_mask\n"
                   "     Default: 0x%x\n"
                   "     Bit 0: eventGMOffset\n"
                   "     Bit 1: eventSyncedToGM\n"
                   "     Bit 2: eventASCapable\n"
                   "     Bit 3: eventGMChanged\n"
                   "  -c composite_event_mask\n"
                   "     Default: 0x%x\n"
                   "     Bit 0: eventGMOffset\n"
                   "     Bit 1: eventSyncedToGM\n"
                   "     Bit 2: eventASCapable\n"
                   "  -u gm offset upper limit (ns)\n"
                   "     Default: %d ns\n"
                   "  -l gm offset lower limit (ns)\n"
                   "     Default: %d ns\n"
                   "  -i idle time (s)\n"
                   "     Default: %d s\n"
                   "  -t timeout in waiting notification event (s)\n"
                   "     Default: %d s\n",
                   argv[0], subscription.event_mask,
                   subscription.composite_event_mask,
                   subscription.threshold[clkmgr_c_threshold_gm_offset].upper_limit,
                   subscription.threshold[clkmgr_c_threshold_gm_offset].lower_limit,
                   idle_time, timeout);
            return EXIT_SUCCESS;
        default:
            printf("Usage of %s :\n"
                   "Options:\n"
                   "  -s subscribe_event_mask\n"
                   "     Default: 0x%x\n"
                   "     Bit 0: eventGMOffset\n"
                   "     Bit 1: eventSyncedToGM\n"
                   "     Bit 2: eventASCapable\n"
                   "     Bit 3: eventGMChanged\n"
                   "  -c composite_event_mask\n"
                   "     Default: 0x%x\n"
                   "     Bit 0: eventGMOffset\n"
                   "     Bit 1: eventSyncedToGM\n"
                   "     Bit 2: eventASCapable\n"
                   "  -u gm offset upper limit (ns)\n"
                   "     Default: %d ns\n"
                   "  -l gm offset lower limit (ns)\n"
                   "     Default: %d ns\n"
                   "  -i idle time (s)\n"
                   "     Default: %d s\n"
                   "  -t timeout in waiting notification event (s)\n"
                   "     Default: %d s\n",
                   argv[0], subscription.event_mask,
                   subscription.composite_event_mask,
                   subscription.threshold[clkmgr_c_threshold_gm_offset].upper_limit,
                   subscription.threshold[clkmgr_c_threshold_gm_offset].lower_limit,
                   idle_time, timeout);
            return EXIT_FAILURE;
        }
    }

    client_ptr = clkmgr_c_client_create();
    if (client_ptr == NULL) {
        printf("[clkmgr] Failure in creating clkmgr Proxy !!!\n");
        return EXIT_FAILURE;
    }

    if (clkmgr_c_connect(client_ptr) == false) {
        printf("[clkmgr] Failure in connecting !!!\n");
        ret = EXIT_FAILURE;
        goto do_exit;
    }

    sleep(1);

    if (clkmgr_c_subscribe(client_ptr, subscription, &event_state) == false) {
        printf("[clkmgr] Failure in subscribing to clkmgr Proxy !!!\n");
        ret = EXIT_FAILURE;
        goto do_exit;
    }

    printf("[clkmgr][%.3f] Obtained data from Subscription Event:\n",
        getMonotonicTime());
    printf("+---------------------------+--------------------+\n");
    printf("| %-25s | %-18s |\n", "Event", "Event Status");
    if (subscription.event_mask) {
        printf("+---------------------------+--------------------+\n");
    }
    if (subscription.event_mask & clkmgr_c_event_gm_offset) {
        printf("| %-25s | %-18d |\n", "offset_in_range",
            event_state.offset_in_range);
    }
    if (subscription.event_mask & clkmgr_c_event_synced2gm) {
        printf("| %-25s | %-18d |\n", "synced_to_primary_clock", event_state.synced_to_primary_clock);
    }
    if (subscription.event_mask & clkmgr_c_event_as_capable) {
        printf("| %-25s | %-18d |\n", "as_capable", event_state.as_capable);
    }
    if (subscription.event_mask & clkmgr_c_event_gm_changed) {
        printf("| %-25s | %-18d |\n", "gm_Changed", event_state.gm_changed);
    }
    printf("+---------------------------+--------------------+\n");
    printf("| %-25s | %02x%02x%02x.%02x%02x.%02x%02x%02x |\n", "UUID",
        event_state.gm_identity[0], event_state.gm_identity[1],
        event_state.gm_identity[2], event_state.gm_identity[3],
        event_state.gm_identity[4], event_state.gm_identity[5],
        event_state.gm_identity[6], event_state.gm_identity[7]);
    printf("| %-25s | %-15ld ns |\n",
            "clock_offset", event_state.clock_offset);
    printf("| %-25s | %-16.3f s |\n",
            "notification_timestamp", event_state.notification_timestamp / 1e9);
    printf("+---------------------------+--------------------+\n");
    if (subscription.composite_event_mask) {
        printf("| %-25s | %-18d |\n", "composite_event",
            event_state.composite_event);
    }
    if (subscription.composite_event_mask & clkmgr_c_event_gm_offset) {
        printf("| - %-23s | %-18s |\n", "offset_in_range", " ");
    }
    if (subscription.composite_event_mask & clkmgr_c_event_synced2gm) {
        printf("| - %-19s | %-18s |\n", "synced_to_primary_clock", " ");
    }
    if (subscription.composite_event_mask & clkmgr_c_event_as_capable) {
        printf("| - %-23s | %-18s |\n", "as_capable", " ");
    }
    if (subscription.composite_event_mask) {
        printf("+---------------------------+--------------------+\n\n");
    } else {
        printf("\n");
    }

    sleep(1);

    while (1) {
        printf("[clkmgr][%.3f] Waiting for Notification Event...\n",
            getMonotonicTime());
        retval = clkmgr_c_status_wait(client_ptr, timeout, &event_state , &event_count);
        if (!retval) {
            printf("[clkmgr][%.3f] No event status changes identified in %d seconds.\n\n",
                getMonotonicTime(), timeout);
            printf("[clkmgr][%.3f] sleep for %d seconds...\n\n",
                getMonotonicTime(), idle_time);
            sleep(idle_time);
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
        if (subscription.event_mask) {
        printf("+---------------------------+--------------+-------------+\n");
        }
        if (subscription.event_mask & clkmgr_c_event_gm_offset) {
            printf("| %-25s | %-12d | %-11d |\n", "offset_in_range",
                event_state.offset_in_range,
                event_count.offset_in_range_event_count);
        }
        if (subscription.event_mask & clkmgr_c_event_synced2gm) {
            printf("| %-25s | %-12d | %-11d |\n", "synced_to_primary_clock",
               event_state.synced_to_primary_clock, event_count.synced_to_gm_event_count);
        }
        if (subscription.event_mask & clkmgr_c_event_as_capable) {
            printf("| %-25s | %-12d | %-11d |\n", "as_capable",
                event_state.as_capable, event_count.as_capable_event_count);
        }
        if (subscription.event_mask & clkmgr_c_event_gm_changed) {
            printf("| %-25s | %-12d | %-11d |\n", "gm_Changed",
                event_state.gm_changed, event_count.gm_changed_event_count);
        }
        printf("+---------------------------+--------------+-------------+\n");
        printf("| %-25s |     %02x%02x%02x.%02x%02x.%02x%02x%02x     |\n",
            "UUID", event_state.gm_identity[0], event_state.gm_identity[1],
            event_state.gm_identity[2], event_state.gm_identity[3],
            event_state.gm_identity[4], event_state.gm_identity[5],
            event_state.gm_identity[6], event_state.gm_identity[7]);
        printf("| %-25s |     %-19ld ns |\n",
            "clock_offset", event_state.clock_offset);
        printf("| %-25s |     %-20.3f s |\n",
            "notification_timestamp", event_state.notification_timestamp / 1e9);
        printf("+---------------------------+--------------+-------------+\n");
        if (subscription.composite_event_mask) {
            printf("| %-25s | %-12d | %-11d |\n", "composite_event",
                   event_state.composite_event, event_count.composite_event_count);
        }
        if (subscription.composite_event_mask & clkmgr_c_event_gm_offset) {
            printf("| - %-23s | %-12s | %-11s |\n", "offset_in_range", "", "");
        }
        if (subscription.composite_event_mask & clkmgr_c_event_synced2gm) {
            printf("| - %-19s | %-12s | %-11s |\n", "synced_to_primary_clock", "", "");
        }
        if (subscription.composite_event_mask & clkmgr_c_event_as_capable) {
            printf("| - %-23s | %-12s | %-11s |\n", "as_capable", "", "");
        }
        if (subscription.composite_event_mask) {
            printf("+---------------------------+--------------+-------------+\n\n");
        } else {
            printf("\n");
        }

        printf("[clkmgr][%.3f] sleep for %d seconds...\n\n",
            getMonotonicTime(), idle_time);
        sleep(idle_time);
    }

do_exit:
    clkmgr_c_client_destroy(client_ptr);

    return ret;
}