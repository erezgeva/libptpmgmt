<!-- SPDX-License-Identifier: GFDL-1.3-no-invariants-or-later
     SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. -->
# The diagram of clockmanager(CM)  usage in system : 

Test app <----> client runtime(libclkmgr.so) <----> clkmgr_proxy <----> libptpmgmt.so <----> ptp4l

# How to Clone and Build the Intel Customized Linux PTP:

We have applied several patches to the latest version of Linux PTP project
(https://git.code.sf.net/p/linuxptp/code), including permanent subscription.
The repository with our custom changes can be found at
https://github.com/intel-staging/linux-ptp_iaclocklib. It should be used
together with this Clock Manager application.

1. Clone the repository:
    ```bash
    git clone https://github.com/intel-staging/linux-ptp_iaclocklib.git
    ```

2. Navigate to the cloned directory:
    ```bash
    cd linux-ptp_iaclocklib
    ```

3. Build the application:
    ```bash
    make
    ```

# How to Clone and Build the Clock Manager together with libptpmgmt:

Clock manager is basically divided to 2 parts: libclkmgr.so + clkmgr_proxy.
libclkmgr.so provided API for an user application to connect, subscribe and
receive ptp4l events, whereas clkmgr_proxy uses libptpmgmt.so API
(dynamically linked) to subscribe and listen to notify event from ptp4l.
libclkmgr.so is communicating with clkmgr_proxy using message queue.

1. Install package dependencies:
    ```bash
    sudo apt-get install -y swig libtool-bin cppcheck doxygen ctags astyle dot epstopdf valgrind
    ```

2. Install Real-Time Priority Inheritance Library (librtpi):
    ```bash
    git clone https://github.com/dvhart/librtpi.git
    cd librtpi
    autoreconf --install
    ./configure
    sudo make install
   ```

3. Clone the repository:
    ```bash
    git clone https://github.com/intel-staging/libptpmgmt_iaclocklib
    ```

4. Navigate to the cloned directory:
    ```bash
    cd libptpmgmt_iaclocklib
    ```

5. Build the application:
    ```bash
    autoreconf -i
    ./configure
    make
    ```

6. Outcome : you must have 3 binaries created:
    ```bash
    .libs/libptpmgmt.so
    clkmgr/client/libclkmgr.so
    clkmgr/proxy/clkmgr_proxy
    ```

# How to Build the Sample Application:

We provided a sample code (not a product!) of test application which will uses
Clock Manager API to track latest status of ptp4l. You should use it as a
reference.

1. Navigate to the sample directory:
    ```bash
    cd libptpmgmt_iaclocklib/clkmgr/sample
    ```

2. Build the application:
    ```bash
    make
    ```

3. Outcome : you must have 2 binary created:
    ```bash
    clkmgr_test
    clkmgr_c_test
    ```

# How to test :

1. Run the Intel customized ptp4l application:
    ```bash
    cd linux-ptp_iaclocklib
    sudo ./ptp4l -i <interface name> -f configs/igc.cfg
    ```

2. Run the clkmgr_proxy application:
    ```bash
    cd libptpmgmt_iaclocklib/clkmgr/proxy
    sudo ./run_proxy.sh <optional arguments>
    ```

3. Run the cpp sample application
    ```bash
    cd libptpmgmt_iaclocklib/clkmgr/client
    sudo ./run_clkmgr_test.sh <optional arguments>

    ```
4. Run the c sample application
    ```bash
    cd libptpmgmt_iaclocklib/clkmgr/client
    sudo ./run_clkmgr_c_test.sh <optional arguments>
    ```

Usage of proxy daemon (clkmgr_proxy) :
```bash
Options:
  -t transport specific
     Default: 0x0
```

Usage of sample application (clkmgr_test) :
```bash
Options:
-s subscribe_event_mask
    Default: 0xf
    Bit 0: gmOffsetEvent
    Bit 1: syncedToPrimaryClockEvent
    Bit 2: asCapableEvent
    Bit 3: gmChangedEvent
-c composite_event_mask
    Default: 0x7
    Bit 0: gmOffsetEvent
    Bit 1: syncedToPrimaryClockEvent
    Bit 2: asCapableEvent
-u upper master offset (ns)
    Default: 100000 ns
-l lower master offset (ns)
    Default: -100000 ns
-i idle time (s)
    Default: 1 s
-t timeout in waiting notification event (s)
    Default: 10 s
```

Example output of sample application (clkmgr_test) :
```bash
[clkmgr] Connected. Session ID : 1
[clkmgr] set subscribe event : clkmgr_event : event[0] = 15
[clkmgr] set composite event : clkmgr_event : event[0] = 7

Upper Master Offset: 100000 ns
Lower Master Offset: -100000 ns

[clkmgr][5754097.973] Obtained data from Subscription Event:
+---------------------------+--------------------+
| Event                     | Event Status       |
+---------------------------+--------------------+
| offset_in_range           | 1                  |
| synced_to_primary_clock   | 1                  |
| as_capable                | 1                  |
| gm_Changed                | 1                  |
+---------------------------+--------------------+
| UUID                      | 00a1c1.fffe.000000 |
+---------------------------+--------------------+
| composite_event           | 1                  |
| - offset_in_range         |                    |
| - synced_to_primary_clock |                    |
| - as_capable              |                    |
+---------------------------+--------------------+

```

Note :
```bash
In the absence of a primary clock (GM), the clock offset defaults to 0, which
it's anticipated that the offset_in_range event will be TRUE. Consequently,
the synced_to_primary_clock event is used to ensure that the offset_in_range
event indicates either a high-quality clock synchronization (in-sync) or that
the primary clock is not present (out-of-sync).
```
