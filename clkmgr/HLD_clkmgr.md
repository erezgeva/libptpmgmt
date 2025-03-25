<!-- SPDX-License-Identifier: GFDL-1.3-no-invariants-or-later
     SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. -->
# Clock Manager codeflow

Clock Manager (clkmgr) is a software for monitoring network time synchronization
on the local platform and reporting the time synchronization status to any
application executing on the local system and subscribe to it.
The clkmgr communicates with the time synchronization daemons, like ptp4l from
Linux PTP Project and chronyd from Chrony Project to get real-time
synchronization status. Any changes to the subscribed event state will
generate event notifications to the application.

The Clock Manager is a 2-part implementation. It provides a client-runtime
library and a proxy daemon. We assume the Clock Manager is always installed in
the same local machine as the ptp4l and chronyd.

![High Level Design of Clock Manager Library](./image/hld_clock_mgr.png)
__Figure 1 - High Level Design of Clock Manager Library__

* **C/C++ sample applications** - User space applications which are provided to
showcase how to use the API from client-runtime library. Please note that the
sample applications provided are only for user reference and is not for
production usage.

* **Client-runtime library (libclkmgr)** - A dynamic library that provides a set
of APIs for C/C++ user space applications to connect, disconnect, subscribe, and
obtain real-time ptp4l and chronyd synchronization status from proxy daemon.
This library is written in C++ with BSD-3-Clause license. It communicates with
proxy daemon via message queues. 

* **Proxy daemon (clkmgr_proxy)** - A daemon that is connecting to multiple
clock sync daemons via UDS communication to obtain their latest synchronization
status. A local platform should only have single proxy daemon. The proxy daemon
communicates with ptp4l via libptpmgmt library and communicates with chronyd via
libchrony library. If any changes detected on the synchronization status, proxy
daemon will immediately notify all the clients that subscribed to it. 

## Compilation and test step
* [Build project documentation](./TEST_clkmgr.md)

## Supported event
In current state, total of four ptp4l events are supported. The supported
telemetry events are:

1. Offset in Range – Binary event indicating whether the offset between the
primary clock and secondary clock is within predefined upper and lower limit.

2. Synced to Primary Clock – A binary event indicating whether the port state is
time receiver. 

3. AS Capable – Binary event indicating a change in 802.1AS Capable.

4. GM Change – A binary event indicating whether the primary clock is changed
and a string event providing primary clock’s UUID.

5. Composite event – A binary event that combines multiple events into one event.
An example is combining AS Capable, GM Offset, and Synced to GM to compose a
“clock is ready to use” event. In this example, if these three parameters are
all within the acceptable range, the composite event will become TRUE. If any
one of the three events are not in the acceptable range, the composite event
will become FALSE, indicating the clock is not yet ready to be consumed.

Additional PTP data:
1. GM UUID – The UUID is a unique identifier for the Grandmaster Clock in a PTP
network. The Grandmaster Clock is the primary reference clock in the network,
responsible for providing the time to all other clocks.

2. Clock Offset - The offset refers to the time difference (in nanoseconds)
between the primary clock and secondary clock.

3. Notification Timestamp - The timestamp is a nanosecond-precision timestamp
that records the time of the last notification event to ensure precise tracking
of notification events.

## Available API for c++ sample application

1. clkmgr_connect()
- This function will establish a connection to the clkmgr_proxy.
- Once connected, it will assign client ID for the client_runtime that connected
to the clkmgr_proxy.

2. clkmgr_subscribe()
- This function generates a subscribe message signaling interest in specific
supported event types.

3. clkmgr_subscribe_by_name()
- This function use `timeBaseName` to generates a subscribe message signaling
interest in specific event types.

4. clkmgr_status_wait()
- This function waits for a specified timeout period for any event changes.
- The wait function blocks until an undelivered event is queued.
- If the event is already queued for delivery when the wait function is called,
the call exits immediately, returning the queued event notification(s).
- Multiple event notifications may be delivered per call, but the wait function
returns when a single event is queued. The exception is for composed events.
- Multiple composed events can be used to wait for one of many or all intrinsic
events. The application may specify a timeout.
- If there are no events ready to be delivered before the timeout elapses, the
function returns with a timeout error.
- When timeout is equal to -1, this function waits infinitely until there is
event changes occurs.

5. clkmgr_status_wait_by_name()
- This function use `timeBaseName` to subscribe and waits for a specified
timeout period for any event changes.
- The wait function blocks until an undelivered event is queued.
- If the event is already queued for delivery when the wait function is called,
the call exits immediately, returning the queued event notification(s).
- Multiple event notifications may be delivered per call, but the wait function
returns when a single event is queued. The exception is for composed events.
- Multiple composed events can be used to wait for one of many or all intrinsic
events. The application may specify a timeout.
- If there are no events ready to be delivered before the timeout elapses, the
function returns with a timeout error.
- When timeout is equal to -1, this function waits infinitely until there is
event changes occurs.

6. clkmgr_disconnect()
- This function performs disconnect process with clkmgr_proxy by sending
disconnect message.
- It will delete the ptp event for subscription and state reference for
notification.

7. clkmgr_gettime()
- This function retrieve the time of the CLOCK_REALTIME.

## Available API for c sample application

1. clkmgr_c_connect()
- This function will establish a connection to the clkmgr_proxy.
- Once connected, it will assign client ID for the client_runtime that connected
to the clkmgr_proxy.

2. clkmgr_c_subscribe()
- This function generates a subscribe message signaling interest in specific
supported event types.

3. clkmgr_c_subscribe_by_name()
- This function use `timeBaseName` to generates a subscribe message signaling
interest in specific event types.

4. clkmgr_c_status_wait()
- This function waits for a specified timeout period for any event changes.
- The wait function blocks until an undelivered event is queued.
- If the event is already queued for delivery when the wait function is called,
the call exits immediately, returning the queued event notification(s).
- Multiple event notifications may be delivered per call, but the wait function
returns when a single event is queued. The exception is for composed events.
- Multiple composed events can be used to wait for one of many or all intrinsic
events. The application may specify a timeout.
- If there are no events ready to be delivered before the timeout elapses, the
function returns with a timeout error.
- When timeout is equal to -1, this function waits infinitely until there is
event changes occurs.

5. clkmgr_status_wait_by_name()
- This function use `timeBaseName` to subscribe and waits for a specified
timeout period for any event changes.
- The wait function blocks until an undelivered event is queued.
- If the event is already queued for delivery when the wait function is called,
the call exits immediately, returning the queued event notification(s).
- Multiple event notifications may be delivered per call, but the wait function
returns when a single event is queued. The exception is for composed events.
- Multiple composed events can be used to wait for one of many or all intrinsic
events. The application may specify a timeout.
- If there are no events ready to be delivered before the timeout elapses, the
function returns with a timeout error.
- When timeout is equal to -1, this function waits infinitely until there is
event changes occurs.

6. clkmgr_c_disconnect()
- This function performs disconnect process with clkmgr_proxy by sending
disconnect message.
- It will delete the ptp event for subscription and state reference for
notification.

7. clkmgr_c_client_fetch()
- This function fetch the single client object and return a pointer to it

8. clkmgr_c_gettime()
- This function retrieve the time of the CLOCK_REALTIME.
