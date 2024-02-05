dnl SPDX-License-Identifier: LGPL-3.0-or-later
dnl SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */
dnl
dnl @file
dnl @brief List of management IDs
dnl
dnl @author Erez Geva <ErezGeva2@@gmail.com>
dnl @copyright © 2022 Erez Geva
dnl
dnl NA   - no function needed
dnl UF   - need function for parsing
dnl UFS  - need function for parsing having variable size
dnl UFB  - need function for parsing and build
dnl UFBS - need function for parsing and build having variable size
dnl
dnl size: > 0  fixed size dataField
dnl         0  No dataField (with NA)
dnl        -2  Variable length dataField, need calculation
dnl
dnl Shortcuts defined in msg.cpp
dnl use_GSC => A_GET | A_SET | A_COMMAND
dnl use_GS  => A_GET | A_SET
dnl
dnl Name                               value scope  allow       size   func
A(NULL_PTP_MANAGEMENT,                 0000, port,  use_GSC,    0,     NA)
A(CLOCK_DESCRIPTION,                   0001, port,  A_GET,     -2,     UFS)
A(USER_DESCRIPTION,                    0002, clock, use_GS,    -2,     UFBS)
A(SAVE_IN_NON_VOLATILE_STORAGE,        0003, clock, A_COMMAND,  0,     NA)
A(RESET_NON_VOLATILE_STORAGE,          0004, clock, A_COMMAND,  0,     NA)
A(INITIALIZE,                          0005, clock, A_COMMAND,  2,     UFB)
A(FAULT_LOG,                           0006, clock, A_GET,     -2,     UFS)
A(FAULT_LOG_RESET,                     0007, clock, A_COMMAND,  0,     NA)
A(DEFAULT_DATA_SET,                    2000, clock, A_GET,     20,     UF)
A(CURRENT_DATA_SET,                    2001, clock, A_GET,     18,     UF)
A(PARENT_DATA_SET,                     2002, clock, A_GET,     32,     UF)
A(TIME_PROPERTIES_DATA_SET,            2003, clock, A_GET,      4,     UF)
A(PORT_DATA_SET,                       2004, port,  A_GET,     26,     UF)
A(PRIORITY1,                           2005, clock, use_GS,     2,     UFB)
A(PRIORITY2,                           2006, clock, use_GS,     2,     UFB)
A(DOMAIN,                              2007, clock, use_GS,     2,     UFB)
A(SLAVE_ONLY,                          2008, clock, use_GS,     2,     UFB)
A(LOG_ANNOUNCE_INTERVAL,               2009, port,  use_GS,     2,     UFB)
A(ANNOUNCE_RECEIPT_TIMEOUT,            200a, port,  use_GS,     2,     UFB)
A(LOG_SYNC_INTERVAL,                   200b, port,  use_GS,     2,     UFB)
A(VERSION_NUMBER,                      200c, port,  use_GS,     2,     UFB)
A(ENABLE_PORT,                         200d, port,  A_COMMAND,  0,     NA)
A(DISABLE_PORT,                        200e, port,  A_COMMAND,  0,     NA)
A(TIME,                                200f, clock, use_GS,    10,     UFB)
A(CLOCK_ACCURACY,                      2010, clock, use_GS,     2,     UFB)
A(UTC_PROPERTIES,                      2011, clock, use_GS,     4,     UFB)
A(TRACEABILITY_PROPERTIES,             2012, clock, use_GS,     2,     UFB)
A(TIMESCALE_PROPERTIES,                2013, clock, use_GS,     2,     UFB)
A(UNICAST_NEGOTIATION_ENABLE,          2014, port,  use_GS,     2,     UFB)
A(PATH_TRACE_LIST,                     2015, clock, A_GET,     -2,     UFS)
A(PATH_TRACE_ENABLE,                   2016, clock, use_GS,     2,     UFB)
A(GRANDMASTER_CLUSTER_TABLE,           2017, clock, use_GS,    -2,     UFBS)
A(UNICAST_MASTER_TABLE,                2018, port,  use_GS,    -2,     UFBS)
A(UNICAST_MASTER_MAX_TABLE_SIZE,       2019, port,  A_GET,      2,     UF)
A(ACCEPTABLE_MASTER_TABLE,             201a, clock, use_GS,    -2,     UFBS)
A(ACCEPTABLE_MASTER_TABLE_ENABLED,     201b, port,  use_GS,     2,     UFB)
A(ACCEPTABLE_MASTER_MAX_TABLE_SIZE,    201c, clock, A_GET,      2,     UF)
A(ALTERNATE_MASTER,                    201d, port,  use_GS,     4,     UFB)
A(ALTERNATE_TIME_OFFSET_ENABLE,        201e, clock, use_GS,     2,     UFB)
A(ALTERNATE_TIME_OFFSET_NAME,          201f, clock, use_GS,    -2,     UFBS)
A(ALTERNATE_TIME_OFFSET_MAX_KEY,       2020, clock, A_GET,      2,     UF)
A(ALTERNATE_TIME_OFFSET_PROPERTIES,    2021, clock, use_GS,    16,     UFB)
A(TRANSPARENT_CLOCK_PORT_DATA_SET,     4001, port,  A_GET,     20,     UF)
A(LOG_MIN_PDELAY_REQ_INTERVAL,         6001, port,  use_GS,     2,     UFB)
dnl Deprecated in "IEEE Std 1588-2019"
A(TRANSPARENT_CLOCK_DEFAULT_DATA_SET,  4000, clock, A_GET,     12,     UF)
A(PRIMARY_DOMAIN,                      4002, clock, use_GS,     2,     UFB)
A(DELAY_MECHANISM,                     6000, port,  use_GS,     2,     UFB)
dnl From "IEEE Std 1588-2019"
A(EXTERNAL_PORT_CONFIGURATION_ENABLED, 3000, clock, use_GS,     2,     UFB)
A(MASTER_ONLY,                         3001, port,  use_GS,     2,     UFB)
A(HOLDOVER_UPGRADE_ENABLE,             3002, clock, use_GS,     2,     UFB)
A(EXT_PORT_CONFIG_PORT_DATA_SET,       3003, port,  use_GS,     2,     UFB)
dnl linuxptp TLVs (in Implementation-specific C000-DFFF)
dnl Shortcuts defined in msg.cpp
dnl use_GL  => A_GET | A_USE_LINUXPTP
dnl use_GSL => A_GET | A_SET | A_USE_LINUXPTP
A(TIME_STATUS_NP,                      c000, clock, use_GL,    51,     UF)
A(GRANDMASTER_SETTINGS_NP,             c001, clock, use_GSL,    8,     UFB)
A(PORT_DATA_SET_NP,                    c002, port,  use_GSL,    8,     UFB)
A(SUBSCRIBE_EVENTS_NP,                 c003, clock, use_GSL,   66,     UFB)
A(PORT_PROPERTIES_NP,                  c004, port,  use_GL,    -2,     UFS)
A(PORT_STATS_NP,                       c005, port,  use_GL,   266,     UF)
A(SYNCHRONIZATION_UNCERTAIN_NP,        c006, clock, use_GSL,    2,     UFB)
A(PORT_SERVICE_STATS_NP,               c007, port,  use_GL,    90,     UF)
A(UNICAST_MASTER_TABLE_NP,             c008, port,  use_GL,    -2,     UFS)
A(PORT_HWCLOCK_NP,                     c009, port,  use_GL,    16,     UF)
A(POWER_PROFILE_SETTINGS_NP,           c00a, port,  use_GSL,   16,     UFB)
