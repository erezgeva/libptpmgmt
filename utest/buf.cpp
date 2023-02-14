/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Buffer class unit test
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

#include "buf.h"

using namespace ptpmgmt;

// Tests Buffer empty constructor
// Buf()
TEST(BufferTest, MethodEmptyConstructor)
{
    Buf f;
    EXPECT_EQ(f.get(), nullptr);
    EXPECT_EQ(f.size(), 0);
}

// Test create a new memory buffer
// Buf(size_t size)
TEST(BufferTest, MethodSizeConstructor)
{
    Buf f(20);
    EXPECT_NE(f.get(), nullptr);
    EXPECT_EQ(f.size(), 20);
}

// Test alloc a new memory buffer
// bool alloc(size_t size)
TEST(BufferTest, MethodAlloc)
{
    Buf f;
    EXPECT_TRUE(f.alloc(20));
    EXPECT_NE(f.get(), nullptr);
    EXPECT_EQ(f.size(), 20);
}

// Test realloc a new memory buffer
// bool alloc(size_t size)
TEST(BufferTest, MethodRealloc)
{
    Buf f(20);
    EXPECT_TRUE(f.alloc(10)); // Ask for less
    EXPECT_NE(f.get(), nullptr);
    EXPECT_EQ(f.size(), 20); // Buffer retain previous size
    EXPECT_TRUE(f.alloc(30)); // Ask for more
    EXPECT_NE(f.get(), nullptr);
    EXPECT_EQ(f.size(), 30); // Buffer should increase size
}

// Test get memory buffer
// void *get() const
TEST(BufferTest, MethodGet)
{
    Buf f(20);
    EXPECT_NE(f.get(), nullptr);
}

// Test get memory buffer operator
// void *operator()() const
TEST(BufferTest, MethodGetOp)
{
    Buf f(20);
    EXPECT_EQ(f.get(), f());
}

// Test memory buffer size
// size_t size() const
TEST(BufferTest, MethodSize)
{
    Buf f(20);
    EXPECT_EQ(f.size(), 20);
}

// Test if buffer us allocatede
// bool isAlloc() const
TEST(BufferTest, MethodIsAlloc)
{
    Buf f;
    EXPECT_FALSE(f.isAlloc());
    EXPECT_TRUE(f.alloc(10));
    EXPECT_TRUE(f.isAlloc());
}
