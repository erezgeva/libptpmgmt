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

#define MAX_CLOCKS 8

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

bool isPositiveValue(const char *optarg, uint32_t *target,
    const char *errorMessage) {
    char *endptr = NULL;
    long value = strtol(optarg, &endptr, 10);

    if (endptr == optarg || *endptr != '\0') {
        fprintf(stderr, "Invalid argument: %s\n", optarg);
        return false;
    }
    if (value < 0) {
        fprintf(stderr, "%s\n", errorMessage);
        return false;
    }
    if (value > UINT32_MAX) {
        fprintf(stderr, "Out of range: %s\n", optarg);
        return false;
    }
    *target = (uint32_t)value;
    return true;
}

int main(int argc, char *argv[])
{
    Clkmgr_ClockSyncData *syncData = clkmgr_constructClockSyncDataInstance();
    Clkmgr_Subscription *clockSub[MAX_CLOCKS];
    uint32_t  ptp4lClockOffsetThreshold = 100000;
    uint32_t  chronyClockOffsetThreshold = 100000;
    bool subscribeAll = false;
    bool userInput = false;
    int ret = EXIT_SUCCESS;
    uint32_t idleTime = 1;
    uint32_t timeout = 10;
    char input[8];
    int index[8];
    int indexCount = 0;
    struct timespec ts;
    int retval;
    int option;

    uint32_t eventMask = (Clkmgr_eventGMOffset | Clkmgr_eventSyncedToGM |
        Clkmgr_eventASCapable | Clkmgr_eventGMChanged);
    uint32_t compositeEventMask = (Clkmgr_eventGMOffset |
        Clkmgr_eventSyncedToGM | Clkmgr_eventASCapable);

    while ((option = getopt(argc, argv, "aps:c:u:l:i:t:m:n:h")) != -1) {
        switch (option) {
        case 'a':
            subscribeAll = true;
            break;
        case 'p':
            userInput = true;
            break;
        case 's':
            eventMask = strtoul(optarg, NULL, 0);;
            break;
        case 'c':
            compositeEventMask = strtoul(optarg, NULL, 0);
            break;
        case 'l':
            if (!isPositiveValue(optarg, &ptp4lClockOffsetThreshold,
                "Invalid ptp4l GM Offset threshold!")) {
                return EXIT_FAILURE;
            }
            break;
        case 'i':
            idleTime = strtol(optarg, NULL, 10);
            break;
        case 't':
            timeout = strtol(optarg, NULL, 10);
            break;
        case 'm':
            if (!isPositiveValue(optarg, &chronyClockOffsetThreshold,
                "Invalid Chrony Offset threshold!")) {
                return EXIT_FAILURE;
            }
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
                   "  -l gm offset threshold (ns)\n"
                   "     Default: %d ns\n"
                   "  -i idle time (s)\n"
                   "     Default: %d s\n"
                   "  -m chrony offset threshold (ns)\n"
                   "     Default: %d ns\n"
                   "  -t timeout in waiting notification event (s)\n"
                   "     Default: %d s\n",
                   argv[0], eventMask,
                   compositeEventMask,
                   ptp4lClockOffsetThreshold, chronyClockOffsetThreshold,
                   idleTime, timeout);
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
                   "  -l gm offset threshold (ns)\n"
                   "     Default: %d ns\n"
                   "  -i idle time (s)\n"
                   "     Default: %d s\n"
                   "  -m chrony offset threshold (ns)\n"
                   "     Default: %d ns\n"
                   "  -t timeout in waiting notification event (s)\n"
                   "     Default: %d s\n",
                   argv[0], eventMask,
                   compositeEventMask,
                   ptp4lClockOffsetThreshold, chronyClockOffsetThreshold,
                   idleTime, timeout);
            return EXIT_FAILURE;
        }
    }

    if (clkmgr_connect() == false) {
        printf("[clkmgr] Failure in connecting !!!\n");
        ret = EXIT_FAILURE;
        goto do_exit;
    }

    sleep(1);

    size_t indexSize = clkmgr_getTimebaseCfgsSize();
    if(indexSize == 0) {
        printf("[clkmgr] No available clock found !!!\n");
        ret = EXIT_FAILURE;
        goto do_exit;
    }
    printf("[clkmgr] List of available clock: \n");
    for(size_t i = 1; i <= indexSize; i++) {
        if (clkmgr_isTimeBaseIndexPresent(i)) {
            clockSub[i] = clkmgr_constructSubscriptionInstance();
            printf("TimeBaseIndex: %zu\n", i);
            printf("timeBaseName: %s\n", clkmgr_timeBaseName(i));
            if(clkmgr_havePtp(i)) {
                printf("interfaceName: %s\n", clkmgr_ifName(i));
                printf("transportSpecific: %d\n", clkmgr_transportSpecific(i));
                printf("domainNumber: %d\n\n", clkmgr_domainNumber(i));
                clkmgr_setEventMask(clockSub[i], Clkmgr_ptpClock, eventMask);
                clkmgr_setPtpCompositeEventMask(clockSub[i], compositeEventMask);
                clkmgr_setClockOffsetThreshold(clockSub[i], Clkmgr_ptpClock,
                    ptp4lClockOffsetThreshold);
                clkmgr_enableSubscription(clockSub[i], Clkmgr_ptpClock);
            }
            if(clkmgr_haveSysClock(i)) {
                clkmgr_setEventMask(clockSub[i], Clkmgr_sysClock, eventMask);
                clkmgr_setClockOffsetThreshold(clockSub[i], Clkmgr_sysClock,
                    chronyClockOffsetThreshold);
                clkmgr_enableSubscription(clockSub[i], Clkmgr_sysClock);
            }
        } else {
            printf("Failed to get time base configuration for index %ld\n", i);
        }
    }

    if (subscribeAll) {
        for (size_t i = 1; i <= indexSize; i++) {
            if (clkmgr_isTimeBaseIndexPresent(i)) {
                index[indexCount++] = i;
            }
        }
    } else if (userInput) {
        printf("Enter the time base indices to subscribe (comma-separated, default is 1): ");
        fgets(input, sizeof(input), stdin);
        if (strlen(input) > 1) {
            char *token = strtok(input, ", ");
            while (token != NULL) {
                if (isdigit(*token)) {
                    index[indexCount++] = atoi(token);
                } else {
                    printf("Invalid time base index!\n");
                    return EXIT_FAILURE;
                }
                token = strtok(NULL, ", ");
            }
        } else {
            printf("Invalid input. Using default time base index 1.\n");
            index[indexCount++] = 1;
        }
    } else {
        index[indexCount++] = 1;
    }

    for (size_t i = 0; i < indexCount; i++) {
        /* Subscribe to default time base index 1 */
        printf("[clkmgr] Subscribe to time base index: %d\n", index[i]);
        if (clkmgr_subscribe(clockSub[i+1], index[i], syncData) == false) {
            printf("[clkmgr] Failure in subscribing to clkmgr Proxy !!!\n");
            clkmgr_disconnect();
            ret = EXIT_FAILURE;
            goto do_exit;
        }

        printf("[clkmgr][%.3f] Obtained data from Subscription Event:\n",
            getMonotonicTime());
        if (!clkmgr_getTime(&ts)) {
            perror("clock_c_gettime failed");
        } else {
            printf("[clkmgr] Current Time of CLOCK_REALTIME: %ld ns\n",
                (ts.tv_sec * 1000000000) + ts.tv_nsec);
        }

        printf("|---------------------------|------------------------|\n");
        printf("| %-25s | %-22s |\n", "Event", "Event Status");
        if (eventMask) {
            printf("|---------------------------|------------------------|\n");
        }
        if (eventMask & Clkmgr_eventGMOffset) {
            printf("| %-25s | %-22d |\n", "offset_in_range",
                clkmgr_isOffsetInRange(syncData, Clkmgr_ptpClock));
        }
        if (eventMask & Clkmgr_eventSyncedToGM) {
            printf("| %-25s | %-22d |\n", "synced_to_primary_clock",
                clkmgr_isPtpSyncedWithGm(syncData));
        }
        if (eventMask & Clkmgr_eventASCapable) {
            printf("| %-25s | %-22d |\n", "as_capable",
                clkmgr_isPtpAsCapable(syncData));
        }
        if (eventMask & Clkmgr_eventGMChanged) {
            printf("| %-25s | %-22d |\n", "gm_Changed",
                clkmgr_isGmChanged(syncData, Clkmgr_ptpClock));
        }
        printf("|---------------------------|------------------------|\n");
        uint64_t gmClockUUID = clkmgr_getGmIdentity(syncData, Clkmgr_ptpClock);
        uint8_t gmIdentity[8];
        for (int i = 0; i < 8; ++i) {
            gmIdentity[i] = (uint8_t)(gmClockUUID >> (8 * (7 - i)));
        }
        printf("| %-25s | %02x%02x%02x.%02x%02x.%02x%02x%02x     |\n",
            "GM UUID", gmIdentity[0], gmIdentity[1],
            gmIdentity[2], gmIdentity[3],
            gmIdentity[4], gmIdentity[5],
            gmIdentity[6], gmIdentity[7]);
        printf("| %-25s | %-19ld ns |\n",
            "clock_offset", clkmgr_getClockOffset(syncData, Clkmgr_ptpClock));
        printf("| %-25s | %-19ld ns |\n",
                "notification_timestamp",
                clkmgr_getNotificationTimestamp(syncData, Clkmgr_ptpClock));
        printf("| %-25s | %-19ld us |\n",
                "gm_sync_interval",
                clkmgr_getSyncInterval(syncData, Clkmgr_ptpClock));
        printf("|---------------------------|------------------------|\n");
        if (compositeEventMask) {
            printf("| %-25s | %-22d |\n", "composite_event",
                clkmgr_isPtpCompositeEventMet(syncData));
        }
        if (compositeEventMask & Clkmgr_eventGMOffset) {
            printf("| - %-23s | %-22s |\n", "offset_in_range", " ");
        }
        if (compositeEventMask & Clkmgr_eventSyncedToGM) {
            printf("| - %-19s | %-22s |\n", "synced_to_primary_clock", " ");
        }
        if (compositeEventMask & Clkmgr_eventASCapable) {
            printf("| - %-23s | %-22s |\n", "as_capable", " ");
        }
        if (compositeEventMask) {
            printf("|---------------------------|------------------------|\n\n");
        } else {
            printf("\n");
        }
        printf("|---------------------------|------------------------|\n");
        printf("| %-25s | %-22d |\n", "chrony_offset_in_range",
                clkmgr_isOffsetInRange(syncData, Clkmgr_sysClock));
        printf("|---------------------------|------------------------|\n");
        printf("| %-25s | %-19ld ns |\n",
            "chrony_clock_offset",
            clkmgr_getClockOffset(syncData, Clkmgr_sysClock));
        printf("| %-25s | %-19lX    |\n",
            "chrony_clock_reference_id",
            clkmgr_getGmIdentity(syncData, Clkmgr_sysClock));
        printf("| %-25s | %-19ld us |\n",
            "chrony_polling_interval",
            clkmgr_getSyncInterval(syncData, Clkmgr_sysClock));
        printf("|---------------------------|------------------------|\n\n");
    }
    sleep(1);

    while (1) {
        for (size_t i = 0; i < indexCount; i++) {
            printf("[clkmgr][%.3f] Waiting Notification from time base index %d ...\n",
                getMonotonicTime(), index[i]);
            retval = clkmgr_statusWait(timeout, index[i], syncData);
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
            if (!clkmgr_getTime(&ts)) {
                perror("clock_c_gettime failed");
            } else {
                printf("[clkmgr] Current Time of CLOCK_REALTIME: %ld ns\n",
                    (ts.tv_sec * 1000000000) + ts.tv_nsec);
            }
            printf("|---------------------------|--------------|-------------|\n");
            printf("| %-25s | %-12s | %-11s |\n", "Event", "Event Status",
                "Event Count");
            if (eventMask) {
            printf("|---------------------------|--------------|-------------|\n");
            }
            if (eventMask & Clkmgr_eventGMOffset) {
                printf("| %-25s | %-12d | %-11d |\n", "offset_in_range",
                    clkmgr_isOffsetInRange(syncData, Clkmgr_ptpClock),
                    clkmgr_getOffsetInRangeEventCount(syncData, Clkmgr_ptpClock));
            }
            if (eventMask & Clkmgr_eventSyncedToGM) {
                printf("| %-25s | %-12d | %-11d |\n", "synced_to_primary_clock",
                    clkmgr_isPtpSyncedWithGm(syncData),
                    clkmgr_getPtpSyncedWithGmEventCount(syncData));
            }
            if (eventMask & Clkmgr_eventASCapable) {
                printf("| %-25s | %-12d | %-11d |\n", "as_capable",
                    clkmgr_isPtpAsCapable(syncData),
                    clkmgr_getPtpAsCapableEventCount(syncData));
            }
            if (eventMask & Clkmgr_eventGMChanged) {
                printf("| %-25s | %-12d | %-11d |\n", "gm_Changed",
                    clkmgr_isGmChanged(syncData, Clkmgr_ptpClock),
                    clkmgr_getGmChangedEventCount(syncData, Clkmgr_ptpClock));
            }
            printf("|---------------------------|--------------|-------------|\n");
            uint64_t gmClockUUID = clkmgr_getGmIdentity(syncData, Clkmgr_ptpClock);
            uint8_t gmIdentity[8];
            for (int i = 0; i < 8; ++i) {
                gmIdentity[i] = (uint8_t)(gmClockUUID >> (8 * (7 - i)));
            }
            printf("| %-25s |     %02x%02x%02x.%02x%02x.%02x%02x%02x     |\n",
                "GM UUID", gmIdentity[0], gmIdentity[1],
                gmIdentity[2], gmIdentity[3],
                gmIdentity[4], gmIdentity[5],
                gmIdentity[6], gmIdentity[7]);
            printf("| %-25s |     %-19ld ns |\n",
                "clock_offset",
                clkmgr_getClockOffset(syncData, Clkmgr_ptpClock));
            printf("| %-25s |     %-19ld ns |\n",
                "notification_timestamp",
                clkmgr_getNotificationTimestamp(syncData, Clkmgr_ptpClock));
            printf("| %-25s |     %-19ld us |\n",
                "gm_sync_interval", clkmgr_getSyncInterval(syncData, Clkmgr_ptpClock));
            printf("|---------------------------|--------------|-------------|\n");
            if (compositeEventMask) {
                printf("| %-25s | %-12d | %-11d |\n", "composite_event",
                    clkmgr_isPtpCompositeEventMet(syncData),
                    clkmgr_getPtpCompositeEventCount(syncData));
            }
            if (compositeEventMask & Clkmgr_eventGMOffset) {
                printf("| - %-23s | %-12s | %-11s |\n", "offset_in_range", "", "");
            }
            if (compositeEventMask & Clkmgr_eventSyncedToGM) {
                printf("| - %-19s | %-12s | %-11s |\n", "synced_to_primary_clock", "", "");
            }
            if (compositeEventMask & Clkmgr_eventASCapable) {
                printf("| - %-23s | %-12s | %-11s |\n", "as_capable", "", "");
            }
            if (compositeEventMask) {
                printf("|---------------------------|--------------|-------------|\n\n");
            } else {
                printf("\n");
            }
            printf("|---------------------------|----------------------------|\n");
            printf("| %-25s | %-12d | %-11d |\n", "chrony_offset_in_range",
                clkmgr_isOffsetInRange(syncData, Clkmgr_sysClock),
                clkmgr_getOffsetInRangeEventCount(syncData, Clkmgr_sysClock));
            printf("|---------------------------|----------------------------|\n");
            printf("| %-25s |     %-19ld ns |\n",
                "chrony_clock_offset",
                clkmgr_getClockOffset(syncData, Clkmgr_sysClock));
            printf("| %-25s |     %-19lX    |\n",
                "chrony_clock_reference_id",
                clkmgr_getGmIdentity(syncData, Clkmgr_sysClock));
            printf("| %-25s |     %-19ld us |\n",
                "chrony_polling_interval",
                clkmgr_getSyncInterval(syncData, Clkmgr_sysClock));
            printf("|---------------------------|----------------------------|\n\n");

            printf("[clkmgr][%.3f] sleep for %d seconds...\n\n",
                getMonotonicTime(), idleTime);
            sleep(idleTime);
        }
    }

do_exit:
    for (size_t i = 1; i <= indexSize; i++) {
        if (clkmgr_isTimeBaseIndexPresent(i)) {
            clkmgr_destroySubscriptionInstance(clockSub[i]);
        }
    }
    clkmgr_destroyClockSyncDataInstance(syncData);
    clkmgr_disconnect();

    return ret;
}
