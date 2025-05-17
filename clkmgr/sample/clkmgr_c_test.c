/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Test clkmgr client c wrapper code
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 * @note This is a sample code, not a product! You should use it as a reference.
 *  You can compile it with: gcc -Wall clkmgr_c_test.c -o clkmgr_c_test -lclkmgr
 *  or use the Makefile with: make
 *
 */

#include <ctype.h>
#include <string.h>
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
    struct Clkmgr_Event_count event_count = {};
    struct clkmgr_c_subscription subscription = {};
    struct Clkmgr_Event_state event_state = {};
    bool subscribeAll = false;
    bool userInput = false;
    int ret = EXIT_SUCCESS;
    uint32_t idle_time = 1;
    uint32_t timeout = 10;
    char input[8];
    int index[8];
    int index_count = 0;
    struct timespec ts;
    int retval;
    int option;

    subscription.event_mask = (Clkmgr_eventGMOffset | Clkmgr_eventSyncedToGM |
        Clkmgr_eventASCapable | Clkmgr_eventGMChanged);
    subscription.composite_event_mask = (Clkmgr_eventGMOffset |
        Clkmgr_eventSyncedToGM | Clkmgr_eventASCapable);
    subscription.threshold[Clkmgr_thresholdGMOffset].upper_limit = 100000;
    subscription.threshold[Clkmgr_thresholdGMOffset].lower_limit = -100000;
    subscription.threshold[Clkmgr_thresholdChronyOffset].upper_limit = 100000;
    subscription.threshold[Clkmgr_thresholdChronyOffset].lower_limit = -100000;

    while ((option = getopt(argc, argv, "aps:c:u:l:i:t:m:n:h")) != -1) {
        switch (option) {
        case 'a':
            subscribeAll = true;
            break;
        case 'p':
            userInput = true;
            break;
        case 's':
            subscription.event_mask = strtoul(optarg, NULL, 0);;
            break;
        case 'c':
            subscription.composite_event_mask = strtoul(optarg, NULL, 0);
            break;
        case 'u':
            subscription.threshold[Clkmgr_thresholdGMOffset].upper_limit =
                strtol(optarg, NULL, 10);
            break;
        case 'l':
            subscription.threshold[Clkmgr_thresholdGMOffset].lower_limit =
                strtol(optarg, NULL, 10);
            break;
        case 'i':
            idle_time = strtol(optarg, NULL, 10);
            break;
        case 't':
            timeout = strtol(optarg, NULL, 10);
            break;
        case 'm':
            subscription.threshold[Clkmgr_thresholdChronyOffset].upper_limit =
                strtol(optarg, NULL, 10);
            break;
        case 'n':
            subscription.threshold[Clkmgr_thresholdChronyOffset].lower_limit =
                strtol(optarg, NULL, 10);
            break;
        case 'h':
            printf("Usage of %s :\n"
                   "Options:\n"
                   "  -a subscribe to all time base indices\n"
                   "     Default: timeBaseIndex: 1\n"
                   "  -p enable user to subscribe to specific time base indices\n"
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
                   "  -m chrony offset upper limit (ns)\n"
                   "     Default: %d ns\n"
                   "  -n chrony offset lower limit (ns)\n"
                   "     Default: %d ns\n"
                   "  -t timeout in waiting notification event (s)\n"
                   "     Default: %d s\n",
                   argv[0], subscription.event_mask,
                   subscription.composite_event_mask,
                   subscription.threshold[Clkmgr_thresholdGMOffset].upper_limit,
                   subscription.threshold[Clkmgr_thresholdGMOffset].lower_limit,
                   subscription.threshold[Clkmgr_thresholdChronyOffset].upper_limit,
                   subscription.threshold[Clkmgr_thresholdChronyOffset].lower_limit,
                   idle_time, timeout);
            return EXIT_SUCCESS;
        default:
            printf("Usage of %s :\n"
                   "Options:\n"
                   "  -a subscribe to all time base indices\n"
                   "     Default: timeBaseIndex: 1\n"
                   "  -p enable user to subscribe to specific time base indices\n"
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
                   "  -m chrony offset upper limit (ns)\n"
                   "     Default: %d ns\n"
                   "  -n chrony offset lower limit (ns)\n"
                   "     Default: %d ns\n"
                   "  -t timeout in waiting notification event (s)\n"
                   "     Default: %d s\n",
                   argv[0], subscription.event_mask,
                   subscription.composite_event_mask,
                   subscription.threshold[Clkmgr_thresholdGMOffset].upper_limit,
                   subscription.threshold[Clkmgr_thresholdGMOffset].lower_limit,
                   subscription.threshold[Clkmgr_thresholdChronyOffset].upper_limit,
                   subscription.threshold[Clkmgr_thresholdChronyOffset].lower_limit,
                   idle_time, timeout);
            return EXIT_FAILURE;
        }
    }

    if (clkmgr_connect() == false) {
        printf("[clkmgr] Failure in connecting !!!\n");
        ret = EXIT_FAILURE;
        goto do_exit;
    }

    sleep(1);

    size_t index_size = clkmgr_get_timebase_cfgs_size();
    if(index_size == 0) {
        printf("[clkmgr] No available clock found !!!\n");
        ret = EXIT_FAILURE;
        goto do_exit;
    }
    printf("[clkmgr] List of available clock: \n");
    for(size_t i = 1; i <= index_size; i++) {
        if (clkmgr_isTimeBaseIndexPresent(i)) {
            printf("TimeBaseIndex: %zu\n", i);
            printf("timeBaseName: %s\n", clkmgr_timeBaseName(i));
            if(clkmgr_havePtp(i)) {
                printf("interfaceName: %s\n", clkmgr_ifName(i));
                printf("transportSpecific: %d\n", clkmgr_transportSpecific(i));
                printf("domainNumber: %d\n\n", clkmgr_domainNumber(i));
            }
        } else {
            printf("Failed to get time base configuration for index %ld\n", i);
        }
    }

    if (subscribeAll) {
        for (size_t i = 1; i <= index_size; i++) {
            if (clkmgr_isTimeBaseIndexPresent(i)) {
                index[index_count++] = i;
            }
        }
    } else if (userInput) {
        printf("Enter the time base indices to subscribe (comma-separated, default is 1): ");
        fgets(input, sizeof(input), stdin);
        if (strlen(input) > 1) {
            char *token = strtok(input, ", ");
            while (token != NULL) {
                if (isdigit(*token)) {
                    index[index_count++] = atoi(token);
                } else {
                    printf("Invalid time base index!\n");
                    return EXIT_FAILURE;
                }
                token = strtok(NULL, ", ");
            }
        } else {
            printf("Invalid input. Using default time base index 1.\n");
            index[index_count++] = 1;
        }
    } else {
        index[index_count++] = 1;
    }

    for (size_t i = 0; i < index_count; i++) {
        /* Subscribe to default time base index 1 */
        printf("[clkmgr] Subscribe to time base index: %d\n", index[i]);
        if (clkmgr_subscribe(subscription, index[i], &event_state) == false) {
            printf("[clkmgr] Failure in subscribing to clkmgr Proxy !!!\n");
            ret = EXIT_FAILURE;
            goto do_exit;
        }

        printf("[clkmgr][%.3f] Obtained data from Subscription Event:\n",
            getMonotonicTime());
        if (!clkmgr_gettime(&ts)) {
            perror("clock_c_gettime failed");
        } else {
            printf("[clkmgr] Current Time of CLOCK_REALTIME: %ld ns\n",
                (ts.tv_sec * 1000000000) + ts.tv_nsec);
        }
        printf("|---------------------------|------------------------|\n");
        printf("| %-25s | %-22s |\n", "Event", "Event Status");
        if (subscription.event_mask) {
            printf("|---------------------------|------------------------|\n");
        }
        if (subscription.event_mask & Clkmgr_eventGMOffset) {
            printf("| %-25s | %-22d |\n", "offset_in_range",
                event_state.offset_in_range);
        }
        if (subscription.event_mask & Clkmgr_eventSyncedToGM) {
            printf("| %-25s | %-22d |\n", "synced_to_primary_clock", event_state.synced_to_primary_clock);
        }
        if (subscription.event_mask & Clkmgr_eventASCapable) {
            printf("| %-25s | %-22d |\n", "as_capable", event_state.as_capable);
        }
        if (subscription.event_mask & Clkmgr_eventGMChanged) {
            printf("| %-25s | %-22d |\n", "gm_Changed", event_state.gm_changed);
        }
        printf("|---------------------------|------------------------|\n");
        printf("| %-25s | %02x%02x%02x.%02x%02x.%02x%02x%02x     |\n", "GM UUID",
            event_state.gm_identity[0], event_state.gm_identity[1],
            event_state.gm_identity[2], event_state.gm_identity[3],
            event_state.gm_identity[4], event_state.gm_identity[5],
            event_state.gm_identity[6], event_state.gm_identity[7]);
        printf("| %-25s | %-19ld ns |\n",
                "clock_offset", event_state.clock_offset);
        printf("| %-25s | %-19ld ns |\n",
                "notification_timestamp", event_state.notification_timestamp);
        printf("| %-25s | %-19ld us |\n",
                "gm_sync_interval", event_state.ptp4l_sync_interval);
        printf("|---------------------------|------------------------|\n");
        if (subscription.composite_event_mask) {
            printf("| %-25s | %-22d |\n", "composite_event",
                event_state.composite_event);
        }
        if (subscription.composite_event_mask & Clkmgr_eventGMOffset) {
            printf("| - %-23s | %-22s |\n", "offset_in_range", " ");
        }
        if (subscription.composite_event_mask & Clkmgr_eventSyncedToGM) {
            printf("| - %-19s | %-22s |\n", "synced_to_primary_clock", " ");
        }
        if (subscription.composite_event_mask & Clkmgr_eventASCapable) {
            printf("| - %-23s | %-22s |\n", "as_capable", " ");
        }
        if (subscription.composite_event_mask) {
            printf("|---------------------------|------------------------|\n\n");
        } else {
            printf("\n");
        }
        printf("|---------------------------|------------------------|\n");
        printf("| %-25s | %-22d |\n", "chrony_offset_in_range",
                event_state.chrony_offset_in_range);
        printf("|---------------------------|------------------------|\n");
        printf("| %-25s | %-19ld ns |\n",
            "chrony_clock_offset", event_state.chrony_clock_offset);
        printf("| %-25s | %-19X    |\n",
            "chrony_clock_reference_id", event_state.chrony_reference_id);
        printf("| %-25s | %-19d us |\n",
            "chrony_polling_interval", event_state.polling_interval);
        printf("|---------------------------|------------------------|\n\n");
    }
    sleep(1);

    while (1) {
        for (size_t i = 0; i < index_count; i++) {
            printf("[clkmgr][%.3f] Waiting Notification from time base index %d ...\n",
                getMonotonicTime(), index[i]);
            retval = clkmgr_status_wait(timeout, index[i], &event_state , &event_count);
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
            if (!clkmgr_gettime(&ts)) {
                perror("clock_c_gettime failed");
            } else {
                printf("[clkmgr] Current Time of CLOCK_REALTIME: %ld ns\n",
                    (ts.tv_sec * 1000000000) + ts.tv_nsec);
            }
            printf("|---------------------------|--------------|-------------|\n");
            printf("| %-25s | %-12s | %-11s |\n", "Event", "Event Status",
                "Event Count");
            if (subscription.event_mask) {
            printf("|---------------------------|--------------|-------------|\n");
            }
            if (subscription.event_mask & Clkmgr_eventGMOffset) {
                printf("| %-25s | %-12d | %-11d |\n", "offset_in_range",
                    event_state.offset_in_range,
                    event_count.offset_in_range_event_count);
            }
            if (subscription.event_mask & Clkmgr_eventSyncedToGM) {
                printf("| %-25s | %-12d | %-11d |\n", "synced_to_primary_clock",
                event_state.synced_to_primary_clock, event_count.synced_to_gm_event_count);
            }
            if (subscription.event_mask & Clkmgr_eventASCapable) {
                printf("| %-25s | %-12d | %-11d |\n", "as_capable",
                    event_state.as_capable, event_count.as_capable_event_count);
            }
            if (subscription.event_mask & Clkmgr_eventGMChanged) {
                printf("| %-25s | %-12d | %-11d |\n", "gm_Changed",
                    event_state.gm_changed, event_count.gm_changed_event_count);
            }
            printf("|---------------------------|--------------|-------------|\n");
            printf("| %-25s |     %02x%02x%02x.%02x%02x.%02x%02x%02x     |\n",
                "GM UUID", event_state.gm_identity[0], event_state.gm_identity[1],
                event_state.gm_identity[2], event_state.gm_identity[3],
                event_state.gm_identity[4], event_state.gm_identity[5],
                event_state.gm_identity[6], event_state.gm_identity[7]);
            printf("| %-25s |     %-19ld ns |\n",
                "clock_offset", event_state.clock_offset);
            printf("| %-25s |     %-19ld ns |\n",
                "notification_timestamp", event_state.notification_timestamp);
            printf("| %-25s |     %-19ld us |\n",
                "gm_sync_interval", event_state.ptp4l_sync_interval);
            printf("|---------------------------|--------------|-------------|\n");
            if (subscription.composite_event_mask) {
                printf("| %-25s | %-12d | %-11d |\n", "composite_event",
                    event_state.composite_event, event_count.composite_event_count);
            }
            if (subscription.composite_event_mask & Clkmgr_eventGMOffset) {
                printf("| - %-23s | %-12s | %-11s |\n", "offset_in_range", "", "");
            }
            if (subscription.composite_event_mask & Clkmgr_eventSyncedToGM) {
                printf("| - %-19s | %-12s | %-11s |\n", "synced_to_primary_clock", "", "");
            }
            if (subscription.composite_event_mask & Clkmgr_eventASCapable) {
                printf("| - %-23s | %-12s | %-11s |\n", "as_capable", "", "");
            }
            if (subscription.composite_event_mask) {
                printf("|---------------------------|--------------|-------------|\n\n");
            } else {
                printf("\n");
            }
            printf("|---------------------------|----------------------------|\n");
            printf("| %-25s | %-12d | %-11d |\n", "chrony_offset_in_range",
                event_state.chrony_offset_in_range, event_count.chrony_offset_in_range_event_count);
            printf("|---------------------------|----------------------------|\n");
            printf("| %-25s |     %-19ld ns |\n",
                "chrony_clock_offset", event_state.chrony_clock_offset);
            printf("| %-25s |     %-19X    |\n",
                "chrony_clock_reference_id", event_state.chrony_reference_id);
            printf("| %-25s |     %-19d us |\n",
                "chrony_polling_interval", event_state.polling_interval);
            printf("|---------------------------|----------------------------|\n\n");

            printf("[clkmgr][%.3f] sleep for %d seconds...\n\n",
                getMonotonicTime(), idle_time);
            sleep(idle_time);
        }
    }

do_exit:
    clkmgr_disconnect();

    return ret;
}
