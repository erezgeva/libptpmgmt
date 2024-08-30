/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Json2msg class unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

#include "json.h"

using namespace ptpmgmt;

// Tests selectLib method
// static bool selectLib(const std::string &libName)
TEST(Json2msgTest, MethodSelectLib)
{
    // We use static link :-)
    EXPECT_FALSE(Json2msg::selectLib("jsonc"));
}

// Tests isLibShared method
// static bool isLibShared()
TEST(Json2msgTest, MethodIsLibShared)
{
    // We use static link :-)
    EXPECT_FALSE(Json2msg::isLibShared());
}

// Tests fromJson method
// bool fromJson(const std::string &json)
TEST(Json2msgTest, MethodFromJson)
{
    Json2msg m;
    EXPECT_TRUE(m.fromJson("{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"domainNumber\":0,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"unicastFlag\":true,"
            "\"PTPProfileSpecific\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"NULL_PTP_MANAGEMENT\","
            "\"dataField\":null"
            "}"));
}

// Tests managementId method
// mng_vals_e managementId() const
TEST(Json2msgTest, MethodManagementId)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"domainNumber\":0,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"unicastFlag\":true,"
            "\"PTPProfileSpecific\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"SET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\","
            "\"dataField\" :"
            " {"
            "  \"priority1\":153"
            " }"
            "}"));
    EXPECT_EQ(m.managementId(), PRIORITY1);
}

// Tests dataField method
// const BaseMngTlv *dataField() const
TEST(Json2msgTest, MethodDataField)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"domainNumber\":0,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"unicastFlag\":true,"
            "\"PTPProfileSpecific\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"SET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\","
            "\"dataField\" :"
            " {"
            "  \"priority1\":153"
            " }"
            "}"));
    EXPECT_EQ(m.managementId(), PRIORITY1);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const PRIORITY1_t *t = dynamic_cast<const PRIORITY1_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->priority1, 153);
}

// Tests actionField method
// actionField_e actionField() const
TEST(Json2msgTest, MethodActionField)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"domainNumber\":0,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"unicastFlag\":true,"
            "\"PTPProfileSpecific\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    EXPECT_EQ(m.managementId(), PRIORITY1);
    EXPECT_EQ(m.actionField(), GET);
}

// Tests isUnicast method
// bool isUnicast() const
TEST(Json2msgTest, MethodIsUnicast)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"domainNumber\":0,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"unicastFlag\":true,"
            "\"PTPProfileSpecific\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    EXPECT_TRUE(m.isUnicast());
    EXPECT_TRUE(m.haveIsUnicast());
}

// Tests haveIsUnicast method
// bool haveIsUnicast() const
TEST(Json2msgTest, MethodHaveIsUnicast)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"domainNumber\":0,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"PTPProfileSpecific\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    EXPECT_FALSE(m.haveIsUnicast());
}

// Tests PTPProfileSpecific method
// uint8_t PTPProfileSpecific() const
TEST(Json2msgTest, MethodPTPProfileSpecific)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"domainNumber\":0,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"PTPProfileSpecific\":5,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    EXPECT_EQ(m.PTPProfileSpecific(), 5);
    EXPECT_TRUE(m.havePTPProfileSpecific());
}

// Tests havePTPProfileSpecific method
// bool havePTPProfileSpecific() const
TEST(Json2msgTest, MethodHavePTPProfileSpecific)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"domainNumber\":0,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    EXPECT_FALSE(m.havePTPProfileSpecific());
}

// Tests domainNumber method
// uint8_t domainNumber() const
TEST(Json2msgTest, MethodDomainNumber)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"domainNumber\":9,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    EXPECT_EQ(m.domainNumber(), 9);
    EXPECT_TRUE(m.haveDomainNumber());
}

// Tests haveDomainNumber method
// bool haveDomainNumber() const
TEST(Json2msgTest, MethodHaveDomainNumber)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    EXPECT_FALSE(m.haveDomainNumber());
}

// Tests versionPTP method
// uint8_t versionPTP() const
TEST(Json2msgTest, MethodVersionPTP)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"versionPTP\":2,"
            "\"minorVersionPTP\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    EXPECT_EQ(m.versionPTP(), 2);
    EXPECT_TRUE(m.haveVersionPTP());
}

// Tests haveVersionPTP method
// bool haveVersionPTP() const
TEST(Json2msgTest, MethodHaveVersionPTP)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"minorVersionPTP\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    EXPECT_FALSE(m.haveVersionPTP());
}

// Tests minorVersionPTP method
// uint8_t minorVersionPTP() const
TEST(Json2msgTest, MethodMinorVersionPTP)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"minorVersionPTP\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    EXPECT_EQ(m.minorVersionPTP(), 0);
    EXPECT_TRUE(m.haveMinorVersionPTP());
}

// Tests haveMinorVersionPTP method
// bool haveMinorVersionPTP() const
TEST(Json2msgTest, MethodHaveMinorVersionPTP)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"sequenceId\":1,"
            "\"sdoId\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    EXPECT_FALSE(m.haveMinorVersionPTP());
}

// Tests sequenceId method
// uint16_t sequenceId() const
TEST(Json2msgTest, MethodSequenceId)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"sequenceId\":15,"
            "\"sdoId\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    EXPECT_EQ(m.sequenceId(), 15);
    EXPECT_TRUE(m.haveSequenceId());
}

// Tests haveSequenceId method
// bool haveSequenceId() const
TEST(Json2msgTest, MethodHaveSequenceId)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"sdoId\":0,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    EXPECT_FALSE(m.haveSequenceId());
}

// Tests sdoId method
// uint32_t sdoId() const
TEST(Json2msgTest, MethodSdoId)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"sdoId\":1054,"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    EXPECT_EQ(m.sdoId(), 1054);
    EXPECT_TRUE(m.haveSdoId());
}

// Tests haveSdoId method
// bool haveSdoId() const
TEST(Json2msgTest, MethodHaveSdoId)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"000000.0000.000000\","
            "  \"portNumber\":0"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    EXPECT_FALSE(m.haveSdoId());
}

// Tests srcPort method
// const PortIdentity_t &srcPort() const
TEST(Json2msgTest, MethodSrcPort)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"messageType\":\"Management\","
            "\"sourcePortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"c47d46.fffe.20acae\","
            "  \"portNumber\":7"
            "},"
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t port = { clockId, 7 };
    EXPECT_EQ(m.srcPort(), port);
    EXPECT_TRUE(m.haveSrcPort());
}

// Tests haveSrcPort method
// bool haveSrcPort() const
TEST(Json2msgTest, MethodHaveSrcPort)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"messageType\":\"Management\","
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    EXPECT_FALSE(m.haveSrcPort());
}

// Tests dstPort method
// const PortIdentity_t &dstPort() const
TEST(Json2msgTest, MethodDstPort)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"messageType\":\"Management\","
            "\"targetPortIdentity\" :"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "},"
            "\"actionField\":\"GET\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    ClockIdentity_t clockId = { 255, 255, 255, 255, 255, 255, 255, 255 };
    PortIdentity_t port = { clockId, 0xffff };
    EXPECT_EQ(m.dstPort(), port);
    EXPECT_TRUE(m.haveDstPort());
}

// Tests haveDstPort method
// bool haveDstPort() const
TEST(Json2msgTest, MethodHaveDstPort)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"messageType\":\"Management\","
            "\"tlvType\":\"MANAGEMENT\","
            "\"actionField\":\"GET\","
            "\"managementId\":\"PRIORITY1\""
            "}"));
    EXPECT_FALSE(m.haveDstPort());
}

// Tests minimum to pass
TEST(Json2msgTest, Minimum)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"GET\","
            "\"managementId\":\"NULL_PTP_MANAGEMENT\"}"));
    EXPECT_EQ(m.actionField(), GET);
    EXPECT_EQ(m.managementId(), NULL_PTP_MANAGEMENT);
}

// Tests minimum set to pass
TEST(Json2msgTest, MinimumSet)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"PRIORITY1\",\"dataField\":{"
            " \"priority1\":1"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), PRIORITY1);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const PRIORITY1_t *t = dynamic_cast<const PRIORITY1_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->priority1, 1);
}

// Tests setting Message parameters
TEST(Json2msgTest, MessageParameters)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{"
            "\"actionField\":\"GET\","
            "\"managementId\":\"NULL_PTP_MANAGEMENT\","
            "\"domainNumber\":9,"
            "\"unicastFlag\":true,"
            "\"sdoId\":704,"
            "\"sourcePortIdentity\":"
            "{"
            "  \"clockIdentity\":\"c47d46.fffe.20acae\","
            "  \"portNumber\":7"
            "},"
            "\"targetPortIdentity\":"
            "{"
            "  \"clockIdentity\":\"ffffff.ffff.ffffff\","
            "  \"portNumber\":65535"
            "}"
            "}"));
    EXPECT_EQ(m.domainNumber(), 9);
    EXPECT_TRUE(m.haveDomainNumber());
    EXPECT_TRUE(m.isUnicast());
    EXPECT_TRUE(m.haveIsUnicast());
    ClockIdentity_t c1 = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t p1 = { c1, 7 };
    EXPECT_EQ(m.srcPort(), p1);
    EXPECT_TRUE(m.haveSrcPort());
    ClockIdentity_t c2 = { 255, 255, 255, 255, 255, 255, 255, 255 };
    PortIdentity_t p2 = { c2, 0xffff };
    EXPECT_EQ(m.dstPort(), p2);
    EXPECT_TRUE(m.haveDstPort());
    EXPECT_EQ(m.sdoId(), 704);
    EXPECT_TRUE(m.haveSdoId());
    MsgParams p;
    p.transportSpecific = m.sdoId() >> 4; // Only major is supported
    p.domainNumber = m.domainNumber();
    p.isUnicast = m.isUnicast();
    p.target = m.dstPort();
    p.self_id = m.srcPort();
    EXPECT_EQ(p.transportSpecific, 44);
    EXPECT_EQ(p.domainNumber, 9);
    EXPECT_TRUE(p.isUnicast);
    EXPECT_EQ(p.target, p2);
    EXPECT_EQ(p.self_id, p1);
}

// Tests CLOCK_DESCRIPTION managment ID
TEST(Json2msgTest, CLOCK_DESCRIPTION)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"CLOCK_DESCRIPTION\",\"dataField\":{"
            "\"clockType\":32768,"
            "\"physicalLayerProtocol\":\"IEEE 802.3\","
            "\"physicalAddress\":\"c4:7d:46:20:ac:ae\","
            "\"protocolAddress\":"
            "{"
            "  \"networkProtocol\":\"IEEE_802_3\","
            "  \"addressField\":\"c4:7d:46:20:ac:ae\""
            "},"
            "\"manufacturerIdentity\":\"01:02:03\","
            "\"productDescription\":\";;\","
            "\"revisionData\":\";;\","
            "\"userDescription\":\"test123\","
            "\"profileIdentity\":\"01:02:03:04:05:06\""
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), CLOCK_DESCRIPTION);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const CLOCK_DESCRIPTION_t *t = dynamic_cast<const CLOCK_DESCRIPTION_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->clockType, ordinaryClock);
    EXPECT_STREQ(t->physicalLayerProtocol.string(), "IEEE 802.3");
    EXPECT_EQ(t->physicalAddressLength, 6);
    Binary physicalAddress = Binary("\xc4\x7d\x46\x20\xac\xae", 6);
    EXPECT_EQ(t->physicalAddress, physicalAddress);
    PortAddress_t portAddress = { IEEE_802_3, 6, physicalAddress };
    EXPECT_EQ(t->protocolAddress, portAddress);
    EXPECT_EQ(t->manufacturerIdentity[0], 1);
    EXPECT_EQ(t->manufacturerIdentity[1], 2);
    EXPECT_EQ(t->manufacturerIdentity[2], 3);
    EXPECT_STREQ(t->productDescription.string(), ";;");
    EXPECT_STREQ(t->revisionData.string(), ";;");
    EXPECT_STREQ(t->userDescription.string(), "test123");
    EXPECT_EQ(memcmp(t->profileIdentity, "\x1\x2\x3\x4\x5\x6", 6), 0);
}

// Tests USER_DESCRIPTION managment ID
TEST(Json2msgTest, USER_DESCRIPTION)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"USER_DESCRIPTION\",\"dataField\":{"
            "\"userDescription\":\"test123\""
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), USER_DESCRIPTION);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const USER_DESCRIPTION_t *t = dynamic_cast<const USER_DESCRIPTION_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_STREQ(t->userDescription.string(), "test123");
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests INITIALIZE managment ID
TEST(Json2msgTest, INITIALIZE)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"COMMAND\","
            "\"managementId\":\"INITIALIZE\",\"dataField\":{"
            "\"initializationKey\":4660"
            "}}"));
    EXPECT_EQ(m.actionField(), COMMAND);
    EXPECT_EQ(m.managementId(), INITIALIZE);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const INITIALIZE_t *t = dynamic_cast<const INITIALIZE_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->initializationKey, 0x1234);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests FAULT_LOG managment ID
TEST(Json2msgTest, FAULT_LOG)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"FAULT_LOG\",\"dataField\":{"
            "\"numberOfFaultRecords\":2,"
            "\"faultRecords\":"
            "["
            "  {"
            "    \"faultRecordLength\":50,"
            "    \"faultTime\":9.000709000,"
            "    \"severityCode\":\"Critical\","
            "    \"faultName\":\"error 1\","
            "    \"faultValue\":\"test123\","
            "    \"faultDescription\":\"This is first record\""
            "  },"
            "  {"
            "    \"faultRecordLength\":55,"
            "    \"faultTime\":1791.003018999,"
            "    \"severityCode\":\"Warning\","
            "    \"faultName\":\"error 2\","
            "    \"faultValue\":\"test321\","
            "    \"faultDescription\":\"This is the second record\""
            "  }"
            "]"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), FAULT_LOG);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const FAULT_LOG_t *t = dynamic_cast<const FAULT_LOG_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->numberOfFaultRecords, 2);
    EXPECT_EQ(t->faultRecords[0].faultRecordLength, 50);
    EXPECT_EQ(t->faultRecords[0].faultTime, (float_seconds)9.000709);
    EXPECT_EQ(t->faultRecords[0].severityCode, F_Critical);
    EXPECT_EQ(t->faultRecords[0].faultName.textField, "error 1");
    EXPECT_EQ(t->faultRecords[0].faultValue.textField, "test123");
    EXPECT_EQ(t->faultRecords[0].faultDescription.textField,
        "This is first record");
    EXPECT_EQ(t->faultRecords[1].faultRecordLength, 55);
    EXPECT_EQ(t->faultRecords[1].faultTime, (float_seconds)1791.003019);
    EXPECT_EQ(t->faultRecords[1].severityCode, F_Warning);
    EXPECT_EQ(t->faultRecords[1].faultName.textField, "error 2");
    EXPECT_EQ(t->faultRecords[1].faultValue.textField, "test321");
    EXPECT_EQ(t->faultRecords[1].faultDescription.textField,
        "This is the second record");
}

// Tests DEFAULT_DATA_SET managment ID
TEST(Json2msgTest, DEFAULT_DATA_SET)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"DEFAULT_DATA_SET\",\"dataField\":{"
            "\"twoStepFlag\":true,"
            "\"slaveOnly\":true,"
            "\"numberPorts\":1,"
            "\"priority1\":153,"
            "\"clockQuality\":"
            "{"
            "  \"clockClass\":255,"
            "  \"clockAccuracy\":\"Unknown\","
            "  \"offsetScaledLogVariance\":65535"
            "},"
            "\"priority2\":137,"
            "\"clockIdentity\":\"c47d46.fffe.20acae\","
            "\"domainNumber\":0"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), DEFAULT_DATA_SET);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const DEFAULT_DATA_SET_t *t = dynamic_cast<const DEFAULT_DATA_SET_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 0x3);
    EXPECT_EQ(t->numberPorts, 1);
    EXPECT_EQ(t->priority1, 153);
    EXPECT_EQ(t->clockQuality.clockClass, 255);
    EXPECT_EQ(t->clockQuality.clockAccuracy, Accurate_Unknown);
    EXPECT_EQ(t->clockQuality.offsetScaledLogVariance, 0xffff);
    EXPECT_EQ(t->priority2, 137);
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    EXPECT_EQ(t->clockIdentity, clockId);
    EXPECT_EQ(t->domainNumber, 0);
}

// Tests CURRENT_DATA_SET managment ID
TEST(Json2msgTest, CURRENT_DATA_SET)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"CURRENT_DATA_SET\",\"dataField\":{"
            "\"stepsRemoved\":4660,"
            "\"offsetFromMaster\":3607462104733586053,"
            "\"meanPathDelay\":650212710990086400"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), CURRENT_DATA_SET);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const CURRENT_DATA_SET_t *t = dynamic_cast<const CURRENT_DATA_SET_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->stepsRemoved, 0x1234);
    EXPECT_EQ(t->offsetFromMaster.scaledNanoseconds, 0x321047abcd541285LL);
    EXPECT_EQ(t->meanPathDelay.scaledNanoseconds, 0x0906050403020100LL);
}

// Tests PARENT_DATA_SET managment ID
TEST(Json2msgTest, PARENT_DATA_SET)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"PARENT_DATA_SET\",\"dataField\":{"
            "\"parentPortIdentity\":"
            "{"
            "  \"clockIdentity\":\"c47d46.fffe.20acae\","
            "  \"portNumber\":5"
            "},"
            "\"parentStats\":true,"
            "\"observedParentOffsetScaledLogVariance\":65535,"
            "\"observedParentClockPhaseChangeRate\":2147483647,"
            "\"grandmasterPriority1\":255,"
            "\"grandmasterClockQuality\":"
            "{"
            "  \"clockClass\":255,"
            "  \"clockAccuracy\":\"Unknown\","
            "  \"offsetScaledLogVariance\":65535"
            "},"
            "\"grandmasterPriority2\":255,"
            "\"grandmasterIdentity\":\"c47d46.fffe.20acae\""
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), PARENT_DATA_SET);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const PARENT_DATA_SET_t *t = dynamic_cast<const PARENT_DATA_SET_t *>(d);
    ASSERT_NE(t, nullptr);
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 5 };
    EXPECT_EQ(t->parentPortIdentity, portIdentity);
    EXPECT_EQ(t->flags, 1);
    EXPECT_EQ(t->observedParentOffsetScaledLogVariance, 0xffff);
    EXPECT_EQ(t->observedParentClockPhaseChangeRate, 0x7fffffff);
    EXPECT_EQ(t->grandmasterPriority1, 255);
    EXPECT_EQ(t->grandmasterClockQuality.clockClass, 255);
    EXPECT_EQ(t->grandmasterClockQuality.clockAccuracy, Accurate_Unknown);
    EXPECT_EQ(t->grandmasterClockQuality.offsetScaledLogVariance, 0xffff);
    EXPECT_EQ(t->grandmasterPriority2, 255);
    EXPECT_EQ(t->grandmasterIdentity, clockId);
}

// Tests TIME_PROPERTIES_DATA_SET managment ID
TEST(Json2msgTest, TIME_PROPERTIES_DATA_SET)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"TIME_PROPERTIES_DATA_SET\",\"dataField\":{"
            "\"currentUtcOffset\":37,"
            "\"leap61\":false,"
            "\"leap59\":false,"
            "\"currentUtcOffsetValid\":false,"
            "\"ptpTimescale\":true,"
            "\"timeTraceable\":false,"
            "\"frequencyTraceable\":false,"
            "\"timeSource\":\"INTERNAL_OSCILLATOR\""
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), TIME_PROPERTIES_DATA_SET);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const TIME_PROPERTIES_DATA_SET_t *t =
        dynamic_cast<const TIME_PROPERTIES_DATA_SET_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->currentUtcOffset, 37);
    EXPECT_EQ(t->flags, F_PTP); // ptpTimescale bit
    EXPECT_EQ(t->timeSource, INTERNAL_OSCILLATOR);
}

// Tests PORT_DATA_SET managment ID
TEST(Json2msgTest, PORT_DATA_SET)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"PORT_DATA_SET\",\"dataField\":{"
            "\"portIdentity\":"
            "{"
            "  \"clockIdentity\":\"c47d46.fffe.20acae\","
            "  \"portNumber\":1"
            "},"
            "\"portState\":\"LISTENING\","
            "\"logMinDelayReqInterval\":0,"
            "\"peerMeanPathDelay\":0,"
            "\"logAnnounceInterval\":1,"
            "\"announceReceiptTimeout\":3,"
            "\"logSyncInterval\":0,"
            "\"delayMechanism\":\"P2P\","
            "\"logMinPdelayReqInterval\":0,"
            "\"versionNumber\":2"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), PORT_DATA_SET);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const PORT_DATA_SET_t *t = dynamic_cast<const PORT_DATA_SET_t *>(d);
    ASSERT_NE(t, nullptr);
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    EXPECT_EQ(t->portIdentity, portIdentity);
    EXPECT_EQ(t->portState, LISTENING);
    EXPECT_EQ(t->logMinDelayReqInterval, 0);
    EXPECT_EQ(t->peerMeanPathDelay.scaledNanoseconds, 0);
    EXPECT_EQ(t->logAnnounceInterval, 1);
    EXPECT_EQ(t->announceReceiptTimeout, 3);
    EXPECT_EQ(t->logSyncInterval, 0);
    EXPECT_EQ(t->delayMechanism, P2P);
    EXPECT_EQ(t->logMinPdelayReqInterval, 0);
    EXPECT_EQ(t->versionNumber, 2);
};

// Tests PRIORITY1 managment ID
TEST(Json2msgTest, PRIORITY1)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"PRIORITY1\",\"dataField\":{"
            "\"priority1\":153"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), PRIORITY1);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const PRIORITY1_t *t = dynamic_cast<const PRIORITY1_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->priority1, 153);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests PRIORITY2 managment ID
TEST(Json2msgTest, PRIORITY2)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"PRIORITY2\",\"dataField\":{"
            "\"priority2\":137"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), PRIORITY2);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const PRIORITY2_t *t = dynamic_cast<const PRIORITY2_t *>(d);
    ASSERT_NE(t, nullptr);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
    EXPECT_EQ(t->priority2, 137);
}

// Tests DOMAIN managment ID
TEST(Json2msgTest, DOMAIN)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"DOMAIN\",\"dataField\":{"
            "\"domainNumber\":7"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), DOMAIN);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const DOMAIN_t *t = dynamic_cast<const DOMAIN_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->domainNumber, 7);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests SLAVE_ONLY managment ID
TEST(Json2msgTest, SLAVE_ONLY)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"SLAVE_ONLY\",\"dataField\":{"
            "\"slaveOnly\":true"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), SLAVE_ONLY);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const SLAVE_ONLY_t *t = dynamic_cast<const SLAVE_ONLY_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 1);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests LOG_ANNOUNCE_INTERVAL managment ID
TEST(Json2msgTest, LOG_ANNOUNCE_INTERVAL)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"LOG_ANNOUNCE_INTERVAL\",\"dataField\":{"
            "\"logAnnounceInterval\":1"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), LOG_ANNOUNCE_INTERVAL);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const LOG_ANNOUNCE_INTERVAL_t *t =
        dynamic_cast<const LOG_ANNOUNCE_INTERVAL_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->logAnnounceInterval, 1);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests ANNOUNCE_RECEIPT_TIMEOUT managment ID
TEST(Json2msgTest, ANNOUNCE_RECEIPT_TIMEOUT)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"ANNOUNCE_RECEIPT_TIMEOUT\",\"dataField\":{"
            "\"announceReceiptTimeout\":3"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), ANNOUNCE_RECEIPT_TIMEOUT);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const ANNOUNCE_RECEIPT_TIMEOUT_t *t =
        dynamic_cast<const ANNOUNCE_RECEIPT_TIMEOUT_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->announceReceiptTimeout, 3);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests LOG_SYNC_INTERVAL managment ID
TEST(Json2msgTest, LOG_SYNC_INTERVAL)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"LOG_SYNC_INTERVAL\",\"dataField\":{"
            "\"logSyncInterval\":7"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), LOG_SYNC_INTERVAL);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const LOG_SYNC_INTERVAL_t *t = dynamic_cast<const LOG_SYNC_INTERVAL_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->logSyncInterval, 7);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests VERSION_NUMBER managment ID
TEST(Json2msgTest, VERSION_NUMBER)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"VERSION_NUMBER\",\"dataField\":{"
            "\"versionNumber\":2"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), VERSION_NUMBER);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const VERSION_NUMBER_t *t = dynamic_cast<const VERSION_NUMBER_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->versionNumber, 2);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests TIME managment ID
TEST(Json2msgTest, TIME)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"TIME\",\"dataField\":{"
            "\"currentTime\":13.150000000"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), TIME);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const TIME_t *t = dynamic_cast<const TIME_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->currentTime, (float_seconds)13.15);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests CLOCK_ACCURACY managment ID
TEST(Json2msgTest, CLOCK_ACCURACY)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"CLOCK_ACCURACY\",\"dataField\":{"
            "\"clockAccuracy\":\"Unknown\""
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), CLOCK_ACCURACY);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const CLOCK_ACCURACY_t *t = dynamic_cast<const CLOCK_ACCURACY_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->clockAccuracy, Accurate_Unknown);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests UTC_PROPERTIES managment ID
TEST(Json2msgTest, UTC_PROPERTIES)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"UTC_PROPERTIES\",\"dataField\":{"
            "\"currentUtcOffset\":-21555,"
            "\"leap61\":true,"
            "\"leap59\":true,"
            "\"currentUtcOffsetValid\":true"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), UTC_PROPERTIES);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const UTC_PROPERTIES_t *t = dynamic_cast<const UTC_PROPERTIES_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->currentUtcOffset, -0x5433);
    EXPECT_EQ(t->flags, 7);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests TRACEABILITY_PROPERTIES managment ID
TEST(Json2msgTest, TRACEABILITY_PROPERTIES)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"TRACEABILITY_PROPERTIES\",\"dataField\":{"
            "\"timeTraceable\":true,"
            "\"frequencyTraceable\":true"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), TRACEABILITY_PROPERTIES);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const TRACEABILITY_PROPERTIES_t *t =
        dynamic_cast<const TRACEABILITY_PROPERTIES_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, F_TTRA | F_FTRA);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests TIMESCALE_PROPERTIES managment ID
TEST(Json2msgTest, TIMESCALE_PROPERTIES)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"TIMESCALE_PROPERTIES\",\"dataField\":{"
            "\"ptpTimescale\":true,"
            "\"timeSource\":\"HAND_SET\""
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), TIMESCALE_PROPERTIES);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const TIMESCALE_PROPERTIES_t *t = dynamic_cast<const TIMESCALE_PROPERTIES_t *>
        (d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, F_PTP);
    EXPECT_EQ(t->timeSource, HAND_SET);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests UNICAST_NEGOTIATION_ENABLE managment ID
TEST(Json2msgTest, UNICAST_NEGOTIATION_ENABLE)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"UNICAST_NEGOTIATION_ENABLE\",\"dataField\":{"
            "\"unicastNegotiationPortDS\":true"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), UNICAST_NEGOTIATION_ENABLE);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const UNICAST_NEGOTIATION_ENABLE_t *t =
        dynamic_cast<const UNICAST_NEGOTIATION_ENABLE_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 1);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests PATH_TRACE_LIST managment ID
TEST(Json2msgTest, PATH_TRACE_LIST)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"PATH_TRACE_LIST\",\"dataField\":{"
            "\"pathSequence\":"
            "["
            "  \"c47d46.fffe.20acae\","
            "  \"0c0413.610b.4a0c4a\""
            "]"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), PATH_TRACE_LIST);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const PATH_TRACE_LIST_t *t = dynamic_cast<const PATH_TRACE_LIST_t *>(d);
    ASSERT_NE(t, nullptr);
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    EXPECT_EQ(t->pathSequence[0], clockId);
    ClockIdentity_t c2 = {12, 4, 19, 97, 11, 74, 12, 74};
    EXPECT_EQ(t->pathSequence[1], c2);
}

// Tests PATH_TRACE_ENABLE managment ID
TEST(Json2msgTest, PATH_TRACE_ENABLE)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"PATH_TRACE_ENABLE\",\"dataField\":{"
            "\"pathTraceDS\":true"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), PATH_TRACE_ENABLE);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const PATH_TRACE_ENABLE_t *t = dynamic_cast<const PATH_TRACE_ENABLE_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 1);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests GRANDMASTER_CLUSTER_TABLE managment ID
TEST(Json2msgTest, GRANDMASTER_CLUSTER_TABLE)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"GRANDMASTER_CLUSTER_TABLE\",\"dataField\":{"
            "\"logQueryInterval\":-19,"
            "\"actualTableSize\":2,"
            "\"PortAddress\":"
            "["
            "  {"
            "    \"networkProtocol\":\"IEEE_802_3\","
            "    \"addressField\":\"c4:7d:46:20:ac:ae\""
            "  },"
            "  {"
            "    \"networkProtocol\":\"UDP_IPv4\","
            "    \"addressField\":\"12:34:56:78\""
            "  }"
            "]"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), GRANDMASTER_CLUSTER_TABLE);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const GRANDMASTER_CLUSTER_TABLE_t *t =
        dynamic_cast<const GRANDMASTER_CLUSTER_TABLE_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->logQueryInterval, -19);
    EXPECT_EQ(t->actualTableSize, 2);
    Binary physicalAddress = Binary("\xc4\x7d\x46\x20\xac\xae", 6);
    PortAddress_t portAddress0 = { IEEE_802_3, 6, physicalAddress };
    EXPECT_EQ(t->PortAddress[0], portAddress0);
    Binary ip("\x12\x34\x56\x78", 4);
    PortAddress_t portAddress1 = { UDP_IPv4, 4, ip };
    EXPECT_EQ(t->PortAddress[1], portAddress1);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests UNICAST_MASTER_TABLE managment ID
TEST(Json2msgTest, UNICAST_MASTER_TABLE)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"UNICAST_MASTER_TABLE\",\"dataField\":{"
            "\"logQueryInterval\":-19,"
            "\"actualTableSize\":2,"
            "\"PortAddress\":"
            "["
            "  {"
            "    \"networkProtocol\":\"IEEE_802_3\","
            "    \"addressField\":\"c4:7d:46:20:ac:ae\""
            "  },"
            "  {"
            "    \"networkProtocol\":\"UDP_IPv4\","
            "    \"addressField\":\"12:34:56:78\""
            "  }"
            "]"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), UNICAST_MASTER_TABLE);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const UNICAST_MASTER_TABLE_t *t = dynamic_cast<const UNICAST_MASTER_TABLE_t *>
        (d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->logQueryInterval, -19);
    EXPECT_EQ(t->actualTableSize, 2);
    Binary physicalAddress = Binary("\xc4\x7d\x46\x20\xac\xae", 6);
    PortAddress_t portAddress0 = { IEEE_802_3, 6, physicalAddress };
    EXPECT_EQ(t->PortAddress[0], portAddress0);
    Binary ip("\x12\x34\x56\x78", 4);
    PortAddress_t portAddress1 = { UDP_IPv4, 4, ip };
    EXPECT_EQ(t->PortAddress[1], portAddress1);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests UNICAST_MASTER_MAX_TABLE_SIZE managment ID
TEST(Json2msgTest, UNICAST_MASTER_MAX_TABLE_SIZE)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"UNICAST_MASTER_MAX_TABLE_SIZE\",\"dataField\":{"
            "\"maxTableSize\":8515"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), UNICAST_MASTER_MAX_TABLE_SIZE);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const UNICAST_MASTER_MAX_TABLE_SIZE_t *t =
        dynamic_cast<const UNICAST_MASTER_MAX_TABLE_SIZE_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->maxTableSize, 0x2143);
}

// Tests ACCEPTABLE_MASTER_TABLE managment ID
TEST(Json2msgTest, ACCEPTABLE_MASTER_TABLE)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"ACCEPTABLE_MASTER_TABLE\",\"dataField\":{"
            "\"actualTableSize\":2,"
            "\"list\":"
            "["
            "  {"
            "    \"acceptablePortIdentity\":"
            "    {"
            "      \"clockIdentity\":\"c47d46.fffe.20acae\","
            "      \"portNumber\":1"
            "    },"
            "    \"alternatePriority1\":127"
            "  },"
            "  {"
            "    \"acceptablePortIdentity\":"
            "    {"
            "      \"clockIdentity\":\"090807.0605.040107\","
            "      \"portNumber\":2"
            "    },"
            "    \"alternatePriority1\":111"
            "  }"
            "]"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), ACCEPTABLE_MASTER_TABLE);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const ACCEPTABLE_MASTER_TABLE_t *t =
        dynamic_cast<const ACCEPTABLE_MASTER_TABLE_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->actualTableSize, 2);
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t a0 = { clockId, 1 };
    EXPECT_EQ(t->list[0].acceptablePortIdentity, a0);
    EXPECT_EQ(t->list[0].alternatePriority1, 127);
    PortIdentity_t a1 = { { 9, 8, 7, 6, 5, 4, 1, 7}, 2 };
    EXPECT_EQ(t->list[1].acceptablePortIdentity, a1);
    EXPECT_EQ(t->list[1].alternatePriority1, 111);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests ACCEPTABLE_MASTER_TABLE_ENABLED managment ID
TEST(Json2msgTest, ACCEPTABLE_MASTER_TABLE_ENABLED)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"ACCEPTABLE_MASTER_TABLE_ENABLED\",\"dataField\":{"
            "\"acceptableMasterPortDS\":true"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), ACCEPTABLE_MASTER_TABLE_ENABLED);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const ACCEPTABLE_MASTER_TABLE_ENABLED_t *t =
        dynamic_cast<const ACCEPTABLE_MASTER_TABLE_ENABLED_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 1);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests ACCEPTABLE_MASTER_MAX_TABLE_SIZE managment ID
TEST(Json2msgTest, ACCEPTABLE_MASTER_MAX_TABLE_SIZE)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"ACCEPTABLE_MASTER_MAX_TABLE_SIZE\",\"dataField\":{"
            "\"maxTableSize\":26554"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), ACCEPTABLE_MASTER_MAX_TABLE_SIZE);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const ACCEPTABLE_MASTER_MAX_TABLE_SIZE_t *t =
        dynamic_cast<const ACCEPTABLE_MASTER_MAX_TABLE_SIZE_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->maxTableSize, 0x67ba);
}

// Tests ALTERNATE_MASTER managment ID
TEST(Json2msgTest, ALTERNATE_MASTER)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"ALTERNATE_MASTER\",\"dataField\":{"
            "\"transmitAlternateMulticastSync\":true,"
            "\"logAlternateMulticastSyncInterval\":-17,"
            "\"numberOfAlternateMasters\":210"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), ALTERNATE_MASTER);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const ALTERNATE_MASTER_t *t = dynamic_cast<const ALTERNATE_MASTER_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 1);
    EXPECT_EQ(t->logAlternateMulticastSyncInterval, -17);
    EXPECT_EQ(t->numberOfAlternateMasters, 210);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests ALTERNATE_TIME_OFFSET_ENABLE managment ID
TEST(Json2msgTest, ALTERNATE_TIME_OFFSET_ENABLE)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"ALTERNATE_TIME_OFFSET_ENABLE\",\"dataField\":{"
            "\"keyField\":7,"
            "\"alternateTimescaleOffsetsDS\":true"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), ALTERNATE_TIME_OFFSET_ENABLE);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const ALTERNATE_TIME_OFFSET_ENABLE_t *t =
        dynamic_cast<const ALTERNATE_TIME_OFFSET_ENABLE_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->keyField, 7);
    EXPECT_EQ(t->flags, 1);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests ALTERNATE_TIME_OFFSET_NAME managment ID
TEST(Json2msgTest, ALTERNATE_TIME_OFFSET_NAME)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"ALTERNATE_TIME_OFFSET_NAME\",\"dataField\":{"
            "\"keyField\":11,"
            "\"displayName\":\"123\""
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), ALTERNATE_TIME_OFFSET_NAME);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const ALTERNATE_TIME_OFFSET_NAME_t *t =
        dynamic_cast<const ALTERNATE_TIME_OFFSET_NAME_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->keyField, 11);
    EXPECT_STREQ(t->displayName.string(), "123");
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests ALTERNATE_TIME_OFFSET_MAX_KEY managment ID
TEST(Json2msgTest, ALTERNATE_TIME_OFFSET_MAX_KEY)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"ALTERNATE_TIME_OFFSET_MAX_KEY\",\"dataField\":{"
            "\"maxKey\":9"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), ALTERNATE_TIME_OFFSET_MAX_KEY);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const ALTERNATE_TIME_OFFSET_MAX_KEY_t *t =
        dynamic_cast<const ALTERNATE_TIME_OFFSET_MAX_KEY_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->maxKey, 9);
}

// Tests ALTERNATE_TIME_OFFSET_PROPERTIES managment ID
TEST(Json2msgTest, ALTERNATE_TIME_OFFSET_PROPERTIES)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"ALTERNATE_TIME_OFFSET_PROPERTIES\",\"dataField\":{"
            "\"keyField\":13,"
            "\"currentOffset\":-2145493247,"
            "\"jumpSeconds\":-2147413249,"
            "\"timeOfNextJump\":159585821399185"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), ALTERNATE_TIME_OFFSET_PROPERTIES);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const ALTERNATE_TIME_OFFSET_PROPERTIES_t *t =
        dynamic_cast<const ALTERNATE_TIME_OFFSET_PROPERTIES_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->keyField, 13);
    EXPECT_EQ(t->currentOffset, -2145493247);
    EXPECT_EQ(t->jumpSeconds, -2147413249);
    EXPECT_EQ(t->timeOfNextJump, 0x912478321891LL);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests TRANSPARENT_CLOCK_PORT_DATA_SET managment ID
TEST(Json2msgTest, TRANSPARENT_CLOCK_PORT_DATA_SET)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"TRANSPARENT_CLOCK_PORT_DATA_SET\",\"dataField\":{"
            "\"portIdentity\":"
            "{"
            "  \"clockIdentity\":\"c47d46.fffe.20acae\","
            "  \"portNumber\":1"
            "},"
            "\"transparentClockPortDS\":true,"
            "\"logMinPdelayReqInterval\":-21,"
            "\"peerMeanPathDelay\":-2524141968232996095"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), TRANSPARENT_CLOCK_PORT_DATA_SET);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const TRANSPARENT_CLOCK_PORT_DATA_SET_t *t =
        dynamic_cast<const TRANSPARENT_CLOCK_PORT_DATA_SET_t *>(d);
    ASSERT_NE(t, nullptr);
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    EXPECT_EQ(t->portIdentity, portIdentity);
    EXPECT_EQ(t->flags, 1);
    EXPECT_EQ(t->logMinPdelayReqInterval, -21);
    EXPECT_EQ(t->peerMeanPathDelay.scaledNanoseconds, 0xdcf87240dcd12301LL);
}

// Tests LOG_MIN_PDELAY_REQ_INTERVAL managment ID
TEST(Json2msgTest, LOG_MIN_PDELAY_REQ_INTERVAL)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"LOG_MIN_PDELAY_REQ_INTERVAL\",\"dataField\":{"
            "\"logMinPdelayReqInterval\":9"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), LOG_MIN_PDELAY_REQ_INTERVAL);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const LOG_MIN_PDELAY_REQ_INTERVAL_t *t =
        dynamic_cast<const LOG_MIN_PDELAY_REQ_INTERVAL_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->logMinPdelayReqInterval, 9);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests TRANSPARENT_CLOCK_DEFAULT_DATA_SET managment ID
TEST(Json2msgTest, TRANSPARENT_CLOCK_DEFAULT_DATA_SET)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"TRANSPARENT_CLOCK_DEFAULT_DATA_SET\","
            "\"dataField\":{"
            "\"clockIdentity\":\"c47d46.fffe.20acae\","
            "\"numberPorts\":6010,"
            "\"delayMechanism\":\"NO_MECHANISM\","
            "\"primaryDomain\":18"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), TRANSPARENT_CLOCK_DEFAULT_DATA_SET);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const TRANSPARENT_CLOCK_DEFAULT_DATA_SET_t *t =
        dynamic_cast<const TRANSPARENT_CLOCK_DEFAULT_DATA_SET_t *>(d);
    ASSERT_NE(t, nullptr);
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    EXPECT_EQ(t->clockIdentity, clockId);
    EXPECT_EQ(t->numberPorts, 0x177a);
    EXPECT_EQ(t->delayMechanism, NO_MECHANISM);
    EXPECT_EQ(t->primaryDomain, 18);
}

// Tests PRIMARY_DOMAIN managment ID
TEST(Json2msgTest, PRIMARY_DOMAIN)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"PRIMARY_DOMAIN\",\"dataField\":{"
            "\"primaryDomain\":17"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), PRIMARY_DOMAIN);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const PRIMARY_DOMAIN_t *t = dynamic_cast<const PRIMARY_DOMAIN_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->primaryDomain, 17);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests DELAY_MECHANISM managment ID
TEST(Json2msgTest, DELAY_MECHANISM)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"DELAY_MECHANISM\",\"dataField\":{"
            "\"delayMechanism\":\"P2P\""
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), DELAY_MECHANISM);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const DELAY_MECHANISM_t *t = dynamic_cast<const DELAY_MECHANISM_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->delayMechanism, P2P);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests EXTERNAL_PORT_CONFIGURATION_ENABLED managment ID
TEST(Json2msgTest, EXTERNAL_PORT_CONFIGURATION_ENABLED)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"EXTERNAL_PORT_CONFIGURATION_ENABLED\","
            "\"dataField\":{"
            "\"externalPortConfiguration\":true"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), EXTERNAL_PORT_CONFIGURATION_ENABLED);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const EXTERNAL_PORT_CONFIGURATION_ENABLED_t *t =
        dynamic_cast<const EXTERNAL_PORT_CONFIGURATION_ENABLED_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 1);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests MASTER_ONLY managment ID
TEST(Json2msgTest, MASTER_ONLY)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"MASTER_ONLY\",\"dataField\":{"
            "\"masterOnly\":true"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), MASTER_ONLY);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const MASTER_ONLY_t *t = dynamic_cast<const MASTER_ONLY_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 1);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests HOLDOVER_UPGRADE_ENABLE managment ID
TEST(Json2msgTest, HOLDOVER_UPGRADE_ENABLE)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"HOLDOVER_UPGRADE_ENABLE\",\"dataField\":{"
            "\"holdoverUpgradeDS\":true"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), HOLDOVER_UPGRADE_ENABLE);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const HOLDOVER_UPGRADE_ENABLE_t *t =
        dynamic_cast<const HOLDOVER_UPGRADE_ENABLE_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 1);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests EXT_PORT_CONFIG_PORT_DATA_SET managment ID
TEST(Json2msgTest, EXT_PORT_CONFIG_PORT_DATA_SET)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"EXT_PORT_CONFIG_PORT_DATA_SET\",\"dataField\":{"
            "\"acceptableMasterPortDS\":true,"
            "\"desiredState\":\"PASSIVE\""
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), EXT_PORT_CONFIG_PORT_DATA_SET);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const EXT_PORT_CONFIG_PORT_DATA_SET_t *t =
        dynamic_cast<const EXT_PORT_CONFIG_PORT_DATA_SET_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->flags, 1);
    EXPECT_EQ(t->desiredState, PASSIVE);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests TIME_STATUS_NP managment ID
TEST(Json2msgTest, TIME_STATUS_NP)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"TIME_STATUS_NP\",\"dataField\":{"
            "\"master_offset\":0,"
            "\"ingress_time\":0,"
            "\"cumulativeScaledRateOffset\":0,"
            "\"scaledLastGmPhaseChange\":0,"
            "\"gmTimeBaseIndicator\":0,"
            "\"nanoseconds_msb\":0,"
            "\"nanoseconds_lsb\":0,"
            "\"fractional_nanoseconds\":0,"
            "\"gmPresent\":0,"
            "\"gmIdentity\":\"c47d46.fffe.20acae\""
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), TIME_STATUS_NP);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const TIME_STATUS_NP_t *t = dynamic_cast<const TIME_STATUS_NP_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->master_offset, 0);
    EXPECT_EQ(t->ingress_time, 0);
    EXPECT_EQ(t->cumulativeScaledRateOffset, 0);
    EXPECT_EQ(t->scaledLastGmPhaseChange, 0);
    EXPECT_EQ(t->gmTimeBaseIndicator, 0);
    EXPECT_EQ(t->nanoseconds_msb, 0);
    EXPECT_EQ(t->nanoseconds_lsb, 0);
    EXPECT_EQ(t->fractional_nanoseconds, 0);
    EXPECT_EQ(t->gmPresent, 0);
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    EXPECT_EQ(t->gmIdentity, clockId);
}

// Tests GRANDMASTER_SETTINGS_NP managment ID
TEST(Json2msgTest, GRANDMASTER_SETTINGS_NP)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"GRANDMASTER_SETTINGS_NP\",\"dataField\":{"
            "\"clockQuality\":"
            "{"
            "  \"clockClass\":255,"
            "  \"clockAccuracy\":\"Unknown\","
            "  \"offsetScaledLogVariance\":65535"
            "},"
            "\"currentUtcOffset\":37,"
            "\"leap61\":false,"
            "\"leap59\":false,"
            "\"currentUtcOffsetValid\":false,"
            "\"ptpTimescale\":true,"
            "\"timeTraceable\":false,"
            "\"frequencyTraceable\":false,"
            "\"timeSource\":\"INTERNAL_OSCILLATOR\""
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), GRANDMASTER_SETTINGS_NP);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const GRANDMASTER_SETTINGS_NP_t *t =
        dynamic_cast<const GRANDMASTER_SETTINGS_NP_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->clockQuality.clockClass, 255);
    EXPECT_EQ(t->clockQuality.clockAccuracy, Accurate_Unknown);
    EXPECT_EQ(t->clockQuality.offsetScaledLogVariance, 0xffff);
    EXPECT_EQ(t->currentUtcOffset, 37);
    EXPECT_EQ(t->flags, F_PTP);
    EXPECT_EQ(t->timeSource, INTERNAL_OSCILLATOR);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests PORT_DATA_SET_NP managment ID
TEST(Json2msgTest, PORT_DATA_SET_NP)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"PORT_DATA_SET_NP\",\"dataField\":{"
            "\"neighborPropDelayThresh\":20000000,"
            "\"asCapable\":1"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), PORT_DATA_SET_NP);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const PORT_DATA_SET_NP_t *t = dynamic_cast<const PORT_DATA_SET_NP_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->neighborPropDelayThresh, 20000000);
    EXPECT_EQ(t->asCapable, 1);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests SUBSCRIBE_EVENTS_NP managment ID
TEST(Json2msgTest, SUBSCRIBE_EVENTS_NP)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"SUBSCRIBE_EVENTS_NP\",\"dataField\":{"
            "\"duration\":4660,"
            "\"NOTIFY_PORT_STATE\":true,"
            "\"NOTIFY_TIME_SYNC\":true,"
            "\"NOTIFY_PARENT_DATA_SET\":true,"
            "\"NOTIFY_CMLDS\":true"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), SUBSCRIBE_EVENTS_NP);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const SUBSCRIBE_EVENTS_NP_t *t = dynamic_cast<const SUBSCRIBE_EVENTS_NP_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->duration, 0x1234);
    EXPECT_TRUE(t->getEvent(NOTIFY_PORT_STATE));
    EXPECT_TRUE(t->getEvent(NOTIFY_TIME_SYNC));
    EXPECT_TRUE(t->getEvent(NOTIFY_PARENT_DATA_SET));
    EXPECT_TRUE(t->getEvent(NOTIFY_CMLDS));
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests PORT_PROPERTIES_NP managment ID
TEST(Json2msgTest, PORT_PROPERTIES_NP)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"PORT_PROPERTIES_NP\",\"dataField\":{"
            "\"portIdentity\":"
            "{"
            "  \"clockIdentity\":\"c47d46.fffe.20acae\","
            "  \"portNumber\":1"
            "},"
            "\"portState\":\"LISTENING\","
            "\"timestamping\":\"HARDWARE\","
            "\"interface\":\"enp0s25\""
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), PORT_PROPERTIES_NP);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const PORT_PROPERTIES_NP_t *t = dynamic_cast<const PORT_PROPERTIES_NP_t *>(d);
    ASSERT_NE(t, nullptr);
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    EXPECT_EQ(t->portIdentity, portIdentity);
    EXPECT_EQ(t->portState, LISTENING);
    EXPECT_EQ(t->timestamping, TS_HARDWARE);
    EXPECT_STREQ(t->interface.string(), "enp0s25");
}

// Tests PORT_STATS_NP managment ID
TEST(Json2msgTest, PORT_STATS_NP)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"PORT_STATS_NP\",\"dataField\":{"
            "\"portIdentity\":"
            "{"
            "  \"clockIdentity\":\"c47d46.fffe.20acae\","
            "  \"portNumber\":1"
            "},"
            "\"rx_Sync\":0,"
            "\"rx_Delay_Req\":0,"
            "\"rx_Pdelay_Req\":0,"
            "\"rx_Pdelay_Resp\":0,"
            "\"rx_Follow_Up\":0,"
            "\"rx_Delay_Resp\":0,"
            "\"rx_Pdelay_Resp_Follow_Up\":0,"
            "\"rx_Announce\":0,"
            "\"rx_Signaling\":0,"
            "\"rx_Management\":0,"
            "\"tx_Sync\":0,"
            "\"tx_Delay_Req\":0,"
            "\"tx_Pdelay_Req\":63346,"
            "\"tx_Pdelay_Resp\":0,"
            "\"tx_Follow_Up\":0,"
            "\"tx_Delay_Resp\":0,"
            "\"tx_Pdelay_Resp_Follow_Up\":0,"
            "\"tx_Announce\":0,"
            "\"tx_Signaling\":0,"
            "\"tx_Management\":0"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), PORT_STATS_NP);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const PORT_STATS_NP_t *t = dynamic_cast<const PORT_STATS_NP_t *>(d);
    ASSERT_NE(t, nullptr);
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    EXPECT_EQ(t->portIdentity, portIdentity);
    EXPECT_EQ(t->rxMsgType[STAT_SYNC], 0);
    EXPECT_EQ(t->rxMsgType[STAT_DELAY_REQ], 0);
    EXPECT_EQ(t->rxMsgType[STAT_PDELAY_REQ], 0);
    EXPECT_EQ(t->rxMsgType[STAT_PDELAY_RESP], 0);
    EXPECT_EQ(t->rxMsgType[STAT_FOLLOW_UP], 0);
    EXPECT_EQ(t->rxMsgType[STAT_DELAY_RESP], 0);
    EXPECT_EQ(t->rxMsgType[STAT_PDELAY_RESP_FOLLOW_UP], 0);
    EXPECT_EQ(t->rxMsgType[STAT_ANNOUNCE], 0);
    EXPECT_EQ(t->rxMsgType[STAT_SIGNALING], 0);
    EXPECT_EQ(t->rxMsgType[STAT_MANAGEMENT], 0);
    EXPECT_EQ(t->txMsgType[STAT_SYNC], 0);
    EXPECT_EQ(t->txMsgType[STAT_DELAY_REQ], 0);
    EXPECT_EQ(t->txMsgType[STAT_PDELAY_REQ], 63346);
    EXPECT_EQ(t->txMsgType[STAT_PDELAY_RESP], 0);
    EXPECT_EQ(t->txMsgType[STAT_FOLLOW_UP], 0);
    EXPECT_EQ(t->txMsgType[STAT_DELAY_RESP], 0);
    EXPECT_EQ(t->txMsgType[STAT_PDELAY_RESP_FOLLOW_UP], 0);
    EXPECT_EQ(t->txMsgType[STAT_ANNOUNCE], 0);
    EXPECT_EQ(t->txMsgType[STAT_SIGNALING], 0);
    EXPECT_EQ(t->txMsgType[STAT_MANAGEMENT], 0);
}

// Tests SYNCHRONIZATION_UNCERTAIN_NP managment ID
TEST(Json2msgTest, SYNCHRONIZATION_UNCERTAIN_NP)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"SYNCHRONIZATION_UNCERTAIN_NP\",\"dataField\":{"
            "\"val\":255"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), SYNCHRONIZATION_UNCERTAIN_NP);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const SYNCHRONIZATION_UNCERTAIN_NP_t *t =
        dynamic_cast<const SYNCHRONIZATION_UNCERTAIN_NP_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->val, SYNC_UNCERTAIN_DONTCARE);
    Message msg;
    EXPECT_TRUE(m.setAction(msg));
}

// Tests PORT_SERVICE_STATS_NP managment ID
TEST(Json2msgTest, PORT_SERVICE_STATS_NP)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"PORT_SERVICE_STATS_NP\",\"dataField\":{"
            "\"portIdentity\":"
            "{"
            "  \"clockIdentity\":\"c47d46.fffe.20acae\","
            "  \"portNumber\":1"
            "},"
            "\"announce_timeout\":9041,"
            "\"sync_timeout\":0,"
            "\"delay_timeout\":63346,"
            "\"unicast_service_timeout\":0,"
            "\"unicast_request_timeout\":0,"
            "\"master_announce_timeout\":0,"
            "\"master_sync_timeout\":0,"
            "\"qualification_timeout\":0,"
            "\"sync_mismatch\":0,"
            "\"followup_mismatch\":0"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), PORT_SERVICE_STATS_NP);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const PORT_SERVICE_STATS_NP_t *t =
        dynamic_cast<const PORT_SERVICE_STATS_NP_t *>(d);
    ASSERT_NE(t, nullptr);
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    EXPECT_EQ(t->portIdentity, portIdentity);
    EXPECT_EQ(t->announce_timeout, 9041);
    EXPECT_EQ(t->sync_timeout, 0);
    EXPECT_EQ(t->delay_timeout, 63346);
    EXPECT_EQ(t->unicast_service_timeout, 0);
    EXPECT_EQ(t->unicast_request_timeout, 0);
    EXPECT_EQ(t->master_announce_timeout, 0);
    EXPECT_EQ(t->master_sync_timeout, 0);
    EXPECT_EQ(t->qualification_timeout, 0);
    EXPECT_EQ(t->sync_mismatch, 0);
    EXPECT_EQ(t->followup_mismatch, 0);
}

// Tests UNICAST_MASTER_TABLE_NP managment ID
TEST(Json2msgTest, UNICAST_MASTER_TABLE_NP)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"UNICAST_MASTER_TABLE_NP\",\"dataField\":{"
            "\"actualTableSize\":1,"
            "\"unicastMasters\":"
            "["
            "  {"
            "    \"portIdentity\":"
            "    {"
            "      \"clockIdentity\":\"c47d46.fffe.20acae\","
            "      \"portNumber\":1"
            "    },"
            "    \"clockQuality\":"
            "    {"
            "      \"clockClass\":255,"
            "      \"clockAccuracy\":\"Unknown\","
            "      \"offsetScaledLogVariance\":65535"
            "    },"
            "    \"selected\":true,"
            "    \"portState\":\"HAVE_SYDY\","
            "    \"priority1\":126,"
            "    \"priority2\":134,"
            "    \"portAddress\":"
            "    {"
            "      \"networkProtocol\":\"IEEE_802_3\","
            "      \"addressField\":\"c4:7d:46:20:ac:ae\""
            "    }"
            "  }"
            "]"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), UNICAST_MASTER_TABLE_NP);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const UNICAST_MASTER_TABLE_NP_t *t =
        dynamic_cast<const UNICAST_MASTER_TABLE_NP_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->actualTableSize, 1);
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    EXPECT_EQ(t->unicastMasters[0].portIdentity, portIdentity);
    EXPECT_EQ(t->unicastMasters[0].clockQuality.clockClass, 255);
    EXPECT_EQ(t->unicastMasters[0].clockQuality.clockAccuracy, Accurate_Unknown);
    EXPECT_EQ(t->unicastMasters[0].clockQuality.offsetScaledLogVariance, 0xffff);
    EXPECT_EQ(t->unicastMasters[0].selected, 1); // true => 1
    EXPECT_EQ(t->unicastMasters[0].portState, UC_HAVE_SYDY);
    EXPECT_EQ(t->unicastMasters[0].priority1, 126);
    EXPECT_EQ(t->unicastMasters[0].priority2, 134);
    Binary physicalAddress = Binary("\xc4\x7d\x46\x20\xac\xae", 6);
    PortAddress_t portAddress = { IEEE_802_3, 6, physicalAddress };
    EXPECT_EQ(t->unicastMasters[0].portAddress, portAddress);
}

// Tests PORT_HWCLOCK_NP managment ID
TEST(Json2msgTest, PORT_HWCLOCK_NP)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"PORT_HWCLOCK_NP\",\"dataField\":{"
            "\"portIdentity\":"
            "{"
            "  \"clockIdentity\":\"c47d46.fffe.20acae\","
            "  \"portNumber\":1"
            "},"
            "\"phc_index\":1,"
            "\"flags\":7"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), PORT_HWCLOCK_NP);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const PORT_HWCLOCK_NP_t *t = dynamic_cast<const PORT_HWCLOCK_NP_t *>(d);
    ASSERT_NE(t, nullptr);
    ClockIdentity_t clockId = { 196, 125, 70, 255, 254, 32, 172, 174 };
    PortIdentity_t portIdentity = { clockId, 1 };
    EXPECT_EQ(t->portIdentity, portIdentity);
    EXPECT_EQ(t->phc_index, 1);
    EXPECT_EQ(t->flags, 7);
}

// Tests POWER_PROFILE_SETTINGS_NP managment ID
TEST(Json2msgTest, POWER_PROFILE_SETTINGS_NP)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"POWER_PROFILE_SETTINGS_NP\",\"dataField\":{"
            "\"version\":\"2011\","
            "\"grandmasterID\":56230,"
            "\"grandmasterTimeInaccuracy\":4124796349,"
            "\"networkTimeInaccuracy\":3655058877,"
            "\"totalTimeInaccuracy\":4223530875"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), POWER_PROFILE_SETTINGS_NP);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const POWER_PROFILE_SETTINGS_NP_t *t =
        dynamic_cast<const POWER_PROFILE_SETTINGS_NP_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->version, IEEE_C37_238_VERSION_2011);
    EXPECT_EQ(t->grandmasterID, 56230);
    EXPECT_EQ(t->grandmasterTimeInaccuracy, 4124796349);
    EXPECT_EQ(t->networkTimeInaccuracy, 3655058877);
    EXPECT_EQ(t->totalTimeInaccuracy, 4223530875);
}

// Tests CMLDS_INFO_NP managment ID
TEST(Json2msgTest, CMLDS_INFO_NP)
{
    Json2msg m;
    ASSERT_TRUE(m.fromJson("{\"actionField\":\"SET\","
            "\"managementId\":\"CMLDS_INFO_NP\",\"dataField\":{"
            "\"meanLinkDelay\":201548321,"
            "\"scaledNeighborRateRatio\":1842,"
            "\"as_capable\":1"
            "}}"));
    EXPECT_EQ(m.actionField(), SET);
    EXPECT_EQ(m.managementId(), CMLDS_INFO_NP);
    const BaseMngTlv *d = m.dataField();
    ASSERT_NE(d, nullptr);
    const CMLDS_INFO_NP_t *t = dynamic_cast<const CMLDS_INFO_NP_t *>(d);
    ASSERT_NE(t, nullptr);
    EXPECT_EQ(t->meanLinkDelay.scaledNanoseconds, 201548321LL);
    EXPECT_EQ(t->scaledNeighborRateRatio, 1842);
    EXPECT_EQ(t->as_capable, 1);
}
