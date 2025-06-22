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
event2Sub = (clkmgr.EventGMOffset | clkmgr.EventSyncedToGM |
    clkmgr.EventASCapable | clkmgr.EventGMChanged)
composite_event = (clkmgr.EventGMOffset | clkmgr.EventSyncedToGM |
    clkmgr.EventASCapable)
# Subscribe data
ptp4lSub = clkmgr.PTPClockSubscription()
chronySub = clkmgr.SysClockSubscription()
# Array of indexes to subscribe
index = list()

def signal_handler(signum, frame):
    global signal_flag
    print(' Exit ...')
    signal_flag = True

def isPositiveValue(optarg, errorMessage):
    ret = int(optarg)
    if ret <= 0:
        print(errorMessage)
        sys.exit(2)
    return ret

def ClockManagerGetTime():
    ts = clkmgr.timespec()
    if clkmgr.ClockManager.getTime(ts):
        print("[clkmgr] Current Time of CLOCK_REALTIME: {} ns"
            .format((ts.tv_sec * 1000000000) + ts.tv_nsec))
    else:
        print("clock_gettime failed: {}".format(os.strerror(errno.errorcode)))

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
  -l gm offset threshold (ns)
     Default: {} ns
  -i idle time (s)
     Default: {} s
  -m chrony offset threshold (ns)
     Default: {} ns
  -t timeout in waiting notification event (s)
     Default: {} s
'''[:-1].format(os.path.basename(sys.argv[0]), hex(event2Sub),
    hex(composite_event), ptp4lClockOffsetThreshold,
    idleTime, chronyClockOffsetThreshold, timeout))

def main():
    global subscribeAll, userInput, event2Sub, composite_event, \
        ptp4lClockOffsetThreshold, idleTime, timeout, \
        chronyClockOffsetThreshold
    try:
        opts, args = getopt.gnu_getopt(sys.argv[1:], 'aps:c:l:i:t:m:h')
    except getopt.GetoptError as err:
        print(err)
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
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    signal.signal(signal.SIGHUP, signal_handler)
    ptp4lSub.setEventMask(event2Sub)
    ptp4lSub.setClockOffsetThreshold(ptp4lClockOffsetThreshold)
    ptp4lSub.setCompositeEventMask(composite_event)
    chronySub.setClockOffsetThreshold(chronyClockOffsetThreshold)
    print('[clkmgr] set subscribe event : {}'
        .format(hex(ptp4lSub.getEventMask())))
    print('[clkmgr] set composite event : {}'
        .format(hex(ptp4lSub.getCompositeEventMask())))
    print('GM Offset threshold: {} ns'.format(ptp4lClockOffsetThreshold))
    print('Chrony Offset threshold: {} ns'.format(chronyClockOffsetThreshold))
    if userInput and not subscribeAll:
        line = input('Enter the time base indices to subscribe (comma-separated, default is 1): ')
        if len(line) > 0:
            try:
                idx = int(line)
            except:
                idx = 0
            if idx <= 0:
                print('Invalid time base index: line!')
                sys.exit(2)
            index.append(idx)
        else:
            print('Invalid input. Using default time base index 1.')
    ret = main_body()
    clkmgr.ClockManager.disconnect()
    if ret != None and ret > 0:
        sys.exit(2)

def main_body():
    clockSyncData = clkmgr.ClockSyncData()
    if not clkmgr.ClockManager.connect():
        print('[clkmgr] failure in connecting !!!')
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

    gm_identity = [None] * 8
    hd2 = '|'+('-'*27)+'|'+('-'*24)+'|'
    for idx in index:
        print('Subscribe to time base index: {}'.format(idx))
        if not clkmgr.ClockManager.subscribe(overallSub[idx], idx, clockSyncData):
            print('[clkmgr] Failure in subscribing to clkmgr Proxy !!!')
            return 2
        print('[clkmgr][%.3f] Obtained data from Subscription Event:' % time.time())
        ClockManagerGetTime
        print(hd2)
        print('| %-25s | %-22s |' % ('Event', 'Event Status'))
        ptpClock = clockSyncData.getPtp()
        sysClock = clockSyncData.getSysClock()
        if event2Sub != 0:
            print(hd2)
            if event2Sub & clkmgr.EventGMOffset:
                print('| %-25s | %-22d |' % ('offset_in_range', ptpClock.isOffsetInRange()))
            if event2Sub & clkmgr.EventSyncedToGM:
                print('| %-25s | %-22d |' % ('synced_to_primary_clock', ptpClock.isSyncedWithGm()))

            if event2Sub & clkmgr.EventASCapable:
                print('| %-25s | %-22d |' % ('as_capable', ptpClock.isAsCapable()))
            if event2Sub & clkmgr.EventGMChanged:
                print('| %-25s | %-22d |' % ('gm_Changed', ptpClock.isGmChanged()))
        print(hd2)
        gmClockUUID = ptpClock.getGmIdentity()
        # Copy the uint64_t into the array
        for i in range(8):
            gm_identity[i] = (gmClockUUID >> (8 * (7 - i))) & 0xff
        print('| %-25s | %02x%02x%02x.%02x%02x.%02x%02x%02x     |' % ('GM UUID',
            gm_identity[0], gm_identity[1],
            gm_identity[2], gm_identity[3],
            gm_identity[4], gm_identity[5],
            gm_identity[6], gm_identity[7]))
        print('| %-25s | %-19ld ns |' % ('clock_offset', ptpClock.getClockOffset()))
        print('| %-25s | %-19ld ns |' % ('notification_timestamp', ptpClock.getNotificationTimestamp()))
        print('| %-25s | %-19ld us |' % ('gm_sync_interval', ptpClock.getSyncInterval()))
        print(hd2)
        if composite_event != 0:
            print('| %-25s | %-22d |' % ('composite_event', ptpClock.isCompositeEventMet()))

            if composite_event & clkmgr.EventGMOffset:
                print('| - %-23s | %-22s |' % ('offset_in_range', ''))
            if composite_event & clkmgr.EventSyncedToGM:
                print('| - %-19s | %-22s |' % ('synced_to_primary_clock', ''))
            if composite_event & clkmgr.EventASCapable:
                print('| - %-23s | %-22s |' % ('as_capable', ''))

            print(hd2)
        print()
        print(hd2)
        print('| %-25s | %-22d |' % ('chrony_offset_in_range', sysClock.isOffsetInRange()))
        print(hd2)
        print('| %-25s | %-19ld ns |' % ('chrony_clock_offset', sysClock.getClockOffset()))
        print('| %-25s | %-19lx    |' % ('chrony_clock_reference_id', sysClock.getGmIdentity()))
        print('| %-25s | %-19ld us |' % ('chrony_polling_interval', sysClock.getSyncInterval()))
        print(hd2)
        print()

    time.sleep(1)

    hd2l = '|'+('-'* 27)+'|'+('-'* 28)+'|'
    hd3b = '| %-25s | %-12d | %-11d |'
    hd3 = '|'+('-'* 27)+'|'+('-'* 14)+'|'+('-'* 13)+'|'

    while not signal_flag:
        for idx in index:
            if signal_flag:
                return
            print('[clkmgr][%.3f] Waiting Notification from time base index %d ...' % (time.time(), idx))
            retval = clkmgr.ClockManager.statusWait(timeout, idx, clockSyncData)
            if retval == 0:
                print('[clkmgr][%.3f] No event status changes identified in timeout seconds.' % time.time())
                print()
                print('[clkmgr][%.3f] sleep for %d seconds...' % (time.time(), idleTime))
                print()
                if signal_flag:
                    return
                time.sleep(idleTime)
                continue
            elif retval < 0:
                print('[clkmgr][%.3f] Terminating: lost connection to clkmgr Proxy' % time.time())
                return

            print('[clkmgr][%.3f] Obtained data from Notification Event:' % time.time())
            ClockManagerGetTime
            print(hd3)
            print('| %-25s | %-12s | %-11s |' % ('Event', 'Event Status', 'Event Count'))
            ptpClock = clockSyncData.getPtp()
            sysClock = clockSyncData.getSysClock()
            if event2Sub != 0:
                print(hd3)
                if event2Sub & clkmgr.EventGMOffset:
                    print(hd3b % ('offset_in_range', ptpClock.isOffsetInRange(), ptpClock.getOffsetInRangeEventCount()))
                if event2Sub & clkmgr.EventSyncedToGM:
                    print(hd3b % ('synced_to_primary_clock', ptpClock.isSyncedWithGm(), ptpClock.getSyncedWithGmEventCount()))
                if event2Sub & clkmgr.EventASCapable:
                    print(hd3b % ('as_capable', ptpClock.isAsCapable(), ptpClock.getAsCapableEventCount()))
                if event2Sub & clkmgr.EventGMChanged:
                    print(hd3b % ('gm_Changed', ptpClock.isGmChanged(), ptpClock.getGmChangedEventCount()))
            print(hd3)
            gmClockUUID = ptpClock.getGmIdentity()
            # Copy the uint64_t into the array
            for i in range(8):
                gm_identity[i] = (gmClockUUID >> (8 * (7 - i))) & 0xff
            print('| %-25s |     %02x%02x%02x.%02x%02x.%02x%02x%02x     |' % ('GM UUID',
                gm_identity[0], gm_identity[1],
                gm_identity[2], gm_identity[3],
                gm_identity[4], gm_identity[5],
                gm_identity[6], gm_identity[7]))
            print('| %-25s |     %-19ld ns |' % ('clock_offset', ptpClock.getClockOffset()))
            print('| %-25s |     %-19ld ns |' % ('notification_timestamp', ptpClock.getNotificationTimestamp()))
            print('| %-25s |     %-19ld us |' % ('gm_sync_interval', ptpClock.getSyncInterval()))
            print(hd3)
            if composite_event != 0:
                print(hd3b % ('composite_event', ptpClock.isCompositeEventMet(), ptpClock.getCompositeEventCount()))

                if composite_event & clkmgr.EventGMOffset:
                    print('| - %-23s | %-12s | %-11s |' % ('offset_in_range', '', ''))
                if composite_event & clkmgr.EventSyncedToGM:
                    print('| - %-19s | %-12s | %-11s |' % ('synced_to_primary_clock', '', ''))
                if composite_event & clkmgr.EventASCapable:
                    print('| - %-23s | %-12s | %-11s |' % ('as_capable', '', ''))
                print(hd3)
            print()
            print(hd2l)
            print(hd3b % ('chrony_offset_in_range', sysClock.isOffsetInRange(), sysClock.getOffsetInRangeEventCount()))
            print(hd2l)
            print('| %-25s |     %-19ld ns |' % ('chrony_clock_offset', sysClock.getClockOffset()))
            print('| %-25s |     %-19lx    |' % ('chrony_clock_reference_id', sysClock.getGmIdentity()))
            print('| %-25s |     %-19ld us |' % ('chrony_polling_interval', sysClock.getSyncInterval()))
            print(hd2l)
            print()
            print('[clkmgr][%.3f] sleep for %d seconds...' % (time.time(), idleTime))
            print()
            if signal_flag:
                return
            time.sleep(idleTime)

main()
