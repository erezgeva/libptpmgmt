/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief types structures unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

#include "types.h"

using namespace ptpmgmt;

/*****************************************************************************/
// Unit tests for struct TimeInterval_t

// Tests size method
// static size_t size()
TEST(TimeIntervalTest, MethodSize)
{
    EXPECT_EQ(TimeInterval_t::size(), 8);
}

// Tests get interval method
// double getInterval() const
TEST(TimeIntervalTest, MethodGetInterval)
{
    TimeInterval_t t = { -0x4000 };
    EXPECT_EQ(t.getInterval(), -0.25);
    t = { 0x6000 };
    EXPECT_EQ(t.getInterval(), 0.375);
}

// Tests get integer interval method
// int64_t getIntervalInt() const
TEST(TimeIntervalTest, MethodGetIntervalInt)
{
    TimeInterval_t t = { -0x24000 };
    EXPECT_EQ(t.getIntervalInt(), -2);
    t = { 0x36000 };
    EXPECT_EQ(t.getIntervalInt(), 3);
}

/*****************************************************************************/
// Unit tests for struct Timestamp_t

// Tests empty constructor
// Timestamp_t()
TEST(TimeStampTest, MethodEmptyConstructor)
{
    Timestamp_t t;
    EXPECT_EQ(t.secondsField, 0);
    EXPECT_EQ(t.nanosecondsField, 0);
}

// Tests copy constructor
// Timestamp_t(const Timestamp_t &ts)
TEST(TimeStampTest, MethodCopyConstructor)
{
    Timestamp_t f = {13, 25000};
    Timestamp_t t(f);
    EXPECT_EQ(t.secondsField, 13);
    EXPECT_EQ(t.nanosecondsField, 25000);
}

// Tests full constructor
// Timestamp_t(int64_t secs, uint32_t nsecs)
TEST(TimeStampTest, MethodFullConstructor)
{
    Timestamp_t t(13, 25000);
    EXPECT_EQ(t.secondsField, 13);
    EXPECT_EQ(t.nanosecondsField, 25000);
}

// Tests size method
// static size_t size()
TEST(TimeStampTest, MethodSize)
{
    EXPECT_EQ(Timestamp_t::size(), 10);
}

// Tests string method
// std::string string() const
TEST(TimeStampTest, MethodString)
{
    Timestamp_t t = {13, 25000};
    EXPECT_STREQ(t.string().c_str(), "13.000025000");
}

// Tests string operator
// operator std::string() const
TEST(TimeStampTest, MethodStringOp)
{
    Timestamp_t t = {17, 125000};
    EXPECT_STREQ(((std::string)t).c_str(), "17.000125000");
    std::string s = t;
    EXPECT_STREQ(s.c_str(), "17.000125000");
}

// Tests convert from timespec constructor
// Timestamp_t(const timespec &ts)
TEST(TimeStampTest, MethodTimespecConstructor)
{
    timespec ts = { 13, 125 };
    Timestamp_t t(ts);
    EXPECT_EQ(t.secondsField, 13);
    EXPECT_EQ(t.nanosecondsField, 125);
    ts = { 17, 124 };
    t = ts;
    EXPECT_EQ(t.secondsField, 17);
    EXPECT_EQ(t.nanosecondsField, 124);
}

// Tests convert to timespec with operator
// operator timespec() const
TEST(TimeStampTest, MethodToTimespecOp)
{
    Timestamp_t t = { 13, 125 };
    timespec ts = t;
    EXPECT_EQ(ts.tv_sec, 13);
    EXPECT_EQ(ts.tv_nsec, 125);
    EXPECT_EQ(((timespec)t).tv_sec, 13);
    EXPECT_EQ(((timespec)t).tv_nsec, 125);
}

// Tests convert to timespec
// void toTimespec(timespec &ts) const
TEST(TimeStampTest, MethodToTimespec)
{
    Timestamp_t t = { 13, 125 };
    timespec ts;
    t.toTimespec(ts);
    EXPECT_EQ(ts.tv_sec, 13);
    EXPECT_EQ(ts.tv_nsec, 125);
}

// Tests convert from timeval constructor
// Timestamp_t(const timeval &tv)
TEST(TimeStampTest, MethodTimevalConstructor)
{
    timeval ts = { 13, 125 };
    Timestamp_t t(ts);
    EXPECT_EQ(t.secondsField, 13);
    EXPECT_EQ(t.nanosecondsField, 125000);
    ts = { 17, 124 };
    t = ts;
    EXPECT_EQ(t.secondsField, 17);
    EXPECT_EQ(t.nanosecondsField, 124000);
}

// Tests convert to timeval with operator
// operator timeval() const
TEST(TimeStampTest, MethodToTimevalOp)
{
    Timestamp_t t = { 13, 125045 };
    timeval ts = t;
    EXPECT_EQ(ts.tv_sec, 13);
    EXPECT_EQ(ts.tv_usec, 125);
    EXPECT_EQ(((timeval)t).tv_sec, 13);
    EXPECT_EQ(((timeval)t).tv_usec, 125);
}

// Tests convert to timespec
// void toTimeval(timeval &tv) const
TEST(TimeStampTest, MethodToTimeval)
{
    Timestamp_t t = { 13, 175019 };
    timeval ts;
    t.toTimeval(ts);
    EXPECT_EQ(ts.tv_sec, 13);
    EXPECT_EQ(ts.tv_usec, 175);
}

// Tests convert from seconds constructor
// Timestamp_t(float_seconds seconds)
TEST(TimeStampTest, MethodFromSecondsConstructor)
{
    Timestamp_t t(39.000000654);
    EXPECT_EQ(t.secondsField, 39);
    EXPECT_NEAR(t.nanosecondsField, 654, 1);
    t = 17.000000834;
    EXPECT_EQ(t.secondsField, 17);
    EXPECT_NEAR(t.nanosecondsField, 834, 1);
}

// Tests convert from seconds
// void fromFloat(float_seconds seconds)
TEST(TimeStampTest, MethodFromSeconds)
{
    Timestamp_t t;
    t.fromFloat(53.000000654);
    EXPECT_EQ(t.secondsField, 53);
    EXPECT_NEAR(t.nanosecondsField, 654, 1);
}

// Tests convert to seconds using the casting operator
// operator float_seconds() const
TEST(TimeStampTest, MethodToSecondsOp)
{
    Timestamp_t t = { 53, 654 };
    float_seconds d = t;
    EXPECT_DOUBLE_EQ(d, 53.000000654);
    EXPECT_DOUBLE_EQ((float_seconds)t, 53.000000654);
}

// Tests convert to seconds
// float_seconds toFloat() const
TEST(TimeStampTest, MethodToSeconds)
{
    Timestamp_t t = { 53, 654 };
    EXPECT_DOUBLE_EQ(t.toFloat(), 53.000000654);
}

// Tests convert from nanoseconds
// void fromNanoseconds(uint64_t nanoseconds)
TEST(TimeStampTest, MethodFromNanoseconds)
{
    Timestamp_t t;
    t.fromNanoseconds(53000000654);
    EXPECT_EQ(t.secondsField, 53);
    EXPECT_EQ(t.nanosecondsField, 654);
}

// Tests convert to nanoseconds
// uint64_t toNanoseconds() const
TEST(TimeStampTest, MethodToNanoseconds)
{
    Timestamp_t t = { 53, 654 };
    EXPECT_EQ(t.toNanoseconds(), 53000000654);
}

// Tests equal operator
// bool operator==(const Timestamp_t &ts) const
TEST(TimeStampTest, MethodEqOp)
{
    Timestamp_t t1 = { 53, 654 };
    Timestamp_t t2 = { 53, 654 };
    Timestamp_t t3 = { 53, 694 };
    EXPECT_TRUE(t1 == t2);
    EXPECT_FALSE(t1 == t3);
}

// Tests equal method
// bool eq(const Timestamp_t &ts) const
TEST(TimeStampTest, MethodEq)
{
    Timestamp_t t1 = { 53, 654 };
    Timestamp_t t2 = { 53, 654 };
    Timestamp_t t3 = { 53, 694 };
    EXPECT_TRUE(t1.eq(t2));
    EXPECT_FALSE(t1.eq(t3));
}

// Tests equal to seconds with operator
// bool operator==(float_seconds seconds) const
TEST(TimeStampTest, MethodEqSecondsOp)
{
    Timestamp_t t = { 53, 230000000 };
    float_seconds s = 53.23;
    EXPECT_TRUE(t == s);
}

// Tests equal to seconds
// bool eq(float_seconds seconds) const
TEST(TimeStampTest, MethodEqSeconds)
{
    Timestamp_t t = { 53, 230000000 };
    float_seconds s = 53.23;
    EXPECT_TRUE(t.eq(s));
}

// Tests less operator
// bool operator<(const Timestamp_t &ts) const
TEST(TimeStampTest, MethodLessOp)
{
    Timestamp_t t1 = { 53, 653 };
    Timestamp_t t2 = { 53, 654 };
    EXPECT_TRUE(t1 < t2);
}

// Tests less method
// bool less(const Timestamp_t &ts) const {
TEST(TimeStampTest, MethodLess)
{
    Timestamp_t t1 = { 53, 653 };
    Timestamp_t t2 = { 53, 654 };
    EXPECT_TRUE(t1.less(t2));
}

// Tests less to seconds with operator
// bool operator<(float_seconds seconds) const
TEST(TimeStampTest, MethodLessSecondsOp)
{
    Timestamp_t t = { 53, 230561233 };
    float_seconds s = 53.230561234;
    EXPECT_TRUE(t < s);
}

// Tests less to seconds
// bool less(float_seconds seconds) const
TEST(TimeStampTest, MethodLessSeconds)
{
    Timestamp_t t = { 53, 230561233 };
    float_seconds s = 53.230561234;
    EXPECT_TRUE(t.less(s));
}

// Tests plus operator
// Timestamp_t &operator+(const Timestamp_t &ts)
TEST(TimeStampTest, MethodPlusOp)
{
    Timestamp_t t1 = { 17, 930000000 };
    Timestamp_t t2 = { 24, 540000000 };
    Timestamp_t t = t1 + t2;
    EXPECT_EQ(t.secondsField, 42);
    EXPECT_EQ(t.nanosecondsField, 470000000);
}

// Tests plus assign operator
// Timestamp_t &operator+=(const Timestamp_t &ts)
TEST(TimeStampTest, MethodPlusAssignOp)
{
    Timestamp_t t = { 17, 930000000 };
    Timestamp_t t2 = { 24, 540000000 };
    t += t2;
    EXPECT_EQ(t.secondsField, 42);
    EXPECT_EQ(t.nanosecondsField, 470000000);
}

// Tests add method
// Timestamp_t &add(const Timestamp_t &ts)
TEST(TimeStampTest, MethodAdd)
{
    Timestamp_t t = { 17, 930000000 };
    Timestamp_t t2 = { 24, 540000000 };
    t.add(t2);
    EXPECT_EQ(t.secondsField, 42);
    EXPECT_EQ(t.nanosecondsField, 470000000);
}

// Tests add seconds with plus operator
// Timestamp_t &operator+(float_seconds seconds)
TEST(TimeStampTest, MethodPlusSecondsOp)
{
    Timestamp_t t1 = { 17, 930000012 };
    float_seconds s = 24.540000045;
    Timestamp_t t = t1 + s;
    EXPECT_EQ(t.secondsField, 42);
    EXPECT_NEAR(t.nanosecondsField, 470000057, 1);
}

// Tests add seconds with plus operator
// Timestamp_t &add(float_seconds seconds)
TEST(TimeStampTest, MethodPlusSeconds)
{
    Timestamp_t t = { 17, 930000012 };
    float_seconds s = 24.540000045;
    t.add(s);
    EXPECT_EQ(t.secondsField, 42);
    EXPECT_NEAR(t.nanosecondsField, 470000057, 1);
}

// Tests minus operator
// Timestamp_t &operator-(const Timestamp_t &ts)
TEST(TimeStampTest, MethodMinusOp)
{
    Timestamp_t t1 = { 42, 470000000 };
    Timestamp_t t2 = { 24, 540000000 };
    Timestamp_t t = t1 - t2;
    EXPECT_EQ(t.secondsField, 17);
    EXPECT_EQ(t.nanosecondsField, 930000000);
}

// Tests minus assign operator
// Timestamp_t &operator-=(const Timestamp_t &ts)
TEST(TimeStampTest, MethodMinusAssignOp)
{
    Timestamp_t t = { 42, 470000000 };
    Timestamp_t t2 = { 24, 540000000 };
    t -= t2;
    EXPECT_EQ(t.secondsField, 17);
    EXPECT_EQ(t.nanosecondsField, 930000000);
}

// Tests minus method
// Timestamp_t &subt(const Timestamp_t &ts)
TEST(TimeStampTest, MethodMinus)
{
    Timestamp_t t = { 42, 470000000 };
    Timestamp_t t2 = { 24, 540000000 };
    t.subt(t2);
    EXPECT_EQ(t.secondsField, 17);
    EXPECT_EQ(t.nanosecondsField, 930000000);
}

// Tests minus seconds operator
// Timestamp_t &operator-(float_seconds seconds)
TEST(TimeStampTest, MethodMinusSecondsOp)
{
    Timestamp_t t1 = { 42, 470000000 };
    float_seconds s = 24.540000000;
    Timestamp_t t = t1 - s;
    EXPECT_EQ(t.secondsField, 17);
    EXPECT_EQ(t.nanosecondsField, 930000000);
}

// Tests minus assign seconds operator
// Timestamp_t &operator-=(float_seconds seconds)
TEST(TimeStampTest, MethodMinusAssignSecondsOp)
{
    Timestamp_t t = { 42, 470000000 };
    float_seconds s = 24.540000000;
    t -= s;
    EXPECT_EQ(t.secondsField, 17);
    EXPECT_EQ(t.nanosecondsField, 930000000);
}

// Tests minus seconds method
// Timestamp_t &subt(float_seconds seconds)
TEST(TimeStampTest, MethodMinusSeconds)
{
    Timestamp_t t = { 42, 470000000 };
    float_seconds s = 24.540000000;
    t.subt(s);
    EXPECT_EQ(t.secondsField, 17);
    EXPECT_EQ(t.nanosecondsField, 930000000);
}

/*****************************************************************************/
// Unit tests for struct ClockIdentity_t

// Tests size method
// static size_t size()
TEST(ClockIdentityTest, MethodSize)
{
    EXPECT_EQ(ClockIdentity_t::size(), 8);
}

// Tests string method
// std::string string() const
TEST(ClockIdentityTest, MethodString)
{
    ClockIdentity_t t = { 1, 2, 3, 4, 5, 6, 7, 8 };
    EXPECT_STREQ(t.string().c_str(), "010203.0405.060708");
}

// Tests clear method
// void clear(int val = 0)
TEST(ClockIdentityTest, MethodClear)
{
    ClockIdentity_t t = { 1, 2, 3, 4, 5, 6, 7, 8 };
    t.clear();
    EXPECT_STREQ(t.string().c_str(), "000000.0000.000000");
    t.clear(7);
    EXPECT_STREQ(t.string().c_str(), "070707.0707.070707");
}

// Tests equal operator method
// bool operator==(const ClockIdentity_t &rhs) const
TEST(ClockIdentityTest, MethodEqOp)
{
    ClockIdentity_t t1 = { 1, 2, 3, 4, 5, 6, 7, 8 };
    ClockIdentity_t t2 = { 1, 2, 3, 4, 5, 6, 7, 8 };
    EXPECT_TRUE(t1 == t2);
}

// Tests operator method
// bool eq(const ClockIdentity_t &rhs) const
TEST(ClockIdentityTest, MethodEq)
{
    ClockIdentity_t t1 = { 1, 2, 3, 4, 5, 6, 7, 8 };
    ClockIdentity_t t2 = { 1, 2, 3, 4, 5, 6, 7, 8 };
    EXPECT_TRUE(t1.eq(t2));
}

// Tests less operator method
// bool operator<(const ClockIdentity_t &rhs) const
TEST(ClockIdentityTest, MethodLessOp)
{
    ClockIdentity_t t1 = { 1, 2, 3, 4, 3, 6, 7, 8 };
    ClockIdentity_t t2 = { 1, 2, 3, 4, 5, 6, 7, 8 };
    EXPECT_TRUE(t1 < t2);
}

// Tests less method
// bool less(const ClockIdentity_t &rhs) const
TEST(ClockIdentityTest, MethodLess)
{
    ClockIdentity_t t1 = { 1, 2, 3, 4, 3, 6, 7, 8 };
    ClockIdentity_t t2 = { 1, 2, 3, 4, 5, 6, 7, 8 };
    EXPECT_TRUE(t1.less(t2));
}

// Tests equal operator method with binary
// bool operator==(const Binary &bin) const
TEST(ClockIdentityTest, MethodEqBinOp)
{
    ClockIdentity_t t = { 1, 2, 3, 4, 5, 6, 7, 8 };
    Binary b("\x1\x2\x3\x4\x5\x6\x7\x8");
    EXPECT_TRUE(t == b);
}

// Tests operator method with binary
// bool eq(const Binary &bin) const
TEST(ClockIdentityTest, MethodEqBin)
{
    ClockIdentity_t t = { 1, 2, 3, 4, 5, 6, 7, 8 };
    Binary b("\x1\x2\x3\x4\x5\x6\x7\x8");
    EXPECT_TRUE(t.eq(b));
}

/*****************************************************************************/
// Unit tests for struct PortIdentity_t

// Tests size method
// static size_t size()
TEST(PortIdentityTest, MethodSize)
{
    EXPECT_EQ(PortIdentity_t::size(), 10);
}

// Tests string method
// std::string string() const
TEST(PortIdentityTest, MethodString)
{
    PortIdentity_t t = { { 1, 2, 3, 4, 5, 6, 7, 8 }, 1234 };
    EXPECT_STREQ(t.string().c_str(), "010203.0405.060708-1234");
}

// Tests clear method
// void clear()
TEST(PortIdentityTest, MethodClear)
{
    PortIdentity_t t = { { 1, 2, 3, 4, 5, 6, 7, 8 }, 1234 };
    t.clear();
    EXPECT_STREQ(t.string().c_str(), "000000.0000.000000-0");
}

// Tests equal operator method
// bool operator==(const PortIdentity_t &rhs)
TEST(PortIdentityTest, MethodEqOp)
{
    PortIdentity_t t1 = { { 1, 2, 3, 4, 5, 6, 7, 8 }, 1234 };
    PortIdentity_t t2 = { { 1, 2, 3, 4, 5, 6, 7, 8 }, 1234 };
    EXPECT_TRUE(t1 == t2);
}

// Tests equal method
// bool eq(const PortIdentity_t &rhs) const
TEST(PortIdentityTest, MethodEq)
{
    PortIdentity_t t1 = { { 1, 2, 3, 4, 5, 6, 7, 8 }, 1234 };
    PortIdentity_t t2 = { { 1, 2, 3, 4, 5, 6, 7, 8 }, 1234 };
    EXPECT_TRUE(t1.eq(t2));
}

// Tests less operator method
// bool operator<(const PortIdentity_t &rhs) const
TEST(PortIdentityTest, MethodLessOp)
{
    PortIdentity_t t1 = { { 1, 2, 3, 4, 3, 6, 7, 8 }, 1234 };
    PortIdentity_t t2 = { { 1, 2, 3, 4, 5, 6, 7, 8 }, 1234 };
    EXPECT_TRUE(t1 < t2);
    PortIdentity_t t3 = { { 1, 2, 3, 4, 3, 6, 7, 8 }, 1235 };
    EXPECT_TRUE(t1 < t3);
}

// Tests less method
// bool less(const PortIdentity_t &rhs) const
TEST(PortIdentityTest, MethodLess)
{
    PortIdentity_t t1 = { { 1, 2, 3, 4, 3, 6, 7, 8 }, 1234 };
    PortIdentity_t t2 = { { 1, 2, 3, 4, 5, 6, 7, 8 }, 1234 };
    EXPECT_TRUE(t1.less(t2));
    PortIdentity_t t3 = { { 1, 2, 3, 4, 3, 6, 7, 8 }, 1235 };
    EXPECT_TRUE(t1.less(t3));
}

/*****************************************************************************/
// Unit tests for struct PortAddress_t

// Tests size method
// size_t size() const
TEST(PortAddressTest, MethodSize)
{
    PortAddress_t t = { UDP_IPv4, 0, Binary("\x1\x2\x3\x4") };
    EXPECT_EQ(t.size(), 8);
}

// Tests string method
// std::string string() const
TEST(PortAddressTest, MethodString)
{
    PortAddress_t t = { UDP_IPv4, 0, Binary("\x1\x2\x3\x4") };
    EXPECT_STREQ(t.string().c_str(), "1.2.3.4");
    t = { IEEE_802_3, 0, Binary("\xa1\xb2\xc3\xd4\xe5\xf6") };
    EXPECT_STREQ(t.string().c_str(), "a1:b2:c3:d4:e5:f6");
}

// Tests equal operator method
// bool operator==(const PortAddress_t &rhs) const
TEST(PortAddressTest, MethodEqOp)
{
    PortAddress_t t1 = { UDP_IPv4, 0, Binary("\x1\x2\x3\x4") };
    PortAddress_t t2 = { UDP_IPv4, 0, Binary("\x1\x2\x3\x4") };
    EXPECT_TRUE(t1 == t2);
}

// Tests equal method
// bool eq(const PortAddress_t &rhs) const
TEST(PortAddressTest, MethodEq)
{
    PortAddress_t t1 = { UDP_IPv4, 0, Binary("\x1\x2\x3\x4") };
    PortAddress_t t2 = { UDP_IPv4, 0, Binary("\x1\x2\x3\x4") };
    EXPECT_TRUE(t1.eq(t2));
}

// Tests less operator method
// bool operator<(const PortAddress_t &rhs) const
TEST(PortAddressTest, MethodLessOp)
{
    PortAddress_t t1 = { UDP_IPv4, 0, Binary("\x1\x2\x1\x4") };
    PortAddress_t t2 = { UDP_IPv4, 0, Binary("\x1\x2\x3\x4") };
    EXPECT_TRUE(t1 < t2);
    PortAddress_t t3 = { IEEE_802_3, 0, Binary("\xa1\xb2\xc3\xd4\xe5\xf6") };
    EXPECT_TRUE(t1 < t3);
}

// Tests less method
// bool less(const PortAddress_t &rhs) const
TEST(PortAddressTest, MethodLess)
{
    PortAddress_t t1 = { UDP_IPv4, 0, Binary("\x1\x2\x1\x4") };
    PortAddress_t t2 = { UDP_IPv4, 0, Binary("\x1\x2\x3\x4") };
    EXPECT_TRUE(t1.less(t2));
    PortAddress_t t3 = { IEEE_802_3, 0, Binary("\xa1\xb2\xc3\xd4\xe5\xf6") };
    EXPECT_TRUE(t1.less(t3));
}

/*****************************************************************************/
// Unit tests for struct PTPText_t

// Tests size method
// size_t size() const
TEST(PtpTextTest, MethodSize)
{
    PTPText_t t = { 0, "test" };
    EXPECT_EQ(t.size(), 5);
}

// Tests string method
// const char *string() const
TEST(PtpTextTest, MethodString)
{
    PTPText_t t = { 0, "test" };
    EXPECT_STREQ(t.string(), "test");
}

/*****************************************************************************/
// Unit tests for struct MsgParams

// Tests allow a signal TLV method
// void allowSigTlv(tlvType_e type)
TEST(MsgParamsTest, MethodAllowSigTlv)
{
    MsgParams t;
    t.allowSigTlv(ORGANIZATION_EXTENSION);
    EXPECT_TRUE(t.isSigTlv(ORGANIZATION_EXTENSION));
}

// Tests remove a signal TLV method
// void removeSigTlv(tlvType_e type)
TEST(MsgParamsTest, MethodRemoveSigTlv)
{
    MsgParams t;
    t.allowSigTlv(ORGANIZATION_EXTENSION);
    EXPECT_TRUE(t.isSigTlv(ORGANIZATION_EXTENSION));
    t.allowSigTlv(MANAGEMENT_ERROR_STATUS);
    t.removeSigTlv(ORGANIZATION_EXTENSION);
    EXPECT_FALSE(t.isSigTlv(ORGANIZATION_EXTENSION));
}

// Tests query if a signal TLV present method
// bool isSigTlv(tlvType_e type) const
TEST(MsgParamsTest, MethodIsSigTlv)
{
    MsgParams t;
    t.allowSigTlv(ORGANIZATION_EXTENSION);
    t.allowSigTlv(MANAGEMENT_ERROR_STATUS);
    EXPECT_TRUE(t.isSigTlv(ORGANIZATION_EXTENSION));
    EXPECT_TRUE(t.isSigTlv(MANAGEMENT_ERROR_STATUS));
    EXPECT_FALSE(t.isSigTlv(REQUEST_UNICAST_TRANSMISSION));
}

// Tests count of signal TLVs method
// size_t countSigTlvs() const
TEST(MsgParamsTest, MethodCountSigTlv)
{
    MsgParams t;
    t.filterSignaling = true;
    t.allowSigTlv(ORGANIZATION_EXTENSION);
    t.allowSigTlv(ORGANIZATION_EXTENSION_PROPAGATE);
    t.allowSigTlv(ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE);
    EXPECT_TRUE(t.isSigTlv(ORGANIZATION_EXTENSION));
    EXPECT_TRUE(t.isSigTlv(ORGANIZATION_EXTENSION_PROPAGATE));
    EXPECT_TRUE(t.isSigTlv(ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE));
    EXPECT_EQ(t.countSigTlvs(), 3);
    // Filter the middle TLV :-)
    t.removeSigTlv(ORGANIZATION_EXTENSION_PROPAGATE);
    EXPECT_TRUE(t.isSigTlv(ORGANIZATION_EXTENSION));
    EXPECT_FALSE(t.isSigTlv(ORGANIZATION_EXTENSION_PROPAGATE));
    EXPECT_TRUE(t.isSigTlv(ORGANIZATION_EXTENSION_DO_NOT_PROPAGATE));
    EXPECT_EQ(t.countSigTlvs(), 2);
}
