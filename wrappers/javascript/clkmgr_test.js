/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2026 Intel Corporation. */

/** @file
 *
 * Test JavaScript Clock Manager client
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2026 Intel Corporation.
 *
 * @note This is a sample code, not a product! You should use it as a reference.
 *
 * To run in development environment with JavaScriptCore:
 * rm -f clkmgr.js;LD_LIBRARY_PATH=../../.libs:./jsc ./jsc_run clkmgr_test.js
 *
 * To run in development environment with Node.js using V8 engine:
 * echo "module.exports = require('./v8/clkmgr.node');" > clkmgr.js && LD_LIBRARY_PATH=../../.libs ADD_PATH=./ node clkmgr_test.js
 *
 * To run in development environment with Node.js using N-API:
 * echo "module.exports = require('./napi/clkmgr.node');" > clkmgr.js && LD_LIBRARY_PATH=../../.libs ADD_PATH=./ node clkmgr_test.js
 *
 */

if(typeof process != 'undefined' && process != null) {
  // Bridging Node.js
  ADD_PATH = process.env.ADD_PATH;
  args = process.argv.slice(1);
  // Sleep in seconds
  sleep = function(s){return new Promise((resolve) => setTimeout(resolve, s * 1000));};
  // Signal
  signal = function(name, hadnler) { process.on(name, hadnler); };
  // reading
  const readline = require('readline');
  const rl = readline.createInterface({input:process.stdin,output:process.stdout});
  read = function(str) { return new Promise(r => { rl.question(str, r); }); };
  readClose = function() { rl.close(); };
} else {
  readClose = function() {};
}
if(typeof ADD_PATH === 'undefined' || ADD_PATH == null)
  ADD_PATH = '';
const clkmgr = require(ADD_PATH + 'clkmgr');

function fmt(len, val) {
  const str = val.toString();
  const pad = len - str.length;
  if(pad > 0)
    return ' '.repeat(pad) + str;
  return str;
}

var signal_flag = false;
var clockSyncData = new clkmgr.ClockSyncData;
var ptpClock = clockSyncData.getPtp();
var sysClock = clockSyncData.getSysClock();
var event2Sub = clkmgr.EventOffsetInRange | clkmgr.EventSyncedWithGm |
                clkmgr.EventAsCapable | clkmgr.EventGmChanged;
var composite_event = clkmgr.EventOffsetInRange | clkmgr.EventSyncedWithGm |
                      clkmgr.EventAsCapable;
var chronyEvent = clkmgr.EventOffsetInRange;
var me = args[0].includes('/') ? args[0].split('/').reverse()[0] : args[0];
var ptp4lClockOffsetThreshold = 100000;
var chronyClockOffsetThreshold = 100000;
var idleTime = 1;
var timeout = 10;
var userInput = false;
var subscribeAll = false;
var indexList = Array();
var ptp4lSub = new clkmgr.PTPClockSubscription;
var chronySub = new clkmgr.SysClockSubscription;
var overallSub = {}; // map index to ClockSyncSubscription

function signal_handler() {
  console.log('');
  console.log(' Exit ...');
  signal_flag = true;
}
function getMonotonicTime() {
  // milliseconds to seconds
  return Date.now() / 1000;
}
function isPositiveValue(optarg, errorMessage) {
  const ret = parseInt(optarg);
  if(isNaN(ret) || ret < 0) {
    console.error(errorMessage);
    return NaN;
  }
  return ret;
}
function isValidTimeout(optarg, errorMessage) {
  const ret = parseInt(optarg);
  if(isNaN(ret) || ret < -1) {
    console.error(errorMessage);
    return NaN;
  }
  return ret;
}
function printOut() {
  let ts = new clkmgr.timespec;
  if(clkmgr.ClockManager.getTime(ts))
    console.log(`[clkmgr] Current Time of CLOCK_REALTIME: ${ts.tv_sec * 1000000000 + ts.tv_nsec} ns`);
  else
    console.error('clock_gettime failed');
  console.log('|------------------------------|--------------|-------------|');
  console.log('| Event Count                  |              |             |');
  console.log('|------------------------------|--------------|-------------|');
  if(clockSyncData.havePTP()) {
    if(composite_event)
      console.log('|      ptp_isCompositeEventMet |' +
                  ` ${fmt(12, ptpClock.isCompositeEventMet())} |` +
                  ` ${fmt(11, ptpClock.getCompositeEventCount())} |`);
    if(composite_event & clkmgr.EventOffsetInRange)
      console.log('|              isOffsetInRange |              |             |');
    if(composite_event & clkmgr.EventSyncedWithGm)
      console.log('|               isSyncedWithGm |              |             |');
    if(composite_event & clkmgr.EventAsCapable)
      console.log('|                  isAsCapable |              |             |');
    if(event2Sub)
      console.log('|------------------------------|--------------|-------------|');
    if(event2Sub & clkmgr.EventOffsetInRange)
      console.log('|          ptp_isOffsetInRange |' +
                  ` ${fmt(12, ptpClock.isOffsetInRange())} |` +
                  ` ${fmt(11, ptpClock.getOffsetInRangeEventCount())} |`);
    if(event2Sub & clkmgr.EventSyncedWithGm)
      console.log('|           ptp_isSyncedWithGm |' +
                  ` ${fmt(12, ptpClock.isSyncedWithGm())} |` +
                  ` ${fmt(11, ptpClock.getSyncedWithGmEventCount())} |`);
    if(event2Sub & clkmgr.EventAsCapable)
      console.log('|              ptp_isAsCapable |' +
                  ` ${fmt(12, ptpClock.isAsCapable())} |` +
                  ` ${fmt(11, ptpClock.getAsCapableEventCount())} |`);
    if(event2Sub & clkmgr.EventGmChanged)
      console.log('|              ptp_isGmChanged |' +
                  ` ${fmt(12, ptpClock.isGmChanged())} |` +
                  ` ${fmt(11, ptpClock.getGmChangedEventCount())} |`);
    console.log('|------------------------------|--------------|-------------|');
    console.log('|              ptp_clockOffset |' +
                ` ${fmt(23, ptpClock.getClockOffset())} ns |`);
    console.log('|               ptp_gmIdentity |' +
                ` ${fmt(26, ptpClock.getGmIdentityStr())} |`);
    console.log('|             ptp_syncInterval |' +
                ` ${fmt(23, ptpClock.getSyncInterval())} us |`);
    console.log('|    ptp_notificationTimestamp |' +
                ` ${fmt(23, ptpClock.getNotificationTimestamp())} ns |`);
    console.log('|------------------------------|----------------------------|');
  }
  if(clockSyncData.haveSys()) {
    console.log('|       chrony_isOffsetInRange |' +
                ` ${fmt(12, sysClock.isOffsetInRange())} |` +
                ` ${fmt(11, sysClock.getOffsetInRangeEventCount())} |`);
    console.log('|------------------------------|----------------------------|');
    console.log('|           chrony_clockOffset |' +
                ` ${fmt(23, sysClock.getClockOffset())} ns |`);
    let identityString = '';
    let gmIdentity = sysClock.getGmIdentity();
    for(let i = 0; i < 4; i++) {
      let val = (gmIdentity >> (8 * (3 - i))) & 0xff;
      if(val == 0 || val == 9 || val == 32)
        identityString += ' ';
      else {
        if(val > 32 && val < 127) // Ascii printable range
          identityString += String.fromCharCode(val);
        else
          identityString += '.';
      }
    }
    console.log('|            chrony_gmIdentity |' +
                ` ${fmt(26, identityString)} |`);
    console.log('|          chrony_syncInterval |' +
                ` ${fmt(23, sysClock.getSyncInterval())} us |`);
    console.log('| chrony_notificationTimestamp |' +
                ` ${fmt(23, sysClock.getNotificationTimestamp())} ns |`);
    console.log('|------------------------------|----------------------------|');
  }
  console.log('');
}
function usage(me, err) {
  str = `Usage of ${me} :
Options:
  -a subscribe to all time base indices
     Default: timeBaseIndex: 1
  -p subscribe to comma list of time base indices
  -s subscribe_event_mask
     Default: 0x${event2Sub.toString(16)}
     Bit 0: EventOffsetInRange
     Bit 1: EventSyncedWithGm
     Bit 2: EventAsCapable
     Bit 3: EventGmChanged
  -c composite_event_mask
     Default: 0x${composite_event.toString(16)}
     Bit 0: EventOffsetInRange
     Bit 1: EventSyncedWithGm
     Bit 2: EventAsCapable
  -n chrony_event_mask
     Default: 0x${chronyEvent.toString(16)}
     Bit 0: EventOffsetInRange
  -l gm offset threshold (ns)
     Default: ${ptp4lClockOffsetThreshold} ns
  -i idle time (s)
     Default: ${idleTime} s
  -m chrony offset threshold (ns)
     Default: ${chronyClockOffsetThreshold} ns
  -t timeout in waiting notification event (s)
     Default: ${timeout} s
     -1 : wait indefinitely until at least an event change occurs
      0 : retrieve the latest clock sync data immediately
     >0 : wait up to the specified number of seconds for an event
`;
  if(err)
    console.error(str);
  else
    console.log(str);
  return false;
}
function getopt() {
  const len = args.length;
  let i;
  for(i = 1; i < len; i++) {
    let s = args[i];
    if(s[0] === '-' && s.length == 2) {
      let have_optarg = (i + 1) < len;
      let optarg = have_optarg ? args[i + 1] : '';
      switch(s[1]) {
      case 'h':
        return usage(me, false);
      case 'a':
        subscribeAll = true;
        break;
      case 'p':
        userInput = true;
        break;
      case 's':
        if(!have_optarg) {
          console.error("Argument '" + s + "' lack optarg.");
          return false;
        }
        event2Sub = parseInt(optarg, 0);
        if(isNaN(event2Sub)) {
          console.error('Invalid event mask');
          return false;
        }
        // Mask with proper allowed mask
        event2Sub &= clkmgr.PTP_EVENT_ALL;
        i++;
        break;
      case 'c':
        if(!have_optarg) {
          console.error("Argument '" + s + "' lack optarg.");
          return false;
        }
        composite_event = parseInt(optarg, 0);
        if(isNaN(composite_event)) {
          console.error('Invalid composite event mask');
          return false;
        }
        // Mask with proper allowed mask
        composite_event &= clkmgr.PTP_COMPOSITE_EVENT_ALL;
        i++;
        break;
      case 'n':
        if(!have_optarg) {
          console.error("Argument '" + s + "' lack optarg.");
          return false;
        }
        chronyEvent = parseInt(optarg, 0);
        if(isNaN(chronyEvent)) {
          console.error('Invalid chrony event mask');
          return false;
        }
        // Mask with proper allowed mask
        chronyEvent &= clkmgr.EventOffsetInRange;
        i++;
        break;
      case 'l':
        if(!have_optarg) {
          console.error("Argument '" + s + "' lack optarg.");
          return false;
        }
        ptp4lClockOffsetThreshold = isPositiveValue(optarg, 'Invalid ptp4l GM Offset threshold!');
        if(isNaN(ptp4lClockOffsetThreshold))
          return false;
        i++;
        break;
      case 'i':
        if(!have_optarg) {
          console.error("Argument '" + s + "' lack optarg.");
          return false;
        }
        idleTime = isPositiveValue(optarg, 'Invalid idle time!');
        if(isNaN(idleTime))
          return false;
        i++;
        break;
      case 't':
        if(!have_optarg) {
          console.error("Argument '" + s + "' lack optarg.");
          return false;
        }
        timeout = isValidTimeout(optarg, 'Invalid timeout!');
        if(isNaN(timeout))
          return false;
        i++;
        break;
      case 'm':
        if(!have_optarg) {
          console.error("Argument '" + s + "' lack optarg.");
          return false;
        }
        chronyClockOffsetThreshold = isPositiveValue(optarg, 'Invalid Chrony Offset threshold!');
        if(isNaN(chronyClockOffsetThreshold))
          return false;
        i++;
        break;
      };
    } else {
      console.error('Wrong argument: ' + s);
      return false;
    }
  }
  return true;
}
// We are synchronize here to wait for sleep
async function eventsLoop() {
  await sleep(1);

  ptp4lSub.setEventMask(event2Sub);
  ptp4lSub.setClockOffsetThreshold(ptp4lClockOffsetThreshold);
  ptp4lSub.setCompositeEventMask(composite_event);
  chronySub.setEventMask(chronyEvent);
  chronySub.setClockOffsetThreshold(chronyClockOffsetThreshold);
  console.log('[clkmgr] set subscribe event : 0x' + ptp4lSub.getEventMask().toString(16));
  console.log('[clkmgr] set composite event : 0x' + ptp4lSub.getCompositeEventMask().toString(16));
  console.log('GM Offset threshold: \u00B1' + ptp4lClockOffsetThreshold + ' ns');
  console.log('[clkmgr] set chrony event : 0x' + chronySub.getEventMask().toString(16));
  console.log('Chrony Offset threshold: \u00B1' + chronyClockOffsetThreshold + ' ns');
  console.log('[clkmgr] List of available clock: ');
  // Print out each member of the Time Base configuration
  let size = clkmgr.TimeBaseConfigurations.size();
  for(let i = 1; i <= size; i++) {
    let cfg = clkmgr.TimeBaseConfigurations.getRecord(i);
    let idx = cfg.index();
    console.log('TimeBaseIndex: ' + idx);
    console.log('timeBaseName: ' + cfg.name());
    let clockSub = new clkmgr.ClockSyncSubscription;
    if(cfg.havePtp()) {
      console.log('interfaceName: ' + cfg.ptp().ifName());
      console.log('transportSpecific: ' + cfg.ptp().transportSpecific());
      console.log('domainNumber: ' + cfg.ptp().domainNumber());
      clockSub.setPtpSubscription(ptp4lSub);
    }
    if(cfg.haveSysClock())
      clockSub.setSysSubscription(chronySub);
    overallSub[idx] = clockSub;
    if(subscribeAll)
      indexList.push(idx);
  }
  if(indexList.length == 0)
    indexList.push(1);

  for(let idx of indexList) {
    if(!clkmgr.TimeBaseConfigurations.isTimeBaseIndexPresent(idx)) {
      console.error('[clkmgr] Index ' + idx + ' does not exist');
      return false;
    }
    console.log('[clkmgr] Subscribe to time base index: ' + idx);
    if(!clkmgr.ClockManager.subscribe(overallSub[idx], idx, clockSyncData)) {
      console.error('[clkmgr] Failure in subscribing to clkmgr Proxy !!!');
      return false;
    }
    console.log(`[clkmgr][${getMonotonicTime().toFixed(3)}] Obtained data from Subscription Event:`);
    printOut();
  }
  await sleep(1);

  while(!signal_flag) {
    for(let idx of indexList) {
      if(signal_flag)
        return true;
      console.log(`[clkmgr][${getMonotonicTime().toFixed(3)}] Waiting Notification from time base index ${idx} ...`);

      let retval = clkmgr.ClockManager.statusWait(timeout, idx, clockSyncData);
      switch(retval) {
      case clkmgr.SWRLostConnection:
        console.error(`[clkmgr][${getMonotonicTime().toFixed(3)}] Terminating: lost connection to clkmgr Proxy`);
        return false;
      case clkmgr.SWRInvalidArgument:
        console.error(`[clkmgr][${getMonotonicTime().toFixed(3)}] Terminating: Invalid argument`);
        return false;
      case clkmgr.SWRNoEventDetected:
        console.log(`[clkmgr][${getMonotonicTime().toFixed(3)}] No event status changes identified in ${timeout} seconds.`);
        console.log(`[clkmgr][${getMonotonicTime().toFixed(3)}] sleep for ${idleTime} seconds...`);
        if(signal_flag)
          return true;
        await sleep(idleTime);
        continue;
      case clkmgr.SWREventDetected:
        console.log(`[clkmgr][${getMonotonicTime().toFixed(3)}] Obtained data from Notification Event:`);
        break;
      default:
        console.error(`[clkmgr][${getMonotonicTime().toFixed(3)}] Warning: Should not enter this switch case, unexpected status code ${retval}`);
        return false;
      }
      printOut();

      console.log(`[clkmgr][${getMonotonicTime().toFixed(3)}] sleep for ${idleTime} seconds...`);
      if(signal_flag)
        return true;
      await sleep(idleTime);
    }
  }
  return true;
}
// We are synchronize here to wait for read and sleep
async function main() {
  if(!getopt()) {
    console.error('[clkmgr] Wrong command line !!!');
    throw new Error('[clkmgr] Wrong command line !!!');
    return;
  }

  if(userInput && !subscribeAll) {
    let text = await read('Enter the time base indices to subscribe (comma-separated, default is 1): ');
    for(let a of text.split(',')) {
      let i = parseInt(a);
      if(!isNaN(i) && i > 0)
        indexList.push(i);
    }
  }
  readClose(); // Close reading object, we do not need it any further!
  signal('SIGINT', signal_handler);
  signal('SIGTERM', signal_handler);
  signal('SIGHUP', signal_handler);

  if(!clkmgr.ClockManager.connect()) {
    console.error('[clkmgr] failure in connecting !!!');
    throw new Error('[clkmgr] failure in connecting !!!');
    return;
  }
  let ret = await eventsLoop();
  clkmgr.ClockManager.disconnect();
  if(!ret) {
    console.error('[clkmgr] failure in events loop !!!');
    throw new Error('[clkmgr] failure in events loop !!!');
  }
}
main();
