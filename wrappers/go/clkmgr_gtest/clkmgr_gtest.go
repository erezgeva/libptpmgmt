/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Test GO Clock Manager client
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 * @note This is a sample code, not a product! You should use it as a reference.
 *
 * To run in development environment:
 * LD_PRELOAD=../../.libs/libclkmgr.so clkmgr_gtest/clkmgr_gtest
 *
 */

package main

import (
  "clkmgr"
  "strconv"
  "strings"
  "fmt"
  "path"
  "time"
  "syscall"
  "os"
  "os/signal"
  "github.com/pborman/getopt/v2"
)

func isPositiveValue(ret uint, errorMessage string) uint {
    if ret <= 0 {
        fmt.Fprintln(os.Stderr, errorMessage)
        os.Exit(2)
    }
    return ret
}

func clockManagerGetTime() {
    ts := clkmgr.NewTimespec()
    if clkmgr.ClockManagerGetTime(ts) {
        fmt.Printf("[clkmgr] Current Time of CLOCK_REALTIME: %d ns\n",
                (ts.GetTv_sec() * 1000000000) + uint64(ts.GetTv_nsec()))
    } else {
        fmt.Fprintln(os.Stderr, "clock_gettime failed")
    }
    clkmgr.DeleteTimespec(ts)
}

func getMonotonicTime() float64 {
    return float64(time.Now().UnixMilli()) / 1000.0
}

func main() {
    ptp4lClockOffsetThreshold := uint(100000)
    chronyClockOffsetThreshold := uint(100000)
    idleTime := uint(1)
    timeout := uint(10)
    var index []int64 //  Array of indexes to subscribe
    event2Sub := uint(clkmgr.EventGMOffset | clkmgr.EventSyncedToGM |
            clkmgr.EventASCapable | clkmgr.EventGMChanged)
    composite_event := uint(clkmgr.EventGMOffset | clkmgr.EventSyncedToGM |
            clkmgr.EventASCapable)
    clockSyncData := clkmgr.NewClockSyncData()
    ptp4lSub := clkmgr.NewPTPClockSubscription()
    chronySub := clkmgr.NewSysClockSubscription()
    // Array of ClockSyncSubscription
    var overallSub []clkmgr.ClockSyncSubscription
    helpFlag := getopt.Bool('h', "display help")
    allIndexs := getopt.Bool('a', "all time base indexs")
    userIndexs := getopt.Bool('p', "user select time base index")
    useEventMask := getopt.Uint('s', event2Sub, "subscribe_event_mask")
    useCompEvent := getopt.Uint('c', composite_event, "composite_event_mask")
    usePtpThr := getopt.Uint('l', ptp4lClockOffsetThreshold,
            "gm offset threshold (ns)")
    useIdleTime := getopt.Uint('i', idleTime, "idle time (s)")
    usechronyThr := getopt.Uint('m', chronyClockOffsetThreshold,
            "chrony offset threshold (ns)")
    useTimeout := getopt.Uint('t', timeout,
            "timeout in waiting notification event (s)")
    doExit := -1
    err := getopt.Getopt(nil)
    if err != nil {
        fmt.Fprintln(os.Stderr, err)
        doExit = 2
    } else if *helpFlag {
        doExit = 0
    }
    if doExit >= 0 {
        out := os.Stdout
        if doExit > 0 {
            out = os.Stderr
        }
        fmt.Fprintln(out,  `
Usage of ` + path.Base(os.Args[0]) + ` :
Options:
  -a subscribe to all time base indices
     Default: timeBaseIndex: 1
  -p enable user to subscribe to specific time base indices
  -s subscribe_event_mask
     Default: ` + fmt.Sprintf("0x%x", event2Sub) + `
     Bit 0: EventGMOffset
     Bit 1: EventSyncedToGM
     Bit 2: EventASCapable
     Bit 3: EventGMChanged
  -c composite_event_mask
     Default: ` + fmt.Sprintf("0x%x", composite_event) + `
     Bit 0: EventGMOffset
     Bit 1: EventSyncedToGM
     Bit 2: EventASCapable
  -l gm offset threshold (ns)
     Default: ` + strconv.Itoa(int(ptp4lClockOffsetThreshold)) + ` ns
  -i idle time (s)
     Default: ` + strconv.Itoa(int(idleTime)) + ` s
  -m chrony offset threshold (ns)
     Default: ` + strconv.Itoa(int(chronyClockOffsetThreshold)) + ` ns
  -t timeout in waiting notification event (s)
     Default: ` + strconv.Itoa(int(timeout)) + ` s`)
        os.Exit(doExit)
    }
    subscribeAll := *allIndexs
    userInput := !subscribeAll && *userIndexs
    event2Sub = *useEventMask
    composite_event = *useCompEvent
    ptp4lClockOffsetThreshold = isPositiveValue(*usePtpThr,
            "Invalid ptp4l GM Offset threshold!")
    idleTime = isPositiveValue(*useIdleTime, "Invalid idle time!")
    timeout = isPositiveValue(*useTimeout, "Invalid timeout!")
    chronyClockOffsetThreshold = isPositiveValue(*usechronyThr,
            "Invalid Chrony Offset threshold!")
    ptp4lSub.SetEventMask(event2Sub)
    ptp4lSub.SetClockOffsetThreshold(ptp4lClockOffsetThreshold)
    ptp4lSub.SetCompositeEventMask(composite_event)
    chronySub.SetClockOffsetThreshold(chronyClockOffsetThreshold)
    fmt.Printf("[clkmgr] set subscribe event : 0x%x\n",
            ptp4lSub.GetEventMask())
    fmt.Printf("[clkmgr] set composite event : 0x%x\n",
            ptp4lSub.GetCompositeEventMask())
    fmt.Println("GM Offset threshold: ptp4lClockOffsetThreshold ns")
    fmt.Println("Chrony Offset threshold: chronyClockOffsetThreshold ns")
    if userInput {
        fmt.Print("Enter the time base indices to subscribe " +
                "(comma-separated, default is 1): ")
        var line string
        fmt.Scanln(&line)
        if len(line) > 0  {
            idx, err := strconv.Atoi(line)
            if err != nil || idx <= 0 {
                fmt.Fprintln(os.Stderr, "Invalid time base index:", line)
                os.Exit(2)
            } else {
                index = append(index, int64(idx))
            }
        } else {
            fmt.Println("Invalid input. Using default time base index 1.")
        }
    }
    hd2 := "|"+strings.Repeat("-",27)+"|"+strings.Repeat("-",24)+"|"
    hd2l := "|"+strings.Repeat("-", 27)+"|"+strings.Repeat("-", 28)+"|"
    hd3b := "| %-25s | %-12t | %-11d |"
    hd3 := "|"+strings.Repeat("-", 27)+"|"+strings.Repeat("-", 14)+"|"+
            strings.Repeat("-", 13)+"|"
    signal_flag := false
    c := make(chan os.Signal)
    signal.Notify(c, os.Interrupt, syscall.SIGINT)
    signal.Notify(c, os.Interrupt, syscall.SIGTERM)
    signal.Notify(c, os.Interrupt, syscall.SIGHUP)
    go func() {
        <-c
        fmt.Println(" Exit ...")
        signal_flag = true
    }()
    var dieMsg string
    var size int64
    var i int64
    if !clkmgr.ClockManagerConnect() {
        dieMsg = "[clkmgr] failure in connecting !!!"
        goto do_exit
    }
    time.Sleep(1 * time.Second)
    fmt.Println("[clkmgr] List of available clock:")

    // Print out each member of the Time Base configuration
    size = clkmgr.TimeBaseConfigurationsSize()
    fmt.Println("size ", size )
    i = 1
    overallSub = make([]clkmgr.ClockSyncSubscription, size + 1, size + 1)
    for i <= size {
        cfg := clkmgr.TimeBaseConfigurationsGetRecord(i)
        idx := cfg.Index()
        fmt.Println("TimeBaseIndex:", idx)
        fmt.Println("timeBaseName:", cfg.Name())
        subscribe := clkmgr.NewClockSyncSubscription()
        if cfg.HavePtp() {
            ptp := cfg.Ptp()
            fmt.Println("interfaceName:", ptp.IfName())
            fmt.Println("transportSpecific:", ptp.TransportSpecific())
            fmt.Println("domainNumber:", ptp.DomainNumber())
            subscribe.SetPtpSubscription(ptp4lSub)
        }
        if cfg.HaveSysClock() {
            subscribe.SetSysSubscription(chronySub)
        }
        overallSub[idx] = subscribe
        if subscribeAll {
            index = append(index, idx)
        }
        i++
    }
    // Default
    if len(index) == 0 {
        index = append(index, 1)
    }
    for _, idx := range index {
        fmt.Println("Subscribe to time base index:", idx)
        if !clkmgr.ClockManagerSubscribe(overallSub[idx], idx, clockSyncData) {
            dieMsg = "[clkmgr] Failure in subscribing to clkmgr Proxy !!!"
            goto do_exit
        }
        fmt.Printf("[clkmgr][%.3f] Obtained data from Subscription Event:\n",
            getMonotonicTime())
        clockManagerGetTime()
        fmt.Printf("hd2\n| %-25s | %-22s |\n", "Event", "Event Status")
        ptpClock := clockSyncData.GetPtp()
        sysClock := clockSyncData.GetSysClock()
        if event2Sub != 0 {
            fmt.Println(hd2)
            if event2Sub & uint(clkmgr.EventGMOffset) > 0 {
                fmt.Printf("| %-25s | %-22t |\n", "offset_in_range",
                        ptpClock.IsOffsetInRange())
            }
            if event2Sub & uint(clkmgr.EventSyncedToGM) > 0 {
                fmt.Printf("| %-25s | %-22t |\n", "synced_to_primary_clock",
                        ptpClock.IsSyncedWithGm())
            }
            if event2Sub & uint(clkmgr.EventASCapable) > 0 {
                fmt.Printf("| %-25s | %-22t |\n", "as_capable",
                        ptpClock.IsAsCapable())
            }
            if event2Sub & uint(clkmgr.EventGMChanged) > 0 {
                fmt.Printf("| %-25s | %-22t |\n", "gm_Changed",
                        ptpClock.IsGmChanged())
            }
        }
        fmt.Println(hd2)
        gmClockUUID := ptpClock.GetGmIdentity()
        var gm_identity [8]uint
        i = 0
        for i < 8 {
            gm_identity[i] = uint((gmClockUUID >> (8 * (7 - i))) & 0xff)
            i++
        }
        fmt.Printf("| %-25s | %02x%02x%02x.%02x%02x.%02x%02x%02x     |\n",
                "GM UUID",
            gm_identity[0], gm_identity[1],
            gm_identity[2], gm_identity[3],
            gm_identity[4], gm_identity[5],
            gm_identity[6], gm_identity[7])
        fmt.Printf("| %-25s | %-19d ns |\n",
                "clock_offset", ptpClock.GetClockOffset())
        fmt.Printf("| %-25s | %-19d ns |\n",
                "notification_timestamp", ptpClock.GetNotificationTimestamp())
        fmt.Printf("| %-25s | %-19d us |\n",
                "gm_sync_interval", ptpClock.GetSyncInterval())
        fmt.Println(hd2)
        if composite_event != 0 {
            fmt.Printf("| %-25s | %-22t |\n", "composite_event",
                    ptpClock.IsCompositeEventMet())
            if composite_event & uint(clkmgr.EventGMOffset) > 0 {
                fmt.Printf("| - %-23s | %-22s |\n", "offset_in_range", "")
            }
            if composite_event & uint(clkmgr.EventSyncedToGM) > 0 {
                fmt.Printf("| - %-19s | %-22s |\n",
                        "synced_to_primary_clock", "")
            }
            if composite_event & uint(clkmgr.EventASCapable) > 0 {
                fmt.Printf("| - %-23s | %-22s |\n", "as_capable", "")
            }
            fmt.Println(hd2)
        }
        fmt.Println()
        fmt.Println(hd2)
        fmt.Printf("| %-25s | %-22t |\n", "chrony_offset_in_range",
                sysClock.IsOffsetInRange())
        fmt.Println(hd2)
        fmt.Printf("| %-25s | %-19d ns |\n", "chrony_clock_offset",
                sysClock.GetClockOffset())
        fmt.Printf("| %-25s | %-19x    |\n", "chrony_clock_reference_id",
                sysClock.GetGmIdentity())
        fmt.Printf("| %-25s | %-19d us |\n", "chrony_polling_interval",
                sysClock.GetSyncInterval())
        fmt.Println(hd2)
        fmt.Println()
    }
    time.Sleep(1 * time.Second)

    for !signal_flag {
        inner_loop:
        for _, idx := range index {
            if signal_flag {
                goto do_exit
            }
            fmt.Printf("[clkmgr][%.3f] Waiting Notification " +
                    "from time base index %d ...\n", getMonotonicTime(), idx)
            retval := clkmgr.StatusWaitResult(
                clkmgr.ClockManagerStatusWait(int(timeout), idx, clockSyncData))
            switch retval {
                case clkmgr.SWRLostConnection:
                    fmt.Printf("[clkmgr][%.3f] Terminating: " +
                        "lost connection to clkmgr Proxy\n", getMonotonicTime())
                    goto do_exit
                case clkmgr.SWRInvalidArgument:
                    fmt.Printf("[clkmgr][%.3f] Terminating: " +
                        "Invalid argument\n", getMonotonicTime())
                    goto do_exit
                case clkmgr.SWRNoEventDetected:
                    fmt.Printf("[clkmgr][%.3f] No event status changes " +
                        "identified in %d seconds.\n\n",
                        getMonotonicTime(), timeout)
                    fmt.Printf("[clkmgr][%.3f] sleep for %d seconds...\n\n",
                        getMonotonicTime(), idleTime)
                    if signal_flag {
                        goto do_exit
                    }
                    time.Sleep(time.Duration(idleTime) * time.Second)
                    goto inner_loop;
                case clkmgr.SWREventDetected:
                    fmt.Printf("[clkmgr][%.3f] Obtained data from " +
                        "Notification Event:\n", getMonotonicTime())
                    break
                default:
                    fmt.Printf("[clkmgr][%.3f] Warning: Should not enter " +
                        "this switch case, unexpected status code %d\n",
                        getMonotonicTime(), retval)
                    goto do_exit
            }
            clockManagerGetTime()
            fmt.Printf("hd3\n| %-25s | %-12s | %-11s |\n",
                    "Event", "Event Status", "Event Count")
            ptpClock := clockSyncData.GetPtp()
            sysClock := clockSyncData.GetSysClock()
            if event2Sub != 0 {
                fmt.Println(hd3)
                if event2Sub & uint(clkmgr.EventGMOffset) > 0 {
                    fmt.Printf(hd3b + "\n", "offset_in_range",
                            ptpClock.IsOffsetInRange(),
                            ptpClock.GetOffsetInRangeEventCount())
                }
                if event2Sub & uint(clkmgr.EventSyncedToGM) > 0 {
                    fmt.Printf(hd3b + "\n", "synced_to_primary_clock",
                            ptpClock.IsSyncedWithGm(),
                            ptpClock.GetSyncedWithGmEventCount())
                }
                if event2Sub & uint(clkmgr.EventASCapable) > 0 {
                    fmt.Printf(hd3b + "\n", "as_capable",
                            ptpClock.IsAsCapable(),
                            ptpClock.GetAsCapableEventCount())
                }
                if event2Sub & uint(clkmgr.EventGMChanged) > 0 {
                    fmt.Printf(hd3b + "\n", "gm_Changed",
                            ptpClock.IsGmChanged(),
                            ptpClock.GetGmChangedEventCount())
                }
            }
            fmt.Println(hd3)
            gmClockUUID := ptpClock.GetGmIdentity()
            var gm_identity [8]uint
            i = 0
            for i < 8 {
                gm_identity[i] = uint((gmClockUUID >> (8 * (7 - i))) & 0xff)
                i++
            }
            fmt.Printf("| %-25s |     %02x%02x%02x.%02x%02x.%02x%02x%02x     |\n",
                    "GM UUID",
                gm_identity[0], gm_identity[1],
                gm_identity[2], gm_identity[3],
                gm_identity[4], gm_identity[5],
                gm_identity[6], gm_identity[7])
            fmt.Printf("| %-25s |     %-19d ns |\n",
                    "clock_offset", ptpClock.GetClockOffset())
            fmt.Printf("| %-25s |     %-19d ns |\n",
                    "notification_timestamp",
                    ptpClock.GetNotificationTimestamp())
            fmt.Printf("| %-25s |     %-19d us |\n",
                    "gm_sync_interval", ptpClock.GetSyncInterval())
            fmt.Println(hd3)
            if composite_event != 0 {
                fmt.Printf(hd3b + "\n", "composite_event",
                        ptpClock.IsCompositeEventMet(),
                        ptpClock.GetCompositeEventCount())
                if composite_event & uint(clkmgr.EventGMOffset) > 0 {
                    fmt.Printf("| - %-23s | %-12s | %-11s |\n",
                            "offset_in_range", "", "")
                }
                if composite_event & uint(clkmgr.EventSyncedToGM) > 0 {
                    fmt.Printf("| - %-19s | %-12s | %-11s |\n",
                            "synced_to_primary_clock", "", "")
                }
                if composite_event & uint(clkmgr.EventASCapable) > 0 {
                    fmt.Printf("| - %-23s | %-12s | %-11s |\n",
                            "as_capable", "", "")
                }
                fmt.Println(hd3)
            }
            fmt.Println()
            fmt.Println(hd2l)
            fmt.Printf(hd3b + "\n", "chrony_offset_in_range",
               sysClock.IsOffsetInRange(), sysClock.GetOffsetInRangeEventCount())
            fmt.Println(hd2l)
            fmt.Printf("| %-25s |     %-19d ns |\n",
                "chrony_clock_offset", sysClock.GetClockOffset())
            fmt.Printf("| %-25s |     %-19x    |\n",
                "chrony_clock_reference_id", sysClock.GetGmIdentity())
            fmt.Printf("| %-25s |     %-19d us |\n",
                "chrony_polling_interval", sysClock.GetSyncInterval())
            fmt.Println(hd2l)
            fmt.Println()
            fmt.Printf("[clkmgr][%.3f] sleep for %d seconds...\n\n",
                getMonotonicTime(), idleTime)
            if signal_flag {
                goto do_exit
            }
            time.Sleep(time.Duration(idleTime) * time.Second)
        }
    }

    do_exit:

    clkmgr.ClockManagerDisconnect()
    // Delete objects we allocate
    clkmgr.DeleteClockSyncData(clockSyncData)
    clkmgr.DeletePTPClockSubscription(ptp4lSub)
    clkmgr.DeleteSysClockSubscription(chronySub)
    for _, subscribe := range overallSub {
        clkmgr.DeleteClockSyncSubscription(subscribe)
    }
    // Exit
    if len(dieMsg) > 0 {
        fmt.Fprintln(os.Stderr, dieMsg)
        os.Exit(2)
    }
}
