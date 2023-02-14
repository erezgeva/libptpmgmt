/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief PTP classes unit test
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

#include "ptp.h"
#include "err.h"

using namespace ptpmgmt;

class IfInfoTest : public ::testing::Test, public IfInfo
{
  protected:
    void SetUp() override {
        useTestMode(true);
    }
    void TearDown() override {
        useTestMode(false);
    }
};

// Tests initUsingName method
// bool initUsingName(const std::string &ifName)
// bool isInit() const
// int ifIndex() const
// const std::string &ifName() const
// const char *ifName_c() const
// const Binary &mac() const
// const uint8_t *mac_c() const
// size_t mac_size() const
// int ptpIndex() const
TEST_F(IfInfoTest, MethodInitUsingName)
{
    EXPECT_TRUE(initUsingName("eth0"));
    EXPECT_TRUE(isInit());
    EXPECT_EQ(ifIndex(), 7);
    EXPECT_STREQ(ifName().c_str(), "eth0");
    EXPECT_STREQ(ifName_c(), "eth0");
    uint8_t a[] = { 1, 2, 3, 4, 5, 6 };
    EXPECT_EQ(mac(), Binary(a, sizeof a));
    EXPECT_EQ(memcmp(mac_c(), a, sizeof a), 0);
    EXPECT_EQ(mac_size(), sizeof a);
    EXPECT_EQ(ptpIndex(), 3);
}

// Tests initUsingName method
// bool initUsingIndex(int ifIndex)
// bool isInit() const
// int ifIndex() const
// const std::string &ifName() const
// const char *ifName_c() const
// const Binary &mac() const
// const uint8_t *mac_c() const
// size_t mac_size() const
// int ptpIndex() const
TEST_F(IfInfoTest, MethodInitUsingIndex)
{
    EXPECT_TRUE(initUsingIndex(7));
    EXPECT_TRUE(isInit());
    EXPECT_EQ(ifIndex(), 7);
    EXPECT_STREQ(ifName().c_str(), "eth0");
    EXPECT_STREQ(ifName_c(), "eth0");
    uint8_t a[] = { 1, 2, 3, 4, 5, 6 };
    EXPECT_EQ(mac(), Binary(a, sizeof a));
    EXPECT_EQ(memcmp(mac_c(), a, sizeof a), 0);
    EXPECT_EQ(mac_size(), sizeof a);
    EXPECT_EQ(ptpIndex(), 3);
}

class SysClockTest : public ::testing::Test, public SysClock
{
  protected:
    void SetUp() override {
        useTestMode(true);
    }
    void TearDown() override {
        useTestMode(false);
    }
};

// Tests getTime method
// Timestamp_t getTime() const
TEST_F(SysClockTest, MethodGetTime)
{
    EXPECT_EQ(getTime(), Timestamp_t(1, 0));
    EXPECT_EQ(getTime(), Timestamp_t(2, 0));
}

// Tests setTime method
// bool setTime(const Timestamp_t &ts) const
TEST_F(SysClockTest, MethodSetTime)
{
    EXPECT_TRUE(setTime(Timestamp_t(12, 147)));
}

// Tests offsetClock method
// bool offsetClock(int64_t offset) const
TEST_F(SysClockTest, MethodOffsetClock)
{
    EXPECT_TRUE(offsetClock(17000000029));
    EXPECT_TRUE(offsetClock(-19999999963));
}

// Tests getFreq method
// float_freq getFreq() const
TEST_F(SysClockTest, MethodGetFreq)
{
    float_freq f = getFreq();
    EXPECT_DOUBLE_EQ(f, 77600000003.875732l);
}

// Tests setFreq method
// bool setFreq(float_freq freq) const
TEST_F(SysClockTest, MethodSetFreq)
{
    EXPECT_TRUE(setFreq(376000000.0l));
}

class PtpClockTest : public ::testing::Test, public PtpClock
{
  protected:
    void SetUp() override {
        useTestMode(true);
    }
    void TearDown() override {
        useTestMode(false);
    }
};

// Tests isCharFile method
// static bool isCharFile(const std::string &file)
TEST_F(PtpClockTest, MethodIsCharFile)
{
    EXPECT_TRUE(isCharFile("/dev/ptp0"));
}

// Tests initUsingDevice method
// bool initUsingDevice(const std::string &device, bool readonly = false)
// bool isInit() const
// const std::string &device() const
// const char *device_c() const
TEST_F(PtpClockTest, MethodInitUsingDevice)
{
    EXPECT_TRUE(initUsingDevice("ptp0"));
    EXPECT_TRUE(isInit());
    EXPECT_STREQ(device().c_str(), "/dev/ptp0");
    EXPECT_STREQ(device_c(), "/dev/ptp0");
}

// Tests initUsingIndex method
// bool initUsingIndex(int ptpIndex, bool readonly = false)
// clockid_t clkId() const
// int ptpIndex() const
TEST_F(PtpClockTest, initUsingIndex)
{
    EXPECT_TRUE(initUsingIndex(0));
    EXPECT_TRUE(isInit());
    EXPECT_STREQ(device().c_str(), "/dev/ptp0");
    EXPECT_STREQ(device_c(), "/dev/ptp0");
    EXPECT_LT(clkId(), 0);
    EXPECT_EQ(ptpIndex(), 0);
}

// Tests init readonly
TEST_F(PtpClockTest, initReadOnly)
{
    EXPECT_TRUE(initUsingIndex(1, true));
    EXPECT_TRUE(isInit());
    EXPECT_STREQ(device().c_str(), "/dev/ptp1");
    EXPECT_STREQ(device_c(), "/dev/ptp1");
    EXPECT_LT(clkId(), 0);
    EXPECT_EQ(ptpIndex(), 1);
}

// Tests getTime method
// Timestamp_t getTime() const
TEST_F(PtpClockTest, MethodGetTime)
{
    EXPECT_TRUE(initUsingIndex(1, true));
    EXPECT_EQ(getTime(), Timestamp_t(17, 567));
}

// Tests setTime method
// bool setTime(const Timestamp_t &ts) const
TEST_F(PtpClockTest, MethodSetTime)
{
    EXPECT_TRUE(initUsingIndex(0));
    EXPECT_TRUE(setTime(Timestamp_t(19, 351)));
}

// Tests offsetClock method
// bool offsetClock(int64_t offset) const
TEST_F(PtpClockTest, MethodOffsetClock)
{
    EXPECT_TRUE(initUsingIndex(0));
    EXPECT_TRUE(offsetClock(93000000571));
    EXPECT_TRUE(offsetClock(-19999999963));
}

// Tests getFreq method
// float_freq getFreq() const
TEST_F(PtpClockTest, MethodGetFreq)
{
    EXPECT_TRUE(initUsingIndex(1, true));
    float_freq f = getFreq();
    EXPECT_DOUBLE_EQ(f, 9.979248046875l);
}

// Tests setFreq method
// bool setFreq(float_freq freq) const
TEST_F(PtpClockTest, MethodSetFreq)
{
    EXPECT_TRUE(initUsingIndex(0));
    EXPECT_TRUE(setFreq(234780.0l));
}

// Tests setTimeFromSys method
// bool setTimeFromSys() const
TEST_F(PtpClockTest, MethodSetTimeFromSys)
{
    EXPECT_TRUE(initUsingIndex(0));
    EXPECT_TRUE(setTimeFromSys());
}

// Tests setTimeToSys method
// bool setTimeToSys() const
TEST_F(PtpClockTest, MethodSetTimeToSys)
{
    EXPECT_TRUE(initUsingIndex(0));
    EXPECT_TRUE(setTimeToSys());
}

// Tests fetchCaps method
// bool fetchCaps(PtpCaps_t &caps) const
TEST_F(PtpClockTest, MethodFetchCaps)
{
    EXPECT_TRUE(initUsingIndex(0));
    PtpCaps_t caps;
    EXPECT_TRUE(fetchCaps(caps));
    EXPECT_EQ(caps.max_ppb, 17);
    EXPECT_EQ(caps.num_alarm, 32);
    EXPECT_EQ(caps.num_external_channels, 48);
    EXPECT_EQ(caps.num_periodic_sig, 75);
    EXPECT_TRUE(caps.pps);
    EXPECT_EQ(caps.num_pins, 12);
    EXPECT_TRUE(caps.cross_timestamping);
    EXPECT_FALSE(caps.adjust_phase);
}

// Tests readPin method
// bool readPin(unsigned int index, PtpPin_t &pin) const
TEST_F(PtpClockTest, MethodReadPin)
{
    EXPECT_TRUE(initUsingIndex(0));
    PtpPin_t pin;
    EXPECT_TRUE(readPin(1, pin));
    EXPECT_EQ(pin.index, 1);
    EXPECT_STREQ(pin.description.c_str(), "pin desc");
    EXPECT_EQ(pin.channel, 19);
}

// Tests writePin method
// bool writePin(PtpPin_t &pin) const
TEST_F(PtpClockTest, MethodWritePin)
{
    EXPECT_TRUE(initUsingIndex(0));
    PtpPin_t pin = {
        .index = 2,
        .functional = PTP_PIN_PERIODIC_OUT,
        .channel = 23
    };
    EXPECT_TRUE(writePin(pin));
}

// Tests ExternTSEbable method
// bool ExternTSEbable(unsigned int index, uint8_t flags) const
TEST_F(PtpClockTest, MethodExternTSEbable)
{
    EXPECT_TRUE(initUsingIndex(0));
    EXPECT_TRUE(ExternTSEbable(7, PTP_EXTERN_TS_RISING_EDGE));
}

// Tests ExternTSDisable method
// bool ExternTSDisable(unsigned int index) const
TEST_F(PtpClockTest, MethodExternTSDisable)
{
    EXPECT_TRUE(initUsingIndex(0));
    EXPECT_TRUE(ExternTSDisable(9));
}

// Tests setPinPeriod method
// bool setPinPeriod(unsigned int index, PtpPinPeriodDef_t times,
//     uint8_t flags = 0) const;
TEST_F(PtpClockTest, MethodSetPinPeriod)
{
    EXPECT_TRUE(initUsingIndex(0));
    PtpPinPeriodDef_t times;
    times.period = Timestamp_t(76, 154);
    EXPECT_TRUE(setPinPeriod(11, times));
}

// Tests setPtpPpsEvent method
// bool setPtpPpsEvent(bool enable) const
TEST_F(PtpClockTest, MethodSetPtpPpsEvent)
{
    EXPECT_TRUE(initUsingIndex(0));
    EXPECT_TRUE(setPtpPpsEvent(true));
}

// Tests samplePtpSys method
// bool samplePtpSys(size_t count, std::vector<PtpSample_t> &samples) const
TEST_F(PtpClockTest, MethodSamplePtpSys)
{
    EXPECT_TRUE(initUsingIndex(0));
    std::vector<PtpSample_t> samples;
    EXPECT_TRUE(samplePtpSys(5, samples));
    EXPECT_EQ(samples.size(), 2);
    EXPECT_EQ(samples[0].sysClk, Timestamp_t(11, 33));
    EXPECT_EQ(samples[0].phcClk, Timestamp_t(22, 44));
    EXPECT_EQ(samples[1].sysClk, Timestamp_t(71, 63));
    EXPECT_EQ(samples[1].phcClk, Timestamp_t(62, 84));
}

// Tests extSamplePtpSys method
// bool extSamplePtpSys(size_t count, std::vector<PtpSampleExt_t> &samples) const
TEST_F(PtpClockTest, MethodExtSamplePtpSys)
{
    EXPECT_TRUE(initUsingIndex(0));
    std::vector<PtpSampleExt_t> samples;
    bool ret = extSamplePtpSys(7, samples);
    if(!ret) {
        EXPECT_STREQ(Error::getMsg().c_str(),
            "Old kernel, PTP_SYS_OFFSET_EXTENDED ioctl is not supported");
        return;
    }
    EXPECT_EQ(samples.size(), 2);
    EXPECT_EQ(samples[0].before, Timestamp_t(11, 33));
    EXPECT_EQ(samples[0].phcClk, Timestamp_t(22, 44));
    EXPECT_EQ(samples[0].after, Timestamp_t(84, 91));
    EXPECT_EQ(samples[1].before, Timestamp_t(71, 63));
    EXPECT_EQ(samples[1].phcClk, Timestamp_t(62, 84));
    EXPECT_EQ(samples[1].after, Timestamp_t(45, 753));
}

// Tests preciseSamplePtpSys method
// bool preciseSamplePtpSys(PtpSamplePrecise_t &sample) const
TEST_F(PtpClockTest, MethodPreciseSamplePtpSys)
{
    EXPECT_TRUE(initUsingIndex(0));
    PtpSamplePrecise_t sample;
    EXPECT_TRUE(preciseSamplePtpSys(sample));
    EXPECT_EQ(sample.phcClk, Timestamp_t(17, 135));
    EXPECT_EQ(sample.sysClk, Timestamp_t(415, 182));
    EXPECT_EQ(sample.monoClk, Timestamp_t(9413, 3654));
}

// Tests readEvent method
// bool readEvent(PtpEvent_t &event) const
TEST_F(PtpClockTest, MethodReadEvent)
{
    EXPECT_TRUE(initUsingIndex(0));
    PtpEvent_t event;
    EXPECT_TRUE(readEvent(event));
    EXPECT_EQ(event.index, 19);
    EXPECT_EQ(event.time, Timestamp_t(123, 712));
}

// Tests readEvents method
// bool readEvents(std::vector<PtpEvent_t> &events, size_t max = 0) const
TEST_F(PtpClockTest, MethodReadEvents)
{
    EXPECT_TRUE(initUsingIndex(0));
    std::vector<PtpEvent_t> events;
    EXPECT_TRUE(readEvents(events, 10));
    EXPECT_EQ(events.size(), 3);
    EXPECT_EQ(events[0].index, 2);
    EXPECT_EQ(events[0].time, Timestamp_t(34, 7856));
    EXPECT_EQ(events[1].index, 6);
    EXPECT_EQ(events[1].time, Timestamp_t(541, 468));
    EXPECT_EQ(events[2].index, 3);
    EXPECT_EQ(events[2].time, Timestamp_t(1587, 12));
}
