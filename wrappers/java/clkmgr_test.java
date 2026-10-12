/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2026 Intel Corporation. */

/** @file
 * Test Java Clock Manager client
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2026 Intel Corporation.
 *
 * @note This is a sample code, not a product! You should use it as a reference.
 *
 * Compile in development environment:
 * javac -cp clkmgr.jar clkmgr_test.java
 *
 * To run in development environment:
 * LD_LIBRARY_PATH=../../.libs/:. java -Djava.library.path=. -classpath . clkmgr_test
 *
 */

import java.math.BigInteger;
import java.awt.event.KeyEvent;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Scanner;
import clkmgr.*;

class clkmgr_client {
  final Scanner scanner = new Scanner(System.in);
  boolean have_optarg = false;
  boolean subscribeAll = false;
  boolean userInput = false;
  int event2Sub = EventIndex.EventOffsetInRange.swigValue() |
                  EventIndex.EventSyncedWithGm.swigValue() |
                  EventIndex.EventAsCapable.swigValue() |
                  EventIndex.EventGmChanged.swigValue();
  int composite_event = EventIndex.EventOffsetInRange.swigValue() |
                        EventIndex.EventSyncedWithGm.swigValue() |
                        EventIndex.EventAsCapable.swigValue();
  int chrony_event = EventIndex.EventOffsetInRange.swigValue();
  ClockSyncData clockSyncData = new ClockSyncData();
  int ptp4lClockOffsetThreshold = 100_000;
  int chronyClockOffsetThreshold = 100_000;
  int idleTime = 1;
  int timeout = 10;

  public boolean isPrintableChar(char c) {
    Character.UnicodeBlock block = Character.UnicodeBlock.of( c );
    return (!Character.isISOControl(c)) && c != KeyEvent.CHAR_UNDEFINED &&
            block != null && block != Character.UnicodeBlock.SPECIALS;
  }
  int isPositiveValue(String optarg, String errorMessage) {
    int ret = Integer.parseInt(optarg);
    if(ret >= 0)
      return ret;
    System.err.println(errorMessage);
    System.exit(2);
    return 0;
  }
  int isValidTimeout(String optarg, String errorMessage) {
    int ret = Integer.parseInt(optarg);
    if(ret >= -1)
      return ret;
    System.err.println(errorMessage);
    System.exit(2);
    return 0;
  }
  void clockManagerGetTime() {
    timespec ts = new timespec();
    if(ClockManager.getTime(ts))
      System.out.printf("[clkmgr] Current Time of CLOCK_REALTIME: %d ns%n",
                (ts.getTv_sec() * 1_000_000_000) + ts.getTv_nsec());
    else
      System.err.println("clock_gettime failed");
  }
  double getMonotonicTime() {
    return System.currentTimeMillis() / 1e3;
  }
  void printOut() {
    String hd2l = "|" + "-".repeat(30) + "|" + "-".repeat(28) + "|";
    String hd3b = "| %-28s | %-12b | %-11d |";
    String hd3 = "|" + "-".repeat(30) + "|" + "-".repeat(14) + "|" + "-".repeat(13) + "|";
    clockManagerGetTime();
    System.out.printf(hd3 + "%n| %-28s | %-12s | %-11s |%n", "Events", "Event Status", "Event Count");
    System.out.println(hd3);
    PTPClockEvent ptpClock = clockSyncData.getPtp();
    SysClockEvent sysClock = clockSyncData.getSysClock();
    if(clockSyncData.havePTP()) {
      if(composite_event > 0) {
        System.out.printf(hd3b + "%n", "ptp_isCompositeEventMet", ptpClock.isCompositeEventMet(), ptpClock.getCompositeEventCount());
        if((composite_event & EventIndex.EventOffsetInRange.swigValue()) > 0)
          System.out.printf("| - %-26s | %-12s | %-11s |%n", "isOffsetInRange", "", "");
        if((composite_event & EventIndex.EventSyncedWithGm.swigValue()) > 0)
          System.out.printf("| - %-26s | %-12s | %-11s |%n", "isSyncedWithGm", "", "");
        if((composite_event & EventIndex.EventAsCapable.swigValue()) > 0)
          System.out.printf("| - %-26s | %-12s | %-11s |%n", "isAsCapable", "", "");
      }
      if(event2Sub > 0) {
        System.out.println(hd3);
        if((event2Sub & EventIndex.EventOffsetInRange.swigValue()) > 0)
          System.out.printf(hd3b + "%n", "ptp_isOffsetInRange", ptpClock.isOffsetInRange(), ptpClock.getOffsetInRangeEventCount());
        if((event2Sub & EventIndex.EventSyncedWithGm.swigValue()) > 0)
          System.out.printf(hd3b + "%n", "ptp_isSyncedWithGm", ptpClock.isSyncedWithGm(), ptpClock.getSyncedWithGmEventCount());
        if((event2Sub & EventIndex.EventAsCapable.swigValue()) > 0)
          System.out.printf(hd3b + "%n", "ptp_isAsCapable", ptpClock.isAsCapable(), ptpClock.getAsCapableEventCount());
        if((event2Sub & EventIndex.EventGmChanged.swigValue()) > 0)
          System.out.printf(hd3b + "%n", "ptp_isGmChanged", ptpClock.isGmChanged(), ptpClock.getGmChangedEventCount());
      }
      System.out.println(hd3);
      String gmClockUUID = ptpClock.getGmIdentityStr();
      System.out.printf("| %-28s |     %-19d ns |%n", "ptp_clockOffset", ptpClock.getClockOffset());
      System.out.printf("| %-28s |     %s     |%n", "ptp_gmIdentity", gmClockUUID);
      System.out.printf("| %-28s |     %-19d us |%n| %-28s |     %-19d ns |%n", "ptp_syncInterval", ptpClock.getSyncInterval()
          , "ptp_notificationTimestamp", ptpClock.getNotificationTimestamp());
      System.out.printf(hd2l + "%n%n");
    }
    if(clockSyncData.haveSys()) {
      String identityString = "";
      BigInteger gmIdentity = sysClock.getGmIdentity();
      for (int i = 0; i < 4; i++) {
        int val = (int)(gmIdentity.longValue() >> (8 * (3 - i))) & 0xFF;
        if(val == 0 || val == 9)
          identityString = identityString + " ";
        else {
          char c = (char)val;
          if(isPrintableChar(c))
              identityString = identityString + c;
          else
              identityString = identityString + ".";
        }
      }
      System.out.printf(hd3b + "%n" +
        hd2l + "%n| %-28s |     %-19d ns |%n| %-28s |     %-19s    |%n| %-28s |     %-19d us |%n| %-28s |     %-19d ns |%n" +
        hd2l + "%n%n"
        , "chrony_isOffsetInRange", sysClock.isOffsetInRange(),
            sysClock.getOffsetInRangeEventCount()
        , "chrony_clockOffset", sysClock.getClockOffset()
        , "chrony_gmIdentity", identityString
        , "chrony_syncInterval", sysClock.getSyncInterval()
        , "chrony_notificationTimestamp", sysClock.getNotificationTimestamp());
    }
  }
  String usage(String me) {
    System.out.printf("""
    Usage of %s :
    Options:
      -a subscribe to all time base indices
         Default: timeBaseIndex: 1
      -p enable user to subscribe to specific time base indices
      -s subscribe_event_mask
         Default: 0x%x
         Bit 0: EventOffsetInRange
         Bit 1: EventSyncedWithGm
         Bit 2: EventAsCapable
         Bit 3: EventGmChanged
      -c composite_event_mask
         Default: 0x%x
         Bit 0: EventOffsetInRange
         Bit 1: EventSyncedWithGm
         Bit 2: EventAsCapable
      -n chrony_event_mask
         Default: 0x%x
         Bit 0: EventOffsetInRange
      -l gm offset threshold (ns)
         Default: %d ns
      -i idle time (s)
         Default: %d s
      -m chrony offset threshold (ns)
         Default: %d ns
      -t timeout in waiting notification event (s)
         Default: %d s
         -1 : wait indefinitely until at least an event change occurs
          0 : retrieve the latest clock sync data immediately
         >0 : wait up to the specified number of seconds for an event
    """, me, event2Sub,
    composite_event, chrony_event,
    ptp4lClockOffsetThreshold, idleTime,
    chronyClockOffsetThreshold, timeout);
    return "";
  }
  int check(String s) {
    if(!have_optarg) {
      System.err.println("Argument '" + s + "' lack optarg");
      System.exit(2);
    }
    return 1;
  }
  void getopt(String args[]) {
    int len = args.length;
    for(int i = 0; i < len; i++) {
      String s = args[i];
      if(s.startsWith("-") && s.length() == 2) {
        String optarg;
        have_optarg = (i + 1) < len;
        if(have_optarg)
          optarg = args[i + 1];
        else
          optarg = "";
        switch(s.charAt(1)) {
          case 'h':
            Path path = Paths.get(System.getProperty("jdk.launcher.sourcefile"));
            usage(path.getFileName().toString());
            break;
          case 'a':
            subscribeAll = true;
            break;
          case 'p':
            userInput = true;
            break;
          case 's':
            i += check(s);
            event2Sub = Integer.parseInt(optarg, 0);
            break;
          case 'c':
            i += check(s);
            composite_event = Integer.parseInt(optarg, 0);
            break;
          case 'n':
            i += check(s);
            chrony_event = Integer.parseInt(optarg, 0);
            break;
          case 'l':
            i += check(s);
            ptp4lClockOffsetThreshold = isPositiveValue(optarg, "Invalid ptp4l GM Offset threshold!");
            break;
          case 'i':
            i += check(s);
            idleTime = isPositiveValue(optarg, "Invalid idle time!");
            break;
          case 't':
            i += check(s);
            timeout = isValidTimeout(optarg, "Invalid timeout!");
            break;
          case 'm':
            i += check(s);
            chronyClockOffsetThreshold = isPositiveValue(optarg, "Invalid Chrony Offset threshold!");
            break;
        };
      } else {
        System.err.println("Wrong argument: " + s);
        System.exit(2);
      }
    }
  }
  public String run(String args[]) {
    String dieMsg = "";
    ArrayList<Integer> indexList = new ArrayList<Integer>(); // Array of indexes to subscribe
    PTPClockSubscription ptp4lSub = new PTPClockSubscription();
    SysClockSubscription chronySub = new SysClockSubscription();
    Map<Integer,ClockSyncSubscription> overallSub = new HashMap<Integer,ClockSyncSubscription>();
    getopt(args);
    ptp4lSub.setEventMask(event2Sub);
    ptp4lSub.setClockOffsetThreshold(ptp4lClockOffsetThreshold);
    ptp4lSub.setCompositeEventMask(composite_event);
    chronySub.setEventMask(chrony_event);
    chronySub.setClockOffsetThreshold(chronyClockOffsetThreshold);
    System.out.println("[clkmgr] set subscribe event : 0x" + Long.toHexString(ptp4lSub.getEventMask()));
    System.out.println("[clkmgr] set composite event : 0x" + Long.toHexString(ptp4lSub.getCompositeEventMask()));
    System.out.println("GM Offset threshold: ptp4lClockOffsetThreshold ns");
    System.out.println("[clkmgr] set chrony event : 0x" + Long.toHexString(chronySub.getEventMask()));
    System.out.println("Chrony Offset threshold: chronyClockOffsetThreshold ns");

    if(userInput) {
      System.out.printf("Enter the time base indices to subscribe " +
        "(comma-separated, default is 1): ");
      String line = scanner.nextLine();
      if(!line.isEmpty()) {
        for(String str : line.split(",")) {
          int idx = 0;
          try {
            idx = Integer.parseInt(str.strip());
          } catch (NumberFormatException e) {
            System.err.println("Invalid time base index: " + str);
            System.exit(2);
          }
          if(idx <= 0) {
            System.err.println("Invalid time base index: " + idx);
            System.exit(2);
          }
          indexList.add(idx);
        }
      } else
        System.out.println("Invalid input. Using default time base index 1.");
    }

    final Thread cur = Thread.currentThread();
    AtomicBoolean work = new AtomicBoolean(true);
    // control-c handler
    Runtime.getRuntime().addShutdownHook(new Thread(() -> {
      System.out.println(" Exit ...");
      synchronized(cur) {
        work.set(false);
        try {
          cur.wait();
        } catch(InterruptedException e) {
          e.printStackTrace();
        }
      }
    }));

    try {
      synchronized(cur) {
        if(!ClockManager.connect()) {
          dieMsg = "[clkmgr] failure in connecting !!!";
          return dieMsg;
        }
        cur.wait(1_000);
        System.out.println("[clkmgr] List of available clock:");
        // Print out each member of the Time Base configuration
        long size = TimeBaseConfigurations.size();
        for(long i = 1; i <= size; i++) {
          TimeBaseRecord cfg = TimeBaseConfigurations.getRecord(i);
          int idx = (int)cfg.index();
          System.out.println("TimeBaseIndex: " + idx);
          System.out.println("timeBaseName: " + cfg.name());
          ClockSyncSubscription Subscribe = new ClockSyncSubscription();
          if(cfg.havePtp()) {
            PTPCfg ptp = cfg.ptp();
            System.out.println("interfaceName: " + ptp.ifName());
            System.out.println("transportSpecific: " + ptp.transportSpecific());
            System.out.println("domainNumber: " + ptp.domainNumber());
            Subscribe.setPtpSubscription(ptp4lSub);
          }
          if(cfg.haveSysClock())
            Subscribe.setSysSubscription(chronySub);
          overallSub.put(idx, Subscribe);
          if(subscribeAll)
            indexList.add(idx);
        }
        // Default
        if(indexList.isEmpty())
          indexList.add(1);
        for(int idx : indexList) {
          if(!TimeBaseConfigurations.isTimeBaseIndexPresent(idx)) {
            dieMsg = "[clkmgr] Index " + idx + "does not exist";
            return dieMsg;
          }
          System.out.println("[clkmgr] Subscribe to time base index: " + idx);
          ClockSyncSubscription subscribe = overallSub.get(idx);
          if(subscribe == null) {
            dieMsg = "[clkmgr] Index " + idx + " does not have ClockSyncSubscription!";
            return dieMsg;
          }
          if(!ClockManager.subscribe(subscribe, idx, clockSyncData)) {
            dieMsg = "[clkmgr] Failure in subscribing to clkmgr Proxy !!!";
            return dieMsg;
          }
          System.out.printf("[clkmgr][%.3f] Obtained data from Subscription Event:%n",
            getMonotonicTime());
          printOut();
        }
        cur.wait(1_000);
        String hd2l = "|" + "-".repeat(27) + "|" + "-".repeat(28) + "|";
        String hd3b = "| %-25s | %-12d | %-11d |";
        String hd3 = "|" + "-".repeat(27) + "|" + "-".repeat(14) +
          "|" + "-".repeat(13) + "|";

        while(work.get()) {
          Iterator<Integer> itIndex = indexList.iterator();
          while(work.get() && itIndex.hasNext()) {
            int idx = itIndex.next();
            System.out.printf("[clkmgr][%.3f] Waiting Notification from " +
              "time base index " + idx + "...%n", getMonotonicTime());
            StatusWaitResult retval = ClockManager.statusWait(timeout, idx, clockSyncData);
            if(retval == StatusWaitResult.SWRLostConnection) {
              System.out.printf("[clkmgr][%.3f] Terminating: " +
                "lost connection to clkmgr Proxy%n", getMonotonicTime());
              return dieMsg;
            } else if(retval == StatusWaitResult.SWRInvalidArgument) {
              dieMsg = String.format("[clkmgr][%.3f] Terminating: Invalid argument%n",
                getMonotonicTime());
              return dieMsg;
            } else if(retval == StatusWaitResult.SWRNoEventDetected) {
              System.out.printf("[clkmgr][%.3f] No event status changes identified " +
                "in " + timeout + " seconds.%n%n", getMonotonicTime());
              System.out.printf("[clkmgr][%.3f] sleep for " + idleTime + " seconds...%n%n",
                getMonotonicTime());
              if(!work.get())
                return dieMsg;
              cur.wait(idleTime * 1_000);
              continue;
            } else if(retval == StatusWaitResult.SWREventDetected) {
              System.out.printf("[clkmgr][%.3f] Obtained data from Notification Event:%n",
                getMonotonicTime());
            } else {
              System.out.printf("[clkmgr][%.3f] Warning: Should not enter " +
                "this switch case, unexpected status code retval%n",
                getMonotonicTime());
              return dieMsg;
            }
            printOut();
            System.out.printf("[clkmgr][%.3f] sleep for %d seconds...%n%n",
              getMonotonicTime(), idleTime);
            if(!work.get())
              return dieMsg;
            cur.wait(idleTime * 1_000);
          }
        }
      }
      // Grace for control-c
      Thread.sleep(500);
    } catch(InterruptedException e) {
      e.printStackTrace();
    }
    synchronized(cur) {
      // release the control-c handler
      cur.notify();
    }
    return dieMsg;
  }
}

public class clkmgr_test {
  static {
    try {
      System.loadLibrary("clkmgr");
    } catch (UnsatisfiedLinkError e) {
      System.err.printf("Fail to load Native library:%n%n" + e);
      System.exit(1);
    }
  }
  public static void main(String args[]) {
    clkmgr_client c = new clkmgr_client();
    String dieMsg = c.run(args);
    ClockManager.disconnect();
    if(!dieMsg.isEmpty()) {
      System.err.println(dieMsg);
      System.exit(2);
    }
  }
}
