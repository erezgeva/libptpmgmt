/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Test clkmgr client code
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
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
#include <sstream>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <cstring>

#include <clockmanager.h>

using namespace clkmgr;

static volatile sig_atomic_t signal_flag = 0;

static ClockManager &cm = ClockManager::fetchSingleInstance();

static ClockSyncData clockSyncData;
static PTPClockEvent &ptpClock = clockSyncData.getPtp();
static SysClockEvent &sysClock = clockSyncData.getSysClock();

static uint32_t event2Sub =
    EventOffsetInRange | EventSyncedWithGm | EventAsCapable | EventGmChanged;

static uint32_t composite_event =
    EventOffsetInRange | EventSyncedWithGm | EventAsCapable;

static uint32_t chronyEvent = EventOffsetInRange;

static uint64_t gmClockUUID;
static timespec ts;

static void signal_handler(int sig)
{
    std::cout << " Exit ..." << std::endl;
    signal_flag = 1;
}

static double getMonotonicTime() {
    timespec timeSpec;

    if (clock_gettime(CLOCK_MONOTONIC, &timeSpec) == -1) {
        perror("clock_gettime failed");
        return -1;
    }

    double seconds = timeSpec.tv_sec;
    double nanoseconds = timeSpec.tv_nsec / 1e9;

    return seconds + nanoseconds;
}

static bool isPositiveValue(const std::string& optarg, uint32_t& target, const std::string& errorMessage) {
    try {
        int value = std::stoi(optarg);
        if (value < 0) {
            std::cerr << errorMessage << std::endl;
            return false;
        }
        target = static_cast<uint32_t>(value);
        return true;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        return false;
    } catch (const std::out_of_range& e) {
        std::cerr << "Out of range: " << e.what() << std::endl;
        return false;
    }
}

static void printOut()
{
    if (!cm.getTime(ts)) {
        perror("clock_gettime failed");
    } else {
        printf("[clkmgr] Current Time of CLOCK_REALTIME: %ld ns\n",
        (ts.tv_sec * 1000000000) + ts.tv_nsec);
    }
    printf("|------------------------------|--------------|-------------|\n");
    printf("| %-28s | %-12s | %-11s |\n", "Events", "Event Status",
        "Event Count");
    printf("|------------------------------|--------------|-------------|\n");
    if (composite_event) {
        printf("| %-28s | %-12d | %-11d |\n", "ptp_isCompositeEventMet",
            ptpClock.isCompositeEventMet(), ptpClock.getCompositeEventCount());
    }
    if (composite_event & EventOffsetInRange) {
        printf("| - %-26s | %-12s | %-11s |\n", "isOffsetInRange", "", "");
    }
    if (composite_event & EventSyncedWithGm) {
        printf("| - %-26s | %-12s | %-11s |\n", "isSyncedWithGm", "", "");
    }
    if (composite_event & EventAsCapable) {
        printf("| - %-26s | %-12s | %-11s |\n", "isAsCapable", "", "");
    }
    if (event2Sub) {
        printf("|------------------------------|--------------|-------------|\n");
    }
    if (event2Sub & EventOffsetInRange) {
        printf("| %-28s | %-12d | %-11d |\n", "ptp_isOffsetInRange",
            ptpClock.isOffsetInRange(),
            ptpClock.getOffsetInRangeEventCount());
    }
    if (event2Sub & EventSyncedWithGm) {
        printf("| %-28s | %-12d | %-11d |\n", "ptp_isSyncedWithGm",
            ptpClock.isSyncedWithGm(), ptpClock.getSyncedWithGmEventCount());
    }
    if (event2Sub & EventAsCapable) {
        printf("| %-28s | %-12d | %-11d |\n", "ptp_isAsCapable",
            ptpClock.isAsCapable(), ptpClock.getAsCapableEventCount());
    }
    if (event2Sub & EventGmChanged) {
        printf("| %-28s | %-12d | %-11d |\n", "ptp_isGmChanged",
            ptpClock.isGmChanged(), ptpClock.getGmChangedEventCount());
    }
    printf("|------------------------------|--------------|-------------|\n");
    printf("| %-28s |     %-19ld ns |\n", "ptp_clockOffset", ptpClock.getClockOffset());
    gmClockUUID = ptpClock.getGmIdentity();
    uint8_t gm_identity[8];
    // Copy the uint64_t into the array
    for (int i = 0; i < 8; ++i) {
        gm_identity[i] = static_cast<uint8_t>(gmClockUUID >> (8 * (7 - i)));
    }
    printf("| %-28s |     %02x%02x%02x.%02x%02x.%02x%02x%02x     |\n",
        "ptp_gmIdentity", gm_identity[0], gm_identity[1], gm_identity[2],
            gm_identity[3],gm_identity[4], gm_identity[5], gm_identity[6],
            gm_identity[7]);
    printf("| %-28s |     %-19ld us |\n", "ptp_syncInterval",
        ptpClock.getSyncInterval());
    printf("| %-28s |     %-19ld ns |\n", "ptp_notificationTimestamp",
        ptpClock.getNotificationTimestamp());
    printf("|------------------------------|----------------------------|\n");
    if (clockSyncData.haveSys()) {
        printf("| %-28s | %-12d | %-11d |\n", "chrony_isOffsetInRange",
            sysClock.isOffsetInRange(), sysClock.getOffsetInRangeEventCount());
        printf("|------------------------------|----------------------------|\n");
        printf("| %-28s |     %-19ld ns |\n",
            "chrony_clockOffset", sysClock.getClockOffset());
        char id[5] = {0}; // 4 characters + null terminator
        for (int i = 0; i < 4; ++i) {
            char byteVal = (sysClock.getGmIdentity() >> (8 * (3 - i))) & 0xFF; // Extract each byte
            id[i] = byteVal;
            if(byteVal == 0 || byteVal == 9)
                id[i] = ' ';
            else if(isprint(byteVal))
                id[i] = byteVal;
            else
                id[i] = '.';
        }
        printf("| %-28s |     %-22s |\n", "chrony_gmIdentity", std::string(id).c_str());
        printf("| %-28s |     %-19ld us |\n",
            "chrony_syncInterval", sysClock.getSyncInterval());
        printf("| %-28s |     %-19ld ns |\n",
            "chrony_notificationTimestamp", sysClock.getNotificationTimestamp());
        printf("|------------------------------|----------------------------|\n");
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    uint32_t ptp4lClockOffsetThreshold = 100000;
    uint32_t chronyClockOffsetThreshold = 100000;
    bool subscribeAll = false;
    bool userInput = false;
    int ret = EXIT_SUCCESS;
    uint32_t idleTime = 1;
    uint32_t timeout = 10;
    std::vector<size_t> index;
    std::string input;
    enum StatusWaitResult retval;
    int option;

    const char *me = strrchr(argv[0], '/');
    // Remove leading path
    me = me == nullptr ? argv[0] : me + 1;

    PTPClockSubscription ptp4lSub;
    SysClockSubscription chronySub;

    std::map<size_t, ClockSyncSubscription> overallSub;

    while ((option = getopt(argc, argv, "aps:c:n:l:i:t:m:h")) != -1) {
        switch (option) {
        case 'a':
            subscribeAll = true;
            break;
        case 'p':
            userInput = true;
            break;
        case 's':
            event2Sub = std::stoul(optarg, nullptr, 0);
            break;
        case 'c':
            composite_event = std::stoul(optarg, nullptr, 0);
            break;
        case 'n':
            chronyEvent = std::stoul(optarg, nullptr, 0);
            break;
        case 'l':
            if (!isPositiveValue(optarg, ptp4lClockOffsetThreshold, "Invalid ptp4l GM Offset threshold!")) {
                return EXIT_FAILURE;
            }
            break;
        case 'i':
            if (!isPositiveValue(optarg, idleTime, "Invalid idle time!")) {
                return EXIT_FAILURE;
            }
            break;
        case 't':
            if (!isPositiveValue(optarg, timeout, "Invalid timeout!")) {
                return EXIT_FAILURE;
            }
            break;
        case 'm':
            if (!isPositiveValue(optarg, chronyClockOffsetThreshold, "Invalid Chrony Offset threshold!")) {
                return EXIT_FAILURE;
            }
            break;
        case 'h':
            std::cout << "Usage of " << me << " :" << std::endl <<
                "Options:" << std::endl <<
                "  -a subscribe to all time base indices" << std::endl <<
                "     Default: timeBaseIndex: 1" << std::endl <<
                "  -p enable user to subscribe to specific time base indices" << std::endl <<
                "  -s subscribe_event_mask" << std::endl <<
                "     Default: 0x" << std::hex << event2Sub << std::endl <<
                "     Bit 0: EventOffsetInRange" << std::endl <<
                "     Bit 1: EventSyncedWithGm" << std::endl <<
                "     Bit 2: EventAsCapable" << std::endl <<
                "     Bit 3: EventGmChanged" << std::endl <<
                "  -c composite_event_mask" << std::endl <<
                "     Default: 0x" << composite_event << std::endl <<
                "     Bit 0: EventOffsetInRange" << std::endl <<
                "     Bit 1: EventSyncedWithGm" << std::endl <<
                "     Bit 2: EventAsCapable" << std::endl <<
                "  -l gm offset threshold (ns)" << std::endl <<
                "     Default: " << std::dec << ptp4lClockOffsetThreshold << " ns" << std::endl <<
                "  -i idle time (s)" << std::endl <<
                "     Default: " << idleTime << " s" << std::endl <<
                "  -m chrony offset threshold (ns)" << std::endl <<
                "     Default: " << chronyClockOffsetThreshold << " ns" << std::endl <<
                "  -n chrony_event_mask" << std::endl <<
                "     Default: 0x" << std::hex << chronyEvent << std::endl <<
                "     Bit 0: EventOffsetInRange" << std::endl <<
                "  -t timeout in waiting notification event (s)" << std::endl <<
                "     Default: " << timeout << " s" << std::endl;
            return EXIT_SUCCESS;
        default:
            std::cerr << "Usage of " << me << " :" << std::endl <<
                "Options:" << std::endl <<
                "  -a subscribe to all time base indices" << std::endl <<
                "     Default: timeBaseIndex: 1" << std::endl <<
                "  -p enable user to subscribe to specific time base indices" << std::endl <<
                "  -s subscribe_event_mask" << std::endl <<
                "     Default: 0x" << std::hex << event2Sub << std::endl <<
                "     Bit 0: EventOffsetInRange" << std::endl <<
                "     Bit 1: EventSyncedWithGm" << std::endl <<
                "     Bit 2: EventAsCapable" << std::endl <<
                "     Bit 3: EventGmChanged" << std::endl <<
                "  -c composite_event_mask" << std::endl <<
                "     Default: 0x" << composite_event << std::endl <<
                "     Bit 0: EventOffsetInRange" << std::endl <<
                "     Bit 1: EventSyncedWithGm" << std::endl <<
                "     Bit 2: EventAsCapable" << std::endl <<
                "  -l gm offset threshold (ns)" << std::endl <<
                "     Default: " << std::dec << ptp4lClockOffsetThreshold << " ns" << std::endl <<
                "  -i idle time (s)" << std::endl <<
                "     Default: " << idleTime << " s" << std::endl <<
                "  -m chrony offset threshold (ns)" << std::endl <<
                "     Default: " << chronyClockOffsetThreshold << " ns" << std::endl <<
                "  -n chrony_event_mask" << std::endl <<
                "     Default: 0x" << std::hex << chronyEvent << std::endl <<
                "     Bit 0: EventOffsetInRange" << std::endl <<
                "  -t timeout in waiting notification event (s)" << std::endl <<
                "     Default: " << timeout << " s" << std::endl;
            return EXIT_FAILURE;
        }
    }
    if (userInput && !subscribeAll) {
        std::cout << "Enter the time base indices to subscribe (comma-separated, default is 1): ";
        std::getline(std::cin, input);
        if (!input.empty()) {
            try {
                std::stringstream ss(input);
                std::string item;
                while (std::getline(ss, item, ',')) {
                    index.push_back(std::stoi(item));
                }
            } catch (const std::exception& e) {
                std::cerr << "Invalid time base index!" << std::endl;
                return EXIT_FAILURE;
            }
        } else {
            std::cerr << "Invalid input. Using default time base index 1." << std::endl;
        }
    }
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGHUP, signal_handler);

    if (!cm.connect()) {
        std::cerr << "[clkmgr] failure in connecting !!!" << std::endl;
        ret = EXIT_FAILURE;
        goto do_exit;
    }

    sleep(1);

    ptp4lSub.setEventMask(event2Sub);
    ptp4lSub.setClockOffsetThreshold(ptp4lClockOffsetThreshold);
    ptp4lSub.setCompositeEventMask(composite_event);
    chronySub.setEventMask(chronyEvent);
    chronySub.setClockOffsetThreshold(chronyClockOffsetThreshold);
    std::cout << "[clkmgr] set subscribe event : 0x"
        << std::hex << ptp4lSub.getEventMask() << std::endl;
    std::cout << "[clkmgr] set composite event : 0x"
        << std::hex << ptp4lSub.getCompositeEventMask() << std::endl;
    std::cout << "GM Offset threshold: \u00B1" << std::dec << ptp4lClockOffsetThreshold << " ns" << std::endl;
    std::cout << "[clkmgr] set chrony event : 0x"
        << std::hex << chronySub.getEventMask() << std::endl;
    std::cout << "Chrony Offset threshold: \u00B1" << std::dec << chronyClockOffsetThreshold << " ns" << std::endl;
    std::cout << "[clkmgr] List of available clock: " << std::endl;
    /* Print out each member of the Time Base configuration */
    for (const auto &cfg : cm.getTimebaseCfgs()) {
        size_t idx = cfg.index();
        std::cout << "TimeBaseIndex: " << idx << std::endl;
        std::cout << "timeBaseName: " << cfg.name() << std::endl;
        if(cfg.havePtp()) {
            std::cout << "interfaceName: " << cfg.ptp().ifName() << std::endl;
            std::cout << "transportSpecific: " << static_cast<int>(cfg.ptp().transportSpecific()) << std::endl;
            std::cout << "domainNumber: " << static_cast<int>(cfg.ptp().domainNumber()) << "" << std::endl << std::endl;
            overallSub[idx].setPtpSubscription(ptp4lSub);
        }
        if(cfg.haveSysClock())
            overallSub[idx].setSysSubscription(chronySub);
        if (subscribeAll)
            index.push_back(idx);
    }

    if (index.size() == 0)
        index.push_back(1);

    for (size_t idx : index) {
        if (!clkmgr::TimeBaseConfigurations::isTimeBaseIndexPresent(idx)) {
            std::cerr << "[clkmgr] Index " << idx << " does not exist" << std::endl;
            ret = EXIT_FAILURE;
            goto do_exit;
        }
        std::cout << "[clkmgr] Subscribe to time base index: " << idx << std::endl;
        if (!cm.subscribe(overallSub[idx], idx, clockSyncData)) {
            std::cerr << "[clkmgr] Failure in subscribing to clkmgr Proxy !!!" << std::endl;
            ret = EXIT_FAILURE;
            goto do_exit;
        }
        printf("[clkmgr][%.3f] Obtained data from Subscription Event:\n",
            getMonotonicTime());
        printOut();
    }
    sleep(1);

    while (!signal_flag) {
        for (size_t idx : index) {
            if (signal_flag)
                goto do_exit;
            printf("[clkmgr][%.3f] Waiting Notification from time base index %zu ...\n",
                getMonotonicTime(), idx);

            retval = cm.statusWait(timeout, idx, clockSyncData);
            switch(retval) {
                case SWRLostConnection:
                    printf("[clkmgr][%.3f] Terminating: lost connection to clkmgr Proxy\n",
                        getMonotonicTime());
                    goto do_exit;
                case SWRInvalidArgument:
                    fprintf(stderr, "[clkmgr][%.3f] Terminating: Invalid argument\n",
                        getMonotonicTime());
                    ret = EXIT_FAILURE;
                    goto do_exit;
                case SWRNoEventDetected:
                    printf("[clkmgr][%.3f] No event status changes identified in %d seconds.\n\n",
                        getMonotonicTime(), timeout);
                    printf("[clkmgr][%.3f] sleep for %d seconds...\n\n",
                        getMonotonicTime(), idleTime);
                    if (signal_flag)
                        goto do_exit;
                    sleep(idleTime);
                    continue;
                case SWREventDetected:
                    printf("[clkmgr][%.3f] Obtained data from Notification Event:\n",
                        getMonotonicTime());
                    break;
                default:
                    printf("[clkmgr][%.3f] Warning: Should not enter this switch case, unexpected status code %d\n",
                        getMonotonicTime(), retval);
                    goto do_exit;
            }

            printOut();

            printf("[clkmgr][%.3f] sleep for %d seconds...\n\n",
                getMonotonicTime(), idleTime);
            if (signal_flag)
                goto do_exit;
            sleep(idleTime);
        }
    }

do_exit:
    cm.disconnect();

    return ret;
}
