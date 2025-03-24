<!-- SPDX-License-Identifier: GFDL-1.3-no-invariants-or-later
     SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. -->
# Clock Manager codeflow

Clock Manager (clkmgr) is a software for monitoring network time synchronization
on the local platform and reporting the time synchronization status to any
application executing on the local system and subscribe to it.
The clkmgr communicates with the time synchronization daemon, like ptp4l from
Linux PTP Project to get real-time telemetry data and synchronization status.
Any changes to the subscribed event state will generate event notifications to
the application.

Clkmgr is a 2-part implementation. It provides a library libclkmgr.so
(aka client-runtime library) and a daemon clkmgr_proxy.

![High Level Design of Clock Manager Library](./image/hld_clock_mgr.png)

* **sample app** - Sample application for customer reference on how to use the
APIs provided by libclkmgr.so library.

* **libclkmgr.so** - A dynamic library that provides a set of C/C++ APIs to
connect, disconnect, subscribe and listen to clkmgr_proxy daemon.
This library is under BSD 3-Clause permissive licence. It communicates with
clkmgr_proxy using message queues. This library is also referred to as
client-runtime library.

* **clkmgr_proxy** - A daemon that is using libptpmgmt api to establish
connection and subscribe towards ptp4l events. The communication is established
using ptp4l UDS (/var/run/ptp4l as we now assumed it is always installed in the
same local machine as the ptp4l)

## Compilation and test step
* [Build project documentation](./TEST_clkmgr.md)

## Supported event

In current state, total of four ptp4l events are supported. The supported
telemetry events are:

1. AS Capable – A binary event indicating a change in 802.1AS Capable.

2. GM Offset – A binary event indicating whether the offset between the primary
clock and secondary clock is within predefined upper and lower limit.

3. Synced to GM – A binary event indicating whether the port state is time
receiver.

4. GM Change – A binary event indicating whether the UUID of primary clock is
changed and a string event providing primary clock’s UUID.

5. Composite event – A binary event that combine multiple events into one event.
An example is combining AS Capable, GM Offset, and Synced to GM to compose a
“clock is ready to use” event. In this example, if these three parameters are
all within the acceptable range, the composite event will become TRUE. If any
one of the three events are not in the acceptable range, the composite event
will become FALSE, indicating the clock is not yet ready to be consumed.

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
