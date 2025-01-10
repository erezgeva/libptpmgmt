<!-- SPDX-License-Identifier: GFDL-1.3-no-invariants-or-later
     SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. -->
# The diagram of Clock Manager usage in system:

Test app <----> client runtime(libclkmgr.so) <----> clkmgr_proxy <----> libptpmgmt.so <----> ptp4l

# How to get Linux PTP:

You can install it from your Linux distribution:  
On Ubuntu and Debian:  
```bash
sudo apt install linuxptp
```
On Fedora:  
```bash
dnf install linuxptp
```

Or get it from [linuxptp](https://linuxptp.nwtime.org/) site.

# How to Clone and Build the Clock Manager together with libptpmgmt:

Clock manager is divided to two parts:  
  * clkmgr_proxy: The Clock manager proxy.  
  * libclkmgr.so: The Clock manager user library.  

We provide libclkmgr.so with API for user applications to connect, subscribe and
receive ptp4l events forward by the Clock manager proxy.  
The Clock manager proxy uses libptpmgmt.so to subscribe and to listen for events received from ptp4l.  
libclkmgr.so communicates with the Clock manager proxy through a message queue.

1. Install package dependencies:  
    ```bash
    sudo apt install swig libtool-bin cppcheck doxygen ctags astyle dot epstopdf valgrind
    ```

2. Install Real-Time Priority Inheritance Library (librtpi):  
    On new Ubuntu and Debian systems:  
    ```bash
    sudo apt install librtpi-dev
    ```
    On older systems, build and install the library:  
    ```bash
    git clone https://github.com/linux-rt/librtpi.git  
    cd librtpi  
    autoreconf --install  
    ./configure  
    make  
    make install
    ```

3. Install libchrony:
    ```bash
    git clone https://gitlab.com/chrony/libchrony.git
    cd libchrony
    make
    make install prefix=/usr/local
    ```

4. Clone the repository:
    ```bash
    git clone https://github.com/erezgeva/libptpmgmt
    ```

5. Navigate to the cloned directory:
    ```bash
    cd libptpmgmt
    ```

6. Build the application:
    ```bash
    autoreconf -i
    ./configure
    make
    ```

7. Outcome: two libraries and one application
    ```bash
    .libs/libptpmgmt.so
    .libs/libclkmgr.so
    clkmgr/proxy/clkmgr_proxy
    ```

# How to Build the Sample Application:

We provided a sample code of testing applications which will uses
Clock Manager API to track latest status of ptp4l.  
The applications are provided for demonstration only.  
We do not recommand to use them for production.  

1. Navigate to the sample directory:
    ```bash
    cd libptpmgmt/clkmgr/sample
    ```

2. Build the application:
    ```bash
    make
    ```

3. Outcome: two binary
    ```bash
    clkmgr_test
    clkmgr_c_test
    ```

# How to test:

1. Run the ptp4l service on both DUT and link partner:
    If linuxptp is installed on system:
    ```bash
    sudo ptp4l -i <interface name>
    ```
    Or you build it yourself
    ```bash
    cd <folder>
    sudo ./ptp4l -i <interface name>
    ```

2. Add ptp device (e.g /dev/ptp0) as refclock for chrony daemon application on DUT:
    ```bash
    echo "refclock PHC /dev/ptp0 poll -6 dpoll -1" >>  /etc/chrony/chrony.conf
    ```
3. Run the chrony daemon application on DUT:
    ```bash
    chronyd -f /etc/chrony/chrony.conf
    ```

4. Run the clkmgr_proxy application on DUT:
    ```bash
    cd libptpmgmt/clkmgr/proxy
    sudo ./run_proxy.sh -t 1
    ```
5. Run the sample application on DUT:

    a. c++ sample application:
    ```bash
    cd libptpmgmt/clkmgr/client
    sudo ./run_clkmgr_test.sh <optional arguments>
    ```

    b. c sample application:
    ```bash
    cd libptpmgmt/clkmgr/client
    sudo ./run_clkmgr_c_test.sh <optional arguments>
    ```

# Examples of result:

Usage of proxy daemon (clkmgr_proxy):
```bash
~/libptpmgmt/clkmgr/proxy# ./run_proxy.sh -h
Usage of ./clkmgr_proxy:
Options:
 -t transport specific
    Default: 0x0
```

Usage of c++ sample application (clkmgr_test):
```bash
~/libptpmgmt/clkmgr/sample# ./run_clkmgr_test.sh -h
Usage of ./clkmgr_test:
Options:
  -s subscribe_event_mask
     Default: 0xf
     Bit 0: eventGMOffset
     Bit 1: eventSyncedToGM
     Bit 2: eventASCapable
     Bit 3: eventGMChanged
  -c composite_event_mask
     Default: 0x7
     Bit 0: eventGMOffset
     Bit 1: eventSyncedToGM
     Bit 2: eventASCapable
  -u gm offset upper limit (ns)
     Default: 100000 ns
  -l gm offset lower limitt (ns)
     Default: -100000 ns
  -i idle time (s)
     Default: 1 s
  -m chrony offset upper limit (ns)
     Default: 100000 ns
  -n chrony offset lower limit (ns)
     Default: -100000 ns
  -t timeout in waiting notification event (s)
     Default: 10 s
```

Example output of c++ sample application (clkmgr_test):
```bash
~/libptpmgmt/clkmgr/sample# ./run_clkmgr_test.sh -n 0 -m 5 -t 0
[clkmgr] Connected. Session ID : 0
[clkmgr] set subscribe event : 0xf
[clkmgr] set composite event : 0x7
GM Offset upper limit: 100000 ns
GM Offset lower limit: -100000 ns
Chrony Offset upper limit: 5 ns
Chrony Offset lower limit: 0 ns

[clkmgr][360038.267] Obtained data from Subscription Event:
+---------------------------+------------------------+
| Event                     | Event Status           |
+---------------------------+------------------------+
| offset_in_range           | 1                      |
| synced_to_primary_clock   | 1                      |
| as_capable                | 1                      |
| gm_Changed                | 1                      |
+---------------------------+------------------------+
| UUID                      | 22abbc.fffe.bb1234     |
| clock_offset              | -2                  ns |
| notification_timestamp    | 1726024045215150041 ns |
+---------------------------+------------------------+
| composite_event           | 1                      |
| - offset_in_range         |                        |
| - synced_to_primary_clock |                        |
| - as_capable              |                        |
+---------------------------+------------------------+

+---------------------------+------------------------+
| chrony offset_in_range    | 0                      |
+---------------------------+------------------------+
| chrony clock_offset       | 5                   ns |
| chrony clock_reference_id | 50484330               |
| chrony polling interval   | 500000              us |
+---------------------------+------------------------+

[clkmgr][360039.268] Waiting for Notification Event...
[clkmgr][360039.268] Obtained data from Notification Event:
+---------------------------+--------------+-------------+
| Event                     | Event Status | Event Count |
+---------------------------+--------------+-------------+
| offset_in_range           | 1            | 1           |
| synced_to_primary_clock   | 1            | 0           |
| as_capable                | 1            | 0           |
| gm_Changed                | 0            | 0           |
+---------------------------+--------------+-------------+
| GM UUID                   |     222211.fffe.011122     |
| clock_offset              |     33                  ns |
| notification_timestamp    |     1929621371292023896 ns |
+---------------------------+--------------+-------------+
| composite_event           | 1            | 1           |
| - offset_in_range         |              |             |
| - synced_to_primary_clock |              |             |
| - as_capable              |              |             |
+---------------------------+--------------+-------------+

+---------------------------+----------------------------+
| chrony offset_in_range    | 1            | 1           |
+---------------------------+----------------------------+
| chrony clock_offset       |     3                   ns |
| chrony clock_reference_id |     50484330               |
| chrony polling_interval   |     500000              us |
+---------------------------+----------------------------+
```

Note:
```
In the absence of a primary clock (GM), the clock offset defaults to 0, which
it's anticipated that the offset_in_range event will be TRUE. Consequently,
the synced_to_primary_clock event is used to ensure that the offset_in_range
event indicates either a high-quality clock synchronization (in-sync) or that
the primary clock is not present (out-of-sync).
```
