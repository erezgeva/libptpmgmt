-- SPDX-License-Identifier: BSD-3-Clause
-- SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.

--[[
 - Test Lua Clock Manager client

 - @author Christopher Hall <christopher.s.hall@@intel.com>
 - @copyright © 2024 Intel Corporation.
 -
 - @note This is a sample code, not a product! You should use it as a reference.
 -
 - To run in development environment:
 - LD_PRELOAD=../../.libs/libclkmgr.so LUA_CPATH="./5.3/?.so;;" lua5.3 clkmgr_test.lua
 - ]]

require 'posix'
require 'clkmgr'
local signal = require 'posix.signal'
local unistd = require 'posix.unistd'
local time = require 'posix.time'

local signal_flag = false

-- Global variables for event configuration and clock sync data
local event2Sub = clkmgr.EventGMOffset + clkmgr.EventSyncedToGM +
    clkmgr.EventASCapable + clkmgr.EventGMChanged
local composite_event = clkmgr.EventGMOffset +
    clkmgr.EventSyncedToGM + clkmgr.EventASCapable
local chrony_event = clkmgr.EventGMOffset
local clockSyncData = nil

local function signal_handler(signo)
    print(' Exit ...')
    signal_flag = true
end

local function isPositiveValue(opt, def, msg)
    local ret
    if opt == nil then
        return def
    end
    ret = tonumber(opt)
    if ret == nil then
        io.stderr:write("'" .. opt .. "' is invalid number\n")
        os.exit(2)
    end
    if ret <= 0 then
        io.stderr:write(msg .. '\n')
        os.exit(2)
    end
    return ret
end

local function toHexNum(opt, def)
    local ret
    if opt == nil then
        return def
    end
    if string.match(opt, '^0[Xx]') ~= nil then
        ret = tonumber(string.sub(opt, 3), 16)
    else
        ret = tonumber(opt)
    end
    if ret == nil then
        io.stderr:write("'" .. opt .. "' is invalid number\n")
        os.exit(2)
    end
    return ret
end

local function haveFlag(val, flag)
    return (val & flag) ~= 0
end

local function uint64ToArry(uid)
    -- Copy the uint64_t into the array
    local i
    local ret = {}
    for i = 0,8 do
        ret[i] = (uid >> (8 * (7 - i))) & 0xff
    end
    return ret
end

local function ClockManagerGetTime()
    local ts = clkmgr.timespec()
    if clkmgr.ClockManager.getTime(ts) then
        print(string.format('[clkmgr] Current Time of CLOCK_REALTIME: %d ns',
            (ts.tv_sec * 1000000000) + ts.tv_nsec))
    else
        local msg = posix.errno()
        io.stderr:write('clock_gettime failed: ' .. msg .. '\n')
    end
end

local function getMonotonicTime()
    local ts = time.clock_gettime(time.CLOCK_MONOTONIC)
    return ts.tv_sec + (ts.tv_nsec / 1000000000.0)
end

local function printOut()
    local hd2l = '|'..string.rep('-', 30)..'|'..string.rep('-', 28)..'|'
    local hd3b = '| %-28s | %-12s | %-11d |'
    local hd3 = '|'..string.rep('-', 30)..'|'..string.rep('-', 14)..
        '|'..string.rep('-', 13)..'|'
    ClockManagerGetTime()
    print(hd3)
    print(string.format('| %-28s | %-12s | %-11s |',
            'Events', 'Event Status', 'Event Count'))
    local ptpClock = clockSyncData:getPtp()
    local sysClock = clockSyncData:getSysClock()
    
    if composite_event ~= 0 then
        print(hd3)
        print(string.format(hd3b, 'ptp_isCompositeEventMet',
            tostring(ptpClock:isCompositeEventMet()),
            ptpClock:getCompositeEventCount()))
        if haveFlag(composite_event, clkmgr.EventGMOffset) then
            print(string.format('| - %-26s | %-12s | %-11s |',
                'isOffsetInRange', '', ''))
        end
        if haveFlag(composite_event, clkmgr.EventSyncedToGM) then
            print(string.format('| - %-26s | %-12s | %-11s |',
                'isSyncedWithGm', '', ''))
        end
        if haveFlag(composite_event, clkmgr.EventASCapable) then
            print(string.format('| - %-26s | %-12s | %-11s |',
                'isAsCapable', '', ''))
        end
    end
    
    if event2Sub ~= 0 then
        print(hd3)
        if haveFlag(event2Sub, clkmgr.EventGMOffset) then
            print(string.format(hd3b, 'ptp_isOffsetInRange',
                tostring(ptpClock:isOffsetInRange()),
                ptpClock:getOffsetInRangeEventCount()))
        end
        if haveFlag(event2Sub, clkmgr.EventSyncedToGM) then
            print(string.format(hd3b, 'ptp_isSyncedWithGm',
                tostring(ptpClock:isSyncedWithGm()),
                ptpClock:getSyncedWithGmEventCount()))
        end
        if haveFlag(event2Sub, clkmgr.EventASCapable) then
            print(string.format(hd3b, 'ptp_isAsCapable',
                tostring(ptpClock:isAsCapable()),
                ptpClock:getAsCapableEventCount()))
        end
        if haveFlag(event2Sub, clkmgr.EventGMChanged) then
            print(string.format(hd3b, 'ptp_isGmChanged',
                tostring(ptpClock:isGmChanged()),
                ptpClock:getGmChangedEventCount()))
        end
    end
    print(hd3)
    
    local gmClockUUID = ptpClock:getGmIdentityStr()
    print(string.format('| %-28s |     %-19d ns |',
        'ptp_clockOffset', ptpClock:getClockOffset()))
    print(string.format('| %-28s |     %s     |',
        'ptp_gmIdentity', gmClockUUID))
    print(string.format('| %-28s |     %-19d us |',
        'ptp_syncInterval', ptpClock:getSyncInterval()))
    print(string.format('| %-28s |     %-19d ns |',
        'ptp_notificationTimestamp', ptpClock:getNotificationTimestamp()))
    
    if clockSyncData:haveSys() ~= 0 then 
        local identityString = ""
        local gmIdentity = sysClock:getGmIdentity()
        for i = 0, 3 do
            local byte = (gmIdentity >> (8 * (3 - i))) & 0xFF
            identityString = identityString .. string.char(byte)
        end
        
        print(hd2l)
        print(string.format(hd3b, 'chrony_isOffsetInRange',
            tostring(sysClock:isOffsetInRange()),
            sysClock:getOffsetInRangeEventCount()))
        print(hd2l)
        print(string.format('| %-28s |     %-19d ns |',
            'chrony_clockOffset', sysClock:getClockOffset()))
        print(string.format('| %-28s |     %-19s    |',
            'chrony_gmIdentity', identityString))
        print(string.format('| %-28s |     %-19d us |',
            'chrony_syncInterval', sysClock:getSyncInterval()))
        print(string.format('| %-28s |     %-19d ns |',
            'chrony_notificationTimestamp', sysClock:getNotificationTimestamp()))
        print(hd2l)
    end
    print()
end

function main()
    local ptp4lClockOffsetThreshold = 100000
    local chronyClockOffsetThreshold = 100000
    local subscribeAll, userInput = false, false
    local timeout, idleTime = 10, 1
    local index = {} -- Array of indexes to subscribe
    clockSyncData = clkmgr.ClockSyncData()
    local ptp4lSub = clkmgr.PTPClockSubscription()
    local chronySub = clkmgr.SysClockSubscription()
    local overallSub = {} -- Array of clkmgr.ClockSyncSubscription
    local me = arg[0]:match'[^/]*$'
    local lastArg
    local opts = {} -- Strore arguments from command line
    local usage = 0
    for k, val in pairs(arg) do
        if k > 0 then
            if lastArg == nil then
                if val == '-h' then
                    usage = 1
                    break
                elseif val == '-a' then
                    subscribeAll = true
                elseif val == '-p' then
                    userInput = true
                elseif string.match(val, '^-[sclnittm]$') ~= nil then
                    lastArg = val
                else
                    lastArg = val
                    usage = 2
                    break
                end
            else
                opts[lastArg] = val
                lastArg = nil
            end
        end
    end
    if usage > 0 or lastArg ~= nil then
        local handle
        if usage == 1 then
            handle = io.stdout
        else
            if usage == 2 then
                io.stderr:write("parameter '" .. lastArg
                    .. "' is not supported \n\n")
            else
                io.stderr:write("parameter '" .. lastArg
                    .. "' does not have argument\n\n")
            end
            handle = io.stderr
        end
        handle:write([=[
Usage of ]=] .. me .. [=[ :
Options:
  -a subscribe to all time base indices
     Default: timeBaseIndex: 1
  -p enable user to subscribe to specific time base indices
  -s subscribe_event_mask
     Default: ]=] .. string.format('0x%x\n', event2Sub) .. [=[
     Bit 0: EventGMOffset
     Bit 1: EventSyncedToGM
     Bit 2: EventASCapable
     Bit 3: EventGMChanged
  -c composite_event_mask
     Default: ]=] .. string.format('0x%x\n', composite_event) .. [=[
     Bit 0: EventGMOffset
     Bit 1: EventSyncedToGM
     Bit 2: EventASCapable
  -n chrony_event_mask
     Default: ]=] .. string.format('0x%x\n', chrony_event) .. [=[
     Bit 0: EventGMOffset
  -l gm offset threshold (ns)
     Default: ]=] .. ptp4lClockOffsetThreshold .. [=[ ns
  -i idle time (s)
     Default: ]=] .. idleTime .. [=[ s
  -m chrony offset threshold (ns)
     Default: ]=] .. chronyClockOffsetThreshold .. [=[ ns
  -t timeout in waiting notification event (s)
     Default: ]=] .. timeout .. ' s\n')
        if usage == 1 then
            os.exit()
        else
            os.exit(2)
        end
    end
    event2Sub = toHexNum(opts['-s'], event2Sub)
    composite_event = toHexNum(opts['-c'], composite_event)
    chrony_event = toHexNum(opts['-n'], chrony_event)
    ptp4lClockOffsetThreshold = isPositiveValue(opts['-l'],
        ptp4lClockOffsetThreshold, 'Invalid ptp4l GM Offset threshold!')
    idleTime = isPositiveValue(opts['-i'], idleTime, 'Invalid idle time!')
    timeout = isPositiveValue(opts['-t'], timeout, 'Invalid timeout!')
    chronyClockOffsetThreshold = isPositiveValue(opts['-m'],
        chronyClockOffsetThreshold, 'Invalid Chrony Offset threshold!')
    ptp4lSub:setEventMask(event2Sub)
    ptp4lSub:setClockOffsetThreshold(ptp4lClockOffsetThreshold)
    ptp4lSub:setCompositeEventMask(composite_event)
    chronySub:setEventMask(chrony_event)
    chronySub:setClockOffsetThreshold(chronyClockOffsetThreshold)
    print(string.format('[clkmgr] set subscribe event : 0x%x',
        ptp4lSub:getEventMask()))
    print(string.format('[clkmgr] set composite event : 0x%x',
        ptp4lSub:getCompositeEventMask()))
    print('GM Offset threshold: ptp4lClockOffsetThreshold ns')
    print(string.format('[clkmgr] set chrony event : 0x%x',
        chronySub:getEventMask()))
    print('Chrony Offset threshold: chronyClockOffsetThreshold ns')
    if userInput and not subscribeAll then
        io.write('Enter the time base indices to subscribe ' ..
            '(comma-separated, default is 1): ')
        local line = io.read()
        if string.len(line) > 0 then
            local s
            for s in string.gmatch(line, "([^, ]*)") do
                if string.len(s) > 0 then
                    local idx = tonumber(s)
                    if idx == nil or idx <= 0 then
                        io.stderr:write('Invalid time base index: ' .. line .. '!\n')
                        os.exit(2)
                    end
                    table.insert(index, idx)
                end
            end
        else
            print('Invalid input. Using default time base index 1.')
        end
    end
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    signal.signal(signal.SIGHUP, signal_handler)
    local dieMsg, size, idx, i
    if not clkmgr.ClockManager.connect() then
        dieMsg = '[clkmgr] failure in connecting !!!'
        goto do_exit
    end
    unistd.sleep(1)
    print('[clkmgr] List of available clock:')
    -- Print out each member of the Time Base configuration
    size = clkmgr.TimeBaseConfigurations.size()
    for i=1,size do
        local cfg = clkmgr.TimeBaseConfigurations.getRecord(i)
        idx = cfg:index()
        print(string.format('TimeBaseIndex: %d', idx))
        print('timeBaseName:', cfg:name())
        local subscribe = clkmgr.ClockSyncSubscription()
        if cfg:havePtp() then
            local ptp = cfg:ptp()
            print('interfaceName: ', ptp:ifName())
            print(string.format('transportSpecific: %d', ptp:transportSpecific()))
            print(string.format('domainNumber: %d', ptp:domainNumber()))
            subscribe:setPtpSubscription(ptp4lSub)
        end
        if cfg:haveSysClock() then
            subscribe:setSysSubscription(chronySub)
        end
        overallSub[idx] = subscribe
        print()
        if subscribeAll then
            table.insert(index, idx)
        end
    end
    -- Default
    if #index == 0 then
        table.insert(index, 1)
    end
    for _, idx in pairs(index) do
        if not clkmgr.TimeBaseConfigurations.isTimeBaseIndexPresent(idx) then
            dieMsg = string.format('[clkmgr] Index %d does not exist', idx)
            goto do_exit
        end
        print('[clkmgr] Subscribe to time base index:', idx)
        if not clkmgr.ClockManager.subscribe(overallSub[idx],
            idx, clockSyncData) then
            dieMsg = '[clkmgr] Failure in subscribing to clkmgr Proxy !!!'
            goto do_exit
        end
        print(string.format('[clkmgr][%.3f] Obtained data from Subscription Event:',
            getMonotonicTime()))
        printOut()
    end
    unistd.sleep(1)

    while not signal_flag do
        ::inner_loop::
        for _, idx in pairs(index) do
            if signal_flag then
                goto do_exit
            end
            print(string.format('[clkmgr][%.3f] Waiting Notification ' ..
                'from time base index %d ...', getMonotonicTime(), idx))
            local retval =
                clkmgr.ClockManager.statusWait(timeout, idx, clockSyncData)
            if retval == clkmgr.SWRLostConnection then
                print(string.format('[clkmgr][%.3f] Terminating: ' ..
                    'lost connection to clkmgr Proxy', getMonotonicTime()))
                goto do_exit
            elseif retval == clkmgr.SWRInvalidArgument then
                dieMsg = string.format('[clkmgr][%.3f] Terminating: ' ..
                    'Invalid argument', getMonotonicTime())
                goto do_exit
            elseif retval == clkmgr.SWRNoEventDetected then
                print(string.format('[clkmgr][%.3f] No event status changes ' ..
                    'identified in %d seconds.', getMonotonicTime(), timeout))
                print()
                print(string.format('[clkmgr][%.3f] sleep for %d seconds...',
                    getMonotonicTime(), idleTime))
                print()
                if signal_flag then
                    goto do_exit
                end
                unistd.sleep(idleTime)
                goto inner_loop
            elseif retval == clkmgr.SWREventDetected then
                print(string.format('[clkmgr][%.3f] Obtained data ' ..
                    'from Notification Event:', getMonotonicTime()))
            else
                print(string.format('[clkmgr][%.3f] Warning: ' ..
                    'Should not enter this switch case, unexpected status code %d',
                    getMonotonicTime(), retval))
                goto do_exit
            end
            printOut()
            print(string.format('[clkmgr][%.3f] sleep for %d seconds...',
                getMonotonicTime(), idleTime))
            print()
            if signal_flag then
                goto do_exit
            end
            unistd.sleep(idleTime)
        end
    end

    ::do_exit::

    clkmgr.ClockManager.disconnect()
    if dieMsg ~= nil then
        io.stderr:write(dieMsg .. '\n')
        os.exit(2)
    end
end
main()
