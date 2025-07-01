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
  "bufio"
  "fmt"
  "path"
  "time"
  "syscall"
  "os"
  "os/signal"
  "github.com/pborman/getopt/v2"
)

var clockSyncData clkmgr.ClockSyncData
var event2Sub uint = uint(clkmgr.EventOffsetInRange | clkmgr.EventSyncedWithGm |
        clkmgr.EventAsCapable | clkmgr.EventGmChanged)
var composite_event uint = uint(clkmgr.EventOffsetInRange |
        clkmgr.EventSyncedWithGm | clkmgr.EventAsCapable)
var chronyEvent uint = uint(clkmgr.EventOffsetInRange)

func deleteOverallSub(overallSub []clkmgr.ClockSyncSubscription) {
    for _, subscribe := range overallSub {
        clkmgr.DeleteClockSyncSubscription(subscribe)
    }
}

func isPositiveValue(ret uint, errorMessage string) uint {
    if ret <= 0 {
        fmt.Fprintln(os.Stderr, errorMessage)
        os.Exit(2)
    }
    return ret
}

func clockManagerGetTime() {
    ts := clkmgr.NewTimespec()
    defer clkmgr.DeleteTimespec(ts)
    if clkmgr.ClockManagerGetTime(ts) {
        fmt.Printf("[clkmgr] Current Time of CLOCK_REALTIME: %d ns\n",
                (ts.GetTv_sec() * 1000000000) + uint64(ts.GetTv_nsec()))
    } else {
        fmt.Fprintln(os.Stderr, "clock_gettime failed")
    }
}

func getMonotonicTime() float64 {
    return float64(time.Now().UnixMilli()) / 1000.0
}

func printOut() {
    hd2l := "|"+strings.Repeat("-", 30)+"|"+strings.Repeat("-", 28)+"|"
    hd3b := "| %-28s | %-12t | %-11d |"
    hd3 := "|"+strings.Repeat("-", 30)+"|"+strings.Repeat("-", 14)+"|"+
            strings.Repeat("-", 13)+"|"
    clockManagerGetTime()
    fmt.Println(hd3)
    fmt.Printf("| %-28s | %-12s | %-11s |\n",
            "Events", "Event Status", "Event Count")
    ptpClock := clockSyncData.GetPtp()
    sysClock := clockSyncData.GetSysClock()
    fmt.Println(hd3)
    if composite_event != 0 {
        fmt.Printf(hd3b + "\n", "ptp_isCompositeEventMet",
                ptpClock.IsCompositeEventMet(),
                ptpClock.GetCompositeEventCount())
        if composite_event & uint(clkmgr.EventOffsetInRange) > 0 {
            fmt.Printf("| - %-26s | %-12s | %-11s |\n",
                    "isOffsetInRange", "", "")
        }
        if composite_event & uint(clkmgr.EventSyncedWithGm) > 0 {
            fmt.Printf("| - %-26s | %-12s | %-11s |\n",
                    "isSyncedWithGm", "", "")
        }
        if composite_event & uint(clkmgr.EventAsCapable) > 0 {
            fmt.Printf("| - %-26s | %-12s | %-11s |\n",
                    "isAsCapable", "", "")
        }
        fmt.Println(hd3)
    }
    if event2Sub != 0 {
        if event2Sub & uint(clkmgr.EventOffsetInRange) > 0 {
            fmt.Printf(hd3b + "\n", "ptp_isOffsetInRange",
                    ptpClock.IsOffsetInRange(),
                    ptpClock.GetOffsetInRangeEventCount())
        }
        if event2Sub & uint(clkmgr.EventSyncedWithGm) > 0 {
            fmt.Printf(hd3b + "\n", "ptp_isSyncedWithGm",
                    ptpClock.IsSyncedWithGm(),
                    ptpClock.GetSyncedWithGmEventCount())
        }
        if event2Sub & uint(clkmgr.EventAsCapable) > 0 {
            fmt.Printf(hd3b + "\n", "ptp_isAsCapable",
                    ptpClock.IsAsCapable(),
                    ptpClock.GetAsCapableEventCount())
        }
        if event2Sub & uint(clkmgr.EventGmChanged) > 0 {
            fmt.Printf(hd3b + "\n", "ptp_isGmChanged",
                    ptpClock.IsGmChanged(),
                    ptpClock.GetGmChangedEventCount())
        }
        fmt.Println(hd3)
    }
    gmClockUUID := ptpClock.GetGmIdentity()
    var gm_identity [8]uint
    for i := 0; i < 8; i++  {
        gm_identity[i] = uint((gmClockUUID >> (8 * (7 - i))) & 0xff)
    }
    fmt.Printf("| %-28s |     %-19d ns |\n",
            "ptp_clockOffset", ptpClock.GetClockOffset())
    fmt.Printf("| %-28s |     %02x%02x%02x.%02x%02x.%02x%02x%02x     |\n",
            "ptp_gmIdentity",
        gm_identity[0], gm_identity[1],
        gm_identity[2], gm_identity[3],
        gm_identity[4], gm_identity[5],
        gm_identity[6], gm_identity[7])
    fmt.Printf("| %-28s |     %-19d us |\n",
            "ptp_syncInterval", ptpClock.GetSyncInterval())
    fmt.Printf("| %-28s |     %-19d ns |\n",
            "ptp_notificationTimestamp",
            ptpClock.GetNotificationTimestamp())
    fmt.Println(hd2l)
    if clockSyncData.HaveSys() {
        fmt.Printf(hd3b + "\n", "chrony_isOffsetInRange",
                sysClock.IsOffsetInRange(),
                sysClock.GetOffsetInRangeEventCount())
        fmt.Println(hd2l)
        fmt.Printf("| %-28s |     %-19d ns |\n",
                "chrony_clockOffset", sysClock.GetClockOffset())
        identityString := ""
        gmClockUUID := sysClock.GetGmIdentity()
        for i := 0; i < 4; i++ {
            byteVal := (gmClockUUID >> (8 * (3 - i))) & 0xFF
            if byteVal == 0 || byteVal == 9 {
                identityString += " "
            } else {
                str := string(byteVal)
                if strconv.IsPrint(rune(str[0])) {
                    identityString += str
                } else {
                    identityString += "."
                }
            }
        }
        fmt.Printf("| %-28s |     %-19s    |\n",
                "chrony_gmIdentity", identityString)
        fmt.Printf("| %-28s |     %-19d us |\n",
                "chrony_syncInterval", sysClock.GetSyncInterval())
        fmt.Printf("| %-28s |     %-19d ns |\n",
                "chrony_notificationTimestamp",
                sysClock.GetNotificationTimestamp())
        fmt.Println(hd2l)
    }
    fmt.Println()
}

func main() {
    ptp4lClockOffsetThreshold := uint(100000)
    chronyClockOffsetThreshold := uint(100000)
    idleTime := uint(1)
    timeout := uint(10)
    var index []int64 // Array of indexes to subscribe
    ptp4lSub := clkmgr.NewPTPClockSubscription()
    defer clkmgr.DeletePTPClockSubscription(ptp4lSub)
    chronySub := clkmgr.NewSysClockSubscription()
    defer clkmgr.DeleteSysClockSubscription(chronySub)
    clockSyncData = clkmgr.NewClockSyncData()
    defer clkmgr.DeleteClockSyncData(clockSyncData)
    // Array of ClockSyncSubscription
    var overallSub []clkmgr.ClockSyncSubscription
    helpFlag := getopt.Bool('h', "display help")
    allIndexs := getopt.Bool('a', "all time base indexs")
    userIndexs := getopt.Bool('p', "user select time base index")
    useEventMask := getopt.Uint('s', event2Sub, "subscribe_event_mask")
    useCompEvent := getopt.Uint('c', composite_event, "composite_event_mask")
    useChronyEvent := getopt.Uint('n', chronyEvent, "chrony_event_mask")
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
        fmt.Fprintln(out, `
Usage of ` + path.Base(os.Args[0]) + ` :
Options:
  -a subscribe to all time base indices
     Default: timeBaseIndex: 1
  -p enable user to subscribe to specific time base indices
  -s subscribe_event_mask
     Default: ` + fmt.Sprintf("0x%x", event2Sub) + `
     Bit 0: EventOffsetInRange
     Bit 1: EventSyncedWithGm
     Bit 2: EventAsCapable
     Bit 3: EventGmChanged
  -c composite_event_mask
     Default: ` + fmt.Sprintf("0x%x", composite_event) + `
     Bit 0: EventOffsetInRange
     Bit 1: EventSyncedWithGm
     Bit 2: EventAsCapable
  -n chrony_event_mask
     Default: ` + fmt.Sprintf("0x%x", chronyEvent) + `
     Bit 0: EventOffsetInRange
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
    chronyEvent = *useChronyEvent
    ptp4lClockOffsetThreshold = isPositiveValue(*usePtpThr,
            "Invalid ptp4l GM Offset threshold!")
    idleTime = isPositiveValue(*useIdleTime, "Invalid idle time!")
    timeout = isPositiveValue(*useTimeout, "Invalid timeout!")
    chronyClockOffsetThreshold = isPositiveValue(*usechronyThr,
            "Invalid Chrony Offset threshold!")
    ptp4lSub.SetEventMask(event2Sub)
    ptp4lSub.SetClockOffsetThreshold(ptp4lClockOffsetThreshold)
    ptp4lSub.SetCompositeEventMask(composite_event)
    chronySub.SetEventMask(chronyEvent)
    chronySub.SetClockOffsetThreshold(chronyClockOffsetThreshold)
    fmt.Printf("[clkmgr] set subscribe event : 0x%x\n",
            ptp4lSub.GetEventMask())
    fmt.Printf("[clkmgr] set composite event : 0x%x\n",
            ptp4lSub.GetCompositeEventMask())
    fmt.Printf("[clkmgr] set chrony event : 0x%x\n", chronyEvent)
    fmt.Printf("GM Offset threshold: %d ns\n", ptp4lClockOffsetThreshold)
    fmt.Printf("Chrony Offset threshold: %d ns\n", chronyClockOffsetThreshold)
    if userInput {
        fmt.Print("Enter the time base indices to subscribe " +
                "(comma-separated, default is 1): ")
        reader := bufio.NewReader(os.Stdin)
        line0, _ := reader.ReadString('\n')
        line := strings.ReplaceAll(strings.TrimSpace(line0), " ", "")
        if len(line) > 0 {
            for _, str := range strings.Split(line, ",") {
                if len(str) > 0 {
                    idx, err := strconv.Atoi(str)
                    if err != nil || idx <= 0 {
                        fmt.Fprintln(os.Stderr, "Invalid time base index:", str)
                        os.Exit(2)
                    } else {
                        index = append(index, int64(idx))
                    }
                }
            }
        } else {
            fmt.Println("Invalid input. Using default time base index 1.")
        }
    }
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
    defer deleteOverallSub(overallSub)
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
        if !clkmgr.TimeBaseConfigurationsIsTimeBaseIndexPresent(idx) {
            dieMsg = fmt.Sprintf("[clkmgr] Index %d does not exist", idx)
            goto do_exit
        }
        fmt.Println("[clkmgr] Subscribe to time base index:", idx)
        if !clkmgr.ClockManagerSubscribe(overallSub[idx], idx, clockSyncData) {
            dieMsg = "[clkmgr] Failure in subscribing to clkmgr Proxy !!!"
            goto do_exit
        }
        fmt.Printf("[clkmgr][%.3f] Obtained data from Subscription Event:\n",
            getMonotonicTime())
        printOut()
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
                    dieMsg = fmt.Sprintf("[clkmgr][%.3f] Terminating: " +
                        "Invalid argument", getMonotonicTime())
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
            printOut()
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

    // Exit
    if len(dieMsg) > 0 {
        fmt.Fprintln(os.Stderr, dieMsg)
        os.Exit(2)
    }
}
