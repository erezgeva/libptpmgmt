<!-- SPDX-License-Identifier: GFDL-1.3-no-invariants-or-later
SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. -->

# Clock Manager Introduction

Introduction Distributed computing applications, such as databases and
industrial automation, require accurate time synchronization (tens of
milliseconds down to single-digit nanoseconds) to coordinate actions and events
across multiple devices. In order to synchronize time across multiple devices,
the local platform time is compared to and adjusted to track a common time
reference such as a PTP GM, an NTP server, or GNSS. To optimize performance,
distributed applications must know how well time on the local platform is
synchronized to the reference time and react to changes in time synchronization
conveyed by telemetry information. Some examples of time synchronization
telemetry are a) the offset from the time reference and b) whether the NTP
server or the current PTP GM is accessible. To enable continued precision time
adoption and future distributed systems usages, time synchronization telemetry
and control must be available to the application using a standard API extensible
to multiple synchronization protocols.

The Clock Manager is a framework for controlling and monitoring network time
synchronization on the local platform. The Clock Manager exposes an API to
enable reporting time synchronization status to any application executing on the
system. It is planned that Clock Manager will not only be able to report
timing telemetry information, but also configure the platform, allowing
privileged applications to make changes to the platform time configuration.

![High-level Overview of the Clock Manager
Functionality](./image/clkmgr_overview.png) __Figure 1 - High-level Overview of
the Clock Manager Functionality__

Figure 1 shows the high-level system interactions between the Clock Manager and
the system software on the local platform. The Clock Manager communicates with
the time synchronization daemon (e.g., Linux PTP or Chrony) to get current
telemetry data and synchronization error and notifies the application of changes
to the relevant state. Any changes to the relevant state will generate event
notifications to the application. This is shown by Clock Manager data path. The
Clock Manager data path is entirely within user-space except for the IPC and
does not introduce any additional kernel modules. The current Linux timekeeping
data path is unchanged.

# Architectural Considerations

For more details about the Clock Manager design, supported events, and current
APIs, please check out the [Design Document](HLD_clkmgr.md). To check out how to
see the Clock Manager in action, please visit the [demo
application](TEST_clkmgr.md). 

## Maintaining Expected Timekeeping Functionality

The Linux timekeeping data path is the fast path. Typically, reading the current
time using vDSO calls such as clock_gettime() incurs only a few tens of
nanoseconds and no context switch. The complete timekeeping data path from time
sync daemon to the application – including clock_adjtime() – incurs a typical
latency of less than 10 microseconds. The relatively slower and parallel Clock
Manager data path requires additional processing and incurs up to 10
milliseconds of latency. This design ensures that the performance of the
existing timekeeping data path in the kernel is unchanged by the Clock Manager
software.

## Licensing 

Applications utilizing Clock Manager must be allowed to be licensed and
released under any free or non-free terms. The time synchronization daemon
(e.g., Linux PTP or Chrony) are licensed under GPL. The Clock Manager framework
is released under the BSD-3-Clause and insulates the application from any
requirement to release the source code. The Clock Manager service is
implemented using a process separate from both the application and time
synchronization daemon. This process is the Clock Manager Proxy (CMP). The CMP
code will include GPL and/or link to GPL code making it a derivative and
requiring that all code in the CMP code be licensed under the GPLv2. Any Clock
Manager support libraries must be released under a GPL-compatible license that
can also be used with proprietary software, such as MIT or modified BSD
licenses.

## Extensibility 

The current Clock Manager development focuses on the PTP time synchronization
protocol, but is extensible to other protocols such as NTP or GNSS. The set of
protocol telemetry events (e.g., changes in clock state) can be extended to any
work with any protocol or any daemon. The Clock Manager can be extended to
support any IPC and protocol to the application and the time synchronization
daemon. Some examples of IPC that could be supported in the future include
shared memory, UNIX sockets, and MQTT.

# Example Usages

## Industrial Systems

Cyberphysical systems have isochronous control loops that run on networks where
timing must be maintained to ensure the desired functionality (e.g., factory
automation, power generation, etc.).  The Clock Manager provides the insights
necessary to understand the network and enable network administrators to take
corrective action if needed.

## Distributed Systems 

It has been shown that improving clock synchronization accuracy for systems such
as distributed databases (e.g., CockroachDB and Spanner) can improve the retry
rate and throughput while maintaining linearizability.  However, this is only
true as long as the system is able to maintain synchronized clocks and tune
parameters based on timing telemetry. Such insight and tuning can be enabled
using the Clock Manager.

# Future Directions

## Support for Multiple Clock Domains 

The IEC/IEEE 60802 standard specifies a profile of TSN for Industrial
Automation. IEC/IEEE 60802 applications require support for multiple time bases
for the Global and Working application clocks. In addition, application clocks
may also be underpinned by multiple redundant time sources using 802.1ASdm. The
Clock Manager framework will provide the capabilities to map application clocks
to Linux POSIX clocks, switch between redundant time sources, and provide
application notifications.
