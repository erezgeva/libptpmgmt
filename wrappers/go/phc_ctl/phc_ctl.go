/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2025 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Impleament linuxptp phc_ctl tool using the libptpmgmt library
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2025 Erez Geva
 *
 */

package main

import (
  "ptpmgmt"
  "strings"
  "strconv"
  "time"
  "fmt"
  "os"
  "path"
  "path/filepath"
//"log/syslog"
)

/* Go uses monotonic clock for duration calculation
 * But lack it for display */

/*
#include <time.h>
static double getMsecs(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_nsec / 1000000000 + ts.tv_sec;
}
*/
import "C"

var appName string
var curCmd string
var useSys bool
var useClock ptpmgmt.BaseClock
var usePhc ptpmgmt.PtpClock
var haveVal bool
var valStr string
var seconds float64

func If[T any](cond bool, vtrue T, vfalse T) T {
    if cond {
        return vtrue
    }
    return vfalse
}
func bStr() string {
    return fmt.Sprintf(appName + "[%.3f]: ", float64(C.getMsecs()))
}
func cTime(ts ptpmgmt.Timestamp_t) string {
    return time.Unix(int64(ts.GetSecondsField()), 0).Format(time.ANSIC) + "\n"
}
func errPrint(msg string) bool {
    fmt.Fprintln(os.Stderr, bStr() + curCmd + ": " + msg)
    return false
}
func errPrintVal[T any](msg string, val T) bool {
     return errPrint(fmt.Sprintf(msg, val))
}
func warnPrint(msg string) bool {
    fmt.Fprintln(os.Stderr, bStr() + curCmd + ": " + msg)
    return true
}
func msgPrint(msg string) bool {
    fmt.Println(bStr() + msg)
    return true
}
func msgPrintVal[T any](msg string, val T) bool {
     return msgPrint(fmt.Sprintf(msg, val))
}

type doFunc func() bool

func do_set() bool {
    if !haveVal {
        if ! useSys {
            if ! usePhc.SetTimeFromSys() {
                return errPrint("fail to set clock from CLOCK_REALTIME")
            }
        }
        ts := useClock.GetTime()
        defer ptpmgmt.DeleteTimestamp_t(ts)
        if ts.IsZero() {
            return errPrint("failed to set clock time")
        }
        return msgPrint("set clock time to " + ts.String() + " or " + cTime(ts))
    }
    if seconds < 0 {
        return errPrint("'" + valStr + "' is out of range")
    }
    ts := ptpmgmt.NewTimestamp_t(seconds)
    defer ptpmgmt.DeleteTimestamp_t(ts)
    if useClock.SetTime(ts) {
        return msgPrint("set clock time to " + ts.String() + " or " + cTime(ts))
    }
    return errPrint("failed to set clock time")
}
func do_get() bool {
    ts := useClock.GetTime()
    defer ptpmgmt.DeleteTimestamp_t(ts)
    if ts.IsZero() {
        return errPrint("failed to get clock time: " + ptpmgmt.ErrorGetError())
    }
    return msgPrint("clock time is " + ts.String() + " or " + cTime(ts))
}
func do_adj() bool {
    if useClock.OffsetClock(int64(seconds * float64(ptpmgmt.GetNSEC_PER_SEC()))) {
        return msgPrintVal("adjusted clock by %.6f seconds", seconds)
    }
    return errPrint("fail adjust clock")
}
func do_freq() bool {
    if !haveVal {
        return msgPrintVal("clock frequency offset is %.6fppb", useClock.GetFreq())
    }
    max := float64(ptpmgmt.GetNSEC_PER_SEC())
    ppb := seconds
    if ppb > max || ppb < -max {
        return errPrint("'" + valStr + "' is out of range.")
    }
    if useClock.SetFreq(ppb) {
        return msgPrintVal("adjusted clock frequency offset to %.6fppb", ppb)
    }
    return errPrint("fail set frequency")
}
func do_phase() bool {
    if ! useClock.SetPhase(int64(seconds * float64(ptpmgmt.GetNSEC_PER_SEC()))) {
        errPrint("fail set phase offset: " + ptpmgmt.ErrorGetError())
    }
    return msgPrintVal("offset of %.6f seconds provided to " +
        "PHC phase control", seconds)
}
func calcSample(before ptpmgmt.Timestamp_t, after ptpmgmt.Timestamp_t,
        phcClk ptpmgmt.Timestamp_t) (int64, int64) {
    return int64(after.ToNanoseconds()) - int64(before.ToNanoseconds()),
        int64((before.ToNanoseconds() + after.ToNanoseconds()) / 2) -
        int64(phcClk.ToNanoseconds())
}
func calcExtSample(sample ptpmgmt.PtpSampleExt_t) (int64, int64) {
    return calcSample(sample.GetBefore(), sample.GetAfter(), sample.GetPhcClk())
}
func calcBasicSample(b_sam ptpmgmt.PtpSample_t,
        a_sam ptpmgmt.PtpSample_t) (int64, int64) {
    return calcSample(b_sam.GetSysClk(), a_sam.GetSysClk(), b_sam.GetPhcClk())
}
func done_cmp(diff int64, precise bool) bool {
    ap := If(precise, "", "approximately ")
    return msgPrintVal("offset from CLOCK_REALTIME is " + ap + "%dns\n", diff)
}
func do_cmp() bool {
    if useSys {
        return errPrint("no point comparing CLOCK_REALTIME to itself")
    }

    precise_sample := ptpmgmt.NewPtpSamplePrecise_t()
    defer ptpmgmt.DeletePtpSamplePrecise_t(precise_sample)
    if usePhc.PreciseSamplePtpSys(precise_sample) {
        // Using precise sample ioctl
        SysClk := precise_sample.GetSysClk()
        PhcClk := precise_sample.GetPhcClk()
        return done_cmp(int64(SysClk.ToNanoseconds()) -
            int64(PhcClk.ToNanoseconds()), true)
    }

    var diff, distance int64
    ext_samples := ptpmgmt.NewPtpSampleExt_v()
    defer ptpmgmt.DeletePtpSampleExt_v(ext_samples)
    if usePhc.ExtSamplePtpSys(9, ext_samples) {
        // Using extended sample ioctl
        distance, diff = calcExtSample(ext_samples.Get(0))
        for i := 1; i < int(ext_samples.Size()); i++ {
            ndistance, ndiff := calcExtSample(ext_samples.Get(i))
            if ndistance < distance {
                diff = ndiff
                distance = ndistance
            }
        }
        return done_cmp(diff, true)
    }

    basic_samples := ptpmgmt.NewPtpSample_v()
    defer ptpmgmt.DeletePtpSample_v(basic_samples)
    if usePhc.SamplePtpSys(10, basic_samples) {
        // Using basic sample ioctl
        distance, diff = calcBasicSample(basic_samples.Get(0), basic_samples.Get(1))
        for i := 1; i < int(basic_samples.Size()) - 1; i++ {
            ndistance, ndiff := calcBasicSample(basic_samples.Get(i),
                    basic_samples.Get(i + 1))
            if ndistance < distance {
                diff = ndiff
                distance = ndistance
            }
        }
        return done_cmp(diff, false)
    }

    // PTP_SYS_OFFSET was add to Kernel v3.7 in 2012, for all ptp devices
    // No point trying with get time ourself.
    return errPrint("failed to compare clocks\n")
}
func do_caps() bool {
    if useSys {
        return warnPrint("CLOCK_REALTIME is not a PHC device.")
    }
    caps := ptpmgmt.NewPtpCaps_t()
    defer ptpmgmt.DeletePtpCaps_t(caps)
    if !usePhc.FetchCaps(caps) {
        return errPrint("get capabilities failed: " + ptpmgmt.ErrorGetError())
    }
    return msgPrint(fmt.Sprintf(`
capabilities:
  %d maximum frequency adjustment (ppb)
  %d programable alarms
  %d external time stamp channels
  %d programmable periodic signals
  %d configurable input/output pins
  %s pulse per second support
  %s cross timestamping support
  %s adjust phase support
`, caps.GetMax_ppb(),
   caps.GetNum_alarm(),
   caps.GetNum_external_channels(),
   caps.GetNum_periodic_sig(),
   caps.GetNum_pins(),
   If(caps.GetPps(), "has", "doesn't have"),
   If(caps.GetCross_timestamping(), "has", "doesn't have"),
   If(caps.GetAdjust_phase(), "has", "doesn't have")))
   max_phase_adj := caps.GetMax_phase_adj()
   if max_phase_adj > 0 {
       msgPrintVal("  %d maximum offset adjustment (ns)\n", max_phase_adj)
   }
   return true
}
func do_pins() bool {
    if useSys {
        return warnPrint("CLOCK_REALTIME is not a PHC device")
    }
    caps := ptpmgmt.NewPtpCaps_t()
    defer ptpmgmt.DeletePtpCaps_t(caps)
    if !usePhc.FetchCaps(caps) {
        return errPrint("get capabilities failed: " + ptpmgmt.ErrorGetError())
    }
    pins := caps.GetNum_pins()
    if pins == 0 {
        return warnPrint("device has no configurable pins")
    }
    ret := fmt.Sprintf(" device has %d configurable input/output pins", pins)
    pin := ptpmgmt.NewPtpPin_t()
    defer ptpmgmt.DeletePtpPin_t(pin)
    for i := 0; i < pins; i++ {
        if !usePhc.ReadPin(uint(i), pin) {
            return errPrintVal("read pin %d fail: " + ptpmgmt.ErrorGetError(), i)
        }
        f := pin.GetFunctional()
        ret += fmt.Sprintf("\n pin %d [" + pin.GetDescription() + "] ", i)
        if f == ptpmgmt.PTP_PIN_UNUSED {
            ret += "not configured"
        } else {
            switch f {
                case ptpmgmt.PTP_PIN_EXTERNAL_TS:
                    ret += "external timestamping"
                case ptpmgmt.PTP_PIN_PERIODIC_OUT:
                    ret += "periodic output"
                case ptpmgmt.PTP_PIN_PHY_SYNC:
                    ret += "physical synchronization"
            }
            ret += fmt.Sprintf(" on channel %d", pin.GetChannel())
        }
    }
    return msgPrint("device configurable pins:\n" + ret)
}
func do_wait() bool {
    if seconds < 0 {
        return errPrint("'" + valStr + "' is out of range")
    }
    time.Sleep(time.Nanosecond * time.Duration(seconds *
                float64(ptpmgmt.GetNSEC_PER_SEC())))
    return msgPrintVal("process slept for %.6f seconds\n", seconds)
}

func Usage() {
    fmt.Fprintln(os.Stderr, `
usage: ` + appName + ` [options] <device> -- [command]

 device         ethernet or ptp clock device
 options
 -q             do not print messages to the syslog (default)
 -v             prints the software version and exits
 -h             prints this message and exits

 commands
 specify commands with arguments. Can specify multiple
 commands to be executed in order. Seconds are read as
 double precision floating point values.
  set   [seconds]  set PHC time (defaults to time on CLOCK_REALTIME)
  get              get PHC time
  adj   <seconds>  adjust PHC time by offset
  freq  [ppb]      adjust PHC frequency (default returns current offset)
  phase <seconds>  pass offset to PHC phase control keyword
  cmp              compare PHC offset to CLOCK_REALTIME
  caps             display device capabilities (default if no command given)
  pins             display device configurable pins
  wait  <seconds>  pause between commands
`)
}

// Return callback, may use argument, message if argument is missed
func cmd2func() (doFunc, bool, string) {
    if strings.HasPrefix(curCmd, "set") {
        return do_set, true, ""
    } else if strings.HasPrefix(curCmd, "get") {
        return do_get, false, ""
    } else if strings.HasPrefix(curCmd, "adj") {
        return do_adj, true, "missing required time argument"
    } else if strings.HasPrefix(curCmd, "freq") {
        return do_freq, true, ""
    } else if strings.HasPrefix(curCmd, "phase") {
        return do_phase, true, "missing required time argument"
    } else if strings.HasPrefix(curCmd, "cmp") {
        return do_cmp, false, ""
    } else if strings.HasPrefix(curCmd, "caps") {
        return do_caps, false, ""
    } else if strings.HasPrefix(curCmd, "pins") {
        return do_pins, false, ""
    } else if strings.HasPrefix(curCmd, "wait") {
        return do_wait, true, "requires sleep duration argument"
    }
    return nil, false, ""
}

func loop(args []string) int {
    lenArgs := len(args)
    if lenArgs < 1 {
        Usage()
        return -1
    }
    device := args[0]
    if device == "CLOCK_REALTIME" {
        useSys = true
        useClock = ptpmgmt.NewSysClock().SwigGetBaseClock()
    } else {
        path, _ := filepath.Abs(device)
        if strings.HasPrefix(path, "/dev/ptp") {
            usePhc = ptpmgmt.NewPtpClock()
            if !usePhc.InitUsingDevice(device) {
                defer ptpmgmt.DeletePtpClock(usePhc)
                fmt.Fprintln(os.Stderr, "fail init '" + device + "' device")
                return -1
            }
        } else {
            ifDev := ptpmgmt.NewIfInfo()
            defer ptpmgmt.DeleteIfInfo(ifDev)
            if !ifDev.InitUsingName(device) {
                fmt.Fprintln(os.Stderr, "Device is not a file nor an " +
                        "ethernet interface: " + device)
                return -1
            }
            idx := ifDev.PtpIndex()
            if idx == ptpmgmt.GetNO_SUCH_PTP() {
                fmt.Fprintln(os.Stderr, "Ethernet interface '" + device +
                        "' does not have an PHC", )
                return -1
            }
            usePhc = ptpmgmt.NewPtpClock()
            if !usePhc.InitUsingIndex(idx) {
                defer ptpmgmt.DeletePtpClock(usePhc)
                fmt.Fprintln(os.Stderr, "fail init '" + device + "' device")
                return -1
            }
        }
        useSys = false
        useClock = usePhc.SwigGetBaseClock()
    }
    defer ptpmgmt.DeleteBaseClock(useClock)
    if lenArgs == 1 {
        if ! do_caps() {
            return -1
        }
        return 0
    }
    for i := 1; i < lenArgs; i++ {
        curCmd = args[i]
        doFun, canUseVal, valMsg := cmd2func()
        if doFun == nil {
            fmt.Fprintln(os.Stderr, bStr() + "unknown command " + curCmd + ".")
            continue
        }
        haveVal = false
        if canUseVal && i + 1 < lenArgs {
            var err error
            valStr = args[i + 1]
            seconds, err = strconv.ParseFloat(valStr, 64)
            haveVal = err == nil
        }
        if len(valMsg) > 0 && !haveVal {
            errPrint(valMsg)
            return -1
        }
        if ! doFun() {
            return -1
        }
        if haveVal {
            i++
        }
    }
    return 0
}

func equit(emsg string) {
    fmt.Fprintln(os.Stderr, emsg)
    Usage()
    os.Exit(-1)
}

func main() {
    appName = path.Base(os.Args[0])
    args := []string{}
    i := 1
    lenArgs := len(os.Args)
    args_done:
    for ; i < lenArgs; i++ {
        arg := os.Args[i]
        switch arg {
            case "-h":
                Usage()
                return
            case "-v":
                fmt.Println(ptpmgmt.LIBPTPMGMT_VER)
                return
            case "--": // Mark end of options!
                i++
                break args_done
            case "-q": // backward compatible, we do not use syslog
            case "-Q": // We always print!
                equit("Option '-Q' is not supported!")
            case "-l": // we do not use syslog
                equit("Option '-l' is not supported!")
                /*
                i++
                if i == lenArgs {
                    equit(os.Args[0] + ": option requires an argument -- 'l'")
                }
                // Base 10, signed octet
                v, e := strconv.ParseInt(os.Args[i], 10, 8)
                min := int64(0) // int64(syslog.LOG_EMERG)
                max := int64(7) // int64(syslog.LOG_DEBUG)
                if e != nil || v < min || v > max {
                    fmt.Fprintln(os.Stderr,
                        fmt.Sprintf("-l: " + os.Args[i] + " is out of range. " +
                        "Must be in the range %d to %d", v, min, max))
                    os.Exit(-1)
                }
                */
            default:
                if arg[0] == '-' { // Do not except any other
                    equit("Wrong option '" + arg + "'")
                }
                args = append(args, os.Args[i])
        }
    }
    for ; i < lenArgs; i++ {
        args = append(args, os.Args[i])
    }
    ret := loop(args)
    if ret != 0 {
        Usage()
        os.Exit(-1)
    }
}

// LD_PRELOAD=../../.libs/libptpmgmt.so phc_ctl/phc_ctl
