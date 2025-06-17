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

#include <clockmanager.h>

using namespace clkmgr;

volatile sig_atomic_t signal_flag = 0;

void signal_handler(int sig)
{
    signal_flag = 1;
}

double getMonotonicTime() {
    timespec timeSpec;

    if (clock_gettime(CLOCK_MONOTONIC, &timeSpec) == -1) {
        perror("clock_gettime failed");
        return -1;
    }

    double seconds = timeSpec.tv_sec;
    double nanoseconds = timeSpec.tv_nsec / 1e9;

    return seconds + nanoseconds;
}

bool isPositiveValue(const std::string& optarg, uint32_t& target, const std::string& errorMessage) {
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

int main(int argc, char *argv[])
{
    uint32_t  ptp4lClockOffsetThreshold = 100000;
    uint32_t  chronyClockOffsetThreshold = 100000;
    bool subscribeAll = false;
    bool userInput = false;
    int ret = EXIT_SUCCESS;
    uint32_t idleTime = 1;
    uint32_t timeout = 10;
    std::vector<int> index;
    std::string input;
    timespec ts;
    int retval;
    int option;

    std::uint32_t event2Sub = {
        (EventGMOffset | EventSyncedToGM | EventASCapable |
        EventGMChanged)
    };

    std::uint32_t composite_event = {
        (EventGMOffset | EventSyncedToGM | EventASCapable)
    };

    ClockSyncData clockSyncData;
    PTPClockEvent &ptpClock = clockSyncData.getPtp();
    SysClockEvent &sysClock = clockSyncData.getSysClock();

    PTPClockSubscription ptp4lSub;
    SysClockSubscription chronySub;

    uint64_t gmClockUUID;
    std::map<size_t, ClockSyncSubscription> overallSub;

    while ((option = getopt(argc, argv, "aps:c:u:l:i:t:n:m:h")) != -1) {
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
            std::cout << "Usage of " << argv[0] << " :\n"
                "Options:\n"
                "  -a subscribe to all time base indices\n"
                "     Default: timeBaseIndex: 1\n"
                "  -p enable user to subscribe to specific time base indices\n"
                "  -s subscribe_event_mask\n"
                "     Default: 0x" << std::hex << event2Sub << "\n"
                "     Bit 0: EventGMOffset\n"
                "     Bit 1: EventSyncedToGM\n"
                "     Bit 2: EventASCapable\n"
                "     Bit 3: EventGMChanged\n"
                "  -c composite_event_mask\n"
                "     Default: 0x" << composite_event << "\n"
                "     Bit 0: EventGMOffset\n"
                "     Bit 1: EventSyncedToGM\n"
                "     Bit 2: EventASCapable\n"
                "  -l gm offset threshold (ns)\n"
                "     Default: " << ptp4lClockOffsetThreshold << " ns\n"
                "  -i idle time (s)\n"
                "     Default: " << idleTime << " s\n"
                "  -m chrony offset threshold (ns)\n"
                "     Default: " << std::dec << chronyClockOffsetThreshold << " ns\n"
                "  -t timeout in waiting notification event (s)\n"
                "     Default: " << timeout << " s\n";
            return EXIT_SUCCESS;
        default:
            std::cerr << "Usage of " << argv[0] << " :\n"
                "Options:\n"
                "  -a subscribe to all time base indices\n"
                "     Default: timeBaseIndex: 1\n"
                "  -p enable user to subscribe to specific time base indices\n"
                "  -s subscribe_event_mask\n"
                "     Default: 0x" << std::hex << event2Sub << "\n"
                "     Bit 0: EventGMOffset\n"
                "     Bit 1: EventSyncedToGM\n"
                "     Bit 2: EventASCapable\n"
                "     Bit 3: EventGMChanged\n"
                "  -c composite_event_mask\n"
                "     Default: 0x" << composite_event << "\n"
                "     Bit 0: EventGMOffset\n"
                "     Bit 1: EventSyncedToGM\n"
                "     Bit 2: EventASCapable\n"
                "  -l gm offset threshold (ns)\n"
                "     Default: " << ptp4lClockOffsetThreshold << " ns\n"
                "  -i idle time (s)\n"
                "     Default: " << idleTime << " s\n"
                "  -m chrony offset threshold (ns)\n"
                "     Default: " << std::dec << chronyClockOffsetThreshold << " ns\n"
                "  -t timeout in waiting notification event (s)\n"
                "     Default: " << timeout << " s\n";
            return EXIT_FAILURE;
        }
    }

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGHUP, signal_handler);

    ClockManager &cm = ClockManager::fetchSingleInstance();

    if (!cm.connect()) {
        std::cout << "[clkmgr] failure in connecting !!!\n";
        ret = EXIT_FAILURE;
        goto do_exit;
    }

    sleep(1);

    ptp4lSub.setEventMask(event2Sub);
    ptp4lSub.setClockOffsetThreshold(ptp4lClockOffsetThreshold);
    ptp4lSub.setCompositeEventMask(composite_event);
    chronySub.setClockOffsetThreshold(chronyClockOffsetThreshold);
    std::cout << "[clkmgr] set subscribe event : 0x"
        << std::hex << ptp4lSub.getEventMask() <<  "\n";
    std::cout << "[clkmgr] set composite event : 0x"
        << std::hex << ptp4lSub.getCompositeEventMask() <<  "\n";
    std::cout << "GM Offset threshold: \u00B1" << std::dec << ptp4lClockOffsetThreshold << " ns\n";
    std::cout << "Chrony Offset threshold: \u00B1" << std::dec << chronyClockOffsetThreshold << " ns\n";
    std::cout << "[clkmgr] List of available clock: \n";
    /* Print out each member of the Time Base configuration */
    for (const auto &cfg : cm.getTimebaseCfgs()) {
        std::cout << "TimeBaseIndex: " << cfg.index() << "\n";
        std::cout << "timeBaseName: " << cfg.name() << "\n";
        if(cfg.havePtp()) {
            std::cout << "interfaceName: " << cfg.ptp().ifName() << "\n";
            std::cout << "transportSpecific: " << static_cast<int>(cfg.ptp().transportSpecific()) << "\n";
            std::cout << "domainNumber: " << static_cast<int>(cfg.ptp().domainNumber()) << "\n\n";
            overallSub[cfg.index()].setPtpSubscription(ptp4lSub);
        }
        if(cfg.haveSysClock())
            overallSub[cfg.index()].setSysSubscription(chronySub);
    }

    if (subscribeAll) {
        for (const auto &cfg : cm.getTimebaseCfgs()) {
            index.push_back(cfg.index());
        }
    } else if (userInput) {
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
                std::cerr << "Invalid time base index!\n";
                return EXIT_FAILURE;
            }
        } else {
            std::cerr << "Invalid input. Using default time base index 1.\n";
            index.push_back(1);
        }
    } else {
        index.push_back(1);
    }

    for (const auto &idx : index) {
        std::cout << "Subscribe to time base index: " << idx << "\n";
        if (!cm.subscribe(overallSub[idx], idx, clockSyncData)) {
            std::cerr << "[clkmgr] Failure in subscribing to clkmgr Proxy !!!\n";
            cm.disconnect();
            return EXIT_FAILURE;
        }
        printf("[clkmgr][%.3f] Obtained data from Subscription Event:\n",
            getMonotonicTime());
        if (!cm.getTime(ts)) {
            perror("clock_gettime failed");
        } else {
            printf("[clkmgr] Current Time of CLOCK_REALTIME: %ld ns\n",
                (ts.tv_sec * 1000000000) + ts.tv_nsec);
        }
        printf("|---------------------------|------------------------|\n");
        printf("| %-25s | %-22s |\n", "Event", "Event Status");
        if (event2Sub) {
            printf("|---------------------------|------------------------|\n");
        }
        if (event2Sub & EventGMOffset) {
            printf("| %-25s | %-22d |\n", "offset_in_range",
                ptpClock.isOffsetInRange());
        }
        if (event2Sub & EventSyncedToGM) {
            printf("| %-25s | %-22d |\n", "synced_to_primary_clock", ptpClock.isSyncedWithGm());
        }
        if (event2Sub & EventASCapable) {
            printf("| %-25s | %-22d |\n", "as_capable", ptpClock.isAsCapable());
        }
        if (event2Sub & EventGMChanged) {
            printf("| %-25s | %-22d |\n", "gm_Changed", ptpClock.isGmChanged());
        }
        printf("|---------------------------|------------------------|\n");
        gmClockUUID = ptpClock.getGmIdentity();
        uint8_t gm_identity[8];
        // Copy the uint64_t into the array
        for (int i = 0; i < 8; ++i) {
            gm_identity[i] = static_cast<uint8_t>(gmClockUUID >> (8 * (7 - i)));
        }
        printf("| %-25s | %02x%02x%02x.%02x%02x.%02x%02x%02x     |\n",
            "GM UUID", gm_identity[0], gm_identity[1],
            gm_identity[2], gm_identity[3],
            gm_identity[4], gm_identity[5],
            gm_identity[6], gm_identity[7]);
        printf("| %-25s | %-19ld ns |\n",
                "clock_offset", ptpClock.getClockOffset());
        printf("| %-25s | %-19ld ns |\n",
                "notification_timestamp", ptpClock.getNotificationTimestamp());
        printf("| %-25s | %-19ld us |\n",
                "gm_sync_interval", ptpClock.getSyncInterval());
        printf("|---------------------------|------------------------|\n");
        if (composite_event) {
            printf("| %-25s | %-22d |\n", "composite_event",
                ptpClock.isCompositeEventMet());
        }
        if (composite_event & EventGMOffset) {
            printf("| - %-23s | %-22s |\n", "offset_in_range", " ");
        }
        if (composite_event & EventSyncedToGM) {
            printf("| - %-19s | %-22s |\n", "synced_to_primary_clock", " ");
        }
        if (composite_event & EventASCapable) {
            printf("| - %-23s | %-22s |\n", "as_capable", " ");
        }
        if (composite_event) {
            printf("|---------------------------|------------------------|\n\n");
        } else {
            printf("\n");
        }
        printf("|---------------------------|------------------------|\n");
        printf("| %-25s | %-22d |\n", "chrony_offset_in_range",
                sysClock.isOffsetInRange());
        printf("|---------------------------|------------------------|\n");
        printf("| %-25s | %-19ld ns |\n",
                "chrony_clock_offset", sysClock.getClockOffset());
        printf("| %-25s | %-19lx    |\n",
            "chrony_clock_reference_id", sysClock.getGmIdentity());
        printf("| %-25s | %-19ld us |\n",
                "chrony_polling_interval", sysClock.getSyncInterval());
        printf("|---------------------------|------------------------|\n\n");
    }
    sleep(1);

    while (!signal_flag) {
        for (const auto &idx : index) {
            printf("[clkmgr][%.3f] Waiting Notification from time base index %d ...\n",
                getMonotonicTime(), idx);

            retval = cm.statusWait(timeout, idx, clockSyncData);

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
            if (!cm.getTime(ts)) {
                perror("clock_gettime failed");
            } else {
                printf("[clkmgr] Current Time of CLOCK_REALTIME: %ld ns\n",
                    (ts.tv_sec * 1000000000) + ts.tv_nsec);
            }
            printf("|---------------------------|--------------|-------------|\n");
            printf("| %-25s | %-12s | %-11s |\n", "Event", "Event Status",
                "Event Count");
            if (event2Sub) {
            printf("|---------------------------|--------------|-------------|\n");
            }
            if (event2Sub & EventGMOffset) {
                printf("| %-25s | %-12d | %-11d |\n", "offset_in_range",
                    ptpClock.isOffsetInRange(),
                    ptpClock.getOffsetInRangeEventCount());
            }
            if (event2Sub & EventSyncedToGM) {
                printf("| %-25s | %-12d | %-11d |\n", "synced_to_primary_clock",
                    ptpClock.isSyncedWithGm(), ptpClock.getSyncedWithGmEventCount());
            }
            if (event2Sub & EventASCapable) {
                printf("| %-25s | %-12d | %-11d |\n", "as_capable",
                    ptpClock.isAsCapable(), ptpClock.getAsCapableEventCount());
            }
            if (event2Sub & EventGMChanged) {
                printf("| %-25s | %-12d | %-11d |\n", "gm_Changed",
                    ptpClock.isGmChanged(), ptpClock.getGmChangedEventCount());
            }
            printf("|---------------------------|--------------|-------------|\n");
            gmClockUUID = ptpClock.getGmIdentity();
            uint8_t gm_identity[8];
            // Copy the uint64_t into the array
            for (int i = 0; i < 8; ++i) {
                gm_identity[i] = static_cast<uint8_t>(gmClockUUID >> (8 * (7 - i)));
            }
            printf("| %-25s |     %02x%02x%02x.%02x%02x.%02x%02x%02x     |\n",
                "GM UUID", gm_identity[0], gm_identity[1],
                gm_identity[2], gm_identity[3],
                gm_identity[4], gm_identity[5],
                gm_identity[6], gm_identity[7]);
            printf("| %-25s |     %-19ld ns |\n",
                "clock_offset", ptpClock.getClockOffset());
            printf("| %-25s |     %-19ld ns |\n",
                "notification_timestamp", ptpClock.getNotificationTimestamp());
            printf("| %-25s |     %-19ld us |\n",
                "gm_sync_interval", ptpClock.getSyncInterval());
            printf("|---------------------------|--------------|-------------|\n");
            if (composite_event) {
                printf("| %-25s | %-12d | %-11d |\n", "composite_event",
                    ptpClock.isCompositeEventMet(), ptpClock.getCompositeEventCount());
            }
            if (composite_event & EventGMOffset) {
                printf("| - %-23s | %-12s | %-11s |\n", "offset_in_range", "", "");
            }
            if (composite_event & EventSyncedToGM) {
                printf("| - %-19s | %-12s | %-11s |\n", "synced_to_primary_clock", "", "");
            }
            if (composite_event & EventASCapable) {
                printf("| - %-23s | %-12s | %-11s |\n", "as_capable", "", "");
            }
            if (composite_event) {
                printf("|---------------------------|--------------|-------------|\n\n");
            } else {
                printf("\n");
            }
            printf("|---------------------------|----------------------------|\n");
            printf("| %-25s | %-12d | %-11d |\n", "chrony_offset_in_range",
                sysClock.isOffsetInRange(), sysClock.getOffsetInRangeEventCount());
            printf("|---------------------------|----------------------------|\n");
            printf("| %-25s |     %-19ld ns |\n",
                "chrony_clock_offset", sysClock.getClockOffset());
            printf("| %-25s |     %-19lx    |\n",
                "chrony_clock_reference_id", sysClock.getGmIdentity());
            printf("| %-25s |     %-19ld us |\n",
                "chrony_polling_interval", sysClock.getSyncInterval());
            printf("|---------------------------|----------------------------|\n\n");

            printf("[clkmgr][%.3f] sleep for %d seconds...\n\n",
                getMonotonicTime(), idleTime);
            sleep(idleTime);
        }
    }

do_exit:
    cm.disconnect();

    return ret;
}
