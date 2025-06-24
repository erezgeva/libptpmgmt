#!/usr/bin/ruby
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
#
# Test Ruby Clock Manager client
#
# @author Christopher Hall <christopher.s.hall@@intel.com>
# @copyright © 2024 Intel Corporation.
#
# @note This is a sample code, not a product! You should use it as a reference.
#
# To run in development environment
# LD_PRELOAD=../../.libs/libclkmgr.so RUBYLIB=. ./clkmgr_test.rb
#
###############################################################################

require 'getoptlong'
require 'clkmgr'

# Use to capture signal break
$signal_flag = false
# Parameters that can be set with command line
$ptp4lClockOffsetThreshold = 100000
$chronyClockOffsetThreshold = 100000
$subscribeAll = false
$idleTime = 1
$timeout = 10
$event2Sub = (Clkmgr::EventGMOffset | Clkmgr::EventSyncedToGM |
    Clkmgr::EventASCapable | Clkmgr::EventGMChanged)
$composite_event = (Clkmgr::EventGMOffset | Clkmgr::EventSyncedToGM |
    Clkmgr::EventASCapable)
# Subscribe data
$ptp4lSub = Clkmgr::PTPClockSubscription.new
$chronySub = Clkmgr::SysClockSubscription.new
# Array of indexes to subscribe
$index = Array.new

def isPositiveValue(optarg, errorMessage)
    ret = optarg.to_i
    if ret <= 0 then
        puts errorMessage
        exit 2
    end
    return ret
end

def clockManagerGetTime
    ts = Clkmgr::Timespec.new
    if Clkmgr::ClockManager.getTime(ts) then
        puts "[Clkmgr] Current Time of CLOCK_REALTIME: #{(ts.tv_sec * 1000000000) +
            ts.tv_nsec} ns"
    else
        puts 'clock_gettime failed'
    end
end

def getMonotonicTime
    return Process.clock_gettime(Process::CLOCK_MONOTONIC)
end

def usage
    puts <<-EOF
Usage of #{File.basename($0)} :
Options
  -a subscribe to all time base indices
     Default: timeBaseIndex: 1
  -p enable user to subscribe to specific time base indices
  -s subscribe_event_mask
     Default: 0x#{$event2Sub.to_s(16)}
     Bit 0: EventGMOffset
     Bit 1: EventSyncedToGM
     Bit 2: EventASCapable
     Bit 3: EventGMChanged
  -c composite_event_mask
     Default: 0x#{$composite_event.to_s(16)}
     Bit 0: EventGMOffset
     Bit 1: EventSyncedToGM
     Bit 2: EventASCapable
  -l gm offset threshold (ns)
     Default: #{$ptp4lClockOffsetThreshold} ns
  -i idle time (s)
     Default: #{$idleTime} s
  -m chrony offset threshold (ns)
     Default: #{$chronyClockOffsetThreshold} ns
  -t $timeout in waiting notification event (s)
     Default: #{$timeout} s
EOF
end

def main
    userInput = false
    opts = GetoptLong.new(
      [ '--a', '-a', GetoptLong::NO_ARGUMENT ],
      [ '--p', '-p', GetoptLong::NO_ARGUMENT ],
      [ '--s', '-s', GetoptLong::REQUIRED_ARGUMENT ],
      [ '--c', '-c', GetoptLong::REQUIRED_ARGUMENT ],
      [ '--l', '-l', GetoptLong::REQUIRED_ARGUMENT ],
      [ '--i', '-i', GetoptLong::REQUIRED_ARGUMENT ],
      [ '--t', '-t', GetoptLong::REQUIRED_ARGUMENT ],
      [ '--m', '-m', GetoptLong::REQUIRED_ARGUMENT ],
      [ '--h', '-h', GetoptLong::NO_ARGUMENT ],
    )
    begin
        opts.each do |opt, arg|
          case opt
            when '--h'
                usage
                return
            when '--a'
                $subscribeAll = true
            when '--p'
                userInput = true
            when '--s'
                $event2Sub = arg.to_i(0)
            when '--c'
                $composite_event = arg.to_i(0)
            when '--l'
                $ptp4lClockOffsetThreshold = isPositiveValue(arg,
                    'Invalid ptp4l GM Offset threshold!')
            when '--i'
                $idleTime = isPositiveValue(arg, 'Invalid idle time!')
            when '--t'
                $timeout = isPositiveValue(arg, 'Invalid $timeout!')
            when '--m'
                $chronyClockOffsetThreshold = isPositiveValue(arg,
                    'Invalid Chrony Offset threshold!')
          end
        end
    rescue
        usage
        exit 2
    end
    $ptp4lSub.setEventMask($event2Sub)
    $ptp4lSub.setClockOffsetThreshold($ptp4lClockOffsetThreshold)
    $ptp4lSub.setCompositeEventMask($composite_event)
    $chronySub.setClockOffsetThreshold($chronyClockOffsetThreshold)
    puts '[Clkmgr] set subscribe event : 0x' + $ptp4lSub.getEventMask().to_s(16)
    puts '[Clkmgr] set composite event : 0x' +
        $ptp4lSub.getCompositeEventMask().to_s(16)
    puts "GM Offset threshold: #{$ptp4lClockOffsetThreshold} ns"
    puts "Chrony Offset threshold: #{$chronyClockOffsetThreshold} ns"
    if userInput and !$subscribeAll then
        puts 'test input'
        print 'Enter the time base indices to subscribe ' +
            '(comma-separated, default is 1): '
        line = gets.chomp
        if line.length > 0 then
            idx = line.to_i
            if idx <= 0 then
                puts 'Invalid time base index: ' + line
                exit 2
            end
            $index.push(idx)
        else
            puts 'Invalid input. Using default time base index 1.'
        end
    end
    Signal.trap('INT') do
        puts ' Exit ...'
        $signal_flag = true
    end
    Signal.trap('TERM') do
        puts ' Exit ...'
        $signal_flag = true
    end
    Signal.trap('HUP') do
        puts ' Exit ...'
        $signal_flag = true
    end
    ret = main_body()
    Clkmgr::ClockManager.disconnect()
    exit 2 if ret > 0
end

def main_body
    clockSyncData = Clkmgr::ClockSyncData.new
    if !Clkmgr::ClockManager.connect() then
        puts '[Clkmgr] failure in connecting !!!'
        return 2
    end
    sleep 1
    puts '[Clkmgr] List of available clock:'
    # Print out each member of the Time Base configuration
    size = Clkmgr::TimeBaseConfigurations.size()
    overallSub = Array.new # Array of Clkmgr::ClockSyncSubscription
    (1..size).each do |i|
        cfg = Clkmgr::TimeBaseConfigurations.getRecord(i)
        idx = cfg.index()
        puts "TimeBaseIndex: #{idx}"
        puts 'timeBaseName: ' + cfg.name()
        subscribe = Clkmgr::ClockSyncSubscription.new
        if cfg.havePtp() then
            ptp = cfg.ptp()
            puts 'interfaceName: ' + ptp.ifName()
            puts "transportSpecific: #{ptp.transportSpecific()}"
            puts "domainNumber: #{ptp.domainNumber()}"
            subscribe.setPtpSubscription($ptp4lSub)
        end
        subscribe.setSysSubscription($chronySub) if cfg.haveSysClock()
        overallSub[idx] = subscribe
        $index.push(idx) if $subscribeAll
    end
    # Default
    $index.push(1) if $index.length == 0
    gm_identity = Array.new(8)
    hd2 = '|'+('-'*27)+'|'+('-'*24)+'|'
    hd2b = '| %-25s | %-22s |'
    $index.each do |idx|
        puts "Subscribe to time base index: #{idx}"
        if !Clkmgr::ClockManager.subscribe(overallSub[idx], idx, clockSyncData) then
            puts '[Clkmgr] Failure in subscribing to Clkmgr Proxy !!!'
            return 2
        end
        puts '[Clkmgr][%.3f] Obtained data from Subscription Event:' %
            getMonotonicTime
        clockManagerGetTime
        puts hd2
        puts hd2b % ['Event', 'Event Status']
        ptpClock = clockSyncData.getPtp()
        sysClock = clockSyncData.getSysClock()
        if $event2Sub != 0 then
            puts hd2
            puts hd2b % ['offset_in_range',
                ptpClock.isOffsetInRange().to_s] if $event2Sub &
                Clkmgr::EventGMOffset
            puts hd2b % ['synced_to_primary_clock',
                ptpClock.isSyncedWithGm().to_s] if $event2Sub &
                Clkmgr::EventSyncedToGM
            puts hd2b % ['as_capable',
                ptpClock.isAsCapable().to_s] if $event2Sub & Clkmgr::EventASCapable
            puts hd2b % ['gm_Changed',
                ptpClock.isGmChanged().to_s] if $event2Sub & Clkmgr::EventGMChanged
        end
        puts hd2
        gmClockUUID = ptpClock.getGmIdentity()
        # Copy the uint64_t into the array
        (0..7).each do |i|
            gm_identity[i] = (gmClockUUID >> (8 * (7 - i))) & 0xff
        end
        puts '| %-25s | %02x%02x%02x.%02x%02x.%02x%02x%02x     |' % ['GM UUID',
            gm_identity[0], gm_identity[1],
            gm_identity[2], gm_identity[3],
            gm_identity[4], gm_identity[5],
            gm_identity[6], gm_identity[7]]
        puts '| %-25s | %-19d ns |' % ['clock_offset', ptpClock.getClockOffset()]
        puts '| %-25s | %-19d ns |' % ['notification_timestamp',
            ptpClock.getNotificationTimestamp()]
        puts '| %-25s | %-19d us |' % ['gm_sync_interval',
            ptpClock.getSyncInterval()]
        puts hd2
        if $composite_event != 0 then
            puts '| %-25s | %-22s |' % ['$composite_event',
                ptpClock.isCompositeEventMet().to_s]
            puts '| - %-23s | %-22s |' % ['offset_in_range',
                ''] if $composite_event & Clkmgr::EventGMOffset
            puts '| - %-19s | %-22s |' % ['synced_to_primary_clock',
                ''] if $composite_event & Clkmgr::EventSyncedToGM
            puts '| - %-23s | %-22s |' % ['as_capable', ''] if $composite_event &
                Clkmgr::EventASCapable
            puts hd2
        end
        puts
        puts hd2
        puts '| %-25s | %-22s |' % ['chrony_offset_in_range',
            sysClock.isOffsetInRange().to_s]
        puts hd2
        puts '| %-25s | %-19d ns |' % ['chrony_clock_offset',
            sysClock.getClockOffset()]
        puts '| %-25s | %-19x    |' % ['chrony_clock_reference_id',
            sysClock.getGmIdentity()]
        puts '| %-25s | %-19d us |' % ['chrony_polling_interval',
            sysClock.getSyncInterval()]
        puts hd2
        puts
    end

    sleep 1

    hd2l = '|'+('-'* 27)+'|'+('-'* 28)+'|'
    hd3b = '| %-25s | %-12s | %-11d |'
    hd3 = '|'+('-'* 27)+'|'+('-'* 14)+'|'+('-'* 13)+'|'

    while !$signal_flag
        $index.each do |idx|
            return 0 if $signal_flag
            puts '[Clkmgr][%.3f] Waiting Notification from ' +
                "time base index #{idx} ..." % getMonotonicTime
            retval = Clkmgr::ClockManager.statusWait($timeout, idx, clockSyncData)
            case retval
                when Clkmgr::SWRLostConnection
                    puts '[Clkmgr][%.3f] Terminating: ' +
                        'lost connection to Clkmgr Proxy' % getMonotonicTime
                    return 0
                when Clkmgr::SWRInvalidArgument
                    puts '[clkmgr][%.3f] Terminating: Invalid argument' %
                        getMonotonicTime
                    return 0
                when Clkmgr::SWRNoEventDetected
                    puts '[Clkmgr][%.3f] No event status changes ' +
                        "identified in #{$timeout} seconds." % getMonotonicTime
                    puts
                    puts "[Clkmgr][%.3f] sleep for #{$idleTime} seconds..." %
                        getMonotonicTime
                when Clkmgr::SWREventDetected
                    puts '[Clkmgr][%.3f] Obtained data from Notification Event:' %
                        getMonotonicTime
                    clockManagerGetTime
                    puts hd3
                    puts '| %-25s | %-12s | %-11s |' % ['Event', 'Event Status',
                        'Event Count']
                    ptpClock = clockSyncData.getPtp()
                    sysClock = clockSyncData.getSysClock()
                    if $event2Sub != 0 then
                        puts hd3
                        puts hd3b % ['offset_in_range',
                            ptpClock.isOffsetInRange().to_s,
                            ptpClock.getOffsetInRangeEventCount()] if $event2Sub &
                            Clkmgr::EventGMOffset
                        puts hd3b % ['synced_to_primary_clock',
                            ptpClock.isSyncedWithGm().to_s,
                            ptpClock.getSyncedWithGmEventCount()] if $event2Sub &
                            Clkmgr::EventSyncedToGM
                        puts hd3b % ['as_capable', ptpClock.isAsCapable().to_s,
                            ptpClock.getAsCapableEventCount()] if $event2Sub &
                            Clkmgr::EventASCapable
                        puts hd3b % ['gm_Changed', ptpClock.isGmChanged().to_s,
                            ptpClock.getGmChangedEventCount()] if $event2Sub &
                            Clkmgr::EventGMChanged
                    end
                    puts hd3
                    gmClockUUID = ptpClock.getGmIdentity()
                    # Copy the uint64_t into the array
                    (0..7).each do |i|
                        gm_identity[i] = (gmClockUUID >> (8 * (7 - i))) & 0xff
                    end
                    puts '| %-25s |     %02x%02x%02x.%02x%02x.%02x%02x%02x     |' %
                        ['GM UUID',
                        gm_identity[0], gm_identity[1],
                        gm_identity[2], gm_identity[3],
                        gm_identity[4], gm_identity[5],
                        gm_identity[6], gm_identity[7]]
                    puts '| %-25s |     %-19d ns |' %
                        ['clock_offset', ptpClock.getClockOffset()]
                    puts '| %-25s |     %-19d ns |' %
                        ['notification_timestamp',
                        ptpClock.getNotificationTimestamp()]
                    puts '| %-25s |     %-19d us |' %
                        ['gm_sync_interval', ptpClock.getSyncInterval()]
                    puts hd3
                    if $composite_event != 0 then
                        puts hd3b % ['$composite_event',
                            ptpClock.isCompositeEventMet(),
                            ptpClock.getCompositeEventCount()]
                        puts '| - %-23s | %-12s | %-11s |' %
                            ['offset_in_range', '',
                            ''] if $composite_event & Clkmgr::EventGMOffset
                        puts '| - %-19s | %-12s | %-11s |' %
                            ['synced_to_primary_clock', '',
                            ''] if $composite_event & Clkmgr::EventSyncedToGM
                        puts '| - %-23s | %-12s | %-11s |' %
                            ['as_capable', '', ''] if $composite_event &
                            Clkmgr::EventASCapable
                        puts hd3
                    end
                    puts
                    puts hd2l
                    puts hd3b % ['chrony_offset_in_range',
                        sysClock.isOffsetInRange(),
                        sysClock.getOffsetInRangeEventCount()]
                    puts hd2l
                    puts '| %-25s |     %-19d ns |' %
                        ['chrony_clock_offset', sysClock.getClockOffset()]
                    puts '| %-25s |     %-19x    |' %
                        ['chrony_clock_reference_id', sysClock.getGmIdentity()]
                    puts '| %-25s |     %-19d us |' %
                        ['chrony_polling_interval', sysClock.getSyncInterval()]
                    puts hd2l
                    puts
                    puts "[Clkmgr][%.3f] sleep for #{$idleTime} seconds..." %
                        getMonotonicTime
                else
                    puts '[clkmgr][%.3f] Warning: Should not enter ' +
                        "this switch when, unexpected status code #{retval}" %
                        getMonotonicTime
                    return 0
             end # case retval
             puts
             return 0 if $signal_flag
             sleep $idleTime
        end # $index.each do
    end # while !$signal_flag
    return 0
end
main
