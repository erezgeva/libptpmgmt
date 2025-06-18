/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief JsonConfigParser class unit tests
 *
 * @author Goh Wei Sheng <wei.sheng.goh@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include "proxy/config_parser.hpp"
#include "common/print.hpp"
#include "jsonParser.h"

using namespace clkmgr;

class JsonConfigParserTest : public ::testing::Test
{
  protected:
    std::string tempJson;

    void SetUp() override {
        setLogLevel(0);
        tempJson = "test_proxy_cfg.json";
    }

    void TearDown() override {
        std::remove(tempJson.c_str());
    }

    void writeFile(const std::string &content) {
        std::ofstream ofs(tempJson);
        ofs << content;
        ofs.close();
    }
};

TEST_F(JsonConfigParserTest, singletonInstance)
{
    JsonConfigParser &inst1 = JsonConfigParser::getInstance();
    JsonConfigParser &inst2 = JsonConfigParser::getInstance();
    EXPECT_EQ(&inst1, &inst2);
}

TEST_F(JsonConfigParserTest, processInvalidJson)
{
    JsonConfigParser &parser = JsonConfigParser::getInstance();
    // Non existent Json file
    ASSERT_FALSE(parser.process_json("nonexistent_file.json"));
    // Malformed Json
    writeFile("{ Malformed Json ");
    ASSERT_FALSE(parser.process_json(tempJson.c_str()));
}

TEST_F(JsonConfigParserTest, missingFields)
{
    writeFile(R"({
      "timeBases": [
        {
          "timeBaseName": "Missing ptp4l",
        }
      ]
    })");
    JsonConfigParser &parser = JsonConfigParser::getInstance();
    ASSERT_FALSE(parser.process_json(tempJson.c_str()));
}

TEST_F(JsonConfigParserTest, invalidStrInput)
{
    JsonConfigParser &parser = JsonConfigParser::getInstance();
    // Test out-of-bounds string length
    writeFile(R"({
      "timeBases": [
        {
          "timeBaseName": "Invalid Clock",
          "ptp4l": {
            "interfaceName": "This string is exactly sixty-five(65) characters
            long for testing",
          }
        }
      ]
    })");
    ASSERT_FALSE(parser.process_json(tempJson.c_str()));
    // Test input invalid data type - int
    writeFile(R"({
      "timeBases": [
        {
          "timeBaseName": "Invalid Clock",
          "ptp4l": {
            "interfaceName": 123,
          }
        }
      ]
    })");
    ASSERT_FALSE(parser.process_json(tempJson.c_str()));
    // Test empty input
    writeFile(R"({
      "timeBases": [
        {
          "timeBaseName": "Invalid Clock",
          "ptp4l": {
            "interfaceName": ,
          }
        }
      ]
    })");
    ASSERT_FALSE(parser.process_json(tempJson.c_str()));
}

TEST_F(JsonConfigParserTest, invalidIntInput)
{
    JsonConfigParser &parser = JsonConfigParser::getInstance();
    // Test out-of-bounds string length
    writeFile(R"({
      "timeBases": [
        {
          "timeBaseName": "Invalid Clock",
          "ptp4l": {
            "domainNumber": 256,
          }
        }
      ]
    })");
    ASSERT_FALSE(parser.process_json(tempJson.c_str()));
    // Test input invalid data type - symbol
    writeFile(R"({
      "timeBases": [
        {
          "timeBaseName": "Invalid Clock",
          "ptp4l": {
            "domainNumber": (^.^),
          }
        }
      ]
    })");
    ASSERT_FALSE(parser.process_json(tempJson.c_str()));
    // Test input invalid data type - string
    writeFile(R"({
      "timeBases": [
        {
          "timeBaseName": "Invalid Clock",
          "ptp4l": {
            "domainNumber": "This is a string, not an integer",
          }
        }
      ]
    })");
    ASSERT_FALSE(parser.process_json(tempJson.c_str()));
}

TEST_F(JsonConfigParserTest, defaultValues)
{
    // Test default values for specific ptp4l and chrony fields
    writeFile(R"({
      "timeBases": [
        {
          "timeBaseName": "Sample Clock",
          "ptp4l": {
          },
          "chrony": {
          }
        }
      ]
    })");
    JsonConfigParser &parser = JsonConfigParser::getInstance();
    ASSERT_TRUE(parser.process_json(tempJson.c_str()));
    ASSERT_EQ(parser.size(), 1u);
    auto it = parser.begin();
    EXPECT_STREQ(it->base.timeBaseName, "Sample Clock");
    EXPECT_STREQ(it->base.interfaceName, "");
    EXPECT_EQ(it->udsAddrPtp4l, "/var/run/ptp4l");
    EXPECT_EQ(it->base.domainNumber, 0);
    EXPECT_EQ(it->base.transportSpecific, 1);
    EXPECT_EQ(it->udsAddrChrony, "/var/run/chrony/chronyd.sock");
    it++;
    EXPECT_EQ(it, parser.end());
}

TEST_F(JsonConfigParserTest, emptyTimeBasesArray)
{
    writeFile(R"({ "timeBases": [] })");
    JsonConfigParser &parser = JsonConfigParser::getInstance();
    ASSERT_TRUE(parser.process_json(tempJson.c_str()));
    EXPECT_EQ(parser.size(), 0u);
}

TEST_F(JsonConfigParserTest, singleTimeBase)
{
    writeFile(R"({
      "timeBases": [
        {
          "timeBaseName": "Sample Clock",
          "ptp4l": {
            "interfaceName": "eth123",
            "udsAddr": "/var/run/ptp4l-sample-123",
            "domainNumber": 123,
            "transportSpecific": 123
          },
          "chrony": {
            "udsAddr": "/var/run/chrony/chronyd-sample.sock"
          }
        }
      ]
    })");
    JsonConfigParser &parser = JsonConfigParser::getInstance();
    ASSERT_TRUE(parser.process_json(tempJson.c_str()));
    ASSERT_EQ(parser.size(), 1u);
    auto it = parser.begin();
    EXPECT_STREQ(it->base.timeBaseName, "Sample Clock");
    EXPECT_STREQ(it->base.interfaceName, "eth123");
    EXPECT_EQ(it->udsAddrPtp4l, "/var/run/ptp4l-sample-123");
    EXPECT_EQ(it->base.domainNumber, 123);
    EXPECT_EQ(it->base.transportSpecific, 123);
    EXPECT_EQ(it->udsAddrChrony, "/var/run/chrony/chronyd-sample.sock");
    it++;
    EXPECT_EQ(it, parser.end());
}

TEST_F(JsonConfigParserTest, multipleTimeBases)
{
    writeFile(R"({
      "timeBases": [
        {
          "timeBaseName": "Global Clock",
          "ptp4l": {
            "interfaceName": "eth0",
            "udsAddr": "/var/run/ptp4l-domain-0",
            "domainNumber": 0,
            "transportSpecific": 1
          },
          "chrony": {
            "udsAddr": "/var/run/chrony/chronyd.sock"
          }
        },
        {
          "timeBaseName": "Working Clock",
          "ptp4l": {
            "interfaceName": "eth1",
            "udsAddr": "/var/run/ptp4l-domain-20",
            "domainNumber": 20,
            "transportSpecific": 1
          }
        }
      ]
    })");
    JsonConfigParser &parser = JsonConfigParser::getInstance();
    ASSERT_TRUE(parser.process_json(tempJson.c_str()));
    ASSERT_EQ(parser.size(), 2u);
    auto it = parser.begin();
    EXPECT_STREQ(it->base.timeBaseName, "Global Clock");
    EXPECT_STREQ(it->base.interfaceName, "eth0");
    EXPECT_EQ(it->base.transportSpecific, 1);
    EXPECT_EQ(it->base.domainNumber, 0);
    EXPECT_EQ(it->udsAddrPtp4l, "/var/run/ptp4l-domain-0");
    EXPECT_EQ(it->udsAddrChrony, "/var/run/chrony/chronyd.sock");
    it++;
    EXPECT_STREQ(it->base.timeBaseName, "Working Clock");
    EXPECT_STREQ(it->base.interfaceName, "eth1");
    EXPECT_EQ(it->base.transportSpecific, 1);
    EXPECT_EQ(it->base.domainNumber, 20);
    EXPECT_EQ(it->udsAddrPtp4l, "/var/run/ptp4l-domain-20");
    it++;
    EXPECT_EQ(it, parser.end());
}
