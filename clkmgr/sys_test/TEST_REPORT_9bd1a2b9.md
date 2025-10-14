<!-- SPDX-License-Identifier: GFDL-1.3-no-invariants-or-later
     SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. -->
# 🧪 Clock Manager Test Report - Git hash 9bd1a2b9

**Project:** `Clock Manager`
**Release Version:** `v2.0`
**Report Date:** `2025-10-07`
**Tested By:** `Liow, Shi Jie Donavan`

---

## 1. Purpose / Objective

This report summarizes the validation of the Clock Manager for libptpmgmt
v2.0, focusing on functional tests that confirm correct reporting of Chrony
 and PTP synchronization status in single and multi-domain environments.

---

## 2. Scope of Testing

  * Clock Manager compilation
  * Single domain event subscription:
    * Single event state and event counter reporting
    * Multiple event state and event counter reporting
    * Composite event state and event counter reporting
    * Chrony event state and event counter reporting
  * Multi domain event subscription:
    * Support for multiple instances of PTP4L
    * Multi-domain PTP4L event state and event counter reporting
  * Graceful handling of back-end process termination
  * Notification timing delay
  * Long hour stress test
  * Platforms/configurations covered:
    * 2 Devices Under Test (DUT) connected in back-to-back topology
    * [DUT #1] ⇄ [Ethernet Link] ⇄ [DUT #2]

---

## 3. Test Environment

* **OS:** `Ubuntu 24.04.3 with Linux Kernel v6.12rt`
* **Hardware / Platform(s):** 
  * CPU: `Intel Atom® x7835RE Processor`
  * NIC: `Intel® Ethernet Controller I226-LM`
* **Dependencies & Versions:**
  * [linuxptp](https://git.code.sf.net/p/linuxptp/code) — `a1eae14`
  * [librtpi](https://github.com/dvhart/librtpi/tree/main) — `a616dcd`
  * [libchrony](https://gitlab.com/chrony/libchrony.git) — `7dc8580`


---

## 4. Test Strategy / Approach

* The Clock Management Library was validated primarily through black-box 
functional testing using test applications that exercise the library’s 
APIs. Test outcomes was verified by comparing API responses to expected 
results under defined conditions. In addition, non-functional stress 
testing was conducted to assess the library’s robustness during 
extended operation.
* Both automated and manual tests were executed, with automation rate of 
97.14%
* CI/CD were tested on Debian/Fedora/Arch Linux and Gentoo containers in 
addition to tests performed on Ubuntu

---

## 5. Test Metrics & Results

| Test Case Metrics        | Value                    |
| ------------------------ | ------------------------ |
| Total Test Cases Planned | `35`                     |
| Executed                 | `35`                     |
| Passed                   | `35`                     |
| Failed                   | `0`                      |
| Skipped / Blocked        | `0`                      |
| Pass Rate                | `100%`                   |

| Requirement Metrics      | Value                    |
| ------------------------ | ------------------------ |
| Total Requirements Planned | `23`                   |
| Executed                 | `23`                     |
| Passed                   | `23`                     |
| Failed                   | `0`                      |
| Skipped / Blocked        | `0`                      |
| Pass Rate                | `100%`                   |

---

## 6. Defects & Failures

- None

---

## 7. Test Highlights & Key Observations

### 24 Hour Loaded Message Queue Stress Test Summary - Single Domain
* Test Objective: 
  * Verify the Clock Manager software is able to continuously report events
  over 24 hours with the message queue constantly loaded
  * Message queue is loaded by using a small GM Offset threshold window 
  (-+5ns) which will trigger events from clock synchronization offset 
  fluctuations (ptp_isOffsetInRange) and composite events 
  (ptp_isCompositeEventMet)
  * Monitor event status and counts hourly over the test duration
  * Test Sequence:
    [DUT1]Start PTP4L Follower + Chrony --> 
    [DUT2]Start PTP4L GM --> 
    [DUT1]Start Clock Manager Proxy --> 
    [DUT1]Start Clock Manager Application
  * Events status and counts are verified based on PTP4L logs
* Test Summary:
  * Result: PASS
  * Duration: 24 hours
  * Sample application command: clkmgr_test -s 0xF -c 0x7 -l 5 -i 3600 -t 0
  * Event Statistics:
    * Total Event Count (24h): 288381
    * Events per second: 3.34
  * Notification Event Table:
    ```
      | Events                       | Event Status | Event Count |
      |------------------------------|--------------|-------------|
      | ptp_isCompositeEventMet      | 1            | 288381      |
      | - isOffsetInRange            |              |             |
      | - isSyncedWithGm             |              |             |
      | - isAsCapable                |              |             |
      |------------------------------|--------------|-------------|
      | ptp_isOffsetInRange          | 1            | 288381      |
      | ptp_isSyncedWithGm           | 1            | 0           |
      | ptp_isAsCapable              | 1            | 0           |
      | ptp_isGmChanged              | 0            | 0           |
      |------------------------------|--------------|-------------|
      | ptp_clockOffset              |     5                   ns |
      | ptp_gmIdentity               |     00a0c9.fffe.000000     |
      | ptp_syncInterval             |     31250               us |
      | ptp_notificationTimestamp    |     1759704244846512716 ns |
      |------------------------------|----------------------------|
      | chrony_isOffsetInRange       | 1            | 0           |
      |------------------------------|----------------------------|
      | chrony_clockOffset           |     3                   ns |
      | chrony_gmIdentity            |     PHC0                   |
      | chrony_syncInterval          |     125000              us |
      | chrony_notificationTimestamp |     1759704244774154928 ns |
    ```
### Notification Time Latency - Single Domain
* Test Objective: 
  * Verify the Clock Manager software notifies the user of an event within 
  93.75ms of event detection
  * After clocks are in sync, an out of sync event is triggered by shifting
   the DUT1 clock. The timestamp of the time shift is logged and compared 
   with the event timestamp received from Clock Manager software
  * Test Sequence:
    [DUT1]Start PTP4L Follower + Chrony --> 
    [DUT2]Start PTP4L GM --> 
    [DUT1]Start Clock Manager Proxy -->
    [DUT1]Start Clock Manager Application --> 
    [DUT1]Trigger out of sync event
* Test Summary:
  * Result: PASS
  * Notification Latency: 6ms
  * Sample application command: clkmgr_test -s 0x1 -c 0 -i 0
  * Test Logs:
    * PTP4L Log (timeshift timestamp = **707225.320**)
      ```
      ptp4l[707225.320]: clockcheck: clock jumped forward or running faster
        than expected! 
      ptp4l[707225.320]: master offset  499997101 s0 freq   +3957 
        path delay         9
      ptp4l[707225.320]: port 1 (enp1s0): SLAVE to UNCALIBRATED on 
        SYNCHRONIZATION_FAULT
      ptp4l[707225.455]: master offset  499997099 s2 freq   +3937 
        path delay         9
      ptp4l[707225.455]: port 1 (enp1s0): UNCALIBRATED to SLAVE on 
        MASTER_CLOCK_SELECTED
      ptp4l[707225.591]: master offset  499997100 s2 freq +62499999 
        path delay         9
      ptp4l[707225.726]: master offset  492188998 s2 freq +62499999 
        path delay         9
      ptp4l[707225.862]: master offset  484376207 s2 freq +62499999 
        path delay         9
      ```
    * Clock Manager software event log (event timestamp = **707225.326**)
      ```
      [clkmgr][707225.326] Obtained data from Notification Event:
      [clkmgr] Current Time of CLOCK_REALTIME: 1760409122147915534 ns
      |------------------------------|--------------|-------------|
      | Events                       | Event Status | Event Count |
      |------------------------------|--------------|-------------|
      |------------------------------|--------------|-------------|
      | ptp_isOffsetInRange          | 1            | 2           |
      |------------------------------|--------------|-------------|
      | ptp_clockOffset              |     0                   ns |
      | ptp_gmIdentity               |     000000.0000.000000     |
      | ptp_syncInterval             |     0                   us |
      | ptp_notificationTimestamp    |     1760409122142451505 ns |
      |------------------------------|----------------------------|
      | chrony_isOffsetInRange       | 0            | 0           |
      |------------------------------|----------------------------|
      | chrony_clockOffset           |     -499997109          ns |
      | chrony_gmIdentity            |     PHC0                   |
      | chrony_syncInterval          |     125000              us |
      | chrony_notificationTimestamp |     1760409122106689468 ns |
      |------------------------------|----------------------------|

      [clkmgr][707225.326] sleep for 0 seconds...
      ```
### Multi-domain event subscription and monitoring
* Test Objective: 
  * Verify the Clock Manager software allows the user to subscribe and 
  monitor multiple PTP4L time domains
  * TimeBaseIndex 1 subscribes to GM Offset Event (ptp_isOffsetInRange)
  * TimeBaseIndex 2 subscribes to GM Changed Event (ptp_isGmChanged)
  * Events status and counts are verified based on PTP4L logs
  * Test Sequence:
    * [DUT1]Start PTP4L Follower + Chrony --> 
    [DUT1]Start Clock Manager Proxy --> 
    [DUT1]Start Clock Manager Application --> 
    3x [DUT2]Start/Restart PTP4L GM
* Test Summary:
  * Result: PASS
  * Sample application command: clkmgr_test -a -s 0xB -c 0x3 -i 70 -l 0
  * Test Logs:
    * Initialize Clock Manager software with multiple time domain 
    subscription information
      ```
      [clkmgr] set subscribe event : 0xb
      [clkmgr] set composite event : 0x3
      GM Offset threshold: 7 ns
      [clkmgr] set chrony event : 0x1
      Chrony Offset threshold: 100000 ns
      [clkmgr] List of available clock: 
      TimeBaseIndex: 1
      timeBaseName: Global Clock
      interfaceName: eth0
      transportSpecific: 1
      domainNumber: 0

      TimeBaseIndex: 2
      timeBaseName: Working Clock
      interfaceName: eth1
      transportSpecific: 1
      domainNumber: 20

      [clkmgr] Subscribe to time base index: 1
      [clkmgr][713036.746] Obtained data from Subscription Event:
      [clkmgr] Current Time of CLOCK_REALTIME: 1760414933567882355 ns
      |------------------------------|--------------|-------------|
      | Events                       | Event Status | Event Count |
      |------------------------------|--------------|-------------|
      | ptp_isCompositeEventMet      | 0            | 0           |
      | - isOffsetInRange            |              |             |
      | - isSyncedWithGm             |              |             |
      |------------------------------|--------------|-------------|
      | ptp_isOffsetInRange          | 1            | 0           |
      | ptp_isSyncedWithGm           | 0            | 0           |
      | ptp_isGmChanged              | 1            | 1           |
      |------------------------------|--------------|-------------|
      | ptp_clockOffset              |     0                   ns |
      | ptp_gmIdentity               |     000001.0001.000000     |
      | ptp_syncInterval             |     125000              us |
      | ptp_notificationTimestamp    |     1760414933567839244 ns |
      |------------------------------|----------------------------|
      | chrony_isOffsetInRange       | 1            | 0           |
      |------------------------------|----------------------------|
      | chrony_clockOffset           |     3                   ns |
      | chrony_gmIdentity            |     PHC0                   |
      | chrony_syncInterval          |     125000              us |
      | chrony_notificationTimestamp |     1760414933567849986 ns |
      |------------------------------|----------------------------|

      [clkmgr] Subscribe to time base index: 2
      [clkmgr][713036.746] Obtained data from Subscription Event:
      [clkmgr] Current Time of CLOCK_REALTIME: 1760414933568001954 ns
      |------------------------------|--------------|-------------|
      | Events                       | Event Status | Event Count |
      |------------------------------|--------------|-------------|
      | ptp_isCompositeEventMet      | 0            | 0           |
      | - isOffsetInRange            |              |             |
      | - isSyncedWithGm             |              |             |
      |------------------------------|--------------|-------------|
      | ptp_isOffsetInRange          | 1            | 0           |
      | ptp_isSyncedWithGm           | 0            | 0           |
      | ptp_isGmChanged              | 1            | 1           |
      |------------------------------|--------------|-------------|
      | ptp_clockOffset              |     0                   ns |
      | ptp_gmIdentity               |     000001.0001.000020     |
      | ptp_syncInterval             |     125000              us |
      | ptp_notificationTimestamp    |     1760414933567984820 ns |
      |------------------------------|----------------------------|
      | chrony_isOffsetInRange       | 1            | 0           |
      |------------------------------|----------------------------|
      | chrony_clockOffset           |     3                   ns |
      | chrony_gmIdentity            |     PHC0                   |
      | chrony_syncInterval          |     125000              us |
      | chrony_notificationTimestamp |     1760414933567985762 ns |
      |------------------------------|----------------------------|

      [clkmgr][713037.746] Waiting Notification from time base index 1 ...
      [clkmgr][713037.746] Waiting Notification from time base index 1 ...
      [clkmgr][713047.756] No event status changes identified in 10 seconds
      ```
    * Capture events over 70 second period
      ```
      [clkmgr][713047.756] sleep for 70 seconds...

      [clkmgr][713117.756] Waiting Notification from time base index 2 ...
      [clkmgr][713117.756] Obtained data from Notification Event:
      [clkmgr] Current Time of CLOCK_REALTIME: 1760415014577832168 ns
      |------------------------------|--------------|-------------|
      | Events                       | Event Status | Event Count |
      |------------------------------|--------------|-------------|
      | ptp_isCompositeEventMet      | 0            | 20          |
      | - isOffsetInRange            |              |             |
      | - isSyncedWithGm             |              |             |
      |------------------------------|--------------|-------------|
      | ptp_isOffsetInRange          | 1            | 22          |
      | ptp_isSyncedWithGm           | 0            | 6           |
      | ptp_isGmChanged              | 1            | 6           |
      |------------------------------|--------------|-------------|
      | ptp_clockOffset              |     0                   ns |
      | ptp_gmIdentity               |     000001.0001.000020     |
      | ptp_syncInterval             |     125000              us |
      | ptp_notificationTimestamp    |     1760414993236011829 ns |
      |------------------------------|----------------------------|
      | chrony_isOffsetInRange       | 1            | 0           |
      |------------------------------|----------------------------|
      | chrony_clockOffset           |     5                   ns |
      | chrony_gmIdentity            |     PHC0                   |
      | chrony_syncInterval          |     125000              us |
      | chrony_notificationTimestamp |     1760415014475874431 ns |
      |------------------------------|----------------------------|

      [clkmgr][713117.756] sleep for 70 seconds...

      [clkmgr][713187.756] Waiting Notification from time base index 1 ...
      [clkmgr][713187.756] Obtained data from Notification Event:
      [clkmgr] Current Time of CLOCK_REALTIME: 1760415084578009704 ns
      |------------------------------|--------------|-------------|
      | Events                       | Event Status | Event Count |
      |------------------------------|--------------|-------------|
      | ptp_isCompositeEventMet      | 0            | 24          |
      | - isOffsetInRange            |              |             |
      | - isSyncedWithGm             |              |             |
      |------------------------------|--------------|-------------|
      | ptp_isOffsetInRange          | 1            | 24          |
      | ptp_isSyncedWithGm           | 0            | 6           |
      | ptp_isGmChanged              | 1            | 6           |
      |------------------------------|--------------|-------------|
      | ptp_clockOffset              |     0                   ns |
      | ptp_gmIdentity               |     000001.0001.000000     |
      | ptp_syncInterval             |     125000              us |
      | ptp_notificationTimestamp    |     1760414993180489874 ns |
      |------------------------------|----------------------------|
      | chrony_isOffsetInRange       | 1            | 0           |
      |------------------------------|----------------------------|
      | chrony_clockOffset           |     -6                  ns |
      | chrony_gmIdentity            |     PHC0                   |
      | chrony_syncInterval          |     125000              us |
      | chrony_notificationTimestamp |     1760415084544686387 ns |
      |------------------------------|----------------------------|

      [clkmgr][713187.756] sleep for 70 seconds...
      ```

---

## 8. Sign-Off

* **Test Lead / QA:** `Liow, Shi Jie Donavan | 2025-10-08`
* **Maintainer / Approver:** `Geva, Erez | 2025-10-08`
