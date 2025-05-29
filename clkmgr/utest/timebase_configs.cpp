/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief TimeBaseConfigurations class unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include "pub/clockmanager.h"
#include "common/connect_msg.hpp"
#include "common/timebase.hpp"

using namespace clkmgr;

class clkmgr::ClientConnectMessage : public ConnectMessage
{
  public:
    ClientConnectMessage() {
        const TimeBaseConfigurations &cfg = TimeBaseConfigurations::getInstance();
        cfg.addTimeBaseCfg({
            .timeBaseIndex = 1,
            .timeBaseName = { 'm', 'e', 0 },
            .interfaceName = { 'e', 't', 'h', '0', 0 },
            .transportSpecific = 4,
            .domainNumber = 1
        });
        cfg.addTimeBaseCfg({
            .timeBaseIndex = 2,
            .timeBaseName = { 't', 'o', 'o', 0 },
            .interfaceName = { 'e', 't', 'h', '1', 0 },
            .transportSpecific = 5,
            .domainNumber = 5
        });
    }
};

class TimeBaseConfigurationsTest : public ::testing::Test
{
  protected:
    void SetUp() override {
        ClientConnectMessage m;
    }
};

// const TimeBaseConfigurations &getInstance()
// size_t size()
TEST_F(TimeBaseConfigurationsTest, size)
{
    const TimeBaseConfigurations &cfg = TimeBaseConfigurations::getInstance();
    EXPECT_EQ(cfg.size(), 2);
}

// bool isTimeBaseIndexPresent(size_t timeBaseIndex)
TEST_F(TimeBaseConfigurationsTest, exist)
{
    const TimeBaseConfigurations &cfg = TimeBaseConfigurations::getInstance();
    EXPECT_FALSE(cfg.isTimeBaseIndexPresent(0));
    EXPECT_TRUE(cfg.isTimeBaseIndexPresent(1));
    EXPECT_TRUE(cfg.isTimeBaseIndexPresent(2));
    EXPECT_FALSE(cfg.isTimeBaseIndexPresent(3));
}

// bool BaseNameToBaseIndex(const std::string &timeBaseName, size_t &timeBaseIndex)
TEST_F(TimeBaseConfigurationsTest, name2index)
{
    size_t timeBaseIndex;
    const TimeBaseConfigurations &cfg = TimeBaseConfigurations::getInstance();
    EXPECT_TRUE(cfg.BaseNameToBaseIndex("me", timeBaseIndex));
    EXPECT_EQ(timeBaseIndex, 1);
    EXPECT_TRUE(cfg.BaseNameToBaseIndex("too", timeBaseIndex));
    EXPECT_EQ(timeBaseIndex, 2);
    EXPECT_FALSE(cfg.BaseNameToBaseIndex("non-exist", timeBaseIndex));
}

// const TimeBaseRecord &getRecord(size_t timeBaseIndex)
// size_t TimeBaseRecord::index() const
// const std::string &TimeBaseRecord::name() const
// const char *TimeBaseRecord::name_c() const
// bool TimeBaseRecord::havePtp() const
// bool TimeBaseRecord::haveSysClock() const
TEST_F(TimeBaseConfigurationsTest, getRecord)
{
    const TimeBaseConfigurations &cfg = TimeBaseConfigurations::getInstance();
    const TimeBaseRecord &rec = cfg.getRecord(1);
    EXPECT_EQ(rec.index(), 1);
    EXPECT_STREQ(rec.name().c_str(), "me");
    EXPECT_STREQ(rec.name_c(), "me");
    EXPECT_TRUE(rec.havePtp());
    EXPECT_TRUE(rec.haveSysClock());
};

// TimeBaseConfigurations::iterator begin()
// TimeBaseConfigurations::iterator end()
// const PTPCfg &TimeBaseRecord::ptp() const;
// const std::string &PTPCfg::ifName() const;
// const char *PTPCfg::ifName_c() const;
// int PTPCfg::ifIndex() const;
// int PTPCfg::ptpIndex() const;
// uint8_t PTPCfg::transportSpecific() const;
// uint8_t PTPCfg::domainNumber() const;
TEST_F(TimeBaseConfigurationsTest, begin)
{
    const TimeBaseConfigurations &cfg = TimeBaseConfigurations::getInstance();
    auto it = cfg.begin();
    EXPECT_EQ((*it).index(), 1);
    it++;
    EXPECT_EQ((*it).index(), 2);
    const PTPCfg &ptp = (*it).ptp();
    EXPECT_STREQ(ptp.ifName().c_str(), "eth1");
    EXPECT_STREQ(ptp.ifName_c(), "eth1");
    EXPECT_EQ(ptp.ifIndex(), -1);
    EXPECT_EQ(ptp.ptpIndex(), -1);
    EXPECT_EQ(ptp.transportSpecific(), 5);
    EXPECT_EQ(ptp.domainNumber(), 5);
    // Loop both records
    size_t i = 1;
    for(auto &it : cfg)
        EXPECT_EQ(it.index(), i++);
};
