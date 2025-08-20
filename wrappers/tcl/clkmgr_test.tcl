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
    if { $ret >= 0 } {
        return $ret
    }
    puts stderr $errorMessage
    exit 2
}

proc isValidTimeout { optarg errorMessage } {
    set ret [expr $optarg ]
    if { $ret >= -1 && [expr {$ret == [expr int($ret)]}] } {
        return $ret
    }
    puts stderr $errorMessage
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

proc getMonotonicTime {} {
    return [expr [clock milliseconds ] / 1000.0 ]
}

proc printOut {} {
    global signal_flag index subscribeAll event2Sub composite_event chrony_event\
           clockSyncData ptp4lSub chronySub idleTime timeout
    ClockManagerGetTime
    set hd2l "|[string repeat - 30]|[string repeat - 28]|"
    set hd3b "| %-28s | %-12d | %-11d |"
    set hd3 "|[string repeat - 30]|[string repeat - 14]|[string repeat - 13]|"
    puts $hd3
    puts [format "| %-28s | %-12s | %-11s |" "Events"\
        "Event Status" "Event Count" ]
    set ptpClock [ clockSyncData getPtp ]
    set sysClock [ clockSyncData getSysClock ]
    puts $hd3
    if { $composite_event != 0 } {
        puts [format $hd3b "ptp_isCompositeEventMet"\
            [ $ptpClock isCompositeEventMet ]\
            [ $ptpClock getCompositeEventCount ]]
        if { $composite_event & $clkmgr::EventOffsetInRange } {
            puts [format "| - %-26s | %-12s | %-11s |"\
                "isOffsetInRange" "" ""]
        }
        if { $composite_event & $clkmgr::EventSyncedWithGm } {
            puts [format "| - %-26s | %-12s | %-11s |"\
                "isSyncedWithGm" "" ""]
        }
        if { $composite_event & $clkmgr::EventAsCapable } {
            puts [format "| - %-26s | %-12s | %-11s |"\
                "isAsCapable" "" ""]
        }
    }
    if { $event2Sub != 0 } {
        puts $hd3
        if { $event2Sub & $clkmgr::EventOffsetInRange } {
            puts [format $hd3b "ptp_isOffsetInRange"\
                [ $ptpClock isOffsetInRange ]\
                [ $ptpClock getOffsetInRangeEventCount ]]
        }
        if { $event2Sub & $clkmgr::EventSyncedWithGm } {
            puts [format $hd3b "ptp_isSyncedWithGm"\
                [ $ptpClock isSyncedWithGm ]\
                [ $ptpClock getSyncedWithGmEventCount ]]
        }
        if { $event2Sub & $clkmgr::EventAsCapable } {
            puts [format $hd3b "ptp_isAsCapable"\
                [ $ptpClock isAsCapable ]\
                [ $ptpClock getAsCapableEventCount ]]
        }
        if { $event2Sub & $clkmgr::EventGmChanged } {
            puts [format $hd3b "ptp_isGmChanged"\
                [ $ptpClock isGmChanged ]\
                [ $ptpClock getGmChangedEventCount ]]
        }
    }
    puts $hd3
    puts [format "| %-28s |     %-19ld ns |" "ptp_clockOffset"\
        [ $ptpClock getClockOffset ]]
    set gmClockUUID [ $ptpClock getGmIdentity ]
    # Copy the uint64_t into the array
    for {set i 0} {$i < 8} {incr i} {
        set id($i) [expr ($gmClockUUID >> (8 * (7 - $i))) & 0xff ]
    }
    puts [format "| %-29s|%s %02x%02x%02x.%02x%02x.%02x%02x%02x %s|"\
        "ptp_gmIdentity" "    "\
        $id(0) $id(1) $id(2) $id(3)\
        $id(4) $id(5) $id(6) $id(7) "    "]
    puts [format "| %-28s |     %-19ld us |" "ptp_syncInterval"\
        [ $ptpClock getSyncInterval ]]
    puts [format "| %-28s |     %-19ld ns |" "ptp_notificationTimestamp"\
        [ $ptpClock getNotificationTimestamp ]]
    puts $hd2l
    if {[clockSyncData haveSys]} {
        puts [format $hd3b "chrony_isOffsetInRange"\
            [ $sysClock isOffsetInRange ]\
            [ $sysClock getOffsetInRangeEventCount ]]
        puts $hd2l
        puts [format "| %-28s |     %-19ld ns |"\
            "chrony_clockOffset" [ $sysClock getClockOffset ]]
        for {set i 0} {$i < 4} {incr i} {
            set byte [expr {([$sysClock getGmIdentity] >> (8 * (3 - $i))) & 0xFF}]
            if { $byte == 0 || $byte == 9 } {
                append identity_string " "
            } else {
                set s [format "%c" $byte]
                if { [ string is print $s ] } {
                    append identity_string $s
                } else {
                    append identity_string "."
                }
            }
        }
        puts [format "| %-28s |     %-19s    |"\
            "chrony_gmIdentity" $identity_string ]
        puts [format "| %-28s |     %-19ld us |"\
            "chrony_syncInterval" [ $sysClock getSyncInterval ]]
        puts [format "| %-28s |     %-19ld ns |" "chrony_notificationTimestamp"\
        [ $sysClock getNotificationTimestamp ]]
    }
    puts $hd2l
    puts ""
}

proc main {} {
    global argv argv0 index subscribeAll event2Sub composite_event chrony_event\
        clockSyncData ptp4lSub chronySub idleTime timeout
    # index list of indexes to subscribe
    set event2Sub [expr $clkmgr::EventOffsetInRange | $clkmgr::EventSyncedWithGm |\
        $clkmgr::EventAsCapable | $clkmgr::EventGmChanged ]
    set composite_event [expr $clkmgr::EventOffsetInRange |\
        $clkmgr::EventSyncedWithGm | $clkmgr::EventAsCapable ]
    set chrony_event $clkmgr::EventOffsetInRange
    set options {
        {h}
        {a}
        {p}
        # event2Sub
        {s.arg "" }
        # composite_event
        {c.arg "" }
        # chrony_event
        {n.arg "" }
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
        set me [file tail $argv0]
        puts "
Usage of $me :
Options:
  -a subscribe to all time base indices
     Default: timeBaseIndex: 1
  -p enable user to subscribe to specific time base indices
  -s subscribe_event_mask
     Default: 0x[format %x $event2Sub ]
     Bit 0: EventOffsetInRange
     Bit 1: EventSyncedWithGm
     Bit 2: EventAsCapable
     Bit 3: EventGmChanged
  -c composite_event_mask
     Default: 0x[format %x $composite_event ]
     Bit 0: EventOffsetInRange
     Bit 1: EventSyncedWithGm
     Bit 2: EventAsCapable
  -n chrony_event_mask
     Default: 0x[format %x $chrony_event ]
     Bit 0: EventOffsetInRange
  -l gm offset threshold (ns)
     Default: 100000 ns
  -i idle time (s)
     Default: 1 s
  -m chrony offset threshold (ns)
     Default: 100000 ns
  -t timeout in waiting notification event (s)
     Default: 10 s
     -1 : wait indefinitely until at least an event change occurs
      0 : retrieve the latest clock sync data immediately
     >0 : wait up to the specified number of seconds for an event"
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
            puts stderr "use "-s" with a number"
            exit 2
        }
    }
    if { [string length $params(c)] > 0} {
        try {
            set composite_event [expr $params(c) ]
        } trap {} {} {
            puts stderr "use "-c" with a number"
            exit 2
        }
    }
    if { [string length $params(n)] > 0} {
        try {
            set chrony_event [expr $params(n) ]
        } trap {} {} {
            puts "use "-n" with a number"
            exit 2
        }
    }
    set ptp4lClockOffsetThreshold [ isPositiveValue $params(l)\
        "Invalid ptp4l GM Offset threshold!" ]
    set idleTime [ isPositiveValue $params(i) "Invalid idle time!" ]
    set timeout [ isValidTimeout $params(t) "Invalid timeout!" ]
    set chronyClockOffsetThreshold [ isPositiveValue $params(m)\
        "Invalid Chrony Offset threshold!" ]
    ptp4lSub setEventMask $event2Sub
    ptp4lSub setClockOffsetThreshold $ptp4lClockOffsetThreshold
    ptp4lSub setCompositeEventMask $composite_event
    chronySub setEventMask $chrony_event
    chronySub setClockOffsetThreshold $chronyClockOffsetThreshold
    puts [format "\[clkmgr] set subscribe event : 0x%x"\
        [ ptp4lSub getEventMask ]]
    puts [format "\[clkmgr] set composite event : 0x%x"\
        [ ptp4lSub getCompositeEventMask ]]
    puts [format "\[clkmgr] set chrony event : 0x%x" $chrony_event]
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
            foreach str [ split $line ", " ] {
                if { [string length $str] > 0} {
                    try {
                        set idx [expr $str ]
                    } trap {} {} {
                        puts stderr "$str is not a number"
                        exit 2
                    }
                    lappend index $idx
                }
            }
        }
    }
    signal trap SIGINT signal_handler
    signal trap SIGTERM signal_handler
    signal trap SIGHUP signal_handler
    set ret [ main_body ]
    clkmgr::ClockManager_disconnect
    if { $ret > 0 } {
        exit 2
    }
}

proc main_body {} {
    global signal_flag index subscribeAll event2Sub composite_event chrony_event\
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

    foreach idx $index {
        if { ! [ clkmgr::TimeBaseConfigurations_isTimeBaseIndexPresent $idx ] }  {
            puts stderr "\[clkmgr] Index $idx does not exist"
            return 2
        }
        puts "\[clkmgr] Subscribe to time base index: $idx"
        if { ! [ clkmgr::ClockManager_subscribe $overallSub($idx)\
            $idx clockSyncData ] } {
            puts stderr "\[clkmgr] Failure in subscribing to clkmgr Proxy !!!"
            return 2
        }
        puts [format "\[clkmgr]\[%.3f] Obtained data from %s"\
            [getMonotonicTime] "Subscription Event:"]
        printOut
    }
    sleep 1

    while { !$signal_flag } {
        foreach idx $index {
            if { $signal_flag } {
                return 0
            }
            puts [format "\[clkmgr]\[%.3f] Waiting Notification from %s"\
                [getMonotonicTime] "time base index $idx ..."]
            set retval [ clkmgr::ClockManager_statusWait $timeout $idx clockSyncData ]
            if { $retval == $clkmgr::SWRLostConnection } {
                puts [format "\[clkmgr]\[%.3f] Terminating: %s"\
                    [getMonotonicTime] "lost connection to clkmgr Proxy"]
                return 0
            } elseif { $retval == $clkmgr::SWRInvalidArgument } {
                puts stderr [format "\[clkmgr]\[%.3f] Terminating: %s"\
                    [getMonotonicTime] "Invalid argument"]
                return 2
            } elseif { $retval == $clkmgr::SWRNoEventDetected } {
                puts [format "\[clkmgr]\[%.3f] No event status changes %s"\
                    [getMonotonicTime] "identified in $timeout seconds."]
                puts ""
                puts [format "\[clkmgr]\[%.3f] sleep for %s"\
                    [getMonotonicTime] "$idleTime seconds..."]
            } elseif { $retval == $clkmgr::SWREventDetected } {
                puts [format "\[clkmgr]\[%.3f] Obtained data from %s"\
                    [getMonotonicTime] "Notification Event:"]
                printOut
                puts [format "\[clkmgr]\[%.3f] sleep for %d seconds..."\
                    [getMonotonicTime] $idleTime ]
            } else {
                puts [format "\[clkmgr]\[%.3f] Warning: Should %s $retval"\
                    [getMonotonicTime]\
                    "not enter this switch case, unexpected status code"]
                return 0
            }
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
