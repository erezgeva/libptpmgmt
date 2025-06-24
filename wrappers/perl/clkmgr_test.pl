#!/usr/bin/perl
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
#
# Test Perl Clock Manager client
#
# @author Christopher Hall <christopher.s.hall@@intel.com>
# @copyright © 2024 Intel Corporation.
#
# @note This is a sample code, not a product! You should use it as a reference.
#
# To run in development environment:
# LD_PRELOAD=../../.libs/libclkmgr.so PERL5LIB=. ./clkmgr_test.pl
#
###############################################################################

use threads;
use threads::shared;
use File::Basename;
use POSIX;
use Time::HiRes;
use Getopt::Std;
use ClkMgrLib;

my $signal_flag :shared = 0;

sub signal_handler()
{
    my($sig) = @_;
    print " Exit ...\n";
    $signal_flag = 1;
}

sub isPositiveValue
{
    my ($optarg, $errorMessage) = @_;
    my $ret = int($optarg);
    die $errorMessage unless $ret > 0;
    $ret;
}

sub ClockManagerGetTime
{
    my $ts = ClkMgrLib::timespec->new;
    if(ClkMgrLib::ClockManager::getTime($ts)) {
        printf "[clkmgr] Current Time of CLOCK_REALTIME: %ld ns\n",
                ($ts->swig_tv_sec_get * 1000000000) + $ts->swig_tv_nsec_get;
    } else {
        POSIX::perror('clock_gettime failed');
    }
}

sub getMonotonicTime
{
    Time::HiRes::clock_gettime(Time::HiRes::CLOCK_MONOTONIC);
}

sub main
{
    my $ptp4lClockOffsetThreshold = 100000;
    my $chronyClockOffsetThreshold = 100000;
    my $subscribeAll = 0;
    my $userInput = 0;
    my $idleTime = 1;
    my $timeout = 10;
    my @index; # Array of indexes to subscribe

    my $event2Sub = $ClkMgrLib::EventGMOffset | $ClkMgrLib::EventSyncedToGM |
        $ClkMgrLib::EventASCapable | $ClkMgrLib::EventGMChanged;
    my $composite_event = $ClkMgrLib::EventGMOffset |
        $ClkMgrLib::EventSyncedToGM | $ClkMgrLib::EventASCapable;

    my $clockSyncData = ClkMgrLib::ClockSyncData->new;

    my $ptp4lSub = ClkMgrLib::PTPClockSubscription->new;
    my $chronySub = ClkMgrLib::SysClockSubscription->new;

    my @overallSub; # Array of ClkMgrLib::ClockSyncSubscription

    my $ret = getopts('aps:c:l:i:t:m:h');
    if($opt_h || !$ret) {
        my $event2SubHex = sprintf '0x%x', $event2Sub;
        my $composite_eventHex = sprintf '0x%x', $composite_event;
        my $name = basename $0;
        my $help = <<"END_MESSAGE";
Usage of $name :
Options:
  -a subscribe to all time base indices
     Default: timeBaseIndex: 1
  -p enable user to subscribe to specific time base indices
  -s subscribe_event_mask
     Default: $event2SubHex
     Bit 0: EventGMOffset
     Bit 1: EventSyncedToGM
     Bit 2: EventASCapable
     Bit 3: EventGMChanged
  -c composite_event_mask
     Default: $composite_eventHex
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
END_MESSAGE
        $ret ? print($help) : die $help;
        return;
    }
    $subscribeAll = $opt_a ? 1 : 0;
    $userInput = $subscribeAll ? 0 : ($opt_p ? 1 : 0);
    $event2Sub = POSIX::strtoul($opt_s, 0) if $opt_s;
    $composite_event = POSIX::strtoul($opt_c, 0) if $opt_c;
    $ptp4lClockOffsetThreshold =
        isPositiveValue($opt_l, 'Invalid ptp4l GM Offset threshold!') if $opt_l;
    $idleTime = isPositiveValue($opt_i, 'Invalid idle time!') if $opt_i;
    $timeout = isPositiveValue($opt_t, 'Invalid timeout!') if $opt_t;
    $chronyClockOffsetThreshold =
        isPositiveValue($opt_m, 'Invalid Chrony Offset threshold!') if $opt_m;
    $ptp4lSub->setEventMask($event2Sub);
    $ptp4lSub->setClockOffsetThreshold($ptp4lClockOffsetThreshold);
    $ptp4lSub->setCompositeEventMask($composite_event);
    $chronySub->setClockOffsetThreshold($chronyClockOffsetThreshold);
    printf "[clkmgr] set subscribe event : 0x%x\n", $ptp4lSub->getEventMask();
    printf "[clkmgr] set composite event : 0x%x\n".
        $ptp4lSub->getCompositeEventMask();
    print "GM Offset threshold: $ptp4lClockOffsetThreshold ns\n";
    print "Chrony Offset threshold: $chronyClockOffsetThreshold ns\n";

    if($userInput) {
        print 'Enter the time base indices to subscribe ' .
              '(comma-separated, default is 1): ';
        my $line = <STDIN>;
        chomp $line;
        unless($line =~ /^$/) {
            my $idx = int($line);
            die 'Invalid time base index: $line!' unless $idx > 0;
            push(@index, $idx);
        } else {
            warn "Invalid input. Using default time base index 1.\n";
        }
    }
    $SIG{'INT'} = \&signal_handler;
    $SIG{'TERM'} = \&signal_handler;
    $SIG{'HUP'} = \&signal_handler;

    my $dieMsg;
    unless(ClkMgrLib::ClockManager::connect()) {
        $dieMsg = '[clkmgr] failure in connecting !!!';
        goto do_exit
    }
    sleep 1;
    print "[clkmgr] List of available clock:\n";
    # Print out each member of the Time Base configuration
    my $size = ClkMgrLib::TimeBaseConfigurations::size();
    for(my $i = 1; $i <= $size; $i++) {
        my $cfg = ClkMgrLib::TimeBaseConfigurations::getRecord($i);
        my $idx = $cfg->index();
        print "TimeBaseIndex: $idx\n";
        print 'timeBaseName: ' . $cfg->name() . "\n";
        my $Subscribe = ClkMgrLib::ClockSyncSubscription->new;
        if($cfg->havePtp()) {
            my $ptp = $cfg->ptp();
            print 'interfaceName: ' . $ptp->ifName() . "\n";
            print 'transportSpecific: ' . $ptp->transportSpecific() . "\n";
            print 'domainNumber: ' . $ptp->domainNumber() . "\n";
            $Subscribe->setPtpSubscription($ptp4lSub);
        }
        $Subscribe->setSysSubscription($chronySub) if $cfg->haveSysClock();
        $overallSub[$idx] = $Subscribe;
        print "\n";
        push(@index, $idx) if $subscribeAll;
    }

    # Default
    push(@index, 1) if 0 == scalar @index;

    my $hd2 = '|'.('-'x 27).'|'.('-'x 24).'|';
    for(@index) {
        my $idx = $_;
        print "Subscribe to time base index: $idx\n";
        unless(ClkMgrLib::ClockManager::subscribe($overallSub[$idx],
            $idx, $clockSyncData)) {
            $dieMsg = '[clkmgr] Failure in subscribing to clkmgr Proxy !!!';
            goto do_exit;
        }
        printf "[clkmgr][%.3f] Obtained data from Subscription Event:\n",
            getMonotonicTime;
        ClockManagerGetTime;
        printf "$hd2\n" .
               "| %-25s | %-22s |\n", 'Event', 'Event Status';
        my $ptpClock = $clockSyncData->getPtp();
        my $sysClock = $clockSyncData->getSysClock();
        if ($event2Sub != 0) {
            print "$hd2\n";
            printf "| %-25s | %-22d |\n",
                'offset_in_range', $ptpClock->isOffsetInRange()
                if $event2Sub & $ClkMgrLib::EventGMOffset;
            printf "| %-25s | %-22d |\n", 'synced_to_primary_clock',
                $ptpClock->isSyncedWithGm()
                if $event2Sub & $ClkMgrLib::EventSyncedToGM;
            printf "| %-25s | %-22d |\n", 'as_capable', $ptpClock->isAsCapable()
                if $event2Sub & $ClkMgrLib::EventASCapable;
            printf "| %-25s | %-22d |\n", 'gm_Changed', $ptpClock->isGmChanged()
                if $event2Sub & $ClkMgrLib::EventGMChanged;
        }
        print "$hd2\n";
        my $gmClockUUID = $ptpClock->getGmIdentity();
        my @gm_identity;
        # Copy the uint64_t into the array
        for (my $i = 0; $i < 8; $i++) {
            $gm_identity[$i] = ($gmClockUUID >> (8 * (7 - $i))) & 0xff;
        }
        printf "| %-25s | %02x%02x%02x.%02x%02x.%02x%02x%02x     |\n",
            'GM UUID',
            $gm_identity[0], $gm_identity[1],
            $gm_identity[2], $gm_identity[3],
            $gm_identity[4], $gm_identity[5],
            $gm_identity[6], $gm_identity[7];
        printf "| %-25s | %-19ld ns |\n" .
               "| %-25s | %-19ld ns |\n" .
               "| %-25s | %-19ld us |\n" .
               "$hd2\n"
               , 'clock_offset', $ptpClock->getClockOffset()
               , 'notification_timestamp', $ptpClock->getNotificationTimestamp()
               , 'gm_sync_interval', $ptpClock->getSyncInterval();
        if ($composite_event != 0) {
            printf "| %-25s | %-22d |\n", 'composite_event',
                $ptpClock->isCompositeEventMet();
            printf "| - %-23s | %-22s |\n", 'offset_in_range', ''
                if $composite_event & $ClkMgrLib::EventGMOffset;
            printf "| - %-19s | %-22s |\n", 'synced_to_primary_clock', ''
                if $composite_event & $ClkMgrLib::EventSyncedToGM;
            printf "| - %-23s | %-22s |\n", 'as_capable', ''
                if $composite_event & $ClkMgrLib::EventASCapable;
            print "$hd2\n\n";
        } else {
            print "\n";
        }
        printf "$hd2\n" .
               "| %-25s | %-22d |\n" .
               "$hd2\n" .
               "| %-25s | %-19ld ns |\n" .
               "| %-25s | %-19lx    |\n" .
               "| %-25s | %-19ld us |\n" .
               "$hd2\n\n"
               , 'chrony_offset_in_range', $sysClock->isOffsetInRange()
               , 'chrony_clock_offset', $sysClock->getClockOffset()
               , 'chrony_clock_reference_id', $sysClock->getGmIdentity()
               , 'chrony_polling_interval', $sysClock->getSyncInterval();
    }
    sleep 1;

    my $hd2l = '|'.('-'x 27).'|'.('-'x 28).'|';
    my $hd3b = '| %-25s | %-12d | %-11d |';
    my $hd3 = '|'.('-'x 27).'|'.('-'x 14).'|'.('-'x 13).'|';

    while (!$signal_flag) {
        inner_loop: for(@index) {
            goto do_exit if $signal_flag;
            my $idx = $_;
            printf '[clkmgr][%.3f] Waiting Notification ' .
                   "from time base index $idx ...\n", getMonotonicTime;

            my $retval =
                ClkMgrLib::ClockManager::statusWait($timeout, $idx, $clockSyncData);
            if($retval == $ClkMgrLib::SWRLostConnection) {
                printf '[clkmgr][%.3f] Terminating: lost connection to ' .
                    "clkmgr Proxy\n", getMonotonicTime;
                goto do_exit;
            } elsif($retval == $ClkMgrLib::SWRInvalidArgument) {
                printf "[clkmgr][%.3f] Terminating: Invalid argument\n",
                    getMonotonicTime;
                goto do_exit;
            } elsif($retval == $ClkMgrLib::SWRNoEventDetected) {
                printf '[clkmgr][%.3f] No event status changes identified ' .
                    "in %d seconds.\n\n", getMonotonicTime, $timeout;
                printf "[clkmgr][%.3f] sleep for %d seconds...\n\n",
                    getMonotonicTime, $idleTime;
                goto do_exit if $signal_flag;
                sleep $idleTime;
                next inner_loop;
            } elsif($retval == $ClkMgrLib::SWREventDetected) {
                printf "[clkmgr][%.3f] Obtained data from Notification Event:\n",
                    getMonotonicTime;
            } else {
                printf '[clkmgr][%.3f] Warning: Should not enter this switch ' .
                    "case, unexpected status code %d\n", getMonotonicTime, $retval;
                goto do_exit;
            }
            ClockManagerGetTime;
            printf "$hd3\n" .
                "| %-25s | %-12s | %-11s |\n",
                'Event', 'Event Status', 'Event Count';
            my $ptpClock = $clockSyncData->getPtp();
            my $sysClock = $clockSyncData->getSysClock();
            if ($event2Sub != 0) {
                print "$hd3\n";
                printf "$hd3b\n", 'offset_in_range',
                    $ptpClock->isOffsetInRange(),
                    $ptpClock->getOffsetInRangeEventCount()
                    if $event2Sub & $ClkMgrLib::EventGMOffset;
                printf "$hd3b\n", 'synced_to_primary_clock',
                    $ptpClock->isSyncedWithGm(),
                    $ptpClock->getSyncedWithGmEventCount()
                    if $event2Sub & $ClkMgrLib::EventSyncedToGM;
                printf "$hd3b\n", 'as_capable',
                    $ptpClock->isAsCapable(), $ptpClock->getAsCapableEventCount()
                    if $event2Sub & $ClkMgrLib::EventASCapable;
                printf "$hd3b\n", 'gm_Changed',
                    $ptpClock->isGmChanged(), $ptpClock->getGmChangedEventCount()
                    if $event2Sub & $ClkMgrLib::EventGMChanged;
            }
            print "$hd3\n";

            my $gmClockUUID = $ptpClock->getGmIdentity();
            my @gm_identity;
            # Copy the uint64_t into the array
            for (my $i = 0; $i < 8; $i++) {
                $gm_identity[$i] = ($gmClockUUID >> (8 * (7 - $i))) & 0xff;
            }
            printf "| %-25s |     %02x%02x%02x.%02x%02x.%02x%02x%02x     |\n",
                'GM UUID',
                $gm_identity[0], $gm_identity[1],
                $gm_identity[2], $gm_identity[3],
                $gm_identity[4], $gm_identity[5],
                $gm_identity[6], $gm_identity[7];
            printf "| %-25s |     %-19ld ns |\n" .
                   "| %-25s |     %-19ld ns |\n" .
                   "| %-25s |     %-19ld us |\n" .
                   "$hd3\n"
                   , 'clock_offset', $ptpClock->getClockOffset()
                   , 'notification_timestamp', $ptpClock->getNotificationTimestamp()
                   , 'gm_sync_interval', $ptpClock->getSyncInterval();
            if ($composite_event != 0) {
                printf "$hd3b\n", 'composite_event',
                    $ptpClock->isCompositeEventMet(),
                    $ptpClock->getCompositeEventCount();
                printf "| - %-23s | %-12s | %-11s |\n", 'offset_in_range', '', ''
                    if $composite_event & $ClkMgrLib::EventGMOffset;
                printf "| - %-19s | %-12s | %-11s |\n",
                    'synced_to_primary_clock', '', ''
                    if $composite_event & $ClkMgrLib::EventSyncedToGM;
                printf "| - %-23s | %-12s | %-11s |\n", 'as_capable', '', ''
                    if $composite_event & $ClkMgrLib::EventASCapable;
                print "$hd3\n\n";
            } else {
                print "\n";
            }
            printf "$hd2l\n" .
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
                   , 'chrony_polling_interval', $sysClock->getSyncInterval();
            printf "[clkmgr][%.3f] sleep for %d seconds...\n\n",
                getMonotonicTime, idleTime;
            goto do_exit if $signal_flag;
            sleep $idleTime;
        }
    }

do_exit:

    ClkMgrLib::ClockManager::disconnect();
    die $dieMsg if $dieMsg;
}
main;
