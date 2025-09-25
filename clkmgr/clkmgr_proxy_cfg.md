<!-- SPDX-License-Identifier: GFDL-1.3-no-invariants-or-later
     SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. -->
---
title: CLKMGR_PROXY_CFG
section: 5
header: JSON File Formats Manual
date: July 2025
---

# NAME

clkmgr_proxy_cfg - **Clock Manager** proxy service JSON configuration file  

# DESCRIPTION

**clkmgr_proxy**(8) reads configuration data from this configuration file. It is
written in JSON format and defines how the **Clock Manager** proxy service should
communicate with the other time synchronization services on the system.  

**Clock Manager** supports multiple time bases. A time base refers to a logical
grouping of clocks that the Clock Manager manages as a unit. Typically, a time
base consists of one **ptp4l**(8) instance or one **chronyd**(8) instance or both.
For example, a time base can consist of both **ptp4l**(8) and **chronyd**(8):
**ptp4l**(8) disciplines the PHC clock, which then serves as the reference clock
for **chronyd**(8) to discipline the system clock. Generally, clock
synchronization services are grouped in the same time base when they are
synchronized to the same clock source. Each time base operates independently,
providing flexibility in monitor time synchronization status for multiple clock
sources within the same system.  

Note: Only one **clkmgr_proxy**(8) instance can run on the system at a time. To
manage multiple synchronization scenarios, configure multiple time bases within
a single proxy instance rather than running multiple proxies.  

The configuration file contains a set of keywords, each representing a parameter
or a group of parameters used to communicate with a time synchronization service.
Each keyword defines a particular aspect of the proxy service's operation, such
as the available time bases, their names, and the configuration parameters to
communicate with the time synchronization services like **ptp4l**(8) and
**chronyd**(8). By configuring the parameters the configuration file determines
how the **Clock Manager** proxy service comunnicate with time synchronization
services within each time base.  

# PARAMETERS

## `timeBases`

JSON array of configuration JSON objects for each available time base.  

## `timeBaseName`

The `timeBaseName` parameter specified a unique name to each time base to
distinguish between different time bases in the configuration. The `timeBaseName`
must be specified within length of 64 octets. Note that a single UTF-8 character
can be stored with multiple octets.  

## `ptp4l`

JSON object specifying configuration for using a **ptp4l**(8) service. If
omitted, **clkmgr_proxy**(8) will not communicate with the **ptp4l**(8) in this
time base. In case of using an empty JSON object, **clkmgr_proxy**(8) will use
its default parameters. Please refer to **ptp4l**(8) for the UDS address, domain
number, and transport specific used by the **ptp4l**(8) service.  

Note: All omitted parameters under the `ptp4l` keyword follow **ptp4l**(8)
defaults, except for `interfaceName`, which is mandatory and must be specified.  

`interfaceName`
: - The name of network interface.  
- The `interfaceName` must be specified within length of 64 octects.

`udsAddr`
: - The Unix Domain Socket address used for inter-process communication with
**ptp4l**(8) service. To find the `udsAddr` for the **ptp4l**(8) service, look
for the `uds_address` parameter in the **ptp4l**(8) configuration file. Refer to
**ptp4l**(8) for more information.  
- Default parameter: "/var/run/ptp/ptp4l"  

`domainNumber`
: - The PTP domain number.  
- Default parameter: 0  

`transportSpecific`
: - Transport specific.  
- Range: [0 .. 255]  
- Default parameter: 0  

## `chrony`

JSON object specifying configuration for using a **chronyd**(8) service. If
omitted, **clkmgr_proxy**(8) will not communicate with the **chronyd**(8) in
this time base. In case of using an empty JSON object, **clkmgr_proxy**(8) will
use its default parameters. Please refer to **chronyd**(8) for the UDS address
used by the **chronyd**(8) service.

Note: All omitted parameters under the `chrony` keyword, follow **chronyd**(8)
defaults.  

`udsAddr`
: - The Unix Domain Socket address used for inter-process communication with
**chronyd**(8) service. To find the `udsAddr` for the **chronyd**(8) service,
look for the `bindcmdaddress` parameter in the **chronyd**(8) configuration file.
Refer to **chronyd**(8) for more information.  
- Default parameter: "/var/run/chrony/chronyd.sock"  

# EXAMPLES

Below are examples of JSON configuration files that refers to different scenarios.  

1. Single time base with single ptp4l instance

    In this scenario, the proxy service subscribes to ONLY one **ptp4l**(8)
    instance under a single time base. No **chronyd**(8) service is subscribed.
    Therefore, the time synchronization data is provided solely by the
    **ptp4l**(8) service. The JSON configuration file includes ONLY the
    **ptp4l**(8) object within the time base.  

    **JSON file content:**  

    ```json
    {  
        "timeBases": [  
            {  
                "timeBaseName": "Global Clock",  
                "ptp4l":  
                {  
                    "interfaceName": "eth0",  
                    "udsAddr": "/var/run/ptp/ptp4l-domain-0",  
                    "domainNumber": 0,  
                    "transportSpecific": 1  
                }  
            }  
        ]  
    }
    ```

1. Single time base with single chrony instance

    In this scenario, the proxy service subscribes to ONLY one **chronyd**(8)
    instance under a single time base. No **ptp4l**(8) service is subscribed.
    Therefore, the time synchronization data is provided solely by the
    **chronyd**(8) service. The JSON configuration file includes ONLY the
    **chronyd**(8) object within the time base.  

    **JSON file content:**  

    ```json
    {  
        "timeBases": [  
            {  
                "timeBaseName": "Global Clock",  
                "chrony":  
                {  
                    "udsAddr": "/var/run/chrony/chronyd.sock"  
                }  
            }  
        ]  
    }
    ```

1. Single time base with default value

    In this scenario, the proxy service subscribes to one **ptp4l**(8) instance
    and one **chronyd**(8) instance, both grouped under a single time base.
    Therefore, the time synchronization data is provided by both the **ptp4l**(8)
    and **chronyd**(8) services. The JSON configuration file includes one
    **ptp4l**(8) and one **chronyd**(8) objects within the same time base. As the
    fields in the **ptp4l**(8) and **chronyd**(8) objects are omitted, default
    values will be assigned, except for `interfaceName` in **ptp4l**(8), which is
    mandatory and must be specified.  

    **JSON file content:**  

    ```json
    {  
        "timeBases": [  
            {  
                "timeBaseName": "Global Clock",  
                "ptp4l":  
                {  
                    "interfaceName": "eth0"  
                },  
                "chrony": {}  
            }  
        ]  
    }
    ```

1. Single time base with assigned values

    In this scenario, the proxy service subscribes to one **ptp4l**(8) instance
    and one **chronyd**(8) instance, both grouped under a single time base.
    Therefore, the time synchronization data is provided by both the **ptp4l**(8)
    and **chronyd**(8) services. The JSON configuration file includes one
    **ptp4l**(8) and one **chronyd**(8) objects within the same time base.  

    **JSON file content:**  

    ```json
    {  
        "timeBases": [  
            {  
                "timeBaseName": "Global Clock",  
                "ptp4l":  
                {  
                    "interfaceName": "eth0",  
                    "udsAddr": "/var/run/ptp/ptp4l",  
                    "domainNumber": 0,  
                    "transportSpecific": 1  
                },  
                "chrony":  
                {  
                    "udsAddr": "/var/run/chrony/chronyd.sock"  
                }  
            }  
        ]  
    }
    ```

1. Multiple time bases

    In this scenario, the proxy service subscribes to multiple **ptp4l**(8)
    instance and one **chronyd**(8) instance. Each **ptp4l**(8) instance is
    configured under its own time base, while the **chronyd**(8) instance is
    grouped with one of the time bases. Therefore, the time synchronization data
    is provided by the **ptp4l**(8) and **chronyd**(8) service in respective
    time base. The JSON configuration file includes multiple time bases, each
    with its own **ptp4l**(8) configuration, and one of them also includes the
    **chronyd**(8) configuration.  

    **JSON file content:**  

    ```json
    {  
        "timeBases": [  
            {  
                "timeBaseName": "Global Clock",  
                "ptp4l":  
                {  
                    "interfaceName": "eth0",  
                    "udsAddr": "/var/run/ptp/ptp4l",  
                    "domainNumber": 0,  
                    "transportSpecific": 1  
                },  
                "chrony":  
                {  
                    "udsAddr": "/var/run/chrony/chronyd.sock"  
                }  
            },  
            {  
                "timeBaseName": "Working Clock",  
                "ptp4l":  
                {  
                    "interfaceName": "eth1",  
                    "udsAddr": "/var/run/ptp/ptp4l-domain-20",  
                    "domainNumber": 20,  
                    "transportSpecific": 1  
                }  
            }  
        ]  
    }
    ```

# FILES

/etc/clkmgr/proxy_cfg.json  

> This is the JSON configuration file used by **clkmgr_proxy**(8).  

# SEE ALSO

**clkmgr_proxy**(8),
**ptp4l**(8),
**chronyd**(8)
