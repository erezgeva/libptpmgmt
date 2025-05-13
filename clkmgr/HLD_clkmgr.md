<!-- SPDX-License-Identifier: GFDL-1.3-no-invariants-or-later
     SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. -->
# Clock Manager Codeflow

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
__Figure 1 - High-Level Design of Clock Manager Library__

* **C/C++ sample applications** - User space applications are provided to
showcase how to use the API from the client-runtime library. Please note that the
sample applications provided are only for user reference and are not for
production usage.

* **Client-runtime library (libclkmgr)** - A dynamic library that provides a set
of APIs for C/C++ user space applications to connect, disconnect, subscribe, and
obtain real-time ptp4l and chronyd synchronization status from the proxy daemon.
This library is written in C++ with a BSD-3-Clause license. It communicates with
a proxy daemon via message queues. 

* **Proxy daemon (clkmgr_proxy)** - A daemon that is connecting to multiple
clock sync daemons via UDS communication to obtain their latest synchronization
status. A local platform should only have single proxy daemon. The proxy daemon
communicates with ptp4l via libptpmgmt library and communicates with chronyd via
libchrony library. If any changes are detected on the synchronization status, the
proxy daemon will immediately notify all the clients that subscribed to it.

## Compilation and Test Step
* [Build project documentation](./TEST_clkmgr.md)

## Supported Event
In the current state, a total of four ptp4l events are supported. The supported
telemetry events are:

1. Offset in Range – A binary event indicates whether the offset between the
primary and secondary clocks is within the predefined upper and lower limits.

1. Synced to Primary Clock – A binary event indicates whether the port state is
a time receiver.

1. AS Capable – A binary event indicates a change in 802.1AS Capable.

1. GM Change – A binary event indicates whether the primary clock is changed
and a string event providing the primary clock’s UUID.

1. Composite event – A binary event that combines multiple events into one event.
An example is combining AS Capable, GM Offset, and Synced to GM to compose a
“clock is ready to use” event. In this example, if these three parameters are
all within the acceptable range, the composite event will become TRUE. If any
of the three events is not in the acceptable range, the composite event
will become FALSE, indicating the clock is not yet ready to be consumed.

Additional PTP data:
1. GM UUID – The UUID is a unique identifier for the Grandmaster Clock in a PTP
network. The Grandmaster Clock is the primary reference clock in the network,
responsible for providing the time to all other clocks.

1. Clock Offset - The offset refers to the time difference (in nanoseconds)
between the primary clock and secondary clock.

1. Notification Timestamp - The timestamp is a nanosecond-precision timestamp
that records the time of the last notification event to ensure precise tracking
of notification events.

## Available API for C++ Sample Application

1. connect()  
  This function will establish a connection to the clkmgr_proxy.  
  Once connected, it will assign a client ID for the client_runtime that is
connected to the clkmgr_proxy.

1. subscribe()  
  This function generates a subscribe message signaling interest in specific
supported event types.

1. subscribe_by_name()  
  This function use `timeBaseName` to generates a subscribe message signaling
interest in specific event types.

1. status_wait()  
  This function waits for a specified timeout period for any event changes.  
  The wait function blocks until an undelivered event is queued.  
  If the event is already queued for delivery when the wait function is called,
the call exits immediately, returning the queued event notification(s).  
  Multiple event notifications may be delivered per call, but the wait function
returns when a single event is queued. The exception is for composed events.  
  Multiple composed events can be used to wait for one of many or all intrinsic
events. The application may specify a timeout.  
  If there are no events ready to be delivered before the timeout elapses, the
function returns with a timeout error.  
  When the timeout is equal to -1, this function waits infinitely until there is
event changes occur.

1. status_wait_by_name()  
  This function use `timeBaseName` to subscribe and waits for a specified
timeout period for any event changes.  
  The wait function blocks until an undelivered event is queued.  
  If the event is already queued for delivery when the wait function is called,
the call exits immediately, returning the queued event notification(s).  
  Multiple event notifications may be delivered per call, but the wait function
returns when a single event is queued. The exception is for composed events.  
  Multiple composed events can be used to wait for one of many or all intrinsic
events. The application may specify a timeout.  
  If there are no events ready to be delivered before the timeout elapses, the
function returns with a timeout error.  
  When timeout is equal to -1, this function waits infinitely until there is
event changes occurs.

1. disconnect()  
  This function performs disconnect process with clkmgr_proxy by sending
disconnect message.  
  It will delete the ptp event for subscription and state reference for
notification.

1. gettime()  
  This function retrieves the time of the CLOCK_REALTIME.

## Available API for C Sample Application

1. clkmgr_connect()  
  This function will establish a connection to the clkmgr_proxy.  
  Once connected, it will assign a client ID for the client_runtime that is
connected to the clkmgr_proxy.  

1. clkmgr_subscribe()  
  This function generates a subscribe message signaling interest in specific
supported event types.

1. clkmgr_subscribe_by_name()  
  This function uses `timeBaseName` to generate a subscribe message signaling
interest in specific event types.

1. clkmgr_status_wait()  
  This function waits for a specified timeout period for any event changes.  
  The wait function blocks until an undelivered event is queued.  
  If the event is already queued for delivery when the wait function is called,
the call exits immediately, returning the queued event notification(s).  
  Multiple event notifications may be delivered per call, but the wait function
returns when a single event is queued. The exception is for composed events.  
  Multiple composed events can be used to wait for one of many or all intrinsic
events. The application may specify a timeout.  
  If no events are ready to be delivered before the timeout elapses, the
function returns with a timeout error.  
  When the timeout equals -1, this function waits infinitely until event changes
occur.

1. status_wait_by_name()  
  This function uses `timeBaseName` to subscribe and waits for a specified
timeout period for any event changes.  
  The wait function blocks until an undelivered event is queued.  
  If the event is already queued for delivery when the wait function is called,
the call exits immediately, returning the queued event notification(s).  
  Multiple event notifications may be delivered per call, but the wait function
returns when a single event is queued. The exception is for composed events.  
  Multiple composed events can be used to wait for one of many or all intrinsic
events. The application may specify a timeout.  
  If no events are ready to be delivered before the timeout elapses, the
function returns with a timeout error.  
  When the timeout equals -1, this function waits infinitely until event changes
occur.

1. clkmgr_disconnect()  
  This function performs the disconnect process with clkmgr_proxy by sending
a disconnect message.  
  It will delete the ptp event for subscription and state reference for
notification.

1. clkmgr_gettime()  
  This function retrieves the time of the CLOCK_REALTIME.
