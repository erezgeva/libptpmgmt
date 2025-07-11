/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Client class unit tests
 *
 * @author Goh Wei Sheng <wei.sheng.goh@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include <gtest/gtest.h>

#include "client/client_state.hpp"
#include "common/print.hpp"
#include "proxy/client.hpp"

using namespace clkmgr;

bool Client::connect_ptp4l()
{
    return true;
}

bool Client::connect_chrony()
{
    return true;
}

// Test ptpEvent, Client
// int64_t clockOffset
// uint64_t syncInterval
// bool syncedWithGM
// bool asCapable
// uint64_t gmClockUUID
// struct ptp_event
// ptpEvent(size_t index)
// void portDataClear()
// void clear()
// void copy()
// static void getPTPEvent(size_t timeBaseIndex, ptp_event &event)
// Index 0
TEST(ClientTest, constructAndCopyPTPEvent)
{
    // Local ptp event structure
    ptpEvent ptp(0);
    // Shared ptp event structure add 0 to see
    ptp_event event = { 0 };
    // Check the initialized values
    EXPECT_EQ(ptp.event.clockOffset, 0);
    EXPECT_EQ(ptp.event.gmClockUUID, 0);
    EXPECT_EQ(ptp.event.syncInterval, 0);
    EXPECT_FALSE(ptp.event.syncedWithGm);
    EXPECT_FALSE(ptp.event.asCapable);
    EXPECT_EQ(event.clockOffset, 0);
    EXPECT_EQ(event.gmClockUUID, 0);
    EXPECT_EQ(event.syncInterval, 0);
    EXPECT_FALSE(event.syncedWithGm);
    EXPECT_FALSE(event.asCapable);
    // Assign the values
    ptp.event.clockOffset = 123;
    ptp.event.gmClockUUID = 100;
    ptp.event.syncInterval = 456;
    ptp.event.syncedWithGm = true;
    ptp.event.asCapable = true;
    // Copy values from local to shared structures
    ptp.copy();
    // Get the shared event and check the values
    Client::getPTPEvent(0, event);
    EXPECT_EQ(event.clockOffset, 123);
    EXPECT_EQ(event.gmClockUUID, 100);
    EXPECT_EQ(event.syncInterval, 456);
    EXPECT_TRUE(event.syncedWithGm);
    EXPECT_TRUE(event.asCapable);
    // Clear and ensure the values are reset
    ptp.clear();
    EXPECT_EQ(ptp.event.clockOffset, 0);
    EXPECT_EQ(ptp.event.gmClockUUID, 0);
    EXPECT_EQ(ptp.event.syncInterval, 0);
    EXPECT_FALSE(ptp.event.syncedWithGm);
    EXPECT_FALSE(ptp.event.asCapable);
    // Reassign to test portDataClear
    ptp.event.clockOffset = -100;
    ptp.event.syncInterval = 2000;
    ptp.event.syncedWithGm = true;
    EXPECT_EQ(ptp.event.clockOffset, -100);
    EXPECT_EQ(ptp.event.syncInterval, 2000);
    EXPECT_TRUE(ptp.event.syncedWithGm);
    // Clear port data
    ptp.portDataClear();
    // Ensure the port data is cleared
    EXPECT_EQ(ptp.event.clockOffset, 0);
    EXPECT_EQ(ptp.event.syncInterval, 0);
    EXPECT_FALSE(ptp.event.syncedWithGm);
}

// Test chronyEvent, Client
// int64_t clockOffset
// uint64_t gmClockUUID
// uint64_t syncInterval
// struct chrony_event
// chronyEvent(size_t index)
// void clear()
// void copy()
// static void getChronyEvent(size_t timeBaseIndex, chrony_event &event)
// Index 1
TEST(ClientTest, constructAndCopyChronyEvent)
{
    // Local chrony event structure
    chronyEvent chrony(1);
    // Shared chrony event structure
    chrony_event event = { 0 };
    // Check the initialized values
    EXPECT_EQ(chrony.event.clockOffset, 0);
    EXPECT_EQ(chrony.event.gmClockUUID, 0);
    EXPECT_EQ(chrony.event.syncInterval, 0);
    EXPECT_EQ(event.clockOffset, 0);
    EXPECT_EQ(event.gmClockUUID, 0);
    EXPECT_EQ(event.syncInterval, 0);
    // Assign the values
    chrony.event.clockOffset = 789;
    chrony.event.gmClockUUID = 42;
    chrony.event.syncInterval = 1000;
    // Copy values from local to shared structures
    chrony.copy();
    // Get the shared event and check the values
    Client::getChronyEvent(1, event);
    EXPECT_EQ(event.clockOffset, 789);
    EXPECT_EQ(event.gmClockUUID, 42);
    EXPECT_EQ(event.syncInterval, 1000);
    // Clear and ensure the values are reset
    chrony.clear();
    EXPECT_EQ(chrony.event.clockOffset, 0);
    EXPECT_EQ(chrony.event.gmClockUUID, 0);
    EXPECT_EQ(chrony.event.syncInterval, 0);
}
