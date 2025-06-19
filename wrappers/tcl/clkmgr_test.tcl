#!/usr/bin/tclsh
# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
#
# Test Tcl Clock Manager client
#
# @author Christopher Hall <christopher.s.hall@@intel.com>
# @copyright © 2024 Intel Corporation.
#
# @note This is a sample code, not a product! You should use it as a reference.
#
# To run in development environment:
# LD_PRELOAD=../../.libs/libclkmgr.so TCLLIBPATH=. ./clkmgr_test.tcl
#
# Run ./pkgIndex_tcl.sh to create pkgIndex.tcl
#
# This code also need: tcllib, tclx
#
###############################################################################

package require Tclx
package require cmdline
package require clkmgr

set signal_flag 0
set index [list]
clkmgr::ClockSyncData clockSyncData
clkmgr::PTPClockSubscription ptp4lSub
clkmgr::SysClockSubscription chronySub

proc signal_handler {} {
    global signal_flag
    puts " Exit ..."
    set signal_flag 1
}

proc isPositiveValue { optarg errorMessage } {
    set ret [expr $optarg ]
    if { $ret > 0 } {
        return $ret
    }
    puts $errorMessage
    exit 2
}

proc ClockManagerGetTime {} {
    set ts [ clkmgr::timespec ]
    if { [ clkmgr::ClockManager_getTime $ts ] } {
        puts [format "\[clkmgr] Current Time of CLOCK_REALTIME: %ld ns" \
            [expr ( [ $ts cget -tv_sec ] * 1000000000 ) +\
            [ $ts cget -tv_nsec ]]]
    } else {
        puts "clock_gettime failed"
    }
}

proc main {} {
    global argv argv0 index subscribeAll event2Sub composite_event\
        clockSyncData ptp4lSub chronySub idleTime timeout
    # index list of indexes to subscribe
    set event2Sub [expr $clkmgr::EventGMOffset | $clkmgr::EventSyncedToGM |\
        $clkmgr::EventASCapable | $clkmgr::EventGMChanged ]
    set composite_event [expr $clkmgr::EventGMOffset |\
        $clkmgr::EventSyncedToGM | $clkmgr::EventASCapable ]
    set options {
        {h}
        {a}
        {p}
        # event2Sub
        {s.arg "" }
        # composite_event
        {c.arg "" }
        # ptp4lClockOffsetThreshold
        {l.arg 100000}
        # idleTime
        {i.arg 1}
        # chronyClockOffsetThreshold
        {m.arg 100000}
        # timeout
        {t.arg 10}
    }
    try {
        array set params [::cmdline::getoptions argv $options "" ]
    } trap {CMDLINE USAGE} {} {
        puts "Invalid option $argv"
        set help 1
    }
    if { [ info exist help ] || $params(h) } {
    puts "
Usage of $argv0:
Options:
  -a subscribe to all time base indices
     Default: timeBaseIndex: 1
  -p enable user to subscribe to specific time base indices
  -s subscribe_event_mask
     Default: 0x[format %x $event2Sub ]
     Bit 0: EventGMOffset
     Bit 1: EventSyncedToGM
     Bit 2: EventASCapable
     Bit 3: EventGMChanged
  -c composite_event_mask
     Default: 0x[format %x $composite_event ]
     Bit 0: EventGMOffset
     Bit 1: EventSyncedToGM
     Bit 2: EventASCapable
  -l gm offset threshold (ns)
     Default: 100000 ns
  -i idle time (s)
     Default: 1 s
  -m chrony offset threshold (ns)
     Default: 100000 ns
  -t timeout in waiting notification event (s)
     Default: 10 s"
        if { [ info exist help ] } {
            exit 2
        }
        return
    }
    set subscribeAll $params(a)
    set userInput [expr $params(p) && !$subscribeAll ]
    if { [string length $params(s)] > 0} {
        try {
            set event2Sub [expr $params(s) ]
        } trap {} {} {
            puts "use "-s" with a number"
            exit 2
        }
    }
    if { [string length $params(c)] > 0} {
        try {
            set composite_event [expr $params(c) ]
        } trap {} {} {
            puts "use "-c" with a number"
            exit 2
        }
    }
    set ptp4lClockOffsetThreshold [ isPositiveValue $params(l)\
        "Invalid ptp4l GM Offset threshold!" ]
    set idleTime [ isPositiveValue $params(i) "Invalid idle time!" ]
    set timeout [ isPositiveValue $params(t) "Invalid timeout!" ]
    set chronyClockOffsetThreshold [ isPositiveValue $params(m)\
        "Invalid Chrony Offset threshold!" ]
    signal trap SIGINT signal_handler
    signal trap SIGTERM signal_handler
    signal trap SIGHUP signal_handler
    ptp4lSub setEventMask $event2Sub
    ptp4lSub setClockOffsetThreshold $ptp4lClockOffsetThreshold
    ptp4lSub setCompositeEventMask $composite_event
    chronySub setClockOffsetThreshold $chronyClockOffsetThreshold
    puts [format "\[clkmgr] set subscribe event : 0x%x"\
        [ ptp4lSub getEventMask ]]
    puts [format "\[clkmgr] set composite event : 0x%x"\
        [ ptp4lSub getCompositeEventMask ]]
    puts "GM Offset threshold: $ptp4lClockOffsetThreshold ns"
    puts "Chrony Offset threshold: $chronyClockOffsetThreshold ns"
    if { $userInput } {
        puts -nonewline "Enter the time base indices to subscribe\
            comma-separated, default is 1 : "
        flush stdout
        set line [ gets stdin ]
        if { [string length $line] == 0} {
            puts "Invalid input. Using default time base index 1."
        } else {
            try {
                set idx [expr $line ]
            } trap {} {} {
                puts "$line is not a number"
                exit 2
            }
            lappend index $idx
        }
    }
    set ret [ main_body ]
    clkmgr::ClockManager_disconnect
    if { $ret > 0 } {
        exit 2
    }
}

proc main_body {} {
    global signal_flag index subscribeAll event2Sub composite_event\
           clockSyncData ptp4lSub chronySub idleTime timeout
    # overallSub # Array of clkmgr::ClockSyncSubscription
    if { ! [ clkmgr::ClockManager_connect ] } {
        puts "\[clkmgr] failure in connecting !!!"
        return 2
    }
    sleep 1
    puts "\[clkmgr] List of available clock:"
    # Print out each member of the Time Base configuration
    set size [ clkmgr::TimeBaseConfigurations_size ]
     for {set i 1} {$i <= $size} {incr i} {
        set cfg [ clkmgr::TimeBaseConfigurations_getRecord $i ]
        set idx [ $cfg index ]
        puts "TimeBaseIndex: $idx"
        puts "timeBaseName: [ $cfg name ]"
        set subscribe [ clkmgr::ClockSyncSubscription ]
        if { [ $cfg havePtp ] } {
            set ptp [ $cfg ptp ]
            puts "interfaceName: [ $ptp ifName ]"
            puts "transportSpecific: [ $ptp transportSpecific ]"
            puts "domainNumber: [ $ptp domainNumber ]"
            $subscribe setPtpSubscription ptp4lSub
        }
        if { [ $cfg haveSysClock ] } {
            $subscribe setSysSubscription chronySub
        }
        set overallSub($idx) $subscribe
        puts ""
        if { $subscribeAll } {
            lappend index $idx
        }
    }

    # Default
    if { [ llength $index ] == 0 } {
        lappend index 1
    }

    set hd2 "|[string repeat - 27]|[string repeat - 24]|"
    foreach idx $index {
        puts "subscribe to time base index: $idx"
        if { ! [ clkmgr::ClockManager_subscribe $overallSub($idx)\
            $idx clockSyncData ] } {
            puts "\[clkmgr] Failure in subscribing to clkmgr Proxy !!!"
            return 2
        }
        puts [format "\[clkmgr]\[%.3f] Obtained data from %s"\
            [expr [clock milliseconds ] / 1000.0 ]\
            "Subscription Event:"]
        ClockManagerGetTime
        puts $hd2
        puts [format "| %-25s | %-22s |" "Event" "Event Status" ]
        set ptpClock [ clockSyncData getPtp ]
        set sysClock [ clockSyncData getSysClock ]
        if { $event2Sub != 0 } {
            puts $hd2
            if { $event2Sub & $clkmgr::EventGMOffset } {
                puts [format "| %-25s | %-22d |" "offset_in_range"\
                    [ $ptpClock isOffsetInRange ]]
            }
            if { $event2Sub & $clkmgr::EventSyncedToGM } {
                puts [format "| %-25s | %-22d |" "synced_to_primary_clock"\
                    [ $ptpClock isSyncedWithGm ]]
            }
            if { $event2Sub & $clkmgr::EventASCapable } {
                puts [format "| %-25s | %-22d |" "as_capable"\
                    [ $ptpClock isAsCapable ]]
            }
            if { $event2Sub & $clkmgr::EventGMChanged } {
                puts [format "| %-25s | %-22d |" "gm_Changed"\
                    [ $ptpClock isGmChanged ]]
            }
        }
        puts $hd2
        set gmClockUUID [ $ptpClock getGmIdentity ]
        # Copy the uint64_t into the array
        for {set i 0} {$i < 8} {incr i} {
            set id($i) [expr ($gmClockUUID >> (8 * (7 - $i))) & 0xff ]
        }
        puts [format "| %-25s | %02x%02x%02x.%02x%02x.%02x%02x%02x     |"\
            "GM UUID" $id(0) $id(1) $id(2) $id(3) $id(4) $id(5) $id(6) $id(7) ]
        puts [format "| %-25s | %-19ld ns |" "clock_offset"\
            [ $ptpClock getClockOffset ]]
        puts [format "| %-25s | %-19ld ns |" "notification_timestamp"\
            [ $ptpClock getNotificationTimestamp ]]
        puts [format "| %-25s | %-19ld us |" "gm_sync_interval"\
            [ $ptpClock getSyncInterval ]]
        puts $hd2
        if { $composite_event != 0 } {
            puts [format "| %-25s | %-22d |" "composite_event"\
                [ $ptpClock isCompositeEventMet ]]
            if { $composite_event & $clkmgr::EventGMOffset } {
                puts [format "| - %-23s | %-22s |" "offset_in_range" "" ]
            }
            if { $composite_event & $clkmgr::EventSyncedToGM } {
                puts [format "| - %-19s | %-22s |"\
                    "synced_to_primary_clock" "" ]
            }
            if { $composite_event & $clkmgr::EventASCapable } {
                puts [format "| - %-23s | %-22s |" "as_capable" "" ]
            }
            puts $hd2
        }
        puts ""
        puts $hd2
        puts [format "| %-25s | %-22d |" "chrony_offset_in_range"\
            [ $sysClock isOffsetInRange ]]
        puts $hd2
        puts [format "| %-25s | %-19ld ns |" "chrony_clock_offset"\
            [ $sysClock getClockOffset ]]
        puts [format "| %-25s | %-19lx    |" "chrony_clock_reference_id"\
            [ $sysClock getGmIdentity ]]
        puts [format "| %-25s | %-19ld us |" "chrony_polling_interval"\
            [ $sysClock getSyncInterval ]]
        puts $hd2
        puts ""
    }
    sleep 1

    set hd2l "|[string repeat - 27]|[string repeat - 28]|"
    set hd3b "| %-25s | %-12d | %-11d |"
    set hd3 "|[string repeat - 27]|[string repeat - 14]|[string repeat - 13]|"

    while { !$signal_flag } {
        foreach idx $index {
            if { $signal_flag } {
                return 0
            }
            puts [format "\[clkmgr]\[%.3f] Waiting Notification from %s"\
                [expr [clock milliseconds ] / 1000.0 ]\
                "time base index $idx ..."]
            set retval [ clkmgr::ClockManager_statusWait $timeout\
                $idx clockSyncData ]
            if { !$retval } {
                puts [format "\[clkmgr]\[%.3f] No event status changes %s"\
                    [expr [clock milliseconds ] / 1000.0 ]\
                    "identified in $timeout seconds."]
                puts ""
                puts [format "\[clkmgr]\[%.3f] sleep for %s"\
                    [expr [clock milliseconds ] / 1000.0 ]\
                    "$idleTime seconds..."]
                puts ""
                if { $signal_flag } {
                    return 0
                }
                sleep $idleTime
                continue
            } elseif { $retval < 0 } {
                puts [format "\[clkmgr]\[%.3f] Terminating: %s"\
                    [expr [clock milliseconds ] / 1000.0 ]\
                    "lost connection to clkmgr Proxy"]
                return 0
            }
            puts [format "\[clkmgr]\[%.3f] Obtained data from %s"\
                [expr [clock milliseconds ] / 1000.0 ]\
                "Notification Event:"]
            ClockManagerGetTime
            puts $hd3
            puts [format "| %-25s | %-12s | %-11s |" "Event"\
                "Event Status" "Event Count" ]
            set ptpClock [ clockSyncData getPtp ]
            set sysClock [ clockSyncData getSysClock ]
            if { $event2Sub != 0 } {
                puts $hd3
                if { $event2Sub & $clkmgr::EventGMOffset } {
                    puts [format $hd3b "offset_in_range"\
                        [ $ptpClock isOffsetInRange ]\
                        [ $ptpClock getOffsetInRangeEventCount ]]
                }
                if { $event2Sub & $clkmgr::EventSyncedToGM } {
                    puts [format $hd3b "synced_to_primary_clock"\
                        [ $ptpClock isSyncedWithGm ]\
                        [ $ptpClock getSyncedWithGmEventCount ]]
                }
                if { $event2Sub & $clkmgr::EventASCapable } {
                    puts [format $hd3b "as_capable"\
                        [ $ptpClock isAsCapable ]\
                        [ $ptpClock getAsCapableEventCount ]]
                }
                if { $event2Sub & $clkmgr::EventGMChanged } {
                    puts [format $hd3b "gm_Changed"\
                        [ $ptpClock isGmChanged ]\
                        [ $ptpClock getGmChangedEventCount ]]
                }
            }
            puts $hd3
            set gmClockUUID [ $ptpClock getGmIdentity ]
            # Copy the uint64_t into the array
            for {set i 0} {$i < 8} {incr i} {
                set id($i) [expr ($gmClockUUID >> (8 * (7 - $i))) & 0xff ]
            }
            puts [format "| %-26s|%s %02x%02x%02x.%02x%02x.%02x%02x%02x %s|"\
                "GM UUID" "    "\
                $id(0) $id(1) $id(2) $id(3)\
                $id(4) $id(5) $id(6) $id(7) "    "]
            puts [format "| %-25s |     %-19ld ns |" "clock_offset"\
                [ $ptpClock getClockOffset ]]
            puts [format "| %-25s |     %-19ld ns |" "notification_timestamp"\
                [ $ptpClock getNotificationTimestamp ]]
            puts [format "| %-25s |     %-19ld us |" "gm_sync_interval"\
                [ $ptpClock getSyncInterval ]]
            puts $hd3
            if { $composite_event != 0 } {
                puts [format $hd3b "composite_event"\
                    [ $ptpClock isCompositeEventMet ]\
                    [ $ptpClock getCompositeEventCount ]]
                if { $composite_event & $clkmgr::EventGMOffset } {
                    puts [format "| - %-23s | %-12s | %-11s |"\
                        "offset_in_range" "" ""]
                }
                if { $composite_event & $clkmgr::EventSyncedToGM } {
                    puts [format "| - %-19s | %-12s | %-11s |"\
                        "synced_to_primary_clock" "" ""]
                }
                if { $composite_event & $clkmgr::EventASCapable } {
                    puts [format "| - %-23s | %-12s | %-11s |"\
                        "as_capable" "" ""]
                }
                puts $hd3
            }
            puts ""
            puts $hd2l
            puts [format $hd3b "chrony_offset_in_range"\
                [ $sysClock isOffsetInRange ]\
                [ $sysClock getOffsetInRangeEventCount ]]
            puts $hd2l
            puts [format "| %-25s |     %-19ld ns |"\
                "chrony_clock_offset" [ $sysClock getClockOffset ]]
            puts [format "| %-25s |     %-19lx    |"\
                "chrony_clock_reference_id" [ $sysClock getGmIdentity ]]
            puts [format "| %-25s |     %-19ld us |"\
                "chrony_polling_interval" [ $sysClock getSyncInterval ]]
            puts $hd2l
            puts ""
            puts [format "\[clkmgr]\[%.3f] sleep for %d seconds..."\
                [expr [clock milliseconds ] / 1000.0 ] $idleTime ]
            puts ""
            if { $signal_flag } {
                return 0
            }
            sleep $idleTime
        }
    }
    return 0
}
main
