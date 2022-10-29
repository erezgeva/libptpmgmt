/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2022 Erez Geva */

/** @file
 * @brief Version functions unit test
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2022 Erez Geva
 *
 */

#include "ver.h"

using namespace ptpmgmt;

// Tests Version major macro
// #define LIBPTPMGMT_VER_MAJ
TEST(VerTest, MacroMajor)
{
    EXPECT_GE(LIBPTPMGMT_VER_MAJ, 0);
}

// Tests Version minor macro
// #define LIBPTPMGMT_VER_MIN
TEST(VerTest, MacroMinor)
{
    EXPECT_GE(LIBPTPMGMT_VER_MIN, 0);
    EXPECT_LE(LIBPTPMGMT_VER_MIN, 0xFF);
}

// Tests Version macro
// #define LIBPTPMGMT_VER_VAL
TEST(VerTest, MacroVal)
{
    EXPECT_EQ(LIBPTPMGMT_VER_VAL, LIBPTPMGMT_VER_MIN + LIBPTPMGMT_VER_MAJ * 0xFF);
}

// Tests Version string
// #define LIBPTPMGMT_VER
TEST(VerTest, MacroStr)
{
    std::string ver = std::to_string(LIBPTPMGMT_VER_MAJ);
    ver += ".";
    ver += std::to_string(LIBPTPMGMT_VER_MIN);
    EXPECT_STREQ(ver.c_str(), LIBPTPMGMT_VER);
}

// NOTE: macros and function return the same value,
//       since the unit test always run on the same library it test!

// Tests get Version string
// const char *getVersion()
TEST(VerTest, FuncStr)
{
    EXPECT_STREQ(getVersion(), LIBPTPMGMT_VER);
}

// Tests get Version major
// int getVersionMajor()
TEST(VerTest, FuncMajor)
{
    EXPECT_EQ(getVersionMajor(), LIBPTPMGMT_VER_MAJ);
}

// Tests get Version minor
// int getVersionMinor()
TEST(VerTest, FuncMinor)
{
    EXPECT_EQ(getVersionMinor(), LIBPTPMGMT_VER_MIN);
}

// Tests get Version value
// unsigned getVersionValue()
TEST(VerTest, FuncVal)
{
    EXPECT_EQ(getVersionValue(), LIBPTPMGMT_VER_VAL);
}
