/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief pmc tool TLV dump unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2023 Erez Geva
 *
 */

#include "pmc.h"

/* from utest/libsys_pmc.cpp */
extern const char *getPmcOut();
/* from ptp-tools/pmc_dump.cpp */
#define IDENT "\n\t\t"

// Tests dump CLOCK_DESCRIPTION tlv
TEST(PmcDumpTest, CLOCK_DESCRIPTION)
{
    Message m;
    CLOCK_DESCRIPTION_t t;
    t.clockType = ordinaryClock;
    t.physicalLayerProtocol.textField = "IEEE 802.3";
    t.physicalAddressLength = 6;
    Binary physicalAddress = Binary("\xc4\x7d\x46\x20\xac\xae", 6);
    t.physicalAddress = physicalAddress;
    PortAddress_t portAddress = { IEEE_802_3, 6, physicalAddress };
    t.protocolAddress = portAddress;
    t.manufacturerIdentity[0] = 1;
    t.manufacturerIdentity[1] = 2;
    t.manufacturerIdentity[2] = 3;
    t.productDescription.textField = ";;";
    t.revisionData.textField = ";;";
    t.userDescription.textField = "test123";
    memcpy(t.profileIdentity, "\x1\x2\x3\x4\x5\x6", 6);
    useTestMode(true);
    call_dump(m, CLOCK_DESCRIPTION, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "clockType             0x8000"
        IDENT "physicalLayerProtocol IEEE 802.3"
        IDENT "physicalAddress       c4:7d:46:20:ac:ae"
        IDENT "protocolAddress       IEEE_802_3 c4:7d:46:20:ac:ae"
        IDENT "manufacturerId        01:02:03"
        IDENT "productDescription    ;;"
        IDENT "revisionData          ;;"
        IDENT "userDescription       test123"
        IDENT "profileId             01:02:03:04:05:06");
}

// Tests dump USER_DESCRIPTION tlv
TEST(PmcDumpTest, USER_DESCRIPTION)
{
    Message m;
    USER_DESCRIPTION_t t;
    t.userDescription.textField = "test123";
    useTestMode(true);
    call_dump(m, USER_DESCRIPTION, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "userDescription  test123");
}

// Tests dump INITIALIZE tlv
TEST(PmcDumpTest, INITIALIZE)
{
    Message m;
    INITIALIZE_t t;
    t.initializationKey = 0x1234;
    useTestMode(true);
    call_dump(m, INITIALIZE, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "initializationKey 4660");
}

// Tests dump FAULT_LOG tlv
TEST(PmcDumpTest, FAULT_LOG)
{
    Message m;
    FAULT_LOG_t t;
    t.numberOfFaultRecords = 2;
    FaultRecord_t r;
    r.faultRecordLength = 50;
    r.faultTime = 9.000709;
    r.severityCode = F_Critical;
    r.faultName.textField = "error 1";
    r.faultValue.textField = "test123";
    r.faultDescription.textField = "This is first record";
    t.faultRecords.push_back(r);
    r.faultRecordLength = 55;
    r.faultTime = 1791.003019;
    r.severityCode = F_Warning;
    r.faultName.textField = "error 2";
    r.faultValue.textField = "test321";
    r.faultDescription.textField = "This is the second record";
    t.faultRecords.push_back(r);
    useTestMode(true);
    call_dump(m, FAULT_LOG, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "numberOfFaultRecords 2"
        IDENT "[0] faultTime        9.000709000"
        IDENT "[0] severityCode     Critical"
        IDENT "[0] faultName        error 1"
        IDENT "[0] faultValue       test123"
        IDENT "[0] faultDescription This is first record"
        IDENT "[1] faultTime        1791.003018999"
        IDENT "[1] severityCode     Warning"
        IDENT "[1] faultName        error 2"
        IDENT "[1] faultValue       test321"
        IDENT "[1] faultDescription This is the second record");
}

// Tests dump DEFAULT_DATA_SET tlv
TEST(PmcDumpTest, DEFAULT_DATA_SET)
{
    Message m;
    DEFAULT_DATA_SET_t t;
    t.flags = 0x3;
    t.numberPorts = 1;
    t.priority1 = 153;
    t.clockQuality.clockClass = 255;
    t.clockQuality.clockAccuracy = Accurate_Unknown;
    t.clockQuality.offsetScaledLogVariance = 0xffff;
    t.priority2 = 137;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    t.clockIdentity = clockId;
    t.domainNumber = 0;
    useTestMode(true);
    call_dump(m, DEFAULT_DATA_SET, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "twoStepFlag             1"
        IDENT "slaveOnly               1"
        IDENT "numberPorts             1"
        IDENT "priority1               153"
        IDENT "clockClass              255"
        IDENT "clockAccuracy           0xfe"
        IDENT "offsetScaledLogVariance 0xffff"
        IDENT "priority2               137"
        IDENT "clockIdentity           c47d46.fffe.20acae"
        IDENT "domainNumber            0");
}

// Tests dump CURRENT_DATA_SET tlv
TEST(PmcDumpTest, CURRENT_DATA_SET)
{
    Message m;
    CURRENT_DATA_SET_t t;
    t.stepsRemoved = 0x1234;
    t.offsetFromMaster.scaledNanoseconds = 55045503307092.1 * 0x10000;
    t.meanPathDelay.scaledNanoseconds = 9921458602754 * 0x10000;
    useTestMode(true);
    call_dump(m, CURRENT_DATA_SET, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "stepsRemoved     4660"
        IDENT "offsetFromMaster 55045503307092.1"
        IDENT "meanPathDelay    9921458602754.0");
}

// Tests dump PARENT_DATA_SET tlv
TEST(PmcDumpTest, PARENT_DATA_SET)
{
    Message m;
    PARENT_DATA_SET_t t;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 5 };
    t.parentPortIdentity = portIdentity;
    t.flags = 1;
    t.observedParentOffsetScaledLogVariance = 0xffff;
    t.observedParentClockPhaseChangeRate = 0x7fffffff;
    t.grandmasterPriority1 = 255;
    t.grandmasterClockQuality.clockClass = 255;
    t.grandmasterClockQuality.clockAccuracy = Accurate_Unknown;
    t.grandmasterClockQuality.offsetScaledLogVariance = 0xffff;
    t.grandmasterPriority2 = 255;
    t.grandmasterIdentity = clockId;
    useTestMode(true);
    call_dump(m, PARENT_DATA_SET, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "parentPortIdentity                    c47d46.fffe.20acae-5"
        IDENT "parentStats                           1"
        IDENT "observedParentOffsetScaledLogVariance 0xffff"
        IDENT "observedParentClockPhaseChangeRate    0x7fffffff"
        IDENT "grandmasterPriority1                  255"
        IDENT "gm.ClockClass                         255"
        IDENT "gm.ClockAccuracy                      0xfe"
        IDENT "gm.OffsetScaledLogVariance            0xffff"
        IDENT "grandmasterPriority2                  255"
        IDENT "grandmasterIdentity                   c47d46.fffe.20acae");
}

// Tests dump TIME_PROPERTIES_DATA_SET tlv
TEST(PmcDumpTest, TIME_PROPERTIES_DATA_SET)
{
    Message m;
    TIME_PROPERTIES_DATA_SET_t t;
    t.currentUtcOffset = 37;
    t.flags = F_PTP; // ptpTimescale bit
    t.timeSource = INTERNAL_OSCILLATOR;
    useTestMode(true);
    call_dump(m, TIME_PROPERTIES_DATA_SET, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "currentUtcOffset      37"
        IDENT "leap61                0"
        IDENT "leap59                0"
        IDENT "currentUtcOffsetValid 0"
        IDENT "ptpTimescale          1"
        IDENT "timeTraceable         0"
        IDENT "frequencyTraceable    0"
        IDENT "timeSource            INTERNAL_OSCILLATOR");
}

// Tests dump PORT_DATA_SET tlv
TEST(PmcDumpTest, PORT_DATA_SET)
{
    Message m;
    PORT_DATA_SET_t t;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    t.portIdentity = portIdentity;
    t.portState = LISTENING;
    t.logMinDelayReqInterval = 0;
    t.peerMeanPathDelay.scaledNanoseconds = 0;
    t.logAnnounceInterval = 1;
    t.announceReceiptTimeout = 3;
    t.logSyncInterval = 0;
    t.delayMechanism = P2P;
    t.logMinPdelayReqInterval = 0;
    t.versionNumber = 2;
    useTestMode(true);
    call_dump(m, PORT_DATA_SET, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "portIdentity            c47d46.fffe.20acae-1"
        IDENT "portState               LISTENING"
        IDENT "logMinDelayReqInterval  0"
        IDENT "peerMeanPathDelay       0"
        IDENT "logAnnounceInterval     1"
        IDENT "announceReceiptTimeout  3"
        IDENT "logSyncInterval         0"
        IDENT "delayMechanism          P2P"
        IDENT "logMinPdelayReqInterval 0"
        IDENT "versionNumber           2");
};

// Tests dump PRIORITY1 tlv
TEST(PmcDumpTest, PRIORITY1)
{
    Message m;
    PRIORITY1_t t;
    t.priority1 = 153;
    useTestMode(true);
    call_dump(m, PRIORITY1, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "priority1 153");
}

// Tests dump PRIORITY2 tlv
TEST(PmcDumpTest, PRIORITY2)
{
    Message m;
    PRIORITY2_t t;
    t.priority2 = 137;
    useTestMode(true);
    call_dump(m, PRIORITY2, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "priority2 137");
}

// Tests dump DOMAIN tlv
TEST(PmcDumpTest, DOMAIN)
{
    Message m;
    DOMAIN_t t;
    t.domainNumber = 7;
    useTestMode(true);
    call_dump(m, DOMAIN, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "domainNumber 7");
}

// Tests dump SLAVE_ONLY tlv
TEST(PmcDumpTest, SLAVE_ONLY)
{
    Message m;
    SLAVE_ONLY_t t;
    t.flags = 1;
    useTestMode(true);
    call_dump(m, SLAVE_ONLY, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "slaveOnly 1");
}

// Tests dump LOG_ANNOUNCE_INTERVAL tlv
TEST(PmcDumpTest, LOG_ANNOUNCE_INTERVAL)
{
    Message m;
    LOG_ANNOUNCE_INTERVAL_t t;
    t.logAnnounceInterval = 1;
    useTestMode(true);
    call_dump(m, LOG_ANNOUNCE_INTERVAL, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "logAnnounceInterval 1");
}

// Tests dump ANNOUNCE_RECEIPT_TIMEOUT tlv
TEST(PmcDumpTest, ANNOUNCE_RECEIPT_TIMEOUT)
{
    Message m;
    ANNOUNCE_RECEIPT_TIMEOUT_t t;
    t.announceReceiptTimeout = 3;
    useTestMode(true);
    call_dump(m, ANNOUNCE_RECEIPT_TIMEOUT, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "announceReceiptTimeout 3");
}

// Tests dump LOG_SYNC_INTERVAL tlv
TEST(PmcDumpTest, LOG_SYNC_INTERVAL)
{
    Message m;
    LOG_SYNC_INTERVAL_t t;
    t.logSyncInterval = 7;
    useTestMode(true);
    call_dump(m, LOG_SYNC_INTERVAL, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "logSyncInterval 7");
}

// Tests dump VERSION_NUMBER tlv
TEST(PmcDumpTest, VERSION_NUMBER)
{
    Message m;
    VERSION_NUMBER_t t;
    t.versionNumber = 2;
    useTestMode(true);
    call_dump(m, VERSION_NUMBER, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "versionNumber 2");
}

// Tests dump TIME tlv
TEST(PmcDumpTest, TIME)
{
    Message m;
    TIME_t t;
    t.currentTime = 13.15;
    useTestMode(true);
    call_dump(m, TIME, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "currentTime 13.150000000");
}

// Tests dump CLOCK_ACCURACY tlv
TEST(PmcDumpTest, CLOCK_ACCURACY)
{
    Message m;
    CLOCK_ACCURACY_t t;
    t.clockAccuracy = Accurate_Unknown;
    useTestMode(true);
    call_dump(m, CLOCK_ACCURACY, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "clockAccuracy 0xfe");
}

// Tests dump UTC_PROPERTIES tlv
TEST(PmcDumpTest, UTC_PROPERTIES)
{
    Message m;
    UTC_PROPERTIES_t t;
    t.currentUtcOffset = -0x5433;
    t.flags = 7;
    useTestMode(true);
    call_dump(m, UTC_PROPERTIES, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "currentUtcOffset      -21555"
        IDENT "leap61                1"
        IDENT "leap59                1"
        IDENT "currentUtcOffsetValid 1");
}

// Tests dump TRACEABILITY_PROPERTIES tlv
TEST(PmcDumpTest, TRACEABILITY_PROPERTIES)
{
    Message m;
    TRACEABILITY_PROPERTIES_t t;
    t.flags = F_TTRA | F_FTRA;
    useTestMode(true);
    call_dump(m, TRACEABILITY_PROPERTIES, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "timeTraceable      1"
        IDENT "frequencyTraceable 1");
}

// Tests dump TIMESCALE_PROPERTIES tlv
TEST(PmcDumpTest, TIMESCALE_PROPERTIES)
{
    Message m;
    TIMESCALE_PROPERTIES_t t;
    t.flags = F_PTP;
    t.timeSource = HAND_SET;
    useTestMode(true);
    call_dump(m, TIMESCALE_PROPERTIES, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "ptpTimescale 1");
}

// Tests dump UNICAST_NEGOTIATION_ENABLE tlv
TEST(PmcDumpTest, UNICAST_NEGOTIATION_ENABLE)
{
    Message m;
    UNICAST_NEGOTIATION_ENABLE_t t;
    t.flags = 1;
    useTestMode(true);
    call_dump(m, UNICAST_NEGOTIATION_ENABLE, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "unicastNegotiationPortDS enabled");
    t.flags = 0;
    useTestMode(true);
    call_dump(m, UNICAST_NEGOTIATION_ENABLE, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "unicastNegotiationPortDS disabled");
}

// Tests dump PATH_TRACE_LIST tlv
TEST(PmcDumpTest, PATH_TRACE_LIST)
{
    Message m;
    PATH_TRACE_LIST_t t;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    t.pathSequence.push_back(clockId);
    ClockIdentity_t c2 = {12, 4, 19, 97, 11, 74, 12, 74};
    t.pathSequence.push_back(c2);
    useTestMode(true);
    call_dump(m, PATH_TRACE_LIST, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "[0] c47d46.fffe.20acae"
        IDENT "[1] 0c0413.610b.4a0c4a");
}

// Tests dump PATH_TRACE_ENABLE tlv
TEST(PmcDumpTest, PATH_TRACE_ENABLE)
{
    Message m;
    PATH_TRACE_ENABLE_t t;
    t.flags = 1;
    useTestMode(true);
    call_dump(m, PATH_TRACE_ENABLE, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "pathTraceDS enabled");
}

// Tests dump GRANDMASTER_CLUSTER_TABLE tlv
TEST(PmcDumpTest, GRANDMASTER_CLUSTER_TABLE)
{
    Message m;
    GRANDMASTER_CLUSTER_TABLE_t t;
    t.logQueryInterval = -19;
    t.actualTableSize = 2;
    Binary physicalAddress = Binary("\xc4\x7d\x46\x20\xac\xae", 6);
    PortAddress_t portAddress0 = { IEEE_802_3, 6, physicalAddress };
    t.PortAddress.push_back(portAddress0);
    Binary ip("\x12\x34\x56\x78", 4);
    PortAddress_t portAddress1 = { UDP_IPv4, 4, ip };
    t.PortAddress.push_back(portAddress1);
    useTestMode(true);
    call_dump(m, GRANDMASTER_CLUSTER_TABLE, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "logQueryInterval -19"
        IDENT "actualTableSize  2"
        IDENT "[0] c4:7d:46:20:ac:ae"
        IDENT "[1] 18.52.86.120");
}

// Tests dump UNICAST_MASTER_TABLE tlv
TEST(PmcDumpTest, UNICAST_MASTER_TABLE)
{
    Message m;
    UNICAST_MASTER_TABLE_t t;
    t.logQueryInterval = -19;
    t.actualTableSize = 2;
    Binary physicalAddress = Binary("\xc4\x7d\x46\x20\xac\xae", 6);
    PortAddress_t portAddress0 = { IEEE_802_3, 6, physicalAddress };
    t.PortAddress.push_back(portAddress0);
    Binary ip("\x12\x34\x56\x78", 4);
    PortAddress_t portAddress1 = { UDP_IPv4, 4, ip };
    t.PortAddress.push_back(portAddress1);
    useTestMode(true);
    call_dump(m, UNICAST_MASTER_TABLE, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "logQueryInterval -19"
        IDENT "actualTableSize  2"
        IDENT "[0] c4:7d:46:20:ac:ae"
        IDENT "[1] 18.52.86.120");
}

// Tests dump UNICAST_MASTER_MAX_TABLE_SIZE tlv
TEST(PmcDumpTest, UNICAST_MASTER_MAX_TABLE_SIZE)
{
    Message m;
    UNICAST_MASTER_MAX_TABLE_SIZE_t t;
    t.maxTableSize = 0x2143;
    useTestMode(true);
    call_dump(m, UNICAST_MASTER_MAX_TABLE_SIZE, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "maxTableSize 8515");
}

// Tests dump ACCEPTABLE_MASTER_TABLE tlv
TEST(PmcDumpTest, ACCEPTABLE_MASTER_TABLE)
{
    Message m;
    ACCEPTABLE_MASTER_TABLE_t t;
    t.actualTableSize = 2;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t a0 = { clockId, 1 };
    t.list.push_back({a0, 127});
    PortIdentity_t a1 = { { 9, 8, 7, 6, 5, 4, 1, 7}, 2 };
    t.list.push_back({a1, 111});
    useTestMode(true);
    call_dump(m, ACCEPTABLE_MASTER_TABLE, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "actualTableSize 2"
        IDENT "[0] acceptablePortIdentity c47d46.fffe.20acae-1"
        IDENT "[0] alternatePriority1     127"
        IDENT "[1] acceptablePortIdentity 090807.0605.040107-2"
        IDENT "[1] alternatePriority1     111");
}

// Tests dump ACCEPTABLE_MASTER_TABLE_ENABLED tlv
TEST(PmcDumpTest, ACCEPTABLE_MASTER_TABLE_ENABLED)
{
    Message m;
    ACCEPTABLE_MASTER_TABLE_ENABLED_t t;
    t.flags = 1;
    useTestMode(true);
    call_dump(m, ACCEPTABLE_MASTER_TABLE_ENABLED, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "acceptableMasterPortDS enabled");
}

// Tests dump ACCEPTABLE_MASTER_MAX_TABLE_SIZE tlv
TEST(PmcDumpTest, ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
{
    Message m;
    ACCEPTABLE_MASTER_MAX_TABLE_SIZE_t t;
    t.maxTableSize = 0x67ba;
    useTestMode(true);
    call_dump(m, ACCEPTABLE_MASTER_MAX_TABLE_SIZE, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "maxTableSize 26554");
}

// Tests dump ALTERNATE_MASTER tlv
TEST(PmcDumpTest, ALTERNATE_MASTER)
{
    Message m;
    ALTERNATE_MASTER_t t;
    t.flags = 1;
    t.logAlternateMulticastSyncInterval = -17;
    t.numberOfAlternateMasters = 210;
    useTestMode(true);
    call_dump(m, ALTERNATE_MASTER, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "transmitAlternateMulticastSync    enabled"
        IDENT "logAlternateMulticastSyncInterval -17"
        IDENT "numberOfAlternateMasters          210");
}

// Tests dump ALTERNATE_TIME_OFFSET_ENABLE tlv
TEST(PmcDumpTest, ALTERNATE_TIME_OFFSET_ENABLE)
{
    Message m;
    ALTERNATE_TIME_OFFSET_ENABLE_t t;
    t.keyField = 7;
    t.flags = 1;
    useTestMode(true);
    call_dump(m, ALTERNATE_TIME_OFFSET_ENABLE, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "keyField       7"
        IDENT "enable         1");
}

// Tests dump ALTERNATE_TIME_OFFSET_NAME tlv
TEST(PmcDumpTest, ALTERNATE_TIME_OFFSET_NAME)
{
    Message m;
    ALTERNATE_TIME_OFFSET_NAME_t t;
    t.keyField = 11;
    t.displayName.textField = "123";
    useTestMode(true);
    call_dump(m, ALTERNATE_TIME_OFFSET_NAME, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "keyField       11"
        IDENT "displayName    123");
}

// Tests dump ALTERNATE_TIME_OFFSET_MAX_KEY tlv
TEST(PmcDumpTest, ALTERNATE_TIME_OFFSET_MAX_KEY)
{
    Message m;
    ALTERNATE_TIME_OFFSET_MAX_KEY_t t;
    t.maxKey = 9;
    useTestMode(true);
    call_dump(m, ALTERNATE_TIME_OFFSET_MAX_KEY, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "maxKey 9");
}

// Tests dump ALTERNATE_TIME_OFFSET_PROPERTIES tlv
TEST(PmcDumpTest, ALTERNATE_TIME_OFFSET_PROPERTIES)
{
    Message m;
    ALTERNATE_TIME_OFFSET_PROPERTIES_t t;
    t.keyField = 13;
    t.currentOffset = -2145493247;
    t.jumpSeconds = -2147413249;
    t.timeOfNextJump = 0x912478321891LL;
    useTestMode(true);
    call_dump(m, ALTERNATE_TIME_OFFSET_PROPERTIES, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "keyField       13"
        IDENT "currentOffset  -2145493247"
        IDENT "jumpSeconds    -2147413249"
        IDENT "timeOfNextJump 159585821399185");
}

// Tests dump TRANSPARENT_CLOCK_PORT_DATA_SET tlv
TEST(PmcDumpTest, TRANSPARENT_CLOCK_PORT_DATA_SET)
{
    Message m;
    TRANSPARENT_CLOCK_PORT_DATA_SET_t t;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    t.portIdentity = portIdentity;
    t.flags = 1;
    t.logMinPdelayReqInterval = -21;
    t.peerMeanPathDelay.scaledNanoseconds = -38515349857070 * 0x10000;
    useTestMode(true);
    call_dump(m, TRANSPARENT_CLOCK_PORT_DATA_SET, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "portIdentity            c47d46.fffe.20acae-1"
        IDENT "transparentClockPortDS  true"
        IDENT "logMinPdelayReqInterval -21"
        IDENT "peerMeanPathDelay       -38515349857070");
}

// Tests dump LOG_MIN_PDELAY_REQ_INTERVAL tlv
TEST(PmcDumpTest, LOG_MIN_PDELAY_REQ_INTERVAL)
{
    Message m;
    LOG_MIN_PDELAY_REQ_INTERVAL_t t;
    t.logMinPdelayReqInterval = 9;
    useTestMode(true);
    call_dump(m, LOG_MIN_PDELAY_REQ_INTERVAL, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "logMinPdelayReqInterval 9");
}

// Tests dump TRANSPARENT_CLOCK_DEFAULT_DATA_SET tlv
TEST(PmcDumpTest, TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
{
    Message m;
    TRANSPARENT_CLOCK_DEFAULT_DATA_SET_t t;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    t.clockIdentity = clockId;
    t.numberPorts = 0x177a;
    t.delayMechanism = NO_MECHANISM;
    t.primaryDomain = 18;
    useTestMode(true);
    call_dump(m, TRANSPARENT_CLOCK_DEFAULT_DATA_SET, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "clockIdentity  c47d46.fffe.20acae"
        IDENT "numberPorts    6010"
        IDENT "delayMechanism NO_MECHANISM"
        IDENT "primaryDomain  18");
}

// Tests dump PRIMARY_DOMAIN tlv
TEST(PmcDumpTest, PRIMARY_DOMAIN)
{
    Message m;
    PRIMARY_DOMAIN_t t;
    t.primaryDomain = 17;
    useTestMode(true);
    call_dump(m, PRIMARY_DOMAIN, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "primaryDomain 17");
}

// Tests dump DELAY_MECHANISM tlv
TEST(PmcDumpTest, DELAY_MECHANISM)
{
    Message m;
    DELAY_MECHANISM_t t;
    t.delayMechanism = P2P;
    useTestMode(true);
    call_dump(m, DELAY_MECHANISM, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "delayMechanism P2P");
}

// Tests dump EXTERNAL_PORT_CONFIGURATION_ENABLED tlv
TEST(PmcDumpTest, EXTERNAL_PORT_CONFIGURATION_ENABLED)
{
    Message m;
    EXTERNAL_PORT_CONFIGURATION_ENABLED_t t;
    t.flags = 1;
    useTestMode(true);
    call_dump(m, EXTERNAL_PORT_CONFIGURATION_ENABLED, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "externalPortConfiguration enabled");
}

// Tests dump MASTER_ONLY tlv
TEST(PmcDumpTest, MASTER_ONLY)
{
    Message m;
    MASTER_ONLY_t t;
    t.flags = 1;
    useTestMode(true);
    call_dump(m, MASTER_ONLY, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "masterOnly 1");
}

// Tests dump HOLDOVER_UPGRADE_ENABLE tlv
TEST(PmcDumpTest, HOLDOVER_UPGRADE_ENABLE)
{
    Message m;
    HOLDOVER_UPGRADE_ENABLE_t t;
    t.flags = 1;
    useTestMode(true);
    call_dump(m, HOLDOVER_UPGRADE_ENABLE, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "holdoverUpgradeDS enabled");
}

// Tests dump EXT_PORT_CONFIG_PORT_DATA_SET tlv
TEST(PmcDumpTest, EXT_PORT_CONFIG_PORT_DATA_SET)
{
    Message m;
    EXT_PORT_CONFIG_PORT_DATA_SET_t t;
    t.flags = 1;
    t.desiredState = PASSIVE;
    useTestMode(true);
    call_dump(m, EXT_PORT_CONFIG_PORT_DATA_SET, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "acceptableMasterPortDS enabled"
        IDENT "desiredState           PASSIVE");
}

// Tests dump TIME_STATUS_NP tlv
TEST(PmcDumpTest, TIME_STATUS_NP)
{
    Message m;
    TIME_STATUS_NP_t t;
    t.master_offset = 0;
    t.ingress_time = 0;
    t.cumulativeScaledRateOffset = 0;
    t.scaledLastGmPhaseChange = 0;
    t.gmTimeBaseIndicator = 0;
    t.nanoseconds_msb = 0;
    t.nanoseconds_lsb = 0;
    t.fractional_nanoseconds = 0;
    t.gmPresent = 0;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    t.gmIdentity = clockId;
    useTestMode(true);
    call_dump(m, TIME_STATUS_NP, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "master_offset              0"
        IDENT "ingress_time               0"
        IDENT "cumulativeScaledRateOffset +0.000000000"
        IDENT "scaledLastGmPhaseChange    0"
        IDENT "gmTimeBaseIndicator        0"
        IDENT "lastGmPhaseChange          0x0000'0000000000000000.0000"
        IDENT "gmPresent                  false"
        IDENT "gmIdentity                 c47d46.fffe.20acae");
}

// Tests dump GRANDMASTER_SETTINGS_NP tlv
TEST(PmcDumpTest, GRANDMASTER_SETTINGS_NP)
{
    Message m;
    GRANDMASTER_SETTINGS_NP_t t;
    t.clockQuality.clockClass = 255;
    t.clockQuality.clockAccuracy = Accurate_Unknown;
    t.clockQuality.offsetScaledLogVariance = 0xffff;
    t.currentUtcOffset = 37;
    t.flags = F_PTP;
    t.timeSource = INTERNAL_OSCILLATOR;
    useTestMode(true);
    call_dump(m, GRANDMASTER_SETTINGS_NP, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "clockClass              255"
        IDENT "clockAccuracy           0xfe"
        IDENT "offsetScaledLogVariance 0xffff"
        IDENT "currentUtcOffset        37"
        IDENT "leap61                  0"
        IDENT "leap59                  0"
        IDENT "currentUtcOffsetValid   0"
        IDENT "ptpTimescale            1"
        IDENT "timeTraceable           0"
        IDENT "frequencyTraceable      0"
        IDENT "timeSource              INTERNAL_OSCILLATOR");
}

// Tests dump PORT_DATA_SET_NP tlv
TEST(PmcDumpTest, PORT_DATA_SET_NP)
{
    Message m;
    PORT_DATA_SET_NP_t t;
    t.neighborPropDelayThresh = 20000000;
    t.asCapable = 1;
    useTestMode(true);
    call_dump(m, PORT_DATA_SET_NP, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "neighborPropDelayThresh 20000000"
        IDENT "asCapable               1");
}

// Tests dump SUBSCRIBE_EVENTS_NP tlv
TEST(PmcDumpTest, SUBSCRIBE_EVENTS_NP)
{
    Message m;
    SUBSCRIBE_EVENTS_NP_t t;
    t.duration = 0x1234;
    t.setEvent(NOTIFY_PORT_STATE);
    t.setEvent(NOTIFY_TIME_SYNC);
    t.setEvent(NOTIFY_PARENT_DATA_SET);
    t.setEvent(NOTIFY_CMLDS);
    useTestMode(true);
    call_dump(m, SUBSCRIBE_EVENTS_NP, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "duration               4660"
        IDENT "NOTIFY_PORT_STATE      on"
        IDENT "NOTIFY_TIME_SYNC       on"
        IDENT "NOTIFY_PARENT_DATA_SET on"
        IDENT "NOTIFY_CMLDS           on");
}

// Tests dump PORT_PROPERTIES_NP tlv
TEST(PmcDumpTest, PORT_PROPERTIES_NP)
{
    Message m;
    PORT_PROPERTIES_NP_t t;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    t.portIdentity = portIdentity;
    t.portState = LISTENING;
    t.timestamping = TS_HARDWARE;
    t.interface.textField = "enp0s25";
    useTestMode(true);
    call_dump(m, PORT_PROPERTIES_NP, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "portIdentity            c47d46.fffe.20acae-1"
        IDENT "portState               LISTENING"
        IDENT "timestamping            HARDWARE"
        IDENT "interface               enp0s25");
}

// Tests dump PORT_STATS_NP tlv
TEST(PmcDumpTest, PORT_STATS_NP)
{
    Message m;
    PORT_STATS_NP_t t;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    t.portIdentity = portIdentity;
    t.rxMsgType[STAT_SYNC] = 0;
    t.rxMsgType[STAT_DELAY_REQ] = 0;
    t.rxMsgType[STAT_PDELAY_REQ] = 0;
    t.rxMsgType[STAT_PDELAY_RESP] = 0;
    t.rxMsgType[STAT_FOLLOW_UP] = 0;
    t.rxMsgType[STAT_DELAY_RESP] = 0;
    t.rxMsgType[STAT_PDELAY_RESP_FOLLOW_UP] = 0;
    t.rxMsgType[STAT_ANNOUNCE] = 0;
    t.rxMsgType[STAT_SIGNALING] = 0;
    t.rxMsgType[STAT_MANAGEMENT] = 0;
    t.txMsgType[STAT_SYNC] = 0;
    t.txMsgType[STAT_DELAY_REQ] = 0;
    t.txMsgType[STAT_PDELAY_REQ] = 63346;
    t.txMsgType[STAT_PDELAY_RESP] = 0;
    t.txMsgType[STAT_FOLLOW_UP] = 0;
    t.txMsgType[STAT_DELAY_RESP] = 0;
    t.txMsgType[STAT_PDELAY_RESP_FOLLOW_UP] = 0;
    t.txMsgType[STAT_ANNOUNCE] = 0;
    t.txMsgType[STAT_SIGNALING] = 0;
    t.txMsgType[STAT_MANAGEMENT] = 0;
    useTestMode(true);
    call_dump(m, PORT_STATS_NP, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "portIdentity              c47d46.fffe.20acae-1"
        IDENT "rx_Sync                   0"
        IDENT "rx_Delay_Req              0"
        IDENT "rx_Pdelay_Req             0"
        IDENT "rx_Pdelay_Resp            0"
        IDENT "rx_Follow_Up              0"
        IDENT "rx_Delay_Resp             0"
        IDENT "rx_Pdelay_Resp_Follow_Up  0"
        IDENT "rx_Announce               0"
        IDENT "rx_Signaling              0"
        IDENT "rx_Management             0"
        IDENT "tx_Sync                   0"
        IDENT "tx_Delay_Req              0"
        IDENT "tx_Pdelay_Req             63346"
        IDENT "tx_Pdelay_Resp            0"
        IDENT "tx_Follow_Up              0"
        IDENT "tx_Delay_Resp             0"
        IDENT "tx_Pdelay_Resp_Follow_Up  0"
        IDENT "tx_Announce               0"
        IDENT "tx_Signaling              0"
        IDENT "tx_Management             0");
}

// Tests dump SYNCHRONIZATION_UNCERTAIN_NP tlv
TEST(PmcDumpTest, SYNCHRONIZATION_UNCERTAIN_NP)
{
    Message m;
    SYNCHRONIZATION_UNCERTAIN_NP_t t;
    t.val = SYNC_UNCERTAIN_DONTCARE;
    useTestMode(true);
    call_dump(m, SYNCHRONIZATION_UNCERTAIN_NP, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "uncertain 255");
}

// Tests dump PORT_SERVICE_STATS_NP tlv
TEST(PmcDumpTest, PORT_SERVICE_STATS_NP)
{
    Message m;
    PORT_SERVICE_STATS_NP_t t;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    t.portIdentity = portIdentity;
    t.announce_timeout = 9041;
    t.sync_timeout = 0;
    t.delay_timeout = 63346;
    t.unicast_service_timeout = 0;
    t.unicast_request_timeout = 0;
    t.master_announce_timeout = 0;
    t.master_sync_timeout = 0;
    t.qualification_timeout = 0;
    t.sync_mismatch = 0;
    t.followup_mismatch = 0;
    useTestMode(true);
    call_dump(m, PORT_SERVICE_STATS_NP, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "portIdentity              c47d46.fffe.20acae-1"
        IDENT "announce_timeout          9041"
        IDENT "sync_timeout              0"
        IDENT "delay_timeout             63346"
        IDENT "unicast_service_timeout   0"
        IDENT "unicast_request_timeout   0"
        IDENT "master_announce_timeout   0"
        IDENT "master_sync_timeout       0"
        IDENT "qualification_timeout     0"
        IDENT "sync_mismatch             0"
        IDENT "followup_mismatch         0");
}

// Tests dump UNICAST_MASTER_TABLE_NP tlv
TEST(PmcDumpTest, UNICAST_MASTER_TABLE_NP)
{
    Message m;
    UNICAST_MASTER_TABLE_NP_t t;
    t.actualTableSize = 1;
    LinuxptpUnicastMaster_t r;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    r.portIdentity = portIdentity;
    r.clockQuality.clockClass = 255;
    r.clockQuality.clockAccuracy = Accurate_Unknown;
    r.clockQuality.offsetScaledLogVariance = 0xffff;
    r.selected = 1;
    r.portState = UC_HAVE_SYDY;
    r.priority1 = 126;
    r.priority2 = 134;
    Binary physicalAddress = Binary("\xc4\x7d\x46\x20\xac\xae", 6);
    PortAddress_t portAddress = { IEEE_802_3, 6, physicalAddress };
    r.portAddress = portAddress;
    t.unicastMasters.push_back(r);
    useTestMode(true);
    call_dump(m, UNICAST_MASTER_TABLE_NP, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "actual_table_size 1"
        IDENT "BM  identity                 address                            "
        "state     clockClass clockQuality offsetScaledLogVariance p1  p2"
        IDENT "yes c47d46.fffe.20acae-1     c4:7d:46:20:ac:ae                  "
        "HAVE_SYDY 255        0xfe         0xffff                  126 134");
}

// Tests dump PORT_HWCLOCK_NP tlv
TEST(PmcDumpTest, PORT_HWCLOCK_NP)
{
    Message m;
    PORT_HWCLOCK_NP_t t;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    t.portIdentity = portIdentity;
    t.phc_index = 1;
    t.flags = 7;
    useTestMode(true);
    call_dump(m, PORT_HWCLOCK_NP, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "portIdentity            c47d46.fffe.20acae-1"
        IDENT "phcIndex                1"
        IDENT "flags                   7");
}

// Tests dump POWER_PROFILE_SETTINGS_NP tlv
TEST(PmcDumpTest, POWER_PROFILE_SETTINGS_NP)
{
    Message m;
    POWER_PROFILE_SETTINGS_NP_t t;
    t.version = IEEE_C37_238_VERSION_2011;
    t.grandmasterID = 56230;
    t.grandmasterTimeInaccuracy = 4124796349;
    t.networkTimeInaccuracy = 3655058877;
    t.totalTimeInaccuracy = 4223530875;
    useTestMode(true);
    call_dump(m, POWER_PROFILE_SETTINGS_NP, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "version                   2011"
        IDENT "grandmasterID             0xdba6"
        IDENT "grandmasterTimeInaccuracy 4124796349"
        IDENT "networkTimeInaccuracy     3655058877"
        IDENT "totalTimeInaccuracy       4223530875");
}

// Tests dump CMLDS_INFO_NP tlv
TEST(PmcDumpTest, CMLDS_INFO_NP)
{
    Message m;
    CMLDS_INFO_NP_t t;
    t.meanLinkDelay.scaledNanoseconds = 1267 * 0x10000;
    t.scaledNeighborRateRatio = 15;
    t.as_capable = 1;
    useTestMode(true);
    call_dump(m, CMLDS_INFO_NP, &t);
    EXPECT_STREQ(getPmcOut(),
        IDENT "meanLinkDelay           1267"
        IDENT "scaledNeighborRateRatio 15"
        IDENT "as_capable              1");
}

// Tests dump SLAVE_RX_SYNC_TIMING_DATA signalling
TEST(PmcSigDumpTest, SLAVE_RX_SYNC_TIMING_DATA)
{
    Message m;
    SLAVE_RX_SYNC_TIMING_DATA_t t;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    t.syncSourcePortIdentity = { clockId, 5 };
    useTestMode(true);
    call_dumpSig(m, SLAVE_RX_SYNC_TIMING_DATA, &t);
    EXPECT_STREQ(getPmcOut(),
        // Space at end follow linuxptp
        "SLAVE_RX_SYNC_TIMING_DATA N 0 "
        IDENT "syncSourcePortIdentity     c47d46.fffe.20acae-5");
    SLAVE_RX_SYNC_TIMING_DATA_rec_t r;
    r.sequenceId = 17;
    r.syncOriginTimestamp = 15.00145;
    r.totalCorrectionField.scaledNanoseconds =  1149 * 0x10000;
    r.scaledCumulativeRateOffset = 34579;
    r.syncEventIngressTimestamp = {13, 4500000};
    t.list.push_back(r);
    r.sequenceId = 21;
    r.syncOriginTimestamp = 42.0013;
    r.totalCorrectionField.scaledNanoseconds = 314 * 0x10000;
    r.scaledCumulativeRateOffset = 14573;
    r.syncEventIngressTimestamp = {55, 1210000};
    t.list.push_back(r);
    useTestMode(true);
    call_dumpSig(m, SLAVE_RX_SYNC_TIMING_DATA, &t);
    EXPECT_STREQ(getPmcOut(),
        // Space at end follow linuxptp
        "SLAVE_RX_SYNC_TIMING_DATA N 2 "
        IDENT "syncSourcePortIdentity     c47d46.fffe.20acae-5"
        IDENT "sequenceId                 17"
        IDENT "syncOriginTimestamp        15.001450000"
        IDENT "totalCorrectionField       1149"
        IDENT "scaledCumulativeRateOffset 34579"
        IDENT "syncEventIngressTimestamp  13.004500000"
        IDENT "sequenceId                 21"
        IDENT "syncOriginTimestamp        42.001300000"
        IDENT "totalCorrectionField       314"
        IDENT "scaledCumulativeRateOffset 14573"
        IDENT "syncEventIngressTimestamp  55.001210000");
}

// Tests dump SLAVE_DELAY_TIMING_DATA_NP signalling
TEST(PmcSigDumpTest, SLAVE_DELAY_TIMING_DATA_NP)
{
    Message m;
    SLAVE_DELAY_TIMING_DATA_NP_t t;
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    t.sourcePortIdentity = { clockId, 3 };
    useTestMode(true);
    call_dumpSig(m, SLAVE_DELAY_TIMING_DATA_NP, &t);
    EXPECT_STREQ(getPmcOut(),
        "SLAVE_DELAY_TIMING_DATA_NP N 0"
        IDENT "sourcePortIdentity         c47d46.fffe.20acae-3");
    SLAVE_DELAY_TIMING_DATA_NP_rec_t r;
    r.sequenceId = 145;
    r.delayOriginTimestamp = { 42, 7560000};
    r.totalCorrectionField.scaledNanoseconds = 3515 * 0x10000;
    r.delayResponseTimestamp = { 91, 357000};
    t.list.push_back(r);
    r.sequenceId = 345;
    r.delayOriginTimestamp = { 156, 1248000};
    r.totalCorrectionField.scaledNanoseconds = 155 * 0x10000;
    r.delayResponseTimestamp = { 91, 357000};
    t.list.push_back(r);
    useTestMode(true);
    call_dumpSig(m, SLAVE_DELAY_TIMING_DATA_NP, &t);
    EXPECT_STREQ(getPmcOut(),
        "SLAVE_DELAY_TIMING_DATA_NP N 2"
        IDENT "sourcePortIdentity         c47d46.fffe.20acae-3"
        IDENT "sequenceId                 145"
        IDENT "delayOriginTimestamp       42.007560000"
        IDENT "totalCorrectionField       3515"
        IDENT "delayResponseTimestamp     91.000357000"
        IDENT "sequenceId                 345"
        IDENT "delayOriginTimestamp       156.001248000"
        IDENT "totalCorrectionField       155"
        IDENT "delayResponseTimestamp     91.000357000");
}
