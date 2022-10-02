/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2022 Erez Geva */

/** @file
 * @brief MessageDispatcher and MessageBulder classes unit test
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2022 Erez Geva
 *
 */

#include "msgCall.h"

using namespace ptpmgmt;

class MessageDispatcherTest : public ::testing::Test, public MessageDispatcher
{
  protected:
    Message msg;
    PRIORITY1_t tlv;
    std::string idStr_PRIORITY1;
    std::string idStr_noTlvCallBack;
    bool noTlvCalled;

    void PRIORITY1_h(const Message &msg, const PRIORITY1_t &l_tlv,
        const char *idStr) override {
        tlv.priority1 = l_tlv.priority1;
        idStr_PRIORITY1 = idStr;
    }
    void noTlv(const Message &msg) override {
        noTlvCalled = true;
    }
    void noTlvCallBack(const Message &msg, const char *idStr) override {
        idStr_noTlvCallBack = idStr;
    }
};

// Tests callHadler method with paresed empty TLV
// void callHadler(const Message &msg)
TEST_F(MessageDispatcherTest, MethodParsedCallHadlerEmptyTLV)
{
    EXPECT_TRUE(msg.setAction(COMMAND, ENABLE_PORT));
    uint8_t buf[70];
    EXPECT_EQ(msg.build(buf, sizeof buf, 1), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of command message
    buf[46] = ACKNOWLEDGE;
    EXPECT_EQ(msg.parse(buf, 54), MNG_PARSE_ERROR_OK);
    noTlvCalled = false;
    // Dispatch ENABLE_PORT
    callHadler(msg);
    // ENABLE_PORT is an empty code, which do not have a TLV, so noTlv is called!
    EXPECT_TRUE(noTlvCalled);
}

// Tests callHadler with method with provided empty TLV
// void callHadler(const Message &msg, mng_vals_e tlv_id, const BaseMngTlv *tlv)
TEST_F(MessageDispatcherTest, MethodProvidedCallHadlerEmptyTLV)
{
    noTlvCalled = false;
    // Dispatch ENABLE_PORT
    callHadler(msg, ENABLE_PORT, nullptr);
    // ENABLE_PORT is an empty code, which do not have a TLV, so noTlv is called!
    EXPECT_TRUE(noTlvCalled);
    BaseMngTlv t;
    noTlvCalled = false;
    // Dispatch ENABLE_PORT
    callHadler(msg, ENABLE_PORT, &t);
    EXPECT_TRUE(noTlvCalled);
}

// Tests callHadler method with paresed TLV
// void callHadler(const Message &msg)
TEST_F(MessageDispatcherTest, MethodParsedCallHadlerTLV)
{
    PRIORITY1_t p;
    p.priority1 = 137;
    tlv.priority1 = 0;
    noTlvCalled = false;
    EXPECT_TRUE(msg.setAction(SET, PRIORITY1, &p));
    uint8_t buf[70];
    EXPECT_EQ(msg.build(buf, sizeof buf, 137), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    EXPECT_EQ(msg.parse(buf, 56), MNG_PARSE_ERROR_OK);
    // Dispatch PRIORITY1 in parsed message
    callHadler(msg);
    // PRIORITY1 have callback
    EXPECT_FALSE(noTlvCalled);
    EXPECT_EQ(tlv.priority1, p.priority1);
    EXPECT_STREQ(idStr_PRIORITY1.c_str(), "PRIORITY1");
}

// Tests callHadler with method with provided TLV
// void callHadler(const Message &msg, mng_vals_e tlv_id, const BaseMngTlv *tlv)
TEST_F(MessageDispatcherTest, MethodProvidedCallHadlerTLV)
{
    PRIORITY1_t p;
    p.priority1 = 137;
    tlv.priority1 = 0;
    noTlvCalled = false;
    // Dispatch PRIORITY1
    callHadler(msg, PRIORITY1, &p);
    // PRIORITY1 have callback
    EXPECT_FALSE(noTlvCalled);
    EXPECT_EQ(tlv.priority1, p.priority1);
    EXPECT_STREQ(idStr_PRIORITY1.c_str(), "PRIORITY1");
}

// Tests callHadler method with paresed TLV without callback
// void callHadler(const Message &msg)
TEST_F(MessageDispatcherTest, MethodParsedCallHadlerTLVNoCallback)
{
    PRIORITY2_t p;
    p.priority2 = 137;
    tlv.priority1 = 0;
    noTlvCalled = false;
    EXPECT_TRUE(msg.setAction(SET, PRIORITY2, &p));
    uint8_t buf[70];
    EXPECT_EQ(msg.build(buf, sizeof buf, 137), MNG_PARSE_ERROR_OK);
    // actionField location IEEE "PTP management message"
    // Change to response action of get/set message
    buf[46] = RESPONSE;
    EXPECT_EQ(msg.parse(buf, 56), MNG_PARSE_ERROR_OK);
    // Dispatch PRIORITY2 in parsed message
    callHadler(msg);
    // PRIORITY2 do not have callback
    EXPECT_FALSE(noTlvCalled);
    EXPECT_EQ(tlv.priority1, 0);
    EXPECT_STREQ(idStr_PRIORITY1.c_str(), "");
    EXPECT_STREQ(idStr_noTlvCallBack.c_str(), "PRIORITY2");
}

// Tests callHadler with method with provided TLV without callback
// void callHadler(const Message &msg, mng_vals_e tlv_id, const BaseMngTlv *tlv)
TEST_F(MessageDispatcherTest, MethodProvidedCallHadlerTLVNoCallback)
{
    PRIORITY2_t p;
    p.priority2 = 137;
    tlv.priority1 = 0;
    noTlvCalled = false;
    // Dispatch PRIORITY2
    callHadler(msg, PRIORITY2, &p);
    // PRIORITY2 do not have callback
    EXPECT_FALSE(noTlvCalled);
    EXPECT_EQ(tlv.priority1, 0);
    EXPECT_STREQ(idStr_PRIORITY1.c_str(), "");
    EXPECT_STREQ(idStr_noTlvCallBack.c_str(), "PRIORITY2");
}

class MessageBulderTest : public ::testing::Test, public MessageBulder
{
  protected:
    bool have_priority1;
    Message msg;
    MessageBulderTest() : MessageBulder(msg), have_priority1(false) {}

    bool PRIORITY1_b(const Message &msg, PRIORITY1_t &tlv) override {
        tlv.priority1 = 97;
        have_priority1 = true;
        return true;
    }
};

// Tests get Message method
//  Message &getMsg()
TEST_F(MessageBulderTest, MethodGetMsg)
{
    // The address of the referece should be equal to the address of the original
    EXPECT_EQ(&msg, &(getMsg()));
}

// Tests build empty TLV
//  bool buildTlv(actionField_e actionField, mng_vals_e tlv_id)
TEST_F(MessageBulderTest, BuildEmptyTLV)
{
    EXPECT_TRUE(buildTlv(COMMAND, ENABLE_PORT));
    EXPECT_FALSE(have_priority1);
}

// Tests build TLV that lack callback
//  bool buildTlv(actionField_e actionField, mng_vals_e tlv_id)
TEST_F(MessageBulderTest, BuildNOCallback)
{
    EXPECT_FALSE(buildTlv(SET, PRIORITY2));
    EXPECT_FALSE(have_priority1);
}

// Tests build TLV that have callback
//  bool buildTlv(actionField_e actionField, mng_vals_e tlv_id)
TEST_F(MessageBulderTest, BuildWithCallback)
{
    EXPECT_TRUE(buildTlv(SET, PRIORITY1));
    EXPECT_TRUE(have_priority1);
}
