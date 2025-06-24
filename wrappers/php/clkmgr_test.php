#!/usr/bin/php
<?php
/* SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 *
 * Test PHP Clock Manager client
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 * @note This is a sample code, not a product! You should use it as a reference.
 *
 * To run in development environment:
 * LD_PRELOAD=../../.libs/libclkmgr.so PHPRC=. ./clkmgr_test.php
 *
 * Run ./php_ini.sh to create php.ini
 *
 */

$signal_flag = false;

declare(ticks = 1);

function signal_handler(int $signo, $siginfo): void
{
    global $signal_flag;
    echo " Exit ...\n";
    $signal_flag = true;
}

function isPositiveValue(string $optarg, string $errorMessage): int
{
    $ret = intval($optarg);
    if($ret > 0)
        return $ret;
    echo "$errorMessage\n";
    exit(2);
}

function clockManagerGetTime(): void
{
    $ts = new timespec();
    if(ClockManager::getTime($ts)) {
        printf("[clkmgr] Current Time of CLOCK_REALTIME: %ld ns\n",
                ($ts->tv_sec * 1000000000) + $ts->tv_nsec);
    } else {
        echo "clock_gettime failed: " . posix_strerror(posix_get_last_error())."\n";
    }
}

function getMonotonicTime()
{
    return microtime(true);
}

function main(): void
{
    global $signal_flag, $argv;
    $ptp4lClockOffsetThreshold = 100000;
    $chronyClockOffsetThreshold = 100000;
    $subscribeAll = false;
    $userInput = false;
    $idleTime = 1;
    $timeout = 10;
    $index = []; # Array of indexes to subscribe
    $event2Sub = EventGMOffset | EventSyncedToGM | EventASCapable | EventGMChanged;
    $composite_event = EventGMOffset | EventSyncedToGM | EventASCapable;
    $clockSyncData = new ClockSyncData();
    $ptp4lSub = new PTPClockSubscription();
    $chronySub = new SysClockSubscription();
    $overallSub = []; # Array of ClockSyncSubscription
    $options = getopt('aps:c:l:i:t:m:h');
    if(array_key_exists('h', $options)) {
        $event2SubHex = dechex($event2Sub);
        $composite_eventHex = dechex($composite_event);
        $me = basename($argv[0]);
        echo <<< END
        Usage of $me :
        Options:
          -a subscribe to all time base indices
             Default: timeBaseIndex: 1
          -p enable user to subscribe to specific time base indices
          -s subscribe_event_mask
             Default: 0x$event2SubHex
             Bit 0: EventGMOffset
             Bit 1: EventSyncedToGM
             Bit 2: EventASCapable
             Bit 3: EventGMChanged
          -c composite_event_mask
             Default: 0x$composite_eventHex
             Bit 0: EventGMOffset
             Bit 1: EventSyncedToGM
             Bit 2: EventASCapable
          -l gm offset threshold (ns)
             Default: $ptp4lClockOffsetThreshold ns
          -i idle time (s)
             Default: $idleTime s
          -m chrony offset threshold (ns)
             Default: $chronyClockOffsetThreshold ns
          -t timeout in waiting notification event (s)
             Default: $timeout s

        END;
        return;
    }
    if(array_key_exists('a', $options))
        $subscribeAll = true;
    else if(array_key_exists('p', $options))
        $userInput = true;
    if(array_key_exists('s', $options))
        $event2Sub = intval($options['s'], 0);
    if(array_key_exists('c', $options))
        $composite_event = intval($options['c'], 0);

    if(array_key_exists('l', $options))
        $ptp4lClockOffsetThreshold = isPositiveValue($options['l'],
            'Invalid ptp4l GM Offset threshold!');
    if(array_key_exists('i', $options))
        $idleTime = isPositiveValue($options['i'], 'Invalid idle time!');
    if(array_key_exists('t', $options))
        $timeout = isPositiveValue($options['t'], 'Invalid timeout!');
    if(array_key_exists('m', $options))
        $chronyClockOffsetThreshold = isPositiveValue($options['m'],
            'Invalid Chrony Offset threshold!');
    $ptp4lSub->setEventMask($event2Sub);
    $ptp4lSub->setClockOffsetThreshold($ptp4lClockOffsetThreshold);
    $ptp4lSub->setCompositeEventMask($composite_event);
    $chronySub->setClockOffsetThreshold($chronyClockOffsetThreshold);
    echo "[clkmgr] set subscribe event : 0x" .
        dechex($ptp4lSub->getEventMask()) . "\n";
    echo "[clkmgr] set composite event : 0x" .
        dechex($ptp4lSub->getCompositeEventMask()) . "\n";
    echo "GM Offset threshold: $ptp4lClockOffsetThreshold ns\n";
    echo "Chrony Offset threshold: $chronyClockOffsetThreshold ns\n";

    if($userInput) {
        $line = readline('Enter the time base indices to subscribe ' .
              '(comma-separated, default is 1): ');
        if(strlen($line) > 0) {
            $idx = intval($line);
            if($idx <= 0) {
                echo "Invalid time base index: $line!\n";
                exit(2);
            }
            $index[] = $idx;
        } else {
            echo "Invalid input. Using default time base index 1.\n";
        }
    }
    pcntl_signal(SIGINT, "signal_handler");
    pcntl_signal(SIGTERM, "signal_handler");
    pcntl_signal(SIGHUP, "signal_handler");

    $dieMsg = '';
    if(!ClockManager::connect()) {
        $dieMsg = '[clkmgr] failure in connecting !!!';
        goto do_exit;
    }
    sleep(1);
    echo "[clkmgr] List of available clock:\n";

    # Print out each member of the Time Base configuration
    $size = TimeBaseConfigurations::size();
    for($i = 1; $i <= $size; $i++) {
        $cfg = TimeBaseConfigurations::getRecord($i);
        $idx = $cfg->index();
        echo "TimeBaseIndex: $idx\n";
        echo 'timeBaseName: ' . $cfg->name() . "\n";
        $Subscribe = new ClockSyncSubscription();
        if($cfg->havePtp()) {
            $ptp = $cfg->ptp();
            echo 'interfaceName: ' . $ptp->ifName() . "\n";
            echo 'transportSpecific: ' . $ptp->transportSpecific() . "\n";
            echo 'domainNumber: ' . $ptp->domainNumber() . "\n";
            $Subscribe->setPtpSubscription($ptp4lSub);
        }
        if($cfg->haveSysClock())
            $Subscribe->setSysSubscription($chronySub);
        $overallSub[$idx] = $Subscribe;
        if($subscribeAll)
            $index[] = $idx;
    }

    # Default
    if(count($index) == 0)
        $index[] = 1;

    $hd2 = '|'.str_repeat('-',27).'|'.str_repeat('-',24).'|';
    foreach($index as $idx) {
        echo "Subscribe to time base index: $idx\n";
        if(!ClockManager::subscribe($overallSub[$idx], $idx, $clockSyncData)) {
            $dieMsg = '[clkmgr] Failure in subscribing to clkmgr Proxy !!!';
            goto do_exit;
        }
        printf("[clkmgr][%.3f] Obtained data from Subscription Event:\n",
            getMonotonicTime());
        clockManagerGetTime();
        printf("$hd2\n| %-25s | %-22s |\n", 'Event', 'Event Status');
        $ptpClock = $clockSyncData->getPtp();
        $sysClock = $clockSyncData->getSysClock();
        if($event2Sub != 0) {
            echo "$hd2\n";
            if($event2Sub & EventGMOffset)
                printf("| %-25s | %-22d |\n", 'offset_in_range',
                    $ptpClock->isOffsetInRange());
            if($event2Sub & EventSyncedToGM)
                printf("| %-25s | %-22d |\n", 'synced_to_primary_clock',
                    $ptpClock->isSyncedWithGm());
            if($event2Sub & EventASCapable)
                printf("| %-25s | %-22d |\n", 'as_capable',
                    $ptpClock->isAsCapable());
            if($event2Sub & EventGMChanged)
                printf("| %-25s | %-22d |\n", 'gm_Changed',
                    $ptpClock->isGmChanged());
        }
        echo "$hd2\n";
        $gmClockUUID = $ptpClock->getGmIdentityStr();
        printf("| %-25s | %s     |\n", 'GM UUID', $gmClockUUID);
        printf("| %-25s | %-19ld ns |\n" .
               "| %-25s | %-19ld ns |\n" .
               "| %-25s | %-19ld us |\n" .
               "$hd2\n"
               , 'clock_offset', $ptpClock->getClockOffset()
               , 'notification_timestamp', $ptpClock->getNotificationTimestamp()
               , 'gm_sync_interval', $ptpClock->getSyncInterval());
        if($composite_event != 0) {
            printf("| %-25s | %-22d |\n", 'composite_event',
                $ptpClock->isCompositeEventMet());
            if($composite_event & EventGMOffset)
                printf("| - %-23s | %-22s |\n", 'offset_in_range', '');
            if($composite_event & EventSyncedToGM)
                printf("| - %-19s | %-22s |\n", 'synced_to_primary_clock', '');
            if($composite_event & EventASCapable)
                printf("| - %-23s | %-22s |\n", 'as_capable', '');
            echo "$hd2\n";
        }
        echo "\n";
        printf("$hd2\n" .
               "| %-25s | %-22d |\n" .
               "$hd2\n" .
               "| %-25s | %-19ld ns |\n" .
               "| %-25s | %-19lx    |\n" .
               "| %-25s | %-19ld us |\n" .
               "$hd2\n\n"
               , 'chrony_offset_in_range', $sysClock->isOffsetInRange()
               , 'chrony_clock_offset', $sysClock->getClockOffset()
               , 'chrony_clock_reference_id', $sysClock->getGmIdentity()
               , 'chrony_polling_interval', $sysClock->getSyncInterval());
    }
    sleep(1);

    $hd2l = '|'.str_repeat('-', 27).'|'.str_repeat('-', 28).'|';
    $hd3b = '| %-25s | %-12d | %-11d |';
    $hd3 = '|'.str_repeat('-', 27).'|'.str_repeat('-', 14).
        '|'.str_repeat('-', 13).'|';

    while(!$signal_flag) {
        next_loop:
        foreach($index as $idx) {
            if($signal_flag)
                goto do_exit;
            printf("[clkmgr][%.3f] Waiting Notification from " .
                "time base index $idx ...\n", getMonotonicTime());
            $retval = ClockManager::statusWait($timeout, $idx, $clockSyncData);
            switch($retval) {
                case SWRLostConnection:
                    printf("[clkmgr][%.3f] Terminating: " .
                        "lost connection to clkmgr Proxy\n", getMonotonicTime());
                    goto do_exit;
                case SWRInvalidArgument:
                    printf("[clkmgr][%.3f] Terminating: Invalid argument\n",
                        getMonotonicTime());
                    goto do_exit;
                case SWRNoEventDetected:
                    printf("[clkmgr][%.3f] No event status changes identified " .
                        "in $timeout seconds.\n\n", getMonotonicTime());
                    printf("[clkmgr][%.3f] sleep for $idleTime seconds...\n\n",
                        getMonotonicTime());
                    if($signal_flag)
                        goto do_exit;
                    sleep($idleTime);
                    goto next_loop;
                case SWREventDetected:
                    printf("[clkmgr][%.3f] Obtained data from Notification Event:\n",
                        getMonotonicTime());
                    break;
                default:
                    printf("[clkmgr][%.3f] Warning: Should not enter " .
                           "this switch case, unexpected status code $retval\n",
                           getMonotonicTime());
                    goto do_exit;
            }
            clockManagerGetTime();
            printf("$hd3\n| %-25s | %-12s | %-11s |\n", 'Event',
                'Event Status', 'Event Count');
            $ptpClock = $clockSyncData->getPtp();
            $sysClock = $clockSyncData->getSysClock();
            if($event2Sub != 0) {
                echo "$hd3\n";
                if($event2Sub & EventGMOffset)
                    printf("$hd3b\n", 'offset_in_range',
                        $ptpClock->isOffsetInRange(),
                        $ptpClock->getOffsetInRangeEventCount());
                if($event2Sub & EventSyncedToGM)
                    printf("$hd3b\n", 'synced_to_primary_clock',
                        $ptpClock->isSyncedWithGm(),
                        $ptpClock->getSyncedWithGmEventCount());
                if($event2Sub & EventASCapable)
                    printf("$hd3b\n", 'as_capable', $ptpClock->isAsCapable(),
                        $ptpClock->getAsCapableEventCount());
                if($event2Sub & EventGMChanged)
                    printf("$hd3b\n", 'gm_Changed', $ptpClock->isGmChanged(),
                        $ptpClock->getGmChangedEventCount());
            }
            echo "$hd3\n";
            $gmClockUUID = $ptpClock->getGmIdentityStr();
            printf("| %-25s |     %s     |\n", 'GM UUID', $gmClockUUID);
            printf("| %-25s |     %-19ld ns |\n" .
                   "| %-25s |     %-19ld ns |\n" .
                   "| %-25s |     %-19ld us |\n" .
                   "$hd3\n"
                   , 'clock_offset', $ptpClock->getClockOffset()
                   , 'notification_timestamp', $ptpClock->getNotificationTimestamp()
                   , 'gm_sync_interval', $ptpClock->getSyncInterval());
            if($composite_event != 0) {
                printf("$hd3b\n", 'composite_event',
                    $ptpClock->isCompositeEventMet(),
                    $ptpClock->getCompositeEventCount());
                if($composite_event & EventGMOffset)
                    printf("| - %-23s | %-12s | %-11s |\n",
                        'offset_in_range', '', '');
                if($composite_event & EventSyncedToGM)
                    printf("| - %-19s | %-12s | %-11s |\n",
                        'synced_to_primary_clock', '', '');
                if($composite_event & EventASCapable)
                    printf("| - %-23s | %-12s | %-11s |\n", 'as_capable', '', '');
                echo "$hd3\n";
            }
            echo "\n";
            printf("$hd2l\n" .
                   "$hd3b\n" .
                   "$hd2l\n" .
                   "| %-25s |     %-19ld ns |\n" .
                   "| %-25s |     %-19lx    |\n" .
                   "| %-25s |     %-19ld us |\n" .
                   "$hd2l\n\n"
                   , 'chrony_offset_in_range', $sysClock->isOffsetInRange(),
                     $sysClock->getOffsetInRangeEventCount()
                   , 'chrony_clock_offset', $sysClock->getClockOffset()
                   , 'chrony_clock_reference_id', $sysClock->getGmIdentity()
                   , 'chrony_polling_interval', $sysClock->getSyncInterval());
            printf("[clkmgr][%.3f] sleep for %d seconds...\n\n",
                getMonotonicTime(), $idleTime);
            if($signal_flag)
                goto do_exit;
            sleep($idleTime);
        }
    }

    do_exit:

    ClockManager::disconnect();

    if(strlen($dieMsg) > 0) {
        echo "$dieMsg\n";
        exit(2);
    }
}
main();
