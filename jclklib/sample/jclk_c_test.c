/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file jclk_test.c
 * @brief Test jclk client c wrapper code
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 * @note This is a sample code, not a product! You should use it as a reference.
 *  You can compile it with: gcc -Wall jclk_test.c -o jclk_test -ljclk
 *  or use the Makefile file.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "c/jclklib_client_api_c.h"

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
    struct jcl_c_event_count event_count = {};
    struct jcl_c_subscription subscription = {};
    struct jcl_c_state state = {};
    jcl_c_client_ptr client_ptr;
    int ret = EXIT_SUCCESS;
    int idle_time = 1;
    int timeout = 10;
    int retval;
    int opt;

    subscription.event[0] = ((1<<gmOffsetEvent) | (1<<syncedToPrimaryClockEvent) |
        (1<<asCapableEvent) | (1<<gmChangedEvent));
    subscription.composite_event[0] = ((1<<gmOffsetEvent) |
        (1<<syncedToPrimaryClockEvent) | (1<<asCapableEvent));
    subscription.value[gm_offset].upper = 100000;
    subscription.value[gm_offset].lower = -100000;

    while ((opt = getopt(argc, argv, "s:c:u:l:i:t:h")) != -1) {
        switch (opt) {
        case 's':
            subscription.event[0] = strtoul(optarg, NULL, 0);;
            break;
        case 'c':
            subscription.composite_event[0] = strtoul(optarg, NULL, 0);
            break;
        case 'u':
            subscription.value[gm_offset].upper = strtol(optarg, NULL, 10);
            break;
        case 'l':
            subscription.value[gm_offset].lower = strtol(optarg, NULL, 10);
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
                   "     Bit 0: gmOffsetEvent\n"
                   "     Bit 1: syncedToPrimaryClockEvent\n"
                   "     Bit 2: asCapableEvent\n"
                   "     Bit 3: gmChangedEvent\n"
                   "  -c composite_event_mask\n"
                   "     Default: 0x%x\n"
                   "     Bit 0: gmOffsetEvent\n"
                   "     Bit 1: syncedToPrimaryClockEvent\n"
                   "     Bit 2: asCapableEvent\n"
                   "  -u upper master offset (ns)\n"
                   "     Default: %d ns\n"
                   "  -l lower master offset (ns)\n"
                   "     Default: %d ns\n"
                   "  -i idle time (s)\n"
                   "     Default: %d s\n"
                   "  -t timeout in waiting notification event (s)\n"
                   "     Default: %d s\n",
                   argv[0], subscription.event[0],
                   subscription.composite_event[0],
                   subscription.value[gm_offset].upper,
                   subscription.value[gm_offset].lower, idle_time, timeout);
            return EXIT_SUCCESS;
        default:
            printf("Usage of %s :\n"
                   "Options:\n"
                   "  -s subscribe_event_mask\n"
                   "     Default: 0x%x\n"
                   "     Bit 0: gmOffsetEvent\n"
                   "     Bit 1: syncedToPrimaryClockEvent\n"
                   "     Bit 2: asCapableEvent\n"
                   "     Bit 3: gmChangedEvent\n"
                   "  -c composite_event_mask\n"
                   "     Default: 0x%x\n"
                   "     Bit 0: gmOffsetEvent\n"
                   "     Bit 1: syncedToPrimaryClockEvent\n"
                   "     Bit 2: asCapableEvent\n"
                   "  -u upper master offset (ns)\n"
                   "     Default: %d ns\n"
                   "  -l lower master offset (ns)\n"
                   "     Default: %d ns\n"
                   "  -i idle time (s)\n"
                   "     Default: %d s\n"
                   "  -t timeout in waiting notification event (s)\n"
                   "     Default: %d s\n",
                   argv[0], subscription.event[0],
                   subscription.composite_event[0],
                   subscription.value[gm_offset].upper,
                   subscription.value[gm_offset].lower, idle_time, timeout);
            return EXIT_FAILURE;
        }
    }

    client_ptr = jcl_c_client_create();
    if (client_ptr == NULL) {
        printf("[jclklib] Failure in creating jclklib Proxy !!!\n");
        return EXIT_FAILURE;
    }

    if (jcl_c_connect(client_ptr) == false) {
        printf("[jclklib] Failure in connecting !!!\n");
        ret = EXIT_FAILURE;
        goto do_exit;
    }

    sleep(1);

    if (jcl_c_subscribe(client_ptr, subscription, &state) == false) {
        printf("[jclklib] Failure in subscribing to jclklib Proxy !!!\n");
        ret = EXIT_FAILURE;
        goto do_exit;
    }

    printf("[jclklib][%.3f] Obtained data from Subscription Event:\n",
        getMonotonicTime());
    printf("+---------------------------+--------------------+\n");
    printf("| %-25s | %-18s |\n", "Event", "Event Status");
    if (subscription.event[0]) {
        printf("+---------------------------+--------------------+\n");
    }
    if (subscription.event[0] & (1<<gmOffsetEvent)) {
        printf("| %-25s | %-18d |\n", "offset_in_range",
            state.offset_in_range);
    }
    if (subscription.event[0] & (1<<syncedToPrimaryClockEvent)) {
        printf("| %-25s | %-18d |\n", "synced_to_primary_clock", state.synced_to_primary_clock);
    }
    if (subscription.event[0] & (1<<asCapableEvent)) {
        printf("| %-25s | %-18d |\n", "as_capable", state.as_capable);
    }
    if (subscription.event[0] & (1<<gmChangedEvent)) {
        printf("| %-25s | %-18d |\n", "gm_Changed", state.gm_changed);
        printf("+---------------------------+--------------------+\n");
        printf("| %-25s | %02x%02x%02x.%02x%02x.%02x%02x%02x |\n", "UUID",
            state.gm_identity[0], state.gm_identity[1],
            state.gm_identity[2], state.gm_identity[3],
            state.gm_identity[4], state.gm_identity[5],
            state.gm_identity[6], state.gm_identity[7]);
        printf("| %-25s | %-15ld ns |\n",
                "clock_offset", state.clock_offset);
        printf("| %-25s | %-16f s |\n",
                "notification_timestamp", state.notification_timestamp);
    }
    printf("+---------------------------+--------------------+\n");
    if (subscription.composite_event[0]) {
        printf("| %-25s | %-18d |\n", "composite_event",
            state.composite_event);
    }
    if (subscription.composite_event[0] & (1<<gmOffsetEvent)) {
        printf("| - %-23s | %-18s |\n", "offset_in_range", " ");
    }
    if (subscription.composite_event[0] & (1<<syncedToPrimaryClockEvent)) {
        printf("| - %-19s | %-18s |\n", "synced_to_primary_clock", " ");
    }
    if (subscription.composite_event[0] & (1<<asCapableEvent)) {
        printf("| - %-23s | %-18s |\n", "as_capable", " ");
    }
    if (subscription.composite_event[0]) {
        printf("+---------------------------+--------------------+\n\n");
    } else {
        printf("\n");
    }

    sleep(1);

    while (1) {
        printf("[jclklib][%.3f] Waiting for Notification Event...\n",
            getMonotonicTime());
        retval = jcl_c_status_wait(client_ptr, timeout, &state , &event_count);
        if (!retval) {
            printf("[jclklib][%.3f] No event status changes identified in %d seconds.\n\n",
                getMonotonicTime(), timeout);
            printf("[jclklib][%.3f] sleep for %d seconds...\n\n",
                getMonotonicTime(), idle_time);
            sleep(idle_time);
            continue;
        } else if (retval < 0) {
            printf("[jclklib][%.3f] Terminating: lost connection to jclklib Proxy\n",
                getMonotonicTime());
            return EXIT_SUCCESS;
        }

        printf("[jclklib][%.3f] Obtained data from Notification Event:\n",
            getMonotonicTime());
        printf("+---------------------------+--------------+-------------+\n");
        printf("| %-25s | %-12s | %-11s |\n", "Event", "Event Status",
            "Event Count");
        if (subscription.event[0]) {
        printf("+---------------------------+--------------+-------------+\n");
        }
        if (subscription.event[0] & (1<<gmOffsetEvent)) {
            printf("| %-25s | %-12d | %-11ld |\n", "offset_in_range",
                state.offset_in_range,
                event_count.offset_in_range_event_count);
        }
        if (subscription.event[0] & (1<<syncedToPrimaryClockEvent)) {
            printf("| %-25s | %-12d | %-11ld |\n", "synced_to_primary_clock",
               state.synced_to_primary_clock, event_count.synced_to_primary_clock_event_count);
        }
        if (subscription.event[0] & (1<<asCapableEvent)) {
            printf("| %-25s | %-12d | %-11ld |\n", "as_capable",
                state.as_capable, event_count.as_capable_event_count);
        }
        if (subscription.event[0] & (1<<gmChangedEvent)) {
            printf("| %-25s | %-12d | %-11ld |\n", "gm_Changed",
                state.gm_changed, event_count.gm_changed_event_count);
            printf("+---------------------------+--------------+-------------+\n");
            printf("| %-25s |     %02x%02x%02x.%02x%02x.%02x%02x%02x     |\n",
                "UUID", state.gm_identity[0], state.gm_identity[1],
                state.gm_identity[2], state.gm_identity[3],
                state.gm_identity[4], state.gm_identity[5],
                state.gm_identity[6], state.gm_identity[7]);
            printf("| %-25s |     %-19ld ns |\n",
                "clock_offset", state.clock_offset);
            printf("| %-25s |     %-20f s |\n",
                "notification_timestamp", state.notification_timestamp);
        }
        printf("+---------------------------+--------------+-------------+\n");
        if (subscription.composite_event[0]) {
            printf("| %-25s | %-12d | %-11ld |\n", "composite_event",
                   state.composite_event, event_count.composite_event_count);
        }
        if (subscription.composite_event[0] & (1<<gmOffsetEvent)) {
            printf("| - %-23s | %-12s | %-11s |\n", "offset_in_range", "", "");
        }
        if (subscription.composite_event[0] & (1<<syncedToPrimaryClockEvent)) {
            printf("| - %-19s | %-12s | %-11s |\n", "synced_to_primary_clock", "", "");
        }
        if (subscription.composite_event[0] & (1<<asCapableEvent)) {
            printf("| - %-23s | %-12s | %-11s |\n", "as_capable", "", "");
        }
        if (subscription.composite_event[0]) {
            printf("+---------------------------+--------------+-------------+\n\n");
        } else {
            printf("\n");
        }

        printf("[jclklib][%.3f] sleep for %d seconds...\n\n",
            getMonotonicTime(), idle_time);
        sleep(idle_time);
    }

do_exit:
    jcl_c_client_destroy(client_ptr);

    return ret;
}
