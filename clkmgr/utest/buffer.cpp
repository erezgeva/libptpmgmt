/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief test internal buffer
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include "common/msgq_tport.hpp"

using namespace clkmgr;

// void setLen(size_t rcvSize)
// void addOffset(size_t offset)
// size_t getOffset() const
// size_t lenLeft() const
TEST(Buffer, lenLeft)
{
    Buffer b;
    b.setLen(250);
    EXPECT_EQ(b.lenLeft(), 250);
    b.addOffset(100);
    EXPECT_EQ(b.getOffset(), 100);
    EXPECT_EQ(b.lenLeft(), 150);
}

// static size_t size()
TEST(Buffer, size)
{
    EXPECT_EQ(Buffer::size(), MAX_BUFFER_LENGTH);
}

// uint8_t *dataOff()
// size_t sizeLeft() const
// const uint8_t *data() const
// void resetOffset()
TEST(Buffer, write)
{
    Buffer b;
    memcpy(b.dataOff(), "test", 4);
    b.addOffset(4);
    memcpy(b.dataOff(), "123", 4);
    b.addOffset(4);
    EXPECT_EQ(b.sizeLeft(), b.size() - 8);
    EXPECT_STREQ((char *)b.data(), "test123");
    b.resetOffset();
    EXPECT_EQ(b.sizeLeft(), b.size());
    EXPECT_EQ(b.dataOff(), b.data());
}
