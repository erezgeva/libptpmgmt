/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief pmc tool build TLV unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2023 Erez Geva
 *
 */

#include "pmc.h"

Message m;
char str[512];
void (*handle)(const BaseMngTlv *);
/* Called from call_data() */
bool sendAction()
{
    /* The TLV is deleted at the end of call_data()
     * we must test the TLV withing the handle callback */
    handle(m.getSendData());
    // Ignore the "sending" messae in call_data()
    useTestMode(true);
    return true;
}

// Tests build USER_DESCRIPTION tlv
void handle_USER_DESCRIPTION(const BaseMngTlv *d);
TEST(PmcBuildTest, USER_DESCRIPTION)
{
    handle = handle_USER_DESCRIPTION;
    bool ret = call_data(m, SET, USER_DESCRIPTION, (char *)"test123");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_USER_DESCRIPTION(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const USER_DESCRIPTION_t *t = dynamic_cast<const USER_DESCRIPTION_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_STREQ(t->userDescription.string(), "test123");
}

// Tests build INITIALIZE tlv
void handle_INITIALIZE(const BaseMngTlv *d);
TEST(PmcBuildTest, INITIALIZE)
{
    handle = handle_INITIALIZE;
    bool ret = call_data(m, COMMAND, INITIALIZE, (char *)"4660");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_INITIALIZE(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const INITIALIZE_t *t = dynamic_cast<const INITIALIZE_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->initializationKey, 4660);
}

// Tests build PRIORITY1 tlv
void handle_PRIORITY1(const BaseMngTlv *d);
TEST(PmcBuildTest, PRIORITY1)
{
    handle = handle_PRIORITY1;
    bool ret = call_data(m, SET, PRIORITY1, (char *)"153");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_PRIORITY1(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const PRIORITY1_t *t = dynamic_cast<const PRIORITY1_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->priority1, 153);
}

// Tests build PRIORITY2 tlv
void handle_PRIORITY2(const BaseMngTlv *d);
TEST(PmcBuildTest, PRIORITY2)
{
    handle = handle_PRIORITY2;
    bool ret = call_data(m, SET, PRIORITY2, (char *)"137");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_PRIORITY2(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const PRIORITY2_t *t = dynamic_cast<const PRIORITY2_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->priority2, 137);
}

// Tests build DOMAIN tlv
void handle_DOMAIN(const BaseMngTlv *d);
TEST(PmcBuildTest, DOMAIN)
{
    handle = handle_DOMAIN;
    bool ret = call_data(m, SET, DOMAIN, (char *)"7");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_DOMAIN(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const DOMAIN_t *t = dynamic_cast<const DOMAIN_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->domainNumber, 7);
}

// Tests build SLAVE_ONLY tlv
void handle_SLAVE_ONLY(const BaseMngTlv *d);
void handle_SLAVE_ONLY_off(const BaseMngTlv *d);
TEST(PmcBuildTest, SLAVE_ONLY)
{
    handle = handle_SLAVE_ONLY;
    bool ret = call_data(m, SET, SLAVE_ONLY, (char *)"trUe");
    useTestMode(false);
    EXPECT_TRUE(ret);
    ret = call_data(m, SET, SLAVE_ONLY, (char *)"oN");
    useTestMode(false);
    EXPECT_TRUE(ret);
    ret = call_data(m, SET, SLAVE_ONLY, (char *)"enAble");
    useTestMode(false);
    EXPECT_TRUE(ret);
    ret = call_data(m, SET, SLAVE_ONLY, (char *)"enAbleD");
    useTestMode(false);
    EXPECT_TRUE(ret);
    ret = call_data(m, SET, SLAVE_ONLY, (char *)"1");
    useTestMode(false);
    EXPECT_TRUE(ret);
    handle = handle_SLAVE_ONLY_off;
    ret = call_data(m, SET, SLAVE_ONLY, (char *)"faLse");
    useTestMode(false);
    EXPECT_TRUE(ret);
    ret = call_data(m, SET, SLAVE_ONLY, (char *)"oFf");
    useTestMode(false);
    EXPECT_TRUE(ret);
    ret = call_data(m, SET, SLAVE_ONLY, (char *)"disAble");
    useTestMode(false);
    EXPECT_TRUE(ret);
    ret = call_data(m, SET, SLAVE_ONLY, (char *)"disAbleD");
    useTestMode(false);
    EXPECT_TRUE(ret);
    ret = call_data(m, SET, SLAVE_ONLY, (char *)"0");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_SLAVE_ONLY(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const SLAVE_ONLY_t *t = dynamic_cast<const SLAVE_ONLY_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 1);
}
void handle_SLAVE_ONLY_off(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const SLAVE_ONLY_t *t = dynamic_cast<const SLAVE_ONLY_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 0);
}

// Tests build LOG_ANNOUNCE_INTERVAL tlv
void handle_LOG_ANNOUNCE_INTERVAL(const BaseMngTlv *d);
TEST(PmcBuildTest, LOG_ANNOUNCE_INTERVAL)
{
    handle = handle_LOG_ANNOUNCE_INTERVAL;
    bool ret = call_data(m, SET, LOG_ANNOUNCE_INTERVAL, (char *)"1");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_LOG_ANNOUNCE_INTERVAL(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const LOG_ANNOUNCE_INTERVAL_t *t =
        dynamic_cast<const LOG_ANNOUNCE_INTERVAL_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->logAnnounceInterval, 1);
}

// Tests build ANNOUNCE_RECEIPT_TIMEOUT tlv
void handle_ANNOUNCE_RECEIPT_TIMEOUT(const BaseMngTlv *d);
TEST(PmcBuildTest, ANNOUNCE_RECEIPT_TIMEOUT)
{
    handle = handle_ANNOUNCE_RECEIPT_TIMEOUT;
    bool ret = call_data(m, SET, ANNOUNCE_RECEIPT_TIMEOUT, (char *)"3");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_ANNOUNCE_RECEIPT_TIMEOUT(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const ANNOUNCE_RECEIPT_TIMEOUT_t *t =
        dynamic_cast<const ANNOUNCE_RECEIPT_TIMEOUT_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->announceReceiptTimeout, 3);
}

// Tests build LOG_SYNC_INTERVAL tlv
void handle_LOG_SYNC_INTERVAL(const BaseMngTlv *d);
TEST(PmcBuildTest, LOG_SYNC_INTERVAL)
{
    handle = handle_LOG_SYNC_INTERVAL;
    bool ret = call_data(m, SET, LOG_SYNC_INTERVAL, (char *)"7");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_LOG_SYNC_INTERVAL(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const LOG_SYNC_INTERVAL_t *t = dynamic_cast<const LOG_SYNC_INTERVAL_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->logSyncInterval, 1);
}

// Tests build VERSION_NUMBER tlv
void handle_VERSION_NUMBER(const BaseMngTlv *d);
TEST(PmcBuildTest, VERSION_NUMBER)
{
    handle = handle_VERSION_NUMBER;
    strcpy(str, "versionnumber 2\nminor 1\n");
    bool ret = call_data(m, SET, VERSION_NUMBER, str);
    useTestMode(false);
    EXPECT_TRUE(ret);
    strcpy(str, "versionNumber 2 minor 1");
    ret = call_data(m, SET, VERSION_NUMBER, str);
    useTestMode(false);
    EXPECT_TRUE(ret);
    strcpy(str, "versionNumber\t2\tminor\n1\t");
    ret = call_data(m, SET, VERSION_NUMBER, str);
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_VERSION_NUMBER(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const VERSION_NUMBER_t *t = dynamic_cast<const VERSION_NUMBER_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->versionNumber, 0x12);
}

// Tests build TIME tlv
void handle_TIME(const BaseMngTlv *d);
TEST(PmcBuildTest, TIME)
{
    handle = handle_TIME;
    strcpy(str, "secondsField 13 nanosecondsField 1500000");
    bool ret = call_data(m, SET, TIME, str);
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_TIME(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const TIME_t *t = dynamic_cast<const TIME_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->currentTime, 13.0015);
}

// Tests build CLOCK_ACCURACY tlv
void handle_CLOCK_ACCURACY(const BaseMngTlv *d);
TEST(PmcBuildTest, CLOCK_ACCURACY)
{
    handle = handle_CLOCK_ACCURACY;
    bool ret = call_data(m, SET, CLOCK_ACCURACY, (char *)"0xfe");
    useTestMode(false);
    EXPECT_TRUE(ret);
    ret = call_data(m, SET, CLOCK_ACCURACY, (char *)"254");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_CLOCK_ACCURACY(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const CLOCK_ACCURACY_t *t = dynamic_cast<const CLOCK_ACCURACY_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->clockAccuracy, Accurate_Unknown);
}

// Tests build UTC_PROPERTIES tlv
void handle_UTC_PROPERTIES(const BaseMngTlv *d);
TEST(PmcBuildTest, UTC_PROPERTIES)
{
    handle = handle_UTC_PROPERTIES;
    strcpy(str,
        "currentUtcOffset -21555 "
        "leap61 true "
        "leap59 true "
        "currentUtcOffsetValid true");
    bool ret = call_data(m, SET, UTC_PROPERTIES, str);
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_UTC_PROPERTIES(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const UTC_PROPERTIES_t *t = dynamic_cast<const UTC_PROPERTIES_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->currentUtcOffset, -21555);
    EXPECT_EQ(t->flags, 7);
}

// Tests build TRACEABILITY_PROPERTIES tlv
void handle_TRACEABILITY_PROPERTIES(const BaseMngTlv *d);
TEST(PmcBuildTest, TRACEABILITY_PROPERTIES)
{
    handle = handle_TRACEABILITY_PROPERTIES;
    strcpy(str, "timeTraceable true frequencyTraceable true");
    bool ret = call_data(m, SET, TRACEABILITY_PROPERTIES, str);
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_TRACEABILITY_PROPERTIES(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const TRACEABILITY_PROPERTIES_t *t =
        dynamic_cast<const TRACEABILITY_PROPERTIES_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, F_TTRA | F_FTRA);
}

// Tests build TIMESCALE_PROPERTIES tlv
void handle_TIMESCALE_PROPERTIES(const BaseMngTlv *d);
TEST(PmcBuildTest, TIMESCALE_PROPERTIES)
{
    handle = handle_TIMESCALE_PROPERTIES;
    bool ret = call_data(m, SET, TIMESCALE_PROPERTIES, (char *)"true");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_TIMESCALE_PROPERTIES(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const TIMESCALE_PROPERTIES_t *t = dynamic_cast<const TIMESCALE_PROPERTIES_t *>
        (d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, F_PTP);
}

// Tests build UNICAST_NEGOTIATION_ENABLE tlv
void handle_UNICAST_NEGOTIATION_ENABLE(const BaseMngTlv *d);
TEST(PmcBuildTest, UNICAST_NEGOTIATION_ENABLE)
{
    handle = handle_UNICAST_NEGOTIATION_ENABLE;
    bool ret = call_data(m, SET, UNICAST_NEGOTIATION_ENABLE, (char *)"true");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_UNICAST_NEGOTIATION_ENABLE(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const UNICAST_NEGOTIATION_ENABLE_t *t =
        dynamic_cast<const UNICAST_NEGOTIATION_ENABLE_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 1);
}

// Tests build PATH_TRACE_ENABLE tlv
void handle_PATH_TRACE_ENABLE(const BaseMngTlv *d);
TEST(PmcBuildTest, PATH_TRACE_ENABLE)
{
    handle = handle_PATH_TRACE_ENABLE;
    bool ret = call_data(m, SET, PATH_TRACE_ENABLE, (char *)"true");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_PATH_TRACE_ENABLE(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const PATH_TRACE_ENABLE_t *t = dynamic_cast<const PATH_TRACE_ENABLE_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 1);
}

// Tests build ACCEPTABLE_MASTER_TABLE_ENABLED tlv
void handle_ACCEPTABLE_MASTER_TABLE_ENABLED(const BaseMngTlv *d);
TEST(PmcBuildTest, ACCEPTABLE_MASTER_TABLE_ENABLED)
{
    handle = handle_ACCEPTABLE_MASTER_TABLE_ENABLED;
    bool ret = call_data(m, SET, ACCEPTABLE_MASTER_TABLE_ENABLED, (char *)"true");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_ACCEPTABLE_MASTER_TABLE_ENABLED(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const ACCEPTABLE_MASTER_TABLE_ENABLED_t *t =
        dynamic_cast<const ACCEPTABLE_MASTER_TABLE_ENABLED_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 1);
}

// Tests build ALTERNATE_MASTER tlv
void handle_ALTERNATE_MASTER(const BaseMngTlv *d);
TEST(PmcBuildTest, ALTERNATE_MASTER)
{
    handle = handle_ALTERNATE_MASTER;
    strcpy(str,
        "transmitAlternateMulticastSync true "
        "logAlternateMulticastSyncInterval -17 "
        "numberOfAlternateMasters 210");
    bool ret = call_data(m, SET, ALTERNATE_MASTER, str);
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_ALTERNATE_MASTER(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const ALTERNATE_MASTER_t *t = dynamic_cast<const ALTERNATE_MASTER_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 1);
    EXPECT_EQ(t->logAlternateMulticastSyncInterval, -17);
    EXPECT_EQ(t->numberOfAlternateMasters, 210);
}

// Tests build ALTERNATE_TIME_OFFSET_ENABLE tlv
void handle_ALTERNATE_TIME_OFFSET_ENABLE(const BaseMngTlv *d);
TEST(PmcBuildTest, ALTERNATE_TIME_OFFSET_ENABLE)
{
    handle = handle_ALTERNATE_TIME_OFFSET_ENABLE;
    bool ret = call_data(m, SET, ALTERNATE_TIME_OFFSET_ENABLE, (char *)"true");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_ALTERNATE_TIME_OFFSET_ENABLE(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const ALTERNATE_TIME_OFFSET_ENABLE_t *t =
        dynamic_cast<const ALTERNATE_TIME_OFFSET_ENABLE_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 1);
}

// Tests build ALTERNATE_TIME_OFFSET_NAME tlv
void handle_ALTERNATE_TIME_OFFSET_NAME(const BaseMngTlv *d);
TEST(PmcBuildTest, ALTERNATE_TIME_OFFSET_NAME)
{
    handle = handle_ALTERNATE_TIME_OFFSET_NAME;
    strcpy(str, "keyField 11 displayName 123");
    bool ret = call_data(m, SET, ALTERNATE_TIME_OFFSET_NAME, str);
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_ALTERNATE_TIME_OFFSET_NAME(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const ALTERNATE_TIME_OFFSET_NAME_t *t =
        dynamic_cast<const ALTERNATE_TIME_OFFSET_NAME_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->keyField, 11);
    EXPECT_STREQ(t->displayName.string(), "123");
}

// Tests build ALTERNATE_TIME_OFFSET_PROPERTIES tlv
void handle_ALTERNATE_TIME_OFFSET_PROPERTIES(const BaseMngTlv *d);
TEST(PmcBuildTest, ALTERNATE_TIME_OFFSET_PROPERTIES)
{
    handle = handle_ALTERNATE_TIME_OFFSET_PROPERTIES;
    strcpy(str,
        "keyField 13 "
        "currentOffset -2145493247 "
        "jumpSeconds -2147413249 "
        "timeOfNextJump 159585821399185");
    bool ret = call_data(m, SET, ALTERNATE_TIME_OFFSET_PROPERTIES, str);
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_ALTERNATE_TIME_OFFSET_PROPERTIES(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const ALTERNATE_TIME_OFFSET_PROPERTIES_t *t =
        dynamic_cast<const ALTERNATE_TIME_OFFSET_PROPERTIES_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->keyField, 13);
    EXPECT_EQ(t->currentOffset, -2145493247);
    EXPECT_EQ(t->jumpSeconds, -2147413249);
    EXPECT_EQ(t->timeOfNextJump, 0x912478321891LL);
}

// Tests build LOG_MIN_PDELAY_REQ_INTERVAL tlv
void handle_LOG_MIN_PDELAY_REQ_INTERVAL(const BaseMngTlv *d);
TEST(PmcBuildTest, LOG_MIN_PDELAY_REQ_INTERVAL)
{
    handle = handle_LOG_MIN_PDELAY_REQ_INTERVAL;
    bool ret = call_data(m, SET, LOG_MIN_PDELAY_REQ_INTERVAL, (char *)"9");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_LOG_MIN_PDELAY_REQ_INTERVAL(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const LOG_MIN_PDELAY_REQ_INTERVAL_t *t =
        dynamic_cast<const LOG_MIN_PDELAY_REQ_INTERVAL_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->logMinPdelayReqInterval, 9);
}

// Tests build PRIMARY_DOMAIN tlv
void handle_PRIMARY_DOMAIN(const BaseMngTlv *d);
TEST(PmcBuildTest, PRIMARY_DOMAIN)
{
    handle = handle_PRIMARY_DOMAIN;
    bool ret = call_data(m, SET, PRIMARY_DOMAIN, (char *)"17");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_PRIMARY_DOMAIN(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const PRIMARY_DOMAIN_t *t = dynamic_cast<const PRIMARY_DOMAIN_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->primaryDomain, 17);
}

// Tests build DELAY_MECHANISM tlv
void handle_DELAY_MECHANISM(const BaseMngTlv *d);
TEST(PmcBuildTest, DELAY_MECHANISM)
{
    handle = handle_DELAY_MECHANISM;
    bool ret = call_data(m, SET, DELAY_MECHANISM, (char *)"2");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_DELAY_MECHANISM(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const DELAY_MECHANISM_t *t = dynamic_cast<const DELAY_MECHANISM_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->delayMechanism, P2P);
}

// Tests build EXTERNAL_PORT_CONFIGURATION_ENABLED tlv
void handle_EXTERNAL_PORT_CONFIGURATION_ENABLED(const BaseMngTlv *d);
TEST(PmcBuildTest, EXTERNAL_PORT_CONFIGURATION_ENABLED)
{
    handle = handle_EXTERNAL_PORT_CONFIGURATION_ENABLED;
    bool ret =
        call_data(m, SET, EXTERNAL_PORT_CONFIGURATION_ENABLED, (char *)"true");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_EXTERNAL_PORT_CONFIGURATION_ENABLED(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const EXTERNAL_PORT_CONFIGURATION_ENABLED_t *t =
        dynamic_cast<const EXTERNAL_PORT_CONFIGURATION_ENABLED_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 1);
}

// Tests build MASTER_ONLY tlv
void handle_MASTER_ONLY(const BaseMngTlv *d);
TEST(PmcBuildTest, MASTER_ONLY)
{
    handle = handle_MASTER_ONLY;
    bool ret = call_data(m, SET, MASTER_ONLY, (char *)"enable");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_MASTER_ONLY(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const MASTER_ONLY_t *t = dynamic_cast<const MASTER_ONLY_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 1);
}

// Tests build HOLDOVER_UPGRADE_ENABLE tlv
void handle_HOLDOVER_UPGRADE_ENABLE(const BaseMngTlv *d);
TEST(PmcBuildTest, HOLDOVER_UPGRADE_ENABLE)
{
    handle = handle_HOLDOVER_UPGRADE_ENABLE;
    bool ret = call_data(m, SET, HOLDOVER_UPGRADE_ENABLE, (char *)"true");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_HOLDOVER_UPGRADE_ENABLE(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const HOLDOVER_UPGRADE_ENABLE_t *t =
        dynamic_cast<const HOLDOVER_UPGRADE_ENABLE_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 1);
}

// Tests build EXT_PORT_CONFIG_PORT_DATA_SET tlv
void handle_EXT_PORT_CONFIG_PORT_DATA_SET(const BaseMngTlv *d);
TEST(PmcBuildTest, EXT_PORT_CONFIG_PORT_DATA_SET)
{
    handle = handle_EXT_PORT_CONFIG_PORT_DATA_SET;
    strcpy(str, "acceptableMasterPortDS true desiredState PASSIVE");
    bool ret = call_data(m, SET, EXT_PORT_CONFIG_PORT_DATA_SET, str);
    useTestMode(false);
    EXPECT_TRUE(ret);
    strcpy(str, "acceptableMasterPortDS on desiredState 7");
    ret = call_data(m, SET, EXT_PORT_CONFIG_PORT_DATA_SET, str);
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_EXT_PORT_CONFIG_PORT_DATA_SET(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const EXT_PORT_CONFIG_PORT_DATA_SET_t *t =
        dynamic_cast<const EXT_PORT_CONFIG_PORT_DATA_SET_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 1);
    EXPECT_EQ(t->desiredState, PASSIVE);
}

// Tests build GRANDMASTER_SETTINGS_NP tlv
void handle_GRANDMASTER_SETTINGS_NP(const BaseMngTlv *d);
TEST(PmcBuildTest, GRANDMASTER_SETTINGS_NP)
{
    handle = handle_GRANDMASTER_SETTINGS_NP;
    strcpy(str,
        "clockClass 255 "
        "clockAccuracy 0xfe "
        "offsetScaledLogVariance 0xffff "
        "currentUtcOffset -37 "
        "leap61 false "
        "leap59 false "
        "currentUtcOffsetValid false "
        "ptpTimescale true "
        "timeTraceable false "
        "frequencyTraceable false "
        "timeSource INTERNAL_OSCILLATOR");
    bool ret = call_data(m, SET, GRANDMASTER_SETTINGS_NP, str);
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_GRANDMASTER_SETTINGS_NP(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const GRANDMASTER_SETTINGS_NP_t *t =
        dynamic_cast<const GRANDMASTER_SETTINGS_NP_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->clockQuality.clockClass, 255);
    EXPECT_EQ(t->clockQuality.clockAccuracy, Accurate_Unknown);
    EXPECT_EQ(t->clockQuality.offsetScaledLogVariance, 0xffff);
    EXPECT_EQ(t->currentUtcOffset, -37);
    EXPECT_EQ(t->flags, F_PTP);
    EXPECT_EQ(t->timeSource, INTERNAL_OSCILLATOR);
}

// Tests build PORT_DATA_SET_NP tlv
void handle_PORT_DATA_SET_NP(const BaseMngTlv *d);
TEST(PmcBuildTest, PORT_DATA_SET_NP)
{
    handle = handle_PORT_DATA_SET_NP;
    strcpy(str, "neighborPropDelayThresh 20000000 asCapable 1");
    bool ret = call_data(m, SET, PORT_DATA_SET_NP, str);
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_PORT_DATA_SET_NP(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const PORT_DATA_SET_NP_t *t = dynamic_cast<const PORT_DATA_SET_NP_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->neighborPropDelayThresh, 20000000);
    EXPECT_EQ(t->asCapable, 1);
}

// Tests build SUBSCRIBE_EVENTS_NP tlv
void handle_SUBSCRIBE_EVENTS_NP(const BaseMngTlv *d);
TEST(PmcBuildTest, SUBSCRIBE_EVENTS_NP)
{
    handle = handle_SUBSCRIBE_EVENTS_NP;
    strcpy(str,
        "duration 4660 "
        "NOTIFY_PORT_STATE true "
        "NOTIFY_TIME_SYNC true");
    bool ret = call_data(m, SET, SUBSCRIBE_EVENTS_NP, str);
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_SUBSCRIBE_EVENTS_NP(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const SUBSCRIBE_EVENTS_NP_t *t = dynamic_cast<const SUBSCRIBE_EVENTS_NP_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->duration, 0x1234);
    EXPECT_TRUE(t->getEvent(NOTIFY_PORT_STATE));
    EXPECT_TRUE(t->getEvent(NOTIFY_TIME_SYNC));
}

// Tests build SYNCHRONIZATION_UNCERTAIN_NP tlv
void handle_SYNCHRONIZATION_UNCERTAIN_NP(const BaseMngTlv *d);
TEST(PmcBuildTest, SYNCHRONIZATION_UNCERTAIN_NP)
{
    handle = handle_SYNCHRONIZATION_UNCERTAIN_NP;
    bool ret = call_data(m, SET, SYNCHRONIZATION_UNCERTAIN_NP, (char *)"0xff");
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_SYNCHRONIZATION_UNCERTAIN_NP(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const SYNCHRONIZATION_UNCERTAIN_NP_t *t =
        dynamic_cast<const SYNCHRONIZATION_UNCERTAIN_NP_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->val, SYNC_UNCERTAIN_DONTCARE);
}

// Tests build POWER_PROFILE_SETTINGS_NP tlv
void handle_POWER_PROFILE_SETTINGS_NP(const BaseMngTlv *d);
TEST(PmcBuildTest, POWER_PROFILE_SETTINGS_NP)
{
    handle = handle_POWER_PROFILE_SETTINGS_NP;
    strcpy(str,
        "version 2011 "
        "grandmasterID 0xdba6 "
        "grandmasterTimeInaccuracy 4124796349 "
        "networkTimeInaccuracy 3655058877 "
        "totalTimeInaccuracy 4223530875");
    bool ret = call_data(m, SET, POWER_PROFILE_SETTINGS_NP, str);
    useTestMode(false);
    EXPECT_TRUE(ret);
}
void handle_POWER_PROFILE_SETTINGS_NP(const BaseMngTlv *d)
{
    ASSERT_NE(d, nullptr);
    const POWER_PROFILE_SETTINGS_NP_t *t =
        dynamic_cast<const POWER_PROFILE_SETTINGS_NP_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->version, IEEE_C37_238_VERSION_2011);
    EXPECT_EQ(t->grandmasterID, 0xdba6);
    EXPECT_EQ(t->grandmasterTimeInaccuracy, 4124796349);
    EXPECT_EQ(t->networkTimeInaccuracy, 3655058877);
    EXPECT_EQ(t->totalTimeInaccuracy, 4223530875);
}
