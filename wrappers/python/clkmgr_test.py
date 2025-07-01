# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
#
# Test Python Clock Manager client
#
# @author Christopher Hall <christopher.s.hall@@intel.com>
# @copyright © 2024 Intel Corporation.
#
# @note This is a sample code, not a product! You should use it as a reference.
#
# To run in development environment:
# LD_PRELOAD=../../.libs/libclkmgr.so PYTHONPATH=3 python3 clkmgr_test.py
#
###############################################################################

import os
import errno
import sys
import getopt
import signal
import time
import clkmgr

# Use to capture signal break
signal_flag = False
# Parameters that can be set with command line
ptp4lClockOffsetThreshold = 100000
chronyClockOffsetThreshold = 100000
subscribeAll = False
userInput = False
idleTime = 1
timeout = 10
clockSyncData = clkmgr.ClockSyncData()
event2Sub = (clkmgr.EventGMOffset | clkmgr.EventSyncedToGM |
    clkmgr.EventASCapable | clkmgr.EventGMChanged)
composite_event = (clkmgr.EventGMOffset | clkmgr.EventSyncedToGM |
    clkmgr.EventASCapable)
chrony_event = clkmgr.EventGMOffset
# Subscribe data
ptp4lSub = clkmgr.PTPClockSubscription()
chronySub = clkmgr.SysClockSubscription()
# Array of indexes to subscribe
index = list()

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def signal_handler(signum, frame):
    global signal_flag
    print(' Exit ...')
    signal_flag = True

def isPositiveValue(optarg, errorMessage):
    ret = int(optarg)
    if ret <= 0:
        eprint(errorMessage)
        sys.exit(2)
    return ret

def ClockManagerGetTime():
    ts = clkmgr.timespec()
    if clkmgr.ClockManager.getTime(ts):
        print('[clkmgr] Current Time of CLOCK_REALTIME: {} ns'
            .format((ts.tv_sec * 1000000000) + ts.tv_nsec))
    else:
        print('clock_gettime failed: {}'.format(os.strerror(errno.errorcode)))

def getMonotonicTime():
    return time.monotonic();

def printOut():
    ClockManagerGetTime
    hd2l = '|'+('-'* 30)+'|'+('-'* 28)+'|'
    hd3b = '| %-28s | %-12d | %-11d |'
    hd3 = '|'+('-'* 30)+'|'+('-'* 14)+'|'+('-'* 13)+'|'
    gm_identity = [None] * 8
    print(hd3)
    print('| %-28s | %-12s | %-11s |' % ('Events', 'Event Status', 'Event Count'))
    ptpClock = clockSyncData.getPtp()
    sysClock = clockSyncData.getSysClock()
    print(hd3)
    if composite_event != 0:
        print(hd3b % ('ptp_isCompositeEventMet', ptpClock.isCompositeEventMet(), ptpClock.getCompositeEventCount()))
        if composite_event & clkmgr.EventGMOffset:
            print('| - %-26s | %-12s | %-11s |' % ('isOffsetInRange', '', ''))
        if composite_event & clkmgr.EventSyncedToGM:
            print('| - %-26s | %-12s | %-11s |' % ('isSyncedWithGm', '', ''))
        if composite_event & clkmgr.EventASCapable:
            print('| - %-26s | %-12s | %-11s |' % ('isAsCapable', '', ''))
    if event2Sub != 0:
        print(hd3)
        if event2Sub & clkmgr.EventGMOffset:
            print(hd3b % ('ptp_isOffsetInRange', ptpClock.isOffsetInRange(), ptpClock.getOffsetInRangeEventCount()))
        if event2Sub & clkmgr.EventSyncedToGM:
            print(hd3b % ('ptp_isSyncedWithGm', ptpClock.isSyncedWithGm(), ptpClock.getSyncedWithGmEventCount()))
        if event2Sub & clkmgr.EventASCapable:
            print(hd3b % ('ptp_isAsCapable', ptpClock.isAsCapable(), ptpClock.getAsCapableEventCount()))
        if event2Sub & clkmgr.EventGMChanged:
            print(hd3b % ('ptp_isGmChanged', ptpClock.isGmChanged(), ptpClock.getGmChangedEventCount()))
    print(hd3)
    print('| %-28s |     %-19ld ns |' % ('ptp_clockOffset', ptpClock.getClockOffset()))
    gmClockUUID = ptpClock.getGmIdentity()
    # Copy the uint64_t into the array
    for i in range(8):
        gm_identity[i] = (gmClockUUID >> (8 * (7 - i))) & 0xff
    print('| %-28s |     %02x%02x%02x.%02x%02x.%02x%02x%02x     |' % ('ptp_gmIdentity',
        gm_identity[0], gm_identity[1],
        gm_identity[2], gm_identity[3],
        gm_identity[4], gm_identity[5],
        gm_identity[6], gm_identity[7]))
    print('| %-28s |     %-19ld us |' % ('ptp_syncInterval', ptpClock.getSyncInterval()))
    print('| %-28s |     %-19ld ns |' % ('ptp_notificationTimestamp', ptpClock.getNotificationTimestamp()))
    print(hd2l)
    if clockSyncData.haveSys():
        print(hd3b % ('chrony_isOffsetInRange', sysClock.isOffsetInRange(), sysClock.getOffsetInRangeEventCount()))
        print(hd2l)
        print('| %-28s |     %-19ld ns |' % ('chrony_clockOffset', sysClock.getClockOffset()))
        identity_string = ''
        for i in range(4):
            byteVal = (sysClock.getGmIdentity() >> (8 * (3 - i))) & 0xFF
            if byteVal == 0 or byteVal == 9:
                identity_string += ' '
            else:
                s = chr(byteVal) + ''
                if s.isprintable():
                    identity_string += s
                else:
                    identity_string += '.'
        print('| %-28s |     %-19s    |' % ('chrony_gmIdentity', identity_string))
        print('| %-28s |     %-19ld us |' % ('chrony_syncInterval', sysClock.getSyncInterval()))
        print('| %-28s |     %-19ld ns |' % ('chrony_notificationTimestamp', sysClock.getNotificationTimestamp()))
        print(hd2l)
    print()

def usage():
    print('''
Usage of {} :
Options:
  -a subscribe to all time base indices
     Default: timeBaseIndex: 1
  -p enable user to subscribe to specific time base indices
  -s subscribe_event_mask
     Default: {}
     Bit 0: EventGMOffset
     Bit 1: EventSyncedToGM
     Bit 2: EventASCapable
     Bit 3: EventGMChanged
  -c composite_event_mask
     Default: {}
     Bit 0: EventGMOffset
     Bit 1: EventSyncedToGM
     Bit 2: EventASCapable
  -n chrony_event_mask
     Default: {}
     Bit 0: EventGMOffset
  -l gm offset threshold (ns)
     Default: {} ns
  -i idle time (s)
     Default: {} s
  -m chrony offset threshold (ns)
     Default: {} ns
  -t timeout in waiting notification event (s)
     Default: {} s
'''[:-1].format(os.path.basename(sys.argv[0]), hex(event2Sub),
    hex(composite_event), hex(chrony_event), ptp4lClockOffsetThreshold,
    idleTime, chronyClockOffsetThreshold, timeout))

def main():
    global subscribeAll, userInput, event2Sub, composite_event, chrony_event, \
        ptp4lClockOffsetThreshold, idleTime, timeout, \
        chronyClockOffsetThreshold
    try:
        opts, args = getopt.gnu_getopt(sys.argv[1:], 'aps:c:n:l:i:t:m:h')
    except getopt.GetoptError as err:
        eprint(err)
        usage()
        sys.exit(2)
    for o, a in opts:
        if o == '-h':
            usage()
            return
        elif o == '-a':
            subscribeAll = True
        elif o == '-p':
            userInput = True
        elif o == '-s':
            event2Sub = int(a, base=0)
        elif o == '-c':
            composite_event = int(a, base=0)
        elif o == '-n':
            chrony_event = int(a, base=0)
        elif o == '-l':
            ptp4lClockOffsetThreshold = isPositiveValue(a,
                'Invalid ptp4l GM Offset threshold!')
        elif o == '-i':
            idleTime = isPositiveValue(a, 'Invalid idle time!')
        elif o == '-t':
            timeout = isPositiveValue(a, 'Invalid timeout!')
        elif o == '-m':
            chronyClockOffsetThreshold = isPositiveValue(a,
                'Invalid Chrony Offset threshold!')
        else:
            assert False, 'unhandled option'
    ptp4lSub.setEventMask(event2Sub)
    ptp4lSub.setClockOffsetThreshold(ptp4lClockOffsetThreshold)
    ptp4lSub.setCompositeEventMask(composite_event)
    chronySub.setEventMask(chrony_event)
    chronySub.setClockOffsetThreshold(chronyClockOffsetThreshold)
    print('[clkmgr] set subscribe event : {}'
        .format(hex(ptp4lSub.getEventMask())))
    print('[clkmgr] set composite event : {}'
        .format(hex(ptp4lSub.getCompositeEventMask())))
    print('[clkmgr] set chrony event : {}'.format(hex(chrony_event)))
    print('GM Offset threshold: {} ns'.format(ptp4lClockOffsetThreshold))
    print('Chrony Offset threshold: {} ns'.format(chronyClockOffsetThreshold))
    if userInput and not subscribeAll:
        line = input('Enter the time base indices to subscribe ' +
            '(comma-separated, default is 1): ')
        if len(line) > 0:
            for s in line.split(','):
                str = s.strip()
                if len(str) > 0:
                    try:
                        idx = int(str)
                    except:
                        idx = 0
                    if idx <= 0:
                        eprint('Invalid time base index: line!')
                        sys.exit(2)
                    index.append(idx)
        else:
            print('Invalid input. Using default time base index 1.')
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    signal.signal(signal.SIGHUP, signal_handler)
    ret = main_body()
    clkmgr.ClockManager.disconnect()
    if ret != None and ret > 0:
        sys.exit(2)

def main_body():
    if not clkmgr.ClockManager.connect():
        eprint('[clkmgr] failure in connecting !!!')
        return 2
    time.sleep(1)
    print('[clkmgr] List of available clock:')
    # Print out each member of the Time Base configuration
    size = clkmgr.TimeBaseConfigurations.size()
    overallSub = [None] * (size + 1) # Array of clkmgr.ClockSyncSubscription
    for i in range(size):
        cfg = clkmgr.TimeBaseConfigurations.getRecord(i + 1)
        idx = cfg.index()
        print('TimeBaseIndex: {}'.format(idx))
        print('timeBaseName: {}'.format(cfg.name()))
        Subscribe = clkmgr.ClockSyncSubscription()
        if cfg.havePtp():
            ptp = cfg.ptp()
            print('interfaceName: {}'.format(ptp.ifName()))
            print('transportSpecific: {}'.format(ptp.transportSpecific()))
            print('domainNumber: {}'.format(ptp.domainNumber()))
            Subscribe.setPtpSubscription(ptp4lSub)
        if cfg.haveSysClock():
            Subscribe.setSysSubscription(chronySub)
        overallSub[idx] = Subscribe
        if subscribeAll:
            index.append(idx)
    # Default
    if len(index) == 0:
        index.append(1)

    for idx in index:
        if not clkmgr.TimeBaseConfigurations.isTimeBaseIndexPresent(idx):
            eprint("[clkmgr] Index {} does not exist".format(idx))
            return 2
        print('[clkmgr] Subscribe to time base index: {}'.format(idx))
        if not clkmgr.ClockManager.subscribe(overallSub[idx], idx, clockSyncData):
            eprint('[clkmgr] Failure in subscribing to clkmgr Proxy !!!')
            return 2
        print('[clkmgr][%.3f] Obtained data from Subscription Event:' %
            getMonotonicTime())
        printOut()

    time.sleep(1)

    while not signal_flag:
        for idx in index:
            if signal_flag:
                return
            print('[clkmgr][%.3f] Waiting Notification from time base index %d ...' %
                (getMonotonicTime(), idx))
            retval = clkmgr.ClockManager.statusWait(timeout, idx, clockSyncData)
            match retval:
                case clkmgr.SWRLostConnection:
                    print('[clkmgr][%.3f] Terminating: lost connection to clkmgr Proxy' %
                            getMonotonicTime())
                    return
                case clkmgr.SWRInvalidArgument:
                    eprint('[clkmgr][%.3f] Terminating: Invalid argument' %
                         getMonotonicTime())
                    return 2
                case clkmgr.SWRNoEventDetected:
                    print('[clkmgr][%.3f] No event status changes identified in %d seconds.' %
                            (getMonotonicTime(), timeout))
                    print()
                    print('[clkmgr][%.3f] sleep for %d seconds...' %
                        (getMonotonicTime(), idleTime))
                case clkmgr.SWREventDetected:
                    print('[clkmgr][%.3f] Obtained data from Notification Event:' %
                        getMonotonicTime())
                    printOut()
                    print('[clkmgr][%.3f] sleep for %d seconds...' %
                        (getMonotonicTime(), idleTime))
                case _:
                    print('[clkmgr][%.3f] Warning: Should not enter this %s %d' %
                        (getMonotonicTime(),
                         'switch case, unexpected status code', retval))
                    return
            print()
            if signal_flag:
                return
            time.sleep(idleTime)

main()
