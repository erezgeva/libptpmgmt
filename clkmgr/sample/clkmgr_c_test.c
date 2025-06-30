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
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <clockmanager.h>

volatile sig_atomic_t signal_flag = 0;

void signal_handler(int sig)
{
    printf(" Exit ...\n");
    signal_flag = 1;
}

#define MAX_CLOCKS 8


struct timespec ts;

uint32_t eventMask = (Clkmgr_EventGMOffset | Clkmgr_EventSyncedToGM |
    Clkmgr_EventASCapable | Clkmgr_EventGMChanged);
uint32_t compositeEventMask = (Clkmgr_EventGMOffset |
    Clkmgr_EventSyncedToGM | Clkmgr_EventASCapable);
uint32_t chronyEventMask = Clkmgr_EventGMOffset;

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

void printOut(Clkmgr_ClockSyncData *syncData, bool clkmgr_haveSysClock)
{
    if (!clkmgr_getTime(&ts)) {
        perror("clock_c_gettime failed");
    } else {
        printf("[clkmgr] Current Time of CLOCK_REALTIME: %ld ns\n",
            (ts.tv_sec * 1000000000) + ts.tv_nsec);
    }
    printf("|------------------------------|--------------|-------------|\n");
    printf("| %-28s | %-12s | %-11s |\n", "Events", "Event Status",
        "Event Count");
    printf("|------------------------------|--------------|-------------|\n");
    if (compositeEventMask) {
        printf("| %-28s | %-12d | %-11d |\n", "ptp_isCompositeEventMet",
            clkmgr_isPtpCompositeEventMet(syncData),
            clkmgr_getPtpCompositeEventCount(syncData));
    }
    if (compositeEventMask & Clkmgr_EventGMOffset) {
        printf("| - %-26s | %-12s | %-11s |\n", "isOffsetInRange", "", "");
    }
    if (compositeEventMask & Clkmgr_EventSyncedToGM) {
        printf("| - %-26s | %-12s | %-11s |\n", "isSyncedWithGm", "", "");
    }
    if (compositeEventMask & Clkmgr_EventASCapable) {
        printf("| - %-26s | %-12s | %-11s |\n", "isAsCapable", "", "");
    }
    if (eventMask) {
    printf("|------------------------------|--------------|-------------|\n");
    }
    if (eventMask & Clkmgr_EventGMOffset) {
        printf("| %-28s | %-12d | %-11d |\n", "ptp_isOffsetInRange",
            clkmgr_isOffsetInRange(syncData, Clkmgr_PTPClock),
            clkmgr_getOffsetInRangeEventCount(syncData, Clkmgr_PTPClock));
    }
    if (eventMask & Clkmgr_EventSyncedToGM) {
        printf("| %-28s | %-12d | %-11d |\n", "ptp_isSyncedWithGm",
            clkmgr_isPtpSyncedWithGm(syncData),
            clkmgr_getPtpSyncedWithGmEventCount(syncData));
    }
    if (eventMask & Clkmgr_EventASCapable) {
        printf("| %-28s | %-12d | %-11d |\n", "ptp_isAsCapable",
            clkmgr_isPtpAsCapable(syncData),
            clkmgr_getPtpAsCapableEventCount(syncData));
    }
    if (eventMask & Clkmgr_EventGMChanged) {
        printf("| %-28s | %-12d | %-11d |\n", "ptp_isGmChanged",
            clkmgr_isGmChanged(syncData, Clkmgr_PTPClock),
            clkmgr_getGmChangedEventCount(syncData, Clkmgr_PTPClock));
    }
    printf("|------------------------------|--------------|-------------|\n");
    printf("| %-28s |     %-19ld ns |\n", "ptp_clockOffset",
        clkmgr_getClockOffset(syncData, Clkmgr_PTPClock));
    uint64_t gmClockUUID = clkmgr_getGmIdentity(syncData, Clkmgr_PTPClock);
    uint8_t gmIdentity[8];
    for (int i = 0; i < 8; ++i) {
        gmIdentity[i] = (uint8_t)(gmClockUUID >> (8 * (7 - i)));
    }
    printf("| %-28s |     %02x%02x%02x.%02x%02x.%02x%02x%02x     |\n",
        "ptp_gmIdentity", gmIdentity[0], gmIdentity[1], gmIdentity[2], gmIdentity[3],
        gmIdentity[4], gmIdentity[5], gmIdentity[6], gmIdentity[7]);
    printf("| %-28s |     %-19ld us |\n", "ptp_syncInterval",
        clkmgr_getSyncInterval(syncData, Clkmgr_PTPClock));
    printf("| %-28s |     %-19ld ns |\n", "ptp_notificationTimestamp",
        clkmgr_getNotificationTimestamp(syncData, Clkmgr_PTPClock));
    printf("|------------------------------|----------------------------|\n");
    if (clkmgr_haveSysClock) {
        printf("| %-28s | %-12d | %-11d |\n", "chrony_isOffsetInRange",
            clkmgr_isOffsetInRange(syncData, Clkmgr_SysClock),
            clkmgr_getOffsetInRangeEventCount(syncData, Clkmgr_SysClock));
        printf("|------------------------------|----------------------------|\n");
        printf("| %-28s |     %-19ld ns |\n", "chrony_clockOffset",
            clkmgr_getClockOffset(syncData, Clkmgr_SysClock));
        char id[5] = {0}; // 4 characters + null terminator
        for (int i = 0; i < 4; ++i) {
            id[i] = (clkmgr_getGmIdentity(syncData, Clkmgr_SysClock) >>
            (8 * (3 - i))) & 0xFF; // Extract each bytes
        }
        printf("| %-28s |     %-19s    |\n", "chrony_gmIdentity", id);
        printf("| %-28s |     %-19ld us |\n", "chrony_syncInterval",
            clkmgr_getSyncInterval(syncData, Clkmgr_SysClock));
        printf("| %-28s |     %-19ld ns |\n", "chrony_notificationTimestamp",
            clkmgr_getNotificationTimestamp(syncData, Clkmgr_SysClock));
        printf("|------------------------------|----------------------------|\n");
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    Clkmgr_ClockSyncData *syncData = clkmgr_constructClockSyncDataInstance();
    Clkmgr_Subscription *clockSub[MAX_CLOCKS];
    uint32_t  ptp4lClockOffsetThreshold = 100000;
    uint32_t  chronyClockOffsetThreshold = 100000;
    bool subscribeAll = false;
    bool userInput = false;
    int ret = EXIT_FAILURE;
    uint32_t idleTime = 1;
    uint32_t timeout = 10;
    int index[MAX_CLOCKS];
    int indexCount = 0;
    enum Clkmgr_StatusWaitResult retval;
    int option;

    const char *me = strrchr(argv[0], '/');
    // Remove leading path
    me = me == NULL ? argv[0] : me + 1;

    while ((option = getopt(argc, argv, "aps:c:n:l:i:t:m:h")) != -1) {
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
        case 'n':
            chronyEventMask = strtoul(optarg, NULL, 0);
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
                   "     Bit 0: EventGMOffset\n"
                   "     Bit 1: EventSyncedToGM\n"
                   "     Bit 2: EventASCapable\n"
                   "     Bit 3: EventGMChanged\n"
                   "  -c composite_event_mask\n"
                   "     Default: 0x%x\n"
                   "     Bit 0: EventGMOffset\n"
                   "     Bit 1: EventSyncedToGM\n"
                   "     Bit 2: EventASCapable\n"
                   "  -n chrony_event_mask\n"
                   "     Default: 0x%x\n"
                   "     Bit 0: EventGMOffset\n"
                   "  -l gm offset threshold (ns)\n"
                   "     Default: %d ns\n"
                   "  -i idle time (s)\n"
                   "     Default: %d s\n"
                   "  -m chrony offset threshold (ns)\n"
                   "     Default: %d ns\n"
                   "  -t timeout in waiting notification event (s)\n"
                   "     Default: %d s\n",
                   me, eventMask,
                   compositeEventMask, chronyEventMask,
                   ptp4lClockOffsetThreshold, idleTime,
                   chronyClockOffsetThreshold, timeout);
            return EXIT_SUCCESS;
        default:
            printf("Usage of %s :\n"
                   "Options:\n"
                   "  -a subscribe to all time base indices\n"
                   "     Default: timeBaseIndex: 1\n"
                   "  -p enable user to subscribe to specific time base indices\n"
                   "  -s subscribe_event_mask\n"
                   "     Default: 0x%x\n"
                   "     Bit 0: EventGMOffset\n"
                   "     Bit 1: EventSyncedToGM\n"
                   "     Bit 2: EventASCapable\n"
                   "     Bit 3: EventGMChanged\n"
                   "  -c composite_event_mask\n"
                   "     Default: 0x%x\n"
                   "     Bit 0: EventGMOffset\n"
                   "     Bit 1: EventSyncedToGM\n"
                   "     Bit 2: EventASCapable\n"
                   "  -n chrony_event_mask\n"
                   "     Default: 0x%x\n"
                   "     Bit 0: EventGMOffset\n"
                   "  -l gm offset threshold (ns)\n"
                   "     Default: %d ns\n"
                   "  -i idle time (s)\n"
                   "     Default: %d s\n"
                   "  -m chrony offset threshold (ns)\n"
                   "     Default: %d ns\n"
                   "  -t timeout in waiting notification event (s)\n"
                   "     Default: %d s\n",
                   me, eventMask,
                   compositeEventMask, chronyEventMask,
                   ptp4lClockOffsetThreshold, idleTime,
                   chronyClockOffsetThreshold, timeout);
            return EXIT_FAILURE;
        }
    }

    if (userInput && !subscribeAll) {
        char input[100];
        printf("Enter the time base indices to subscribe (comma-separated, default is 1): ");
        fgets(input, sizeof(input), stdin);
        if (strlen(input) > 0) {
            char *token = strtok(input, ", ");
            while (indexCount < MAX_CLOCKS && token != NULL) {
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
        }
    }
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGHUP, signal_handler);

    if (clkmgr_connect() == false) {
        fprintf(stderr, "[clkmgr] Failure in connecting !!!\n");
        ret = EXIT_FAILURE;
        goto do_exit;
    }

    sleep(1);

    size_t indexSize = clkmgr_getTimebaseCfgsSize();
    if(indexSize == 0) {
        fprintf(stderr, "[clkmgr] No available clock found !!!\n");
        ret = EXIT_FAILURE;
        goto do_exit;
    }
    if(indexSize >= MAX_CLOCKS) {
        fprintf(stderr, "[clkmgr] %zu clocks are above support!!!\n", indexSize);
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
                clkmgr_setEventMask(clockSub[i], Clkmgr_PTPClock, eventMask);
                clkmgr_setPtpCompositeEventMask(clockSub[i], compositeEventMask);
                clkmgr_setClockOffsetThreshold(clockSub[i], Clkmgr_PTPClock,
                    ptp4lClockOffsetThreshold);
            }
            if(clkmgr_haveSysClock(i)) {
                clkmgr_setEventMask(clockSub[i], Clkmgr_SysClock, chronyEventMask);
                clkmgr_setClockOffsetThreshold(clockSub[i], Clkmgr_SysClock,
                    chronyClockOffsetThreshold);
            }
            if (subscribeAll)
                index[indexCount++] = i;
        } else {
            printf("Failed to get time base configuration for index %ld\n", i);
        }
    }

    if (indexCount == 0)
        index[indexCount++] = 1;

    for (size_t i = 0; i < indexCount; i++) {
        if (!clkmgr_isTimeBaseIndexPresent(index[i])) {
            fprintf(stderr, "[clkmgr] Index %d does not exist\n", index[i]);
            ret = EXIT_FAILURE;
            goto do_exit;
        }
        printf("[clkmgr] Subscribe to time base index: %d\n", index[i]);
        if (clkmgr_subscribe(clockSub[i+1], index[i], syncData) == false) {
            fprintf(stderr, "[clkmgr] Failure in subscribing to clkmgr Proxy !!!\n");
            ret = EXIT_FAILURE;
            goto do_exit;
        }

        printf("[clkmgr][%.3f] Obtained data from Subscription Event:\n",
            getMonotonicTime());
        printOut(syncData,clkmgr_haveSysClock(index[i]));
    }
    sleep(1);

    while (!signal_flag) {
        for (size_t i = 0; i < indexCount; i++) {
            if (signal_flag)
                goto do_exit;
            printf("[clkmgr][%.3f] Waiting Notification from time base index %d ...\n",
                getMonotonicTime(), index[i]);
            retval = clkmgr_statusWait(timeout, index[i], syncData);
            switch(retval) {
                case Clkmgr_SWRLostConnection:
                    printf("[clkmgr][%.3f] Terminating: lost connection to clkmgr Proxy\n",
                        getMonotonicTime());
                    goto do_exit;
                case Clkmgr_SWRInvalidArgument:
                    fprintf(stderr, "[clkmgr][%.3f] Terminating: Invalid argument\n",
                        getMonotonicTime());
                    ret = EXIT_FAILURE;
                    goto do_exit;
                case Clkmgr_SWRNoEventDetected:
                    printf("[clkmgr][%.3f] No event status changes identified in %d seconds.\n\n",
                        getMonotonicTime(), timeout);
                    printf("[clkmgr][%.3f] sleep for %d seconds...\n\n",
                        getMonotonicTime(), idleTime);
                    if (signal_flag)
                        goto do_exit;
                    sleep(idleTime);
                    continue;
                case Clkmgr_SWREventDetected:
                    printf("[clkmgr][%.3f] Obtained data from Notification Event:\n",
                        getMonotonicTime());
                    break;
                default:
                    printf("[clkmgr][%.3f] Warning: Should not enter this switch case, unexpected status code %d\n",
                        getMonotonicTime(), retval);
                    goto do_exit;
            }

            printOut(syncData,clkmgr_haveSysClock(index[i]));

            printf("[clkmgr][%.3f] sleep for %d seconds...\n\n",
                getMonotonicTime(), idleTime);
            if (signal_flag)
                goto do_exit;
            sleep(idleTime);
        }
    }

    ret = EXIT_SUCCESS;
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
