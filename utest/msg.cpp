/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Message class unit test
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

#include "msg.h"
#include "comp.h"

using namespace ptpmgmt;

// Tests message empty constructor
// Message()
TEST(MessageTest, MethodEmptyConstructor)
{
    Message m;
    const MsgParams &p = m.getParams();
    EXPECT_EQ(p.target.portNumber, 0xffff);
    EXPECT_EQ(p.target.clockIdentity, Binary("\xff\xff\xff\xff\xff\xff\xff\xff"));
}

// Tests message constructor
// Message(const MsgParams &prms)
TEST(MessageTest, MethodConstructor)
{
    MsgParams p1;
    p1.transportSpecific = 0xf;
    p1.domainNumber = 17;
    p1.boundaryHops = 13;
    p1.isUnicast = false;;
    Message m(p1);
    const MsgParams &p = m.getParams();
    EXPECT_EQ(p.transportSpecific, 0xf);
    EXPECT_EQ(p.domainNumber, 17);
    EXPECT_EQ(p.boundaryHops, 13);
    EXPECT_FALSE(p.isUnicast);
}

// Tests get parameters method
// const MsgParams &getParams() const
TEST(MessageTest, MethodGetParams)
{
    MsgParams p1;
    p1.transportSpecific = 0xf;
    p1.domainNumber = 17;
    p1.boundaryHops = 13;
    p1.isUnicast = false;;
    Message m(p1);
    const MsgParams &p = m.getParams();
    EXPECT_EQ(p.transportSpecific, p1.transportSpecific);
    EXPECT_EQ(p.domainNumber, p1.domainNumber);
    EXPECT_EQ(p.boundaryHops, p1.boundaryHops);
    EXPECT_EQ(p.isUnicast, p1.isUnicast);
    EXPECT_EQ(p.implementSpecific, p1.implementSpecific);
    EXPECT_EQ(p.target, p1.target);
    EXPECT_EQ(p.self_id, p1.self_id);
    EXPECT_EQ(p.useZeroGet, p1.useZeroGet);
    EXPECT_EQ(p.rcvSignaling, p1.rcvSignaling);
    EXPECT_EQ(p.filterSignaling, p1.filterSignaling);
}

// Tests set parameters method
// bool updateParams(const MsgParams &prms)
TEST(MessageTest, MethodUpdateParams)
{
    MsgParams p1;
    p1.transportSpecific = 0xf;
    p1.domainNumber = 17;
    p1.boundaryHops = 13;
    p1.isUnicast = false;;
    Message m;
    EXPECT_TRUE(m.updateParams(p1));
    const MsgParams &p = m.getParams();
    EXPECT_EQ(p.transportSpecific, p1.transportSpecific);
    EXPECT_EQ(p.domainNumber, p1.domainNumber);
    EXPECT_EQ(p.boundaryHops, p1.boundaryHops);
    EXPECT_EQ(p.isUnicast, p1.isUnicast);
    EXPECT_EQ(p.implementSpecific, p1.implementSpecific);
    EXPECT_EQ(p.target, p1.target);
    EXPECT_EQ(p.self_id, p1.self_id);
    EXPECT_EQ(p.useZeroGet, p1.useZeroGet);
    EXPECT_EQ(p.rcvSignaling, p1.rcvSignaling);
    EXPECT_EQ(p.filterSignaling, p1.filterSignaling);
}

// Tests get TLV ID method
// mng_vals_e getTlvId() const
TEST(MessageTest, MethodGetTlvId)
{
    Message m;
    EXPECT_EQ(m.getTlvId(), NULL_PTP_MANAGEMENT);
}

// Tests set using all clocks method
// void setAllClocks()
TEST(MessageTest, MethodSetAllClocks)
{
    MsgParams p1;
    p1.target.portNumber = 0x1f1f;
    p1.target.clockIdentity = { 1, 2, 3, 4, 5, 6, 7, 8 };
    Message m(p1);
    m.setAllClocks();
    const MsgParams &p = m.getParams();
    EXPECT_EQ(p.target.portNumber, 0xffff);
    EXPECT_EQ(p.target.clockIdentity, Binary("\xff\xff\xff\xff\xff\xff\xff\xff"));
}

// Tests set using all clocks method
// bool isAllClocks() const
TEST(MessageTest, MethodIsAllClocks)
{
    MsgParams p1;
    p1.target.portNumber = 0x1f1f;
    p1.target.clockIdentity = { 1, 2, 3, 4, 5, 6, 7, 8 };
    Message m(p1);
    EXPECT_FALSE(m.isAllClocks());
    m.setAllClocks();
    EXPECT_TRUE(m.isAllClocks());
}

// Tests using configuration file method
// bool useConfig(const ConfigFile &cfg, const std::string &section = "")
TEST(MessageTest, MethodUseConfig)
{
    ConfigFile f;
    EXPECT_TRUE(f.read_cfg("utest/testing.cfg"));
    Message m;
    EXPECT_TRUE(m.useConfig(f, "dumm"));
    const MsgParams &p = m.getParams();
    EXPECT_EQ(p.transportSpecific, 9);
    EXPECT_EQ(p.domainNumber, 3);
    EXPECT_TRUE(m.useConfig(f));
    EXPECT_EQ(p.transportSpecific, 7);
    EXPECT_EQ(p.domainNumber, 5);
}

// Tests convert error to string method
// static const char *err2str_c(MNG_PARSE_ERROR_e err)
TEST(MessageTest, MethodErr2str)
{
    EXPECT_STREQ(Message::err2str_c(MNG_PARSE_ERROR_OK), "MNG_PARSE_ERROR_OK");
    EXPECT_STREQ(Message::err2str_c(MNG_PARSE_ERROR_MSG), "MNG_PARSE_ERROR_MSG");
    EXPECT_STREQ(Message::err2str_c(MNG_PARSE_ERROR_SIG), "MNG_PARSE_ERROR_SIG");
    EXPECT_STREQ(Message::err2str_c(MNG_PARSE_ERROR_INVALID_ID),
        "MNG_PARSE_ERROR_INVALID_ID");
    EXPECT_STREQ(Message::err2str_c(MNG_PARSE_ERROR_INVALID_TLV),
        "MNG_PARSE_ERROR_INVALID_TLV");
    EXPECT_STREQ(Message::err2str_c(MNG_PARSE_ERROR_MISMATCH_TLV),
        "MNG_PARSE_ERROR_MISMATCH_TLV");
    EXPECT_STREQ(Message::err2str_c(MNG_PARSE_ERROR_SIZE_MISS),
        "MNG_PARSE_ERROR_SIZE_MISS");
    EXPECT_STREQ(Message::err2str_c(MNG_PARSE_ERROR_TOO_SMALL),
        "MNG_PARSE_ERROR_TOO_SMALL");
    EXPECT_STREQ(Message::err2str_c(MNG_PARSE_ERROR_SIZE), "MNG_PARSE_ERROR_SIZE");
    EXPECT_STREQ(Message::err2str_c(MNG_PARSE_ERROR_VAL), "MNG_PARSE_ERROR_VAL");
    EXPECT_STREQ(Message::err2str_c(MNG_PARSE_ERROR_HEADER),
        "MNG_PARSE_ERROR_HEADER");
    EXPECT_STREQ(Message::err2str_c(MNG_PARSE_ERROR_ACTION),
        "MNG_PARSE_ERROR_ACTION");
    EXPECT_STREQ(Message::err2str_c(MNG_PARSE_ERROR_UNSUPPORT),
        "MNG_PARSE_ERROR_UNSUPPORT");
    EXPECT_STREQ(Message::err2str_c(MNG_PARSE_ERROR_MEM), "MNG_PARSE_ERROR_MEM");
}

// tests convert message type to string method
// static const char *type2str_c(msgType_e type)
TEST(MessageTest, MethodType2str)
{
    EXPECT_STREQ(Message::type2str_c(Sync), "Sync");
    EXPECT_STREQ(Message::type2str_c(Delay_Req), "Delay_Req");
    EXPECT_STREQ(Message::type2str_c(Pdelay_Req), "Pdelay_Req");
    EXPECT_STREQ(Message::type2str_c(Pdelay_Resp), "Pdelay_Resp");
    EXPECT_STREQ(Message::type2str_c(Follow_Up), "Follow_Up");
    EXPECT_STREQ(Message::type2str_c(Delay_Resp), "Delay_Resp");
    EXPECT_STREQ(Message::type2str_c(Pdelay_Resp_Follow_Up),
        "Pdelay_Resp_Follow_Up");
    EXPECT_STREQ(Message::type2str_c(Announce), "Announce");
    EXPECT_STREQ(Message::type2str_c(Signaling), "Signaling");
    EXPECT_STREQ(Message::type2str_c(Management), "Management");
}

// tests convert TLV type to string method
// static const char *tlv2str_c(tlvType_e type)
TEST(MessageTest, MethodTlv2str)
{
    EXPECT_STREQ(Message::tlv2str_c(MANAGEMENT), "MANAGEMENT");
    EXPECT_STREQ(Message::tlv2str_c(MANAGEMENT_ERROR_STATUS),
        "MANAGEMENT_ERROR_STATUS");
    EXPECT_STREQ(Message::tlv2str_c(ORGANIZATION_EXTENSION),
        "ORGANIZATION_EXTENSION");
    EXPECT_STREQ(Message::tlv2str_c(REQUEST_UNICAST_TRANSMISSION),
        "REQUEST_UNICAST_TRANSMISSION");
    EXPECT_STREQ(Message::tlv2str_c(GRANT_UNICAST_TRANSMISSION),
        "GRANT_UNICAST_TRANSMISSION");
    EXPECT_STREQ(Message::tlv2str_c(CANCEL_UNICAST_TRANSMISSION),
        "CANCEL_UNICAST_TRANSMISSION");
    EXPECT_STREQ(Message::tlv2str_c(ACKNOWLEDGE_CANCEL_UNICAST_TRANSMISSION),
        "ACKNOWLEDGE_CANCEL_UNICAST_TRANSMISSION");
    EXPECT_STREQ(Message::tlv2str_c(PATH_TRACE), "PATH_TRACE");
    EXPECT_STREQ(Message::tlv2str_c(ALTERNATE_TIME_OFFSET_INDICATOR),
        "ALTERNATE_TIME_OFFSET_INDICATOR");
    EXPECT_STREQ(Message::tlv2str_c(ORGANIZATION_EXTENSION_PROPAGATE),
        "ORGANIZATION_EXTENSION_PROPAGATE");
    EXPECT_STREQ(Message::tlv2str_c(ENHANCED_ACCURACY_METRICS),
        "ENHANCED_ACCURACY_METRICS");
    EXPECT_STREQ(Message::tlv2str_c(ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE),
        "ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE");
    EXPECT_STREQ(Message::tlv2str_c(L1_SYNC), "L1_SYNC");
    EXPECT_STREQ(Message::tlv2str_c(PORT_COMMUNICATION_AVAILABILITY),
        "PORT_COMMUNICATION_AVAILABILITY");
    EXPECT_STREQ(Message::tlv2str_c(PROTOCOL_ADDRESS), "PROTOCOL_ADDRESS");
    EXPECT_STREQ(Message::tlv2str_c(SLAVE_RX_SYNC_TIMING_DATA),
        "SLAVE_RX_SYNC_TIMING_DATA");
    EXPECT_STREQ(Message::tlv2str_c(SLAVE_RX_SYNC_COMPUTED_DATA),
        "SLAVE_RX_SYNC_COMPUTED_DATA");
    EXPECT_STREQ(Message::tlv2str_c(SLAVE_TX_EVENT_TIMESTAMPS),
        "SLAVE_TX_EVENT_TIMESTAMPS");
    EXPECT_STREQ(Message::tlv2str_c(CUMULATIVE_RATE_RATIO),
        "CUMULATIVE_RATE_RATIO");
    EXPECT_STREQ(Message::tlv2str_c(TLV_PAD), "PAD");
    EXPECT_STREQ(Message::tlv2str_c(AUTHENTICATION), "AUTHENTICATION");
    EXPECT_STREQ(Message::tlv2str_c(SLAVE_DELAY_TIMING_DATA_NP),
        "SLAVE_DELAY_TIMING_DATA_NP");
}

// tests convert action to string
// static const char *act2str_c(actionField_e action)
TEST(MessageTest, MethodAct2str)
{
    EXPECT_STREQ(Message::act2str_c(GET), "GET");
    EXPECT_STREQ(Message::act2str_c(SET), "SET");
    EXPECT_STREQ(Message::act2str_c(RESPONSE), "RESPONSE");
    EXPECT_STREQ(Message::act2str_c(COMMAND), "COMMAND");
    EXPECT_STREQ(Message::act2str_c(ACKNOWLEDGE), "ACKNOWLEDGE");
}

// tests convert management id to string
// static const char *mng2str_c(mng_vals_e id)
TEST(MessageTest, MethodMng2str)
{
    EXPECT_STREQ(Message::mng2str_c(FIRST_MNG_ID), "NULL_PTP_MANAGEMENT");
    EXPECT_STREQ(Message::mng2str_c(NULL_PTP_MANAGEMENT), "NULL_PTP_MANAGEMENT");
    EXPECT_STREQ(Message::mng2str_c(NULL_MANAGEMENT), "NULL_PTP_MANAGEMENT");
    EXPECT_STREQ(Message::mng2str_c(CLOCK_DESCRIPTION), "CLOCK_DESCRIPTION");
    EXPECT_STREQ(Message::mng2str_c(USER_DESCRIPTION), "USER_DESCRIPTION");
    EXPECT_STREQ(Message::mng2str_c(SAVE_IN_NON_VOLATILE_STORAGE),
        "SAVE_IN_NON_VOLATILE_STORAGE");
    EXPECT_STREQ(Message::mng2str_c(RESET_NON_VOLATILE_STORAGE),
        "RESET_NON_VOLATILE_STORAGE");
    EXPECT_STREQ(Message::mng2str_c(INITIALIZE), "INITIALIZE");
    EXPECT_STREQ(Message::mng2str_c(FAULT_LOG), "FAULT_LOG");
    EXPECT_STREQ(Message::mng2str_c(FAULT_LOG_RESET), "FAULT_LOG_RESET");
    EXPECT_STREQ(Message::mng2str_c(DEFAULT_DATA_SET), "DEFAULT_DATA_SET");
    EXPECT_STREQ(Message::mng2str_c(CURRENT_DATA_SET), "CURRENT_DATA_SET");
    EXPECT_STREQ(Message::mng2str_c(PARENT_DATA_SET), "PARENT_DATA_SET");
    EXPECT_STREQ(Message::mng2str_c(TIME_PROPERTIES_DATA_SET),
        "TIME_PROPERTIES_DATA_SET");
    EXPECT_STREQ(Message::mng2str_c(PORT_DATA_SET), "PORT_DATA_SET");
    EXPECT_STREQ(Message::mng2str_c(PRIORITY1), "PRIORITY1");
    EXPECT_STREQ(Message::mng2str_c(PRIORITY2), "PRIORITY2");
    EXPECT_STREQ(Message::mng2str_c(DOMAIN), "DOMAIN");
    EXPECT_STREQ(Message::mng2str_c(SLAVE_ONLY), "SLAVE_ONLY");
    EXPECT_STREQ(Message::mng2str_c(LOG_ANNOUNCE_INTERVAL),
        "LOG_ANNOUNCE_INTERVAL");
    EXPECT_STREQ(Message::mng2str_c(ANNOUNCE_RECEIPT_TIMEOUT),
        "ANNOUNCE_RECEIPT_TIMEOUT");
    EXPECT_STREQ(Message::mng2str_c(LOG_SYNC_INTERVAL), "LOG_SYNC_INTERVAL");
    EXPECT_STREQ(Message::mng2str_c(VERSION_NUMBER), "VERSION_NUMBER");
    EXPECT_STREQ(Message::mng2str_c(ENABLE_PORT), "ENABLE_PORT");
    EXPECT_STREQ(Message::mng2str_c(DISABLE_PORT), "DISABLE_PORT");
    EXPECT_STREQ(Message::mng2str_c(TIME), "TIME");
    EXPECT_STREQ(Message::mng2str_c(CLOCK_ACCURACY), "CLOCK_ACCURACY");
    EXPECT_STREQ(Message::mng2str_c(UTC_PROPERTIES), "UTC_PROPERTIES");
    EXPECT_STREQ(Message::mng2str_c(TRACEABILITY_PROPERTIES),
        "TRACEABILITY_PROPERTIES");
    EXPECT_STREQ(Message::mng2str_c(TIMESCALE_PROPERTIES), "TIMESCALE_PROPERTIES");
    EXPECT_STREQ(Message::mng2str_c(UNICAST_NEGOTIATION_ENABLE),
        "UNICAST_NEGOTIATION_ENABLE");
    EXPECT_STREQ(Message::mng2str_c(PATH_TRACE_LIST), "PATH_TRACE_LIST");
    EXPECT_STREQ(Message::mng2str_c(PATH_TRACE_ENABLE), "PATH_TRACE_ENABLE");
    EXPECT_STREQ(Message::mng2str_c(GRANDMASTER_CLUSTER_TABLE),
        "GRANDMASTER_CLUSTER_TABLE");
    EXPECT_STREQ(Message::mng2str_c(UNICAST_MASTER_TABLE), "UNICAST_MASTER_TABLE");
    EXPECT_STREQ(Message::mng2str_c(UNICAST_MASTER_MAX_TABLE_SIZE),
        "UNICAST_MASTER_MAX_TABLE_SIZE");
    EXPECT_STREQ(Message::mng2str_c(ACCEPTABLE_MASTER_TABLE),
        "ACCEPTABLE_MASTER_TABLE");
    EXPECT_STREQ(Message::mng2str_c(ACCEPTABLE_MASTER_TABLE_ENABLED),
        "ACCEPTABLE_MASTER_TABLE_ENABLED");
    EXPECT_STREQ(Message::mng2str_c(ACCEPTABLE_MASTER_MAX_TABLE_SIZE),
        "ACCEPTABLE_MASTER_MAX_TABLE_SIZE");
    EXPECT_STREQ(Message::mng2str_c(ALTERNATE_MASTER), "ALTERNATE_MASTER");
    EXPECT_STREQ(Message::mng2str_c(ALTERNATE_TIME_OFFSET_ENABLE),
        "ALTERNATE_TIME_OFFSET_ENABLE");
    EXPECT_STREQ(Message::mng2str_c(ALTERNATE_TIME_OFFSET_NAME),
        "ALTERNATE_TIME_OFFSET_NAME");
    EXPECT_STREQ(Message::mng2str_c(ALTERNATE_TIME_OFFSET_MAX_KEY),
        "ALTERNATE_TIME_OFFSET_MAX_KEY");
    EXPECT_STREQ(Message::mng2str_c(ALTERNATE_TIME_OFFSET_PROPERTIES),
        "ALTERNATE_TIME_OFFSET_PROPERTIES");
    EXPECT_STREQ(Message::mng2str_c(TRANSPARENT_CLOCK_PORT_DATA_SET),
        "TRANSPARENT_CLOCK_PORT_DATA_SET");
    EXPECT_STREQ(Message::mng2str_c(LOG_MIN_PDELAY_REQ_INTERVAL),
        "LOG_MIN_PDELAY_REQ_INTERVAL");
    EXPECT_STREQ(Message::mng2str_c(TRANSPARENT_CLOCK_DEFAULT_DATA_SET),
        "TRANSPARENT_CLOCK_DEFAULT_DATA_SET");
    EXPECT_STREQ(Message::mng2str_c(PRIMARY_DOMAIN), "PRIMARY_DOMAIN");
    EXPECT_STREQ(Message::mng2str_c(DELAY_MECHANISM), "DELAY_MECHANISM");
    EXPECT_STREQ(Message::mng2str_c(EXTERNAL_PORT_CONFIGURATION_ENABLED),
        "EXTERNAL_PORT_CONFIGURATION_ENABLED");
    EXPECT_STREQ(Message::mng2str_c(MASTER_ONLY), "MASTER_ONLY");
    EXPECT_STREQ(Message::mng2str_c(HOLDOVER_UPGRADE_ENABLE),
        "HOLDOVER_UPGRADE_ENABLE");
    EXPECT_STREQ(Message::mng2str_c(EXT_PORT_CONFIG_PORT_DATA_SET),
        "EXT_PORT_CONFIG_PORT_DATA_SET");
    EXPECT_STREQ(Message::mng2str_c(TIME_STATUS_NP), "TIME_STATUS_NP");
    EXPECT_STREQ(Message::mng2str_c(GRANDMASTER_SETTINGS_NP),
        "GRANDMASTER_SETTINGS_NP");
    EXPECT_STREQ(Message::mng2str_c(PORT_DATA_SET_NP), "PORT_DATA_SET_NP");
    EXPECT_STREQ(Message::mng2str_c(SUBSCRIBE_EVENTS_NP), "SUBSCRIBE_EVENTS_NP");
    EXPECT_STREQ(Message::mng2str_c(PORT_PROPERTIES_NP), "PORT_PROPERTIES_NP");
    EXPECT_STREQ(Message::mng2str_c(PORT_STATS_NP), "PORT_STATS_NP");
    EXPECT_STREQ(Message::mng2str_c(SYNCHRONIZATION_UNCERTAIN_NP),
        "SYNCHRONIZATION_UNCERTAIN_NP");
    EXPECT_STREQ(Message::mng2str_c(PORT_SERVICE_STATS_NP),
        "PORT_SERVICE_STATS_NP");
    EXPECT_STREQ(Message::mng2str_c(UNICAST_MASTER_TABLE_NP),
        "UNICAST_MASTER_TABLE_NP");
    EXPECT_STREQ(Message::mng2str_c(PORT_HWCLOCK_NP), "PORT_HWCLOCK_NP");
    EXPECT_STREQ(Message::mng2str_c(POWER_PROFILE_SETTINGS_NP),
        "POWER_PROFILE_SETTINGS_NP");
    EXPECT_STREQ(Message::mng2str_c(LAST_MNG_ID), "out of range");
    // Confirm compatability
    EXPECT_EQ(FIRST_MNG_ID, NULL_PTP_MANAGEMENT);
    EXPECT_EQ(NULL_MANAGEMENT, NULL_PTP_MANAGEMENT);
}

// tests convert string to management id
// static const bool findMngID(const std::string &str, mng_vals_e &id,
//     bool exact = true)
TEST(MessageTest, MethodFindMngID)
{
    mng_vals_e m;
    EXPECT_TRUE(Message::findMngID("FAULT_LOG", m, true));
    EXPECT_EQ(m, FAULT_LOG);
    EXPECT_TRUE(Message::findMngID("Null", m, false));
    EXPECT_EQ(m, NULL_PTP_MANAGEMENT);
    EXPECT_TRUE(Message::findMngID("UTC_PROPERTIES", m));
    EXPECT_EQ(m, UTC_PROPERTIES);
}

// tests convert management error to string
// static const char *errId2str_c(managementErrorId_e err)
TEST(MessageTest, MethodErrId2str)
{
    EXPECT_STREQ(Message::errId2str_c(RESPONSE_TOO_BIG), "RESPONSE_TOO_BIG");
    EXPECT_STREQ(Message::errId2str_c(NO_SUCH_ID), "NO_SUCH_ID");
    EXPECT_STREQ(Message::errId2str_c(WRONG_LENGTH), "WRONG_LENGTH");
    EXPECT_STREQ(Message::errId2str_c(WRONG_VALUE), "WRONG_VALUE");
    EXPECT_STREQ(Message::errId2str_c(NOT_SETABLE), "NOT_SETABLE");
    EXPECT_STREQ(Message::errId2str_c(NOT_SUPPORTED), "NOT_SUPPORTED");
    EXPECT_STREQ(Message::errId2str_c(GENERAL_ERROR), "GENERAL_ERROR");
}

// tests convert clock type to string
// static const char *clkType2str_c(clockType_e type)
TEST(MessageTest, MethodClkType2str)
{
    EXPECT_STREQ(Message::clkType2str_c(ordinaryClock), "ordinaryClock");
    EXPECT_STREQ(Message::clkType2str_c(boundaryClock), "boundaryClock");
    EXPECT_STREQ(Message::clkType2str_c(p2pTransparentClock),
        "p2pTransparentClock");
    EXPECT_STREQ(Message::clkType2str_c(e2eTransparentClock),
        "e2eTransparentClock");
    EXPECT_STREQ(Message::clkType2str_c(managementClock), "managementClock");
}

// tests convert network protocol to string
// static const char *netProt2str_c(networkProtocol_e protocol)
TEST(MessageTest, MethodNetProt2str)
{
    EXPECT_STREQ(Message::netProt2str_c(UDP_IPv4), "UDP_IPv4");
    EXPECT_STREQ(Message::netProt2str_c(UDP_IPv6), "UDP_IPv6");
    EXPECT_STREQ(Message::netProt2str_c(IEEE_802_3), "IEEE_802_3");
    EXPECT_STREQ(Message::netProt2str_c(DeviceNet), "DeviceNet");
    EXPECT_STREQ(Message::netProt2str_c(ControlNet), "ControlNet");
    EXPECT_STREQ(Message::netProt2str_c(PROFINET), "PROFINET");
}

// tests convert clock accuracy to string
// static const char *clockAcc2str_c(clockAccuracy_e value)
TEST(MessageTest, MethodClockAcc2str)
{
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_1ps), "within_1ps");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_2_5ps), "within_2_5ps");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_10ps), "within_10ps");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_25ps), "within_25ps");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_100ps), "within_100ps");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_250ps), "within_250ps");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_1ns), "within_1ns");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_2_5ns), "within_2_5ns");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_10ns), "within_10ns");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_25ns), "within_25ns");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_100ns), "within_100ns");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_250ns), "within_250ns");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_1us), "within_1us");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_2_5us), "within_2_5us");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_10us), "within_10us");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_25us), "within_25us");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_100us), "within_100us");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_250us), "within_250us");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_1ms), "within_1ms");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_2_5ms), "within_2_5ms");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_10ms), "within_10ms");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_25ms), "within_25ms");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_100ms), "within_100ms");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_250ms), "within_250ms");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_1s), "within_1s");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_within_10s), "within_10s");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_more_10s), "more_10s");
    EXPECT_STREQ(Message::clockAcc2str_c(Accurate_Unknown), "Unknown");
}

// tests convert fault record severity code to string
// static const char *faultRec2str_c(faultRecord_e code)
TEST(MessageTest, MethodFaultRec2str)
{
    EXPECT_STREQ(Message::faultRec2str_c(F_Emergency), "Emergency");
    EXPECT_STREQ(Message::faultRec2str_c(F_Alert), "Alert");
    EXPECT_STREQ(Message::faultRec2str_c(F_Critical), "Critical");
    EXPECT_STREQ(Message::faultRec2str_c(F_Error), "Error");
    EXPECT_STREQ(Message::faultRec2str_c(F_Warning), "Warning");
    EXPECT_STREQ(Message::faultRec2str_c(F_Notice), "Notice");
    EXPECT_STREQ(Message::faultRec2str_c(F_Informational), "Informational");
    EXPECT_STREQ(Message::faultRec2str_c(F_Debug), "Debug");
}

// tests convert time source to string
// static const char *timeSrc2str_c(timeSource_e type)
TEST(MessageTest, MethodTimeSrc2str)
{
    EXPECT_STREQ(Message::timeSrc2str_c(ATOMIC_CLOCK), "ATOMIC_CLOCK");
    EXPECT_STREQ(Message::timeSrc2str_c(GNSS), "GNSS");
    EXPECT_STREQ(Message::timeSrc2str_c(GPS), "GNSS");
    EXPECT_STREQ(Message::timeSrc2str_c(TERRESTRIAL_RADIO), "TERRESTRIAL_RADIO");
    EXPECT_STREQ(Message::timeSrc2str_c(SERIAL_TIME_CODE), "SERIAL_TIME_CODE");
    EXPECT_STREQ(Message::timeSrc2str_c(PTP), "PTP");
    EXPECT_STREQ(Message::timeSrc2str_c(NTP), "NTP");
    EXPECT_STREQ(Message::timeSrc2str_c(HAND_SET), "HAND_SET");
    EXPECT_STREQ(Message::timeSrc2str_c(OTHER), "OTHER");
    EXPECT_STREQ(Message::timeSrc2str_c(INTERNAL_OSCILLATOR),
        "INTERNAL_OSCILLATOR");
}

// tests convert string to time source type
// static const bool findTimeSrc(const std::string &str, timeSource_e &type,
//     bool exact = true)
TEST(MessageTest, MethodFindTimeSrc)
{
    timeSource_e t;
    EXPECT_TRUE(Message::findTimeSrc("ATOMIC_CLOCK", t, true));
    EXPECT_EQ(t, ATOMIC_CLOCK);
    EXPECT_TRUE(Message::findTimeSrc("Atomic", t, false));
    EXPECT_EQ(t, ATOMIC_CLOCK);
    EXPECT_TRUE(Message::findTimeSrc("GNSS", t, true));
    EXPECT_EQ(t, GNSS);
    EXPECT_TRUE(Message::findTimeSrc("Gnss", t, false));
    EXPECT_EQ(t, GNSS);
    EXPECT_TRUE(Message::findTimeSrc("GPS", t, true));
    EXPECT_EQ(t, GPS);
    EXPECT_TRUE(Message::findTimeSrc("Gps", t, false));
    EXPECT_EQ(t, GPS);
    EXPECT_TRUE(Message::findTimeSrc("TERRESTRIAL_RADIO", t, true));
    EXPECT_EQ(t, TERRESTRIAL_RADIO);
    EXPECT_TRUE(Message::findTimeSrc("Terrestrial_Radio", t, false));
    EXPECT_EQ(t, TERRESTRIAL_RADIO);
    EXPECT_TRUE(Message::findTimeSrc("Radio", t, false));
    EXPECT_EQ(t, TERRESTRIAL_RADIO);
    EXPECT_TRUE(Message::findTimeSrc("Terres", t, false));
    EXPECT_EQ(t, TERRESTRIAL_RADIO);
    EXPECT_TRUE(Message::findTimeSrc("SERIAL_TIME_CODE", t, true));
    EXPECT_EQ(t, SERIAL_TIME_CODE);
    EXPECT_TRUE(Message::findTimeSrc("Serial_Time_Code", t, false));
    EXPECT_EQ(t, SERIAL_TIME_CODE);
    EXPECT_TRUE(Message::findTimeSrc("Serial", t, false));
    EXPECT_EQ(t, SERIAL_TIME_CODE);
    EXPECT_TRUE(Message::findTimeSrc("PTP", t, true));
    EXPECT_EQ(t, PTP);
    EXPECT_TRUE(Message::findTimeSrc("Ptp", t, false));
    EXPECT_EQ(t, PTP);
    EXPECT_TRUE(Message::findTimeSrc("NTP", t, true));
    EXPECT_EQ(t, NTP);
    EXPECT_TRUE(Message::findTimeSrc("Ntp", t, false));
    EXPECT_EQ(t, NTP);
    EXPECT_TRUE(Message::findTimeSrc("HAND_SET", t, true));
    EXPECT_EQ(t, HAND_SET);
    EXPECT_TRUE(Message::findTimeSrc("Hand_set", t, false));
    EXPECT_EQ(t, HAND_SET);
    EXPECT_TRUE(Message::findTimeSrc("Hand", t, false));
    EXPECT_EQ(t, HAND_SET);
    EXPECT_TRUE(Message::findTimeSrc("OTHER", t, true));
    EXPECT_EQ(t, OTHER);
    EXPECT_TRUE(Message::findTimeSrc("Other", t, false));
    EXPECT_EQ(t, OTHER);
    EXPECT_TRUE(Message::findTimeSrc("INTERNAL_OSCILLATOR", t, true));
    EXPECT_EQ(t, INTERNAL_OSCILLATOR);
    EXPECT_TRUE(Message::findTimeSrc("Internal_oscillator", t, false));
    EXPECT_EQ(t, INTERNAL_OSCILLATOR);
    EXPECT_TRUE(Message::findTimeSrc("oscil", t, false));
    EXPECT_EQ(t, INTERNAL_OSCILLATOR);
    EXPECT_TRUE(Message::findTimeSrc("Intern", t, false));
    EXPECT_EQ(t, INTERNAL_OSCILLATOR);
    // Confirm compatability
    EXPECT_EQ(GPS, GNSS);
}

// tests convert port state to string
// static const char *portState2str_c(portState_e state)
TEST(MessageTest, MethodPortState2str)
{
    EXPECT_STREQ(Message::portState2str_c(INITIALIZING), "INITIALIZING");
    EXPECT_STREQ(Message::portState2str_c(FAULTY), "FAULTY");
    EXPECT_STREQ(Message::portState2str_c(DISABLED), "DISABLED");
    EXPECT_STREQ(Message::portState2str_c(LISTENING), "LISTENING");
    EXPECT_STREQ(Message::portState2str_c(PRE_MASTER), "PRE_TIME_TRANSMITTER");
    EXPECT_STREQ(Message::portState2str_c(PRE_TIME_TRANSMITTER),
        "PRE_TIME_TRANSMITTER");
    EXPECT_STREQ(Message::portState2str_c(MASTER), "TIME_TRANSMITTER");
    EXPECT_STREQ(Message::portState2str_c(TIME_TRANSMITTER), "TIME_TRANSMITTER");
    EXPECT_STREQ(Message::portState2str_c(PASSIVE), "PASSIVE");
    EXPECT_STREQ(Message::portState2str_c(UNCALIBRATED), "UNCALIBRATED");
    EXPECT_STREQ(Message::portState2str_c(SLAVE), "TIME_RECEIVER");
    EXPECT_STREQ(Message::portState2str_c(TIME_RECEIVER), "TIME_RECEIVER");
}

// tests convert string to port state
// static const bool findPortState(const std::string &str, portState_e &state,
//     bool caseSens = true)
TEST(MessageTest, MethodFindPortState)
{
    portState_e s;
    EXPECT_TRUE(Message::findPortState("INITIALIZING", s, true));
    EXPECT_EQ(s, INITIALIZING);
    EXPECT_TRUE(Message::findPortState("Initializing", s, false));
    EXPECT_EQ(s, INITIALIZING);
    EXPECT_TRUE(Message::findPortState("FAULTY", s, true));
    EXPECT_EQ(s, FAULTY);
    EXPECT_TRUE(Message::findPortState("Faulty", s, false));
    EXPECT_EQ(s, FAULTY);
    EXPECT_TRUE(Message::findPortState("DISABLED", s, true));
    EXPECT_EQ(s, DISABLED);
    EXPECT_TRUE(Message::findPortState("Disabled", s, false));
    EXPECT_EQ(s, DISABLED);
    EXPECT_TRUE(Message::findPortState("LISTENING", s, true));
    EXPECT_EQ(s, LISTENING);
    EXPECT_TRUE(Message::findPortState("Listening", s, false));
    EXPECT_EQ(s, LISTENING);
    EXPECT_TRUE(Message::findPortState("PRE_MASTER", s, true));
    EXPECT_EQ(s, PRE_MASTER);
    EXPECT_TRUE(Message::findPortState("Pre_Master", s, false));
    EXPECT_EQ(s, PRE_MASTER);
    EXPECT_TRUE(Message::findPortState("PRE_TIME_TRANSMITTER", s, true));
    EXPECT_EQ(s, PRE_TIME_TRANSMITTER);
    EXPECT_TRUE(Message::findPortState("Pre_Time_Transmitter", s, false));
    EXPECT_EQ(s, PRE_TIME_TRANSMITTER);
    EXPECT_TRUE(Message::findPortState("MASTER", s, true));
    EXPECT_EQ(s, MASTER);
    EXPECT_TRUE(Message::findPortState("Master", s, false));
    EXPECT_EQ(s, MASTER);
    EXPECT_TRUE(Message::findPortState("TIME_TRANSMITTER", s, true));
    EXPECT_EQ(s, TIME_TRANSMITTER);
    EXPECT_TRUE(Message::findPortState("Time_Transmitter", s, false));
    EXPECT_EQ(s, TIME_TRANSMITTER);
    EXPECT_TRUE(Message::findPortState("PASSIVE", s, true));
    EXPECT_EQ(s, PASSIVE);
    EXPECT_TRUE(Message::findPortState("Passive", s, false));
    EXPECT_EQ(s, PASSIVE);
    EXPECT_TRUE(Message::findPortState("UNCALIBRATED", s, true));
    EXPECT_EQ(s, UNCALIBRATED);
    EXPECT_TRUE(Message::findPortState("Uncalibrated", s, false));
    EXPECT_EQ(s, UNCALIBRATED);
    EXPECT_TRUE(Message::findPortState("SLAVE", s, true));
    EXPECT_EQ(s, SLAVE);
    EXPECT_TRUE(Message::findPortState("Slave", s, false));
    EXPECT_EQ(s, SLAVE);
    EXPECT_TRUE(Message::findPortState("TIME_RECEIVER", s, true));
    EXPECT_EQ(s, TIME_RECEIVER);
    EXPECT_TRUE(Message::findPortState("Time_Receiver", s, false));
    EXPECT_EQ(s, TIME_RECEIVER);
    // Confirm compatability
    EXPECT_EQ(PRE_MASTER, PRE_TIME_TRANSMITTER);
    EXPECT_EQ(MASTER, TIME_TRANSMITTER);
    EXPECT_EQ(SLAVE, TIME_RECEIVER);
}

// tests convert delay mechanism to string
// static const char *delayMech2str_c(delayMechanism_e type)
TEST(MessageTest, MethodDelayMech2str)
{
    EXPECT_STREQ(Message::delayMech2str_c(AUTO), "AUTO");
    EXPECT_STREQ(Message::delayMech2str_c(E2E), "E2E");
    EXPECT_STREQ(Message::delayMech2str_c(P2P), "P2P");
    EXPECT_STREQ(Message::delayMech2str_c(NO_MECHANISM), "NO_MECHANISM");
    EXPECT_STREQ(Message::delayMech2str_c(COMMON_P2P), "COMMON_P2P");
    EXPECT_STREQ(Message::delayMech2str_c(SPECIAL), "SPECIAL");
}

// tests convert string to delay mechanism
// static const bool findDelayMech(const std::string &str, delayMechanism_e &type,
//     bool exact = true);
TEST(MessageTest, MethodFindDelayMech)
{
    delayMechanism_e t;
    EXPECT_TRUE(Message::findDelayMech("AUTO", t, true));
    EXPECT_EQ(t, AUTO);
    EXPECT_TRUE(Message::findDelayMech("Auto", t, false));
    EXPECT_EQ(t, AUTO);
    EXPECT_TRUE(Message::findDelayMech("E2E", t, true));
    EXPECT_EQ(t, E2E);
    EXPECT_TRUE(Message::findDelayMech("e2e", t, false));
    EXPECT_EQ(t, E2E);
    EXPECT_TRUE(Message::findDelayMech("P2P", t, true));
    EXPECT_EQ(t, P2P);
    EXPECT_TRUE(Message::findDelayMech("p2p", t, false));
    EXPECT_EQ(t, P2P);
    EXPECT_TRUE(Message::findDelayMech("NO_MECHANISM", t, true));
    EXPECT_EQ(t, NO_MECHANISM);
    EXPECT_TRUE(Message::findDelayMech("No_mechanism", t, false));
    EXPECT_EQ(t, NO_MECHANISM);
    EXPECT_TRUE(Message::findDelayMech("COMMON_P2P", t, true));
    EXPECT_EQ(t, COMMON_P2P);
    EXPECT_TRUE(Message::findDelayMech("Common_p2p", t, false));
    EXPECT_EQ(t, COMMON_P2P);
    EXPECT_TRUE(Message::findDelayMech("SPECIAL", t, true));
    EXPECT_EQ(t, SPECIAL);
    EXPECT_TRUE(Message::findDelayMech("Special", t, false));
    EXPECT_EQ(t, SPECIAL);
}

// tests convert linuxptp time stamp type to string
// static const char *ts2str_c(linuxptpTimeStamp_e type)
TEST(MessageTest, MethodTs2str)
{
    EXPECT_STREQ(Message::ts2str_c(TS_SOFTWARE), "SOFTWARE");
    EXPECT_STREQ(Message::ts2str_c(TS_HARDWARE), "HARDWARE");
    EXPECT_STREQ(Message::ts2str_c(TS_LEGACY_HW), "LEGACY_HW");
    EXPECT_STREQ(Message::ts2str_c(TS_ONESTEP), "ONESTEP");
    EXPECT_STREQ(Message::ts2str_c(TS_P2P1STEP), "P2P1STEP");
}

// tests convert linuxptp power profile version to string
// static const char *pwr2str_c(linuxptpPowerProfileVersion_e ver)
TEST(MessageTest, MethodPwr2str)
{
    EXPECT_STREQ(Message::pwr2str_c(IEEE_C37_238_VERSION_NONE), "NONE");
    EXPECT_STREQ(Message::pwr2str_c(IEEE_C37_238_VERSION_2011), "2011");
    EXPECT_STREQ(Message::pwr2str_c(IEEE_C37_238_VERSION_2017), "2017");
}

// tests convert linuxptp master unicasy state to string
// static const char *us2str_c(linuxptpUnicastState_e state)
TEST(MessageTest, MethodUs2str)
{
    EXPECT_STREQ(Message::us2str_c(UC_WAIT), "WAIT");
    EXPECT_STREQ(Message::us2str_c(UC_HAVE_ANN), "HAVE_ANN");
    EXPECT_STREQ(Message::us2str_c(UC_NEED_SYDY), "NEED_SYDY");
    EXPECT_STREQ(Message::us2str_c(UC_HAVE_SYDY), "HAVE_SYDY");
}

// Test if leap 61 seconds flag is enabled method
// static bool is_LI_61(uint8_t flags)
TEST(MessageTest, MethodIs_LI_61)
{
    EXPECT_TRUE(Message::is_LI_61(F_LI_61));
    EXPECT_TRUE(Message::is_LI_61(0xff));
    EXPECT_FALSE(Message::is_LI_61(0));
}

// Test if leap 59 seconds flag is enabled
// static bool is_LI_59(uint8_t flags)
TEST(MessageTest, MethodIs_LI_59)
{
    EXPECT_TRUE(Message::is_LI_59(F_LI_59));
    EXPECT_TRUE(Message::is_LI_59(0xff));
    EXPECT_FALSE(Message::is_LI_59(0));
}

// Test if UTC offset is valid flag is enabled
// static bool is_UTCV(uint8_t flags)
TEST(MessageTest, MethodIs_UTCV)
{
    EXPECT_TRUE(Message::is_UTCV(F_UTCV));
    EXPECT_TRUE(Message::is_UTCV(0xff));
    EXPECT_FALSE(Message::is_UTCV(0));
}

// Test if is PTP instance flag is enabled
// static bool is_PTP(uint8_t flags)
TEST(MessageTest, MethodIs_PTP)
{
    EXPECT_TRUE(Message::is_PTP(F_PTP));
    EXPECT_TRUE(Message::is_PTP(0xff));
    EXPECT_FALSE(Message::is_PTP(0));
}

// Test if timescale is traceable flag is enabled
// static bool is_TTRA(uint8_t flags)
TEST(MessageTest, MethodIs_TTRA)
{
    EXPECT_TRUE(Message::is_TTRA(F_TTRA));
    EXPECT_TRUE(Message::is_TTRA(0xff));
    EXPECT_FALSE(Message::is_TTRA(0));
}

// Test if frequency is traceable flag is enabled
// static bool is_FTRA(uint8_t flags)
TEST(MessageTest, MethodIs_FTRA)
{
    EXPECT_TRUE(Message::is_FTRA(F_FTRA));
    EXPECT_TRUE(Message::is_FTRA(0xff));
    EXPECT_FALSE(Message::is_FTRA(0));
}

// test if management TLV id uses empty dataField
// static bool isEmpty(mng_vals_e id)
TEST(MessageTest, MethodIsEmpty)
{
    EXPECT_TRUE(Message::isEmpty(NULL_PTP_MANAGEMENT));
    EXPECT_FALSE(Message::isEmpty(CLOCK_DESCRIPTION));
    EXPECT_FALSE(Message::isEmpty(USER_DESCRIPTION));
    EXPECT_TRUE(Message::isEmpty(SAVE_IN_NON_VOLATILE_STORAGE));
    EXPECT_TRUE(Message::isEmpty(RESET_NON_VOLATILE_STORAGE));
    EXPECT_FALSE(Message::isEmpty(INITIALIZE));
    EXPECT_FALSE(Message::isEmpty(FAULT_LOG));
    EXPECT_TRUE(Message::isEmpty(FAULT_LOG_RESET));
    EXPECT_FALSE(Message::isEmpty(DEFAULT_DATA_SET));
    EXPECT_FALSE(Message::isEmpty(CURRENT_DATA_SET));
    EXPECT_FALSE(Message::isEmpty(PARENT_DATA_SET));
    EXPECT_FALSE(Message::isEmpty(TIME_PROPERTIES_DATA_SET));
    EXPECT_FALSE(Message::isEmpty(PORT_DATA_SET));
    EXPECT_FALSE(Message::isEmpty(PRIORITY1));
    EXPECT_FALSE(Message::isEmpty(PRIORITY2));
    EXPECT_FALSE(Message::isEmpty(DOMAIN));
    EXPECT_FALSE(Message::isEmpty(SLAVE_ONLY));
    EXPECT_FALSE(Message::isEmpty(LOG_ANNOUNCE_INTERVAL));
    EXPECT_FALSE(Message::isEmpty(ANNOUNCE_RECEIPT_TIMEOUT));
    EXPECT_FALSE(Message::isEmpty(LOG_SYNC_INTERVAL));
    EXPECT_FALSE(Message::isEmpty(VERSION_NUMBER));
    EXPECT_TRUE(Message::isEmpty(ENABLE_PORT));
    EXPECT_TRUE(Message::isEmpty(DISABLE_PORT));
    EXPECT_FALSE(Message::isEmpty(TIME));
    EXPECT_FALSE(Message::isEmpty(CLOCK_ACCURACY));
    EXPECT_FALSE(Message::isEmpty(UTC_PROPERTIES));
    EXPECT_FALSE(Message::isEmpty(TRACEABILITY_PROPERTIES));
    EXPECT_FALSE(Message::isEmpty(TIMESCALE_PROPERTIES));
    EXPECT_FALSE(Message::isEmpty(UNICAST_NEGOTIATION_ENABLE));
    EXPECT_FALSE(Message::isEmpty(PATH_TRACE_LIST));
    EXPECT_FALSE(Message::isEmpty(PATH_TRACE_ENABLE));
    EXPECT_FALSE(Message::isEmpty(GRANDMASTER_CLUSTER_TABLE));
    EXPECT_FALSE(Message::isEmpty(UNICAST_MASTER_TABLE));
    EXPECT_FALSE(Message::isEmpty(UNICAST_MASTER_MAX_TABLE_SIZE));
    EXPECT_FALSE(Message::isEmpty(ACCEPTABLE_MASTER_TABLE));
    EXPECT_FALSE(Message::isEmpty(ACCEPTABLE_MASTER_TABLE_ENABLED));
    EXPECT_FALSE(Message::isEmpty(ACCEPTABLE_MASTER_MAX_TABLE_SIZE));
    EXPECT_FALSE(Message::isEmpty(ALTERNATE_MASTER));
    EXPECT_FALSE(Message::isEmpty(ALTERNATE_TIME_OFFSET_ENABLE));
    EXPECT_FALSE(Message::isEmpty(ALTERNATE_TIME_OFFSET_NAME));
    EXPECT_FALSE(Message::isEmpty(ALTERNATE_TIME_OFFSET_MAX_KEY));
    EXPECT_FALSE(Message::isEmpty(ALTERNATE_TIME_OFFSET_PROPERTIES));
    EXPECT_FALSE(Message::isEmpty(TRANSPARENT_CLOCK_PORT_DATA_SET));
    EXPECT_FALSE(Message::isEmpty(LOG_MIN_PDELAY_REQ_INTERVAL));
    EXPECT_FALSE(Message::isEmpty(TRANSPARENT_CLOCK_DEFAULT_DATA_SET));
    EXPECT_FALSE(Message::isEmpty(PRIMARY_DOMAIN));
    EXPECT_FALSE(Message::isEmpty(DELAY_MECHANISM));
    EXPECT_FALSE(Message::isEmpty(EXTERNAL_PORT_CONFIGURATION_ENABLED));
    EXPECT_FALSE(Message::isEmpty(MASTER_ONLY));
    EXPECT_FALSE(Message::isEmpty(HOLDOVER_UPGRADE_ENABLE));
    EXPECT_FALSE(Message::isEmpty(EXT_PORT_CONFIG_PORT_DATA_SET));
    EXPECT_FALSE(Message::isEmpty(TIME_STATUS_NP));
    EXPECT_FALSE(Message::isEmpty(GRANDMASTER_SETTINGS_NP));
    EXPECT_FALSE(Message::isEmpty(PORT_DATA_SET_NP));
    EXPECT_FALSE(Message::isEmpty(SUBSCRIBE_EVENTS_NP));
    EXPECT_FALSE(Message::isEmpty(PORT_PROPERTIES_NP));
    EXPECT_FALSE(Message::isEmpty(PORT_STATS_NP));
    EXPECT_FALSE(Message::isEmpty(SYNCHRONIZATION_UNCERTAIN_NP));
    EXPECT_FALSE(Message::isEmpty(PORT_SERVICE_STATS_NP));
    EXPECT_FALSE(Message::isEmpty(UNICAST_MASTER_TABLE_NP));
    EXPECT_FALSE(Message::isEmpty(PORT_HWCLOCK_NP));
    EXPECT_FALSE(Message::isEmpty(POWER_PROFILE_SETTINGS_NP));
}

// Test if management TLV is valid for use method
// bool isValidId(mng_vals_e id)
TEST(MessageTest, MethodIsValidId)
{
    Message m;
    EXPECT_TRUE(m.isValidId(PRIORITY1));
    EXPECT_TRUE(m.isValidId(PRIORITY2));
    EXPECT_TRUE(m.isValidId(DOMAIN));
    EXPECT_TRUE(m.isValidId(SLAVE_ONLY));
    EXPECT_TRUE(m.isValidId(VERSION_NUMBER));
    EXPECT_TRUE(m.isValidId(ENABLE_PORT));
    EXPECT_TRUE(m.isValidId(DISABLE_PORT));
    EXPECT_TRUE(m.isValidId(TIME));
    EXPECT_TRUE(m.isValidId(CLOCK_ACCURACY));
    EXPECT_TRUE(m.isValidId(UTC_PROPERTIES));
    EXPECT_TRUE(m.isValidId(TIME_STATUS_NP));
    EXPECT_TRUE(m.isValidId(PORT_DATA_SET_NP));
    EXPECT_TRUE(m.isValidId(PORT_PROPERTIES_NP));
    EXPECT_TRUE(m.isValidId(PORT_STATS_NP));
    MsgParams p = m.getParams();
    p.implementSpecific = noImplementSpecific;
    EXPECT_TRUE(m.updateParams(p));
    EXPECT_FALSE(m.isValidId(TIME_STATUS_NP));
    EXPECT_FALSE(m.isValidId(PORT_DATA_SET_NP));
    EXPECT_FALSE(m.isValidId(PORT_PROPERTIES_NP));
    EXPECT_FALSE(m.isValidId(PORT_STATS_NP));
}

// Test set management TLV to build
// bool setAction(actionField_e actionField, mng_vals_e tlv_id,
//     const BaseMngTlv *dataSend = nullptr)
TEST(MessageTest, MethodSetAction)
{
    Message m;
    EXPECT_TRUE(m.setAction(GET, PRIORITY1));
    EXPECT_EQ(m.getSendAction(), GET);
    PRIORITY1_t p;
    p.priority1 = 0x7f;
    EXPECT_TRUE(m.setAction(SET, PRIORITY1, &p));
    EXPECT_EQ(m.getSendAction(), SET);
    m.clearData();
}

// Test clear Data
// void clearData()
TEST(MessageTest, MethodClearData)
{
    Message m;
    PRIORITY1_t p;
    p.priority1 = 0x7f;
    EXPECT_TRUE(m.setAction(SET, PRIORITY1, &p));
    EXPECT_EQ(m.getSendAction(), SET);
    m.clearData();
    EXPECT_EQ(m.getSendAction(), GET);
}

// Test build from memory buffer
// MNG_PARSE_ERROR_e build(void *buf, size_t bufSize, uint16_t sequence)
TEST(MessageTest, MethodBuildVoid)
{
    Message m;
    PRIORITY1_t p;
    p.priority1 = 0x7f;
    EXPECT_TRUE(m.setAction(SET, PRIORITY1, &p));
    EXPECT_EQ(m.getSendAction(), SET);
    ssize_t plen = m.getMsgPlanedLen();
    EXPECT_EQ(plen, 56);
    uint8_t buf[70];
    EXPECT_EQ(m.build(buf, sizeof buf, 137), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(m.getMsgLen(), plen);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    EXPECT_EQ(m.parse(buf, plen), MNG_PARSE_ERROR_OK);
}

// Test build using buffer object
// MNG_PARSE_ERROR_e build(Buf &buf, uint16_t sequence)
TEST(MessageTest, MethodBuildObj)
{
    Message m;
    PRIORITY1_t p;
    p.priority1 = 0x7f;
    EXPECT_TRUE(m.setAction(SET, PRIORITY1, &p));
    EXPECT_EQ(m.getSendAction(), SET);
    ssize_t plen = m.getMsgPlanedLen();
    EXPECT_EQ(plen, 56);
    Buf buf;
    EXPECT_TRUE(buf.alloc(70));
    EXPECT_EQ(m.build(buf, 137), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(m.getMsgLen(), plen);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    ((uint8_t *)buf())[46] = RESPONSE;
    EXPECT_EQ(m.parse(buf, plen), MNG_PARSE_ERROR_OK);
}

// Test clear Data
// actionField_e getSendAction() const
TEST(MessageTest, MethodGetSendAction)
{
    Message m;
    EXPECT_EQ(m.getSendAction(), GET);
    PRIORITY1_t p;
    p.priority1 = 0x7f;
    EXPECT_TRUE(m.setAction(SET, PRIORITY1, &p));
    EXPECT_EQ(m.getSendAction(), SET);
}

// Test get build message length
// size_t getMsgLen() const
TEST(MessageTest, MethodGetMsgLen)
{
    Message m;
    PRIORITY1_t p;
    p.priority1 = 0x7f;
    EXPECT_TRUE(m.setAction(SET, PRIORITY1, &p));
    EXPECT_EQ(m.getSendAction(), SET);
    uint8_t buf[70];
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(m.getMsgLen(), 56);
}

// Test build using buffer object
// ssize_t getMsgPlanedLen() const
TEST(MessageTest, MethodGetMsgPlanedLen)
{
    Message m;
    PRIORITY1_t p;
    p.priority1 = 1;
    EXPECT_TRUE(m.setAction(SET, PRIORITY1, &p));
    EXPECT_EQ(m.getSendAction(), SET);
    EXPECT_EQ(m.getMsgPlanedLen(), 56);
}

// Test parse from memory buffer
// MNG_PARSE_ERROR_e parse(const void *buf, ssize_t msgSize)
TEST(MessageTest, MethodParseVoid)
{
    Message m;
    PRIORITY1_t p;
    p.priority1 = 0x7f;
    EXPECT_TRUE(m.setAction(SET, PRIORITY1, &p));
    EXPECT_EQ(m.getSendAction(), SET);
    ssize_t plen = m.getMsgPlanedLen();
    uint8_t buf[70];
    EXPECT_EQ(m.build(buf, sizeof buf, 137), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    EXPECT_EQ(m.parse(buf, plen), MNG_PARSE_ERROR_OK);
}

// Test parse using buffer object
// MNG_PARSE_ERROR_e parse(const Buf &buf, ssize_t msgSize)
TEST(MessageTest, MethodParseObj)
{
    Message m;
    PRIORITY1_t p;
    p.priority1 = 1;
    EXPECT_TRUE(m.setAction(SET, PRIORITY1, &p));
    Buf buf;
    EXPECT_TRUE(buf.alloc(70));
    EXPECT_EQ(m.build(buf, 1), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    ((uint8_t *)buf())[46] = RESPONSE;
    EXPECT_EQ(m.parse(buf, 56), MNG_PARSE_ERROR_OK);
}

// Test parse using buffer object
// actionField_e getReplyAction() const
TEST(MessageTest, MethodGetReplyAction)
{
    Message m;
    PRIORITY1_t p;
    p.priority1 = 1;
    EXPECT_TRUE(m.setAction(SET, PRIORITY1, &p));
    uint8_t buf[70];
    EXPECT_EQ(m.build(buf, sizeof buf, 137), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    EXPECT_EQ(m.parse(buf, 56), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(m.getReplyAction(), RESPONSE);
}

// Test if message is unicast
// bool isUnicast() const
TEST(MessageTest, MethodIsUnicast)
{
    Message m;
    MsgParams p = m.getParams();
    EXPECT_TRUE(m.setAction(GET, PRIORITY1));
    uint8_t buf[70];
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    EXPECT_EQ(m.parse(buf, 54), MNG_PARSE_ERROR_OK);
    EXPECT_TRUE(m.isUnicast());
    p.isUnicast = false;
    EXPECT_TRUE(m.updateParams(p));
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    EXPECT_EQ(m.parse(buf, 54), MNG_PARSE_ERROR_OK);
    EXPECT_FALSE(m.isUnicast());
}

// Test get PTP Profile Specific
// uint8_t getPTPProfileSpecific() const
TEST(MessageTest, MethodGetPTPProfileSpecific)
{
    Message m;
    EXPECT_TRUE(m.setAction(GET, PRIORITY1));
    uint8_t buf[60];
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    EXPECT_EQ(m.parse(buf, 54), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(m.getPTPProfileSpecific(), 0);
}

// Test get message sequence
// uint16_t getSequence() const
TEST(MessageTest, MethodGetSequence)
{
    Message m;
    EXPECT_TRUE(m.setAction(GET, PRIORITY1));
    uint8_t buf[60];
    EXPECT_EQ(m.build(buf, sizeof buf, 0x4231), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    EXPECT_EQ(m.parse(buf, 54), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(m.getSequence(), 0x4231);
}

// Test get peer port address of parsed message
// const PortIdentity_t &getPeer() const
TEST(MessageTest, MethodGetPeer)
{
    Message m;
    MsgParams p = m.getParams();
    EXPECT_TRUE(m.setAction(GET, PRIORITY1));
    PortIdentity_t t = { { 1, 2, 3, 4, 5, 6, 7, 8}, 0x3184 };
    p.self_id = t;
    EXPECT_TRUE(m.updateParams(p));
    uint8_t buf[60];
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    EXPECT_EQ(m.parse(buf, 54), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(m.getPeer(), t);
}

// Test get target port address of parsed message
// const PortIdentity_t &getTarget() const
TEST(MessageTest, MethodGetTarget)
{
    Message m;
    MsgParams p = m.getParams();
    EXPECT_TRUE(m.setAction(GET, PRIORITY1));
    PortIdentity_t t = { { 1, 2, 3, 4, 5, 6, 7, 8}, 0x3184 };
    p.target = t;
    EXPECT_TRUE(m.updateParams(p));
    uint8_t buf[60];
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    EXPECT_EQ(m.parse(buf, 54), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(m.getTarget(), t);
}

// Test get transport specific ID of parsed message
// uint32_t getSdoId() const
TEST(MessageTest, MethodGetSdoId)
{
    Message m;
    MsgParams p = m.getParams();
    EXPECT_TRUE(m.setAction(GET, PRIORITY1));
    p.transportSpecific = 14;
    EXPECT_TRUE(m.updateParams(p));
    uint8_t buf[60];
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    EXPECT_EQ(m.parse(buf, 54), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(m.getSdoId(), 14 * 0x100);
}

// Test get domain number of parsed message
// uint8_t getDomainNumber() const
TEST(MessageTest, MethodGetDomainNumber)
{
    Message m;
    MsgParams p = m.getParams();
    EXPECT_TRUE(m.setAction(GET, PRIORITY1));
    p.domainNumber = 7;
    EXPECT_TRUE(m.updateParams(p));
    uint8_t buf[60];
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    EXPECT_EQ(m.parse(buf, 54), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(m.getDomainNumber(), 7);
}

// Test get PTP version of parsed message
// uint8_t getVersionPTP() const
TEST(MessageTest, MethodGetVersionPTP)
{
    Message m;
    EXPECT_TRUE(m.setAction(GET, PRIORITY1));
    uint8_t buf[60];
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    EXPECT_EQ(m.parse(buf, 54), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(m.getVersionPTP(), 2); // PTP 2
}

// Test get PTP minor version of parsed message
// uint8_t getMinorVersionPTP() const
TEST(MessageTest, MethodGetMinorVersionPTP)
{
    Message m;
    EXPECT_TRUE(m.setAction(GET, PRIORITY1));
    uint8_t buf[60];
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    EXPECT_EQ(m.parse(buf, 54), MNG_PARSE_ERROR_OK);
    // Currently we use PTP 2.0, but we may upgrade.
    EXPECT_EQ(m.getVersionPTP(), 2);
    EXPECT_GE(m.getMinorVersionPTP(), 0);
}

// Test get tlv data of parsed message
// const BaseMngTlv *getData() const
TEST(MessageTest, MethodGetData)
{
    Message m;
    PRIORITY1_t p;
    p.priority1 = 137;
    EXPECT_TRUE(m.setAction(SET, PRIORITY1, &p));
    uint8_t buf[70];
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    EXPECT_EQ(m.parse(buf, 56), MNG_PARSE_ERROR_OK);
    const BaseMngTlv *data = m.getData();
    EXPECT_NE(data, nullptr);
    const PRIORITY1_t *p1 = dynamic_cast<const PRIORITY1_t *>(data);
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(p1->priority1, p.priority1);
}

// Test get send tlv data
// const BaseMngTlv *getSendData() const
TEST(MessageTest, MethodGetSendData)
{
    Message m;
    PRIORITY1_t p;
    EXPECT_TRUE(m.setAction(SET, PRIORITY1, &p));
    EXPECT_EQ(m.getSendData(), &p);
}

// Test get error of parsed message
// managementErrorId_e getErrId() const
TEST(MessageTest, MethodGetErrId)
{
    Message m;
    EXPECT_TRUE(m.setAction(GET, PRIORITY1));
    uint8_t buf[80];
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // Follow location IEEE "PTP management message"
    // Change to actionField to response of get/set message
    buf[46] = RESPONSE;
    // Create error managementTLV
    uint16_t *mt = (uint16_t *)(buf + 48);
    char displayData[] = "test 123";
    uint8_t l = sizeof displayData - 1; // textField size exclude the null
    int n = l + 1; // lengthField + textField
    n += n & 1;
    mt[0] = cpu_to_net16(MANAGEMENT_ERROR_STATUS); // tlvType
    mt[1] = cpu_to_net16(8 + n); // lengthField
    mt[3] = mt[2]; // cp managementId from "MANAGEMENT TLV"
    mt[2] = cpu_to_net16(WRONG_VALUE); // managementErrorId
    mt[4] = 0; // reserved
    mt[5] = 0; // reserved
    uint8_t *d = buf + 48 + 12; // displayData
    d[0] = l; // displayData.lengthField
    memcpy(d + 1, displayData, l); // displayData.textField
    EXPECT_EQ(m.parse(buf, 48 + 12 + n), MNG_PARSE_ERROR_MSG);
    EXPECT_EQ(m.getErrId(), WRONG_VALUE);
}

// Test get error message of mangment error TLV parsed message
// const std::string &getErrDisplay() const
TEST(MessageTest, MethodGetErrDisplay)
{
    Message m;
    EXPECT_TRUE(m.setAction(GET, PRIORITY1));
    uint8_t buf[70];
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // Follow location IEEE "PTP management message"
    // Change to actionField to response of get/set message
    buf[46] = RESPONSE;
    // Create error managementTLV
    uint16_t *mt = (uint16_t *)(buf + 48);
    char displayData[] = "test 123";
    uint8_t l = sizeof displayData - 1; // textField size exclude the null
    int n = l + 1; // lengthField + textField
    n += n & 1;
    mt[0] = cpu_to_net16(MANAGEMENT_ERROR_STATUS); // tlvType
    mt[1] = cpu_to_net16(8 + n); // lengthField
    mt[3] = mt[2]; // cp managementId from "MANAGEMENT TLV"
    mt[2] = cpu_to_net16(WRONG_VALUE); // managementErrorId
    mt[4] = 0; // reserved
    mt[5] = 0; // reserved
    uint8_t *d = buf + 48 + 12; // displayData
    d[0] = l; // displayData.lengthField
    memcpy(d + 1, displayData, l); // displayData.textField
    EXPECT_EQ(m.parse(buf, 48 + 12 + n), MNG_PARSE_ERROR_MSG);
    EXPECT_STREQ(m.getErrDisplay().c_str(), displayData);
}

// Test get error message of mangment error TLV parsed message
// const char *getErrDisplay_c() const
TEST(MessageTest, MethodGetErrDisplayC)
{
    Message m;
    EXPECT_TRUE(m.setAction(GET, PRIORITY1));
    uint8_t buf[70];
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // Follow location IEEE "PTP management message"
    // Change to actionField to response of get/set message
    buf[46] = RESPONSE;
    // Create error managementTLV
    uint16_t *mt = (uint16_t *)(buf + 48);
    char displayData[] = "test 123";
    uint8_t l = sizeof displayData - 1; // textField size exclude the null
    int n = l + 1; // lengthField + textField
    n += n & 1;
    mt[0] = cpu_to_net16(MANAGEMENT_ERROR_STATUS); // tlvType
    mt[1] = cpu_to_net16(8 + n); // lengthField
    mt[3] = mt[2]; // cp managementId from "MANAGEMENT TLV"
    mt[2] = cpu_to_net16(WRONG_VALUE); // managementErrorId
    mt[4] = 0; // reserved
    mt[5] = 0; // reserved
    uint8_t *d = buf + 48 + 12; // displayData
    d[0] = l; // displayData.lengthField
    memcpy(d + 1, displayData, l); // displayData.textField
    EXPECT_EQ(m.parse(buf, 48 + 12 + n), MNG_PARSE_ERROR_MSG);
    EXPECT_STREQ(m.getErrDisplay_c(), displayData);
}

// Test get error message of mangment error TLV parsed message
// bool isLastMsgSig() const
TEST(MessageTest, MethodIsLastMsgSig)
{
    Message m;
    PRIORITY1_t p;
    p.priority1 = 1;
    EXPECT_TRUE(m.setAction(SET, PRIORITY1, &p));
    uint8_t buf[70];
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    // MNG msg = 36 + 10 targetPortIdentity + 4 = 48
    // MNG msg 48 + 6 Mng TLV + 2 PRIORITY1 TLV = 56
    EXPECT_EQ(m.parse(buf, 56), MNG_PARSE_ERROR_OK);
    EXPECT_FALSE(m.isLastMsgSig());
    // signaling = 36 header + 10 targetPortIdentity = 44
    // signaling MSG 44 + 6 Mng TLV + 2 PRIORITY1 TLV = 52
    buf[0] = (buf[0] & 0xf0) | Signaling; // messageType
    buf[32] = 5; // controlField
    // Move the 8 bytes of Mng TLV
    for(int i = 0; i < 8; i++)
        buf[44 + i] = buf[48 + i];
    MsgParams mp = m.getParams();
    mp.rcvSignaling = true;
    EXPECT_TRUE(m.updateParams(mp));
    // valueField already have Mng TLV at the proper place :-)
    EXPECT_EQ(m.parse(buf, 52), MNG_PARSE_ERROR_SIG);
    EXPECT_TRUE(m.isLastMsgSig());
}

// Test get parsed PTP message type
// msgType_e getType() const
TEST(MessageTest, MethodGetType)
{
    Message m;
    EXPECT_TRUE(m.setAction(GET, PRIORITY1));
    uint8_t buf[60];
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    EXPECT_EQ(m.parse(buf, 54), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(m.getType(), Management);
}

// Test get parsed PTP managment message managment type
// tlvType_e getMngType() const
TEST(MessageTest, MethodGetMngType)
{
    Message m;
    EXPECT_TRUE(m.setAction(GET, PRIORITY1));
    uint8_t buf[60];
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    EXPECT_EQ(m.parse(buf, 54), MNG_PARSE_ERROR_OK);
    EXPECT_EQ(m.getMngType(), MANAGEMENT);
}

// Test travers signaling message of TLVs
// bool traversSigTlvs(const std::function<bool
//     (const Message &msg, tlvType_e tlvType,
//         const BaseSigTlv *tlv)> callback) const
static bool verifyPr1(const Message &, tlvType_e tlvType, const BaseSigTlv *tlv)
{
    const MANAGEMENT_t *mng = dynamic_cast<const MANAGEMENT_t *>(tlv);
    const PRIORITY1_t *p1 = nullptr;
    if(mng != nullptr && mng->managementId == PRIORITY1)
        p1 = dynamic_cast<const PRIORITY1_t *>(mng->tlvData.get());
    return tlvType == MANAGEMENT && p1 != nullptr && p1->priority1 == 137;
}
TEST(MessageTest, MethodTraversSigTlvs)
{
    Message m;
    PRIORITY1_t p;
    p.priority1 = 137;
    EXPECT_TRUE(m.setAction(SET, PRIORITY1, &p));
    uint8_t buf[70];
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    // MNG msg = 36 + 10 targetPortIdentity + 4 = 48
    // MNG msg 48 + 6 Mng TLV + 2 PRIORITY1 TLV = 56
    // signaling = 36 header + 10 targetPortIdentity = 44
    // signaling MSG 44 + 6 Mng TLV + 2 PRIORITY1 TLV = 52
    buf[0] = (buf[0] & 0xf0) | Signaling; // messageType
    buf[32] = 5; // controlField
    // Move the 8 bytes of Mng TLV
    for(int i = 0; i < 8; i++)
        buf[44 + i] = buf[48 + i];
    MsgParams mp = m.getParams();
    mp.rcvSignaling = true;
    mp.filterSignaling = false;
    EXPECT_TRUE(m.updateParams(mp));
    // valueField already have Mng TLV at the proper place :-)
    EXPECT_EQ(m.parse(buf, 52), MNG_PARSE_ERROR_SIG);
    EXPECT_TRUE(m.traversSigTlvs(verifyPr1));
}

// Test get number of TLVs in a PTP signaling message
// size_t getSigTlvsCount() const
TEST(MessageTest, MethodGetSigTlvsCount)
{
    Message m;
    PRIORITY1_t p;
    p.priority1 = 137;
    EXPECT_TRUE(m.setAction(SET, PRIORITY1, &p));
    uint8_t buf[70];
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    // MNG msg = 36 + 10 targetPortIdentity + 4 = 48
    // MNG msg 48 + 6 Mng TLV + 2 PRIORITY1 TLV = 56
    // signaling = 36 header + 10 targetPortIdentity = 44
    // signaling MSG 44 + 6 Mng TLV + 2 PRIORITY1 TLV = 52
    buf[0] = (buf[0] & 0xf0) | Signaling; // messageType
    buf[32] = 5; // controlField
    // Move the 8 bytes of Mng TLV
    for(int i = 0; i < 8; i++)
        buf[44 + i] = buf[48 + i];
    MsgParams mp = m.getParams();
    mp.rcvSignaling = true;
    mp.filterSignaling = false;
    EXPECT_TRUE(m.updateParams(mp));
    // valueField already have Mng TLV at the proper place :-)
    EXPECT_EQ(m.parse(buf, 52), MNG_PARSE_ERROR_SIG);
    EXPECT_EQ(m.getSigTlvsCount(), 1);
}

// Test get a TLV and get its type from a PTP signaling message
// const BaseSigTlv *getSigTlv(size_t position) const
// tlvType_e getSigTlvType(size_t position) const
TEST(MessageTest, MethodGetSigTlv)
{
    Message m;
    PRIORITY1_t p;
    p.priority1 = 137;
    EXPECT_TRUE(m.setAction(SET, PRIORITY1, &p));
    uint8_t buf[70];
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    // MNG msg = 36 + 10 targetPortIdentity + 4 = 48
    // MNG msg 48 + 6 Mng TLV + 2 PRIORITY1 TLV = 56
    // signaling = 36 header + 10 targetPortIdentity = 44
    // signaling MSG 44 + 6 Mng TLV + 2 PRIORITY1 TLV = 52
    buf[0] = (buf[0] & 0xf0) | Signaling; // messageType
    buf[32] = 5; // controlField
    // Move the 8 bytes of Mng TLV
    for(int i = 0; i < 8; i++)
        buf[44 + i] = buf[48 + i];
    MsgParams mp = m.getParams();
    mp.rcvSignaling = true;
    mp.filterSignaling = false;
    EXPECT_TRUE(m.updateParams(mp));
    // valueField already have Mng TLV at the proper place :-)
    EXPECT_EQ(m.parse(buf, 52), MNG_PARSE_ERROR_SIG);
    EXPECT_EQ(m.getSigTlvType(0), MANAGEMENT);
    const BaseSigTlv *tlv = m.getSigTlv(0);
    EXPECT_NE(tlv, nullptr);
    const MANAGEMENT_t *mng = dynamic_cast<const MANAGEMENT_t *>(tlv);
    ASSERT_NE(mng, nullptr);
    EXPECT_EQ(mng->managementId, PRIORITY1);
    EXPECT_EQ(m.getSigMngTlvType(0), PRIORITY1);
    const PRIORITY1_t *p1 = dynamic_cast<const PRIORITY1_t *>(mng->tlvData.get());
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(p1->priority1, 137);
}

// Test get a managment TLV and its type from a PTP signaling message
// const BaseMngTlv *getSigMngTlv(size_t position) const
// mng_vals_e getSigMngTlvType(size_t position) const
TEST(MessageTest, MethodGetSigMngTlvType)
{
    Message m;
    PRIORITY1_t p;
    p.priority1 = 137;
    EXPECT_TRUE(m.setAction(SET, PRIORITY1, &p));
    uint8_t buf[70];
    EXPECT_EQ(m.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    // MNG msg = 36 + 10 targetPortIdentity + 4 = 48
    // MNG msg 48 + 6 Mng TLV + 2 PRIORITY1 TLV = 56
    // signaling = 36 header + 10 targetPortIdentity = 44
    // signaling MSG 44 + 6 Mng TLV + 2 PRIORITY1 TLV = 52
    buf[0] = (buf[0] & 0xf0) | Signaling; // messageType
    buf[32] = 5; // controlField
    // Move the 8 bytes of Mng TLV
    for(int i = 0; i < 8; i++)
        buf[44 + i] = buf[48 + i];
    MsgParams mp = m.getParams();
    mp.rcvSignaling = true;
    mp.filterSignaling = false;
    EXPECT_TRUE(m.updateParams(mp));
    // valueField already have Mng TLV at the proper place :-)
    EXPECT_EQ(m.parse(buf, 52), MNG_PARSE_ERROR_SIG);
    EXPECT_EQ(m.getSigMngTlvType(0), PRIORITY1);
    const PRIORITY1_t *p1 = dynamic_cast<const PRIORITY1_t *>(m.getSigMngTlv(0));
    ASSERT_NE(p1, nullptr);
    EXPECT_EQ(p1->priority1, 137);
}
