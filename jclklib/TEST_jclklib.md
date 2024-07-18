<!-- SPDX-License-Identifier: GFDL-1.3-no-invariants-or-later
     SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. -->
# The diagram of clockmanager(CM)  usage in system : 

Test app <----> client runtime(libjclk.so) <----> jclklib_proxy <----> libptpmgmt.so <----> ptp4l

# How to Clone and Build the Intel Customized Linux PTP:

We have applied several patches to the latest version of Linux PTP project
(https://git.code.sf.net/p/linuxptp/code), including servo state notify event.
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

Clock manager is basically divided to 2 parts: libjclk.so + jclklib_proxy.
libjclk.so provided API for an user application to connect, subscribe and
receive ptp4l events, whereas jclklib_proxy uses libptpmgmt.so API
(dynamically linked) to subscribe and listen to notify event from ptp4l.
Jclkib.so is communicating with jclklib_proxy using message queue.

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
    autoheader
    autoconf
    ./configure
    make
    ```

6. Outcome : you must have 3 binaries created:
    ```bash
    libptpmgmt.so
    jclklib/client/libjclk.so
    jclklib/proxy/jclklib_proxy
    ```

# How to Build the Sample Application:

We provided a sample code (not a product!) of test application which will uses
Clock Manager API to track latest status of ptp4l. You should use it as a
reference.

1. Navigate to the sample directory:
    ```bash
    cd libptpmgmt_iaclocklib/jclklib/sample
    ```

2. Build the application:
    ```bash
    make
    ```

3. Outcome : you must have 2 binary created:
    ```bash
    jclk_test
    jclk_c_test
    ```

# How to test :

1. Run the Intel customized ptp4l application:
    ```bash
    cd linux-ptp_iaclocklib
    sudo ./ptp4l -i <interface name>
    ```

2. Run the jclk_proxy application:
    ```bash
    cd libptpmgmt_iaclocklib/jclklib/proxy
    sudo ./run_proxy.sh
    ```

3. Run the cpp sample application
    ```bash
    cd libptpmgmt_iaclocklib/jclklib/client
    sudo ./run_jclk_test.sh <optional arguments>

    ```
4. Run the c sample application
    ```bash
    cd libptpmgmt_iaclocklib/jclklib/client
    sudo ./run_jclk_c_test.sh <optional arguments>
    ```

Usage of sample application (jclk_test) :
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

Example output of sample application (jclk_test) :
```bash
[jclklib] Connected. Session ID : 1
[jclklib] set subscribe event : jcl_event : event[0] = 15
[jclklib] set composite event : jcl_event : event[0] = 7

Upper Master Offset: 100000 ns
Lower Master Offset: -100000 ns

[jclklib][5754097.973] Obtained data from Subscription Event:
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
In the absence of a Grandmaster (GM), the master offset defaults to 0, which
it's anticipated that the offset_in_range event will be TRUE. Consequently,
the servo_locked event is used to ensure that the offset_in_range event
indicates either a high-quality clock synchronization (in-sync) or that the
master has been terminated (out-of-sync).
```
