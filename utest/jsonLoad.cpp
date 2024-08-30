/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief unit test for loading Json libraries
 * @note we replace with internal JSON parser,
 *       we leave it for backward compatible with older versions.
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

#include "config.h"
#include "json.h"

using namespace ptpmgmt;
const char *json2 = "{\"actionField\":\"GET\",\"managementId\":\"PRIORITY2\"}";

// Tests automatic load
// static const char *loadLibrary()
// static bool isLibShared()
// bool fromJsonObj(const void *jobj)
TEST(jsonLoadTest, MethodLoadAuto)
{
    Json2msg m;
    const char *lib = m.loadLibrary();
    EXPECT_STREQ(std::string(lib, 19).c_str(), "libptpmgmt_jsonc.so");
    EXPECT_TRUE(m.isLibShared());
    EXPECT_TRUE(m.fromJson(json2));
    EXPECT_EQ(m.managementId(), PRIORITY2);
}

// Tests loading json-c library
// static bool selectLib(const std::string &libName)
// static const char *loadLibrary()
// static bool isLibShared()
// bool fromJsonObj(const void *jobj)
TEST(jsonLoadTest, MethodLoadJsonC)
{
    Json2msg m;
    EXPECT_TRUE(m.selectLib("jsonc"));
    const char *lib = m.loadLibrary();
    EXPECT_STREQ(std::string(lib, 19).c_str(), "libptpmgmt_jsonc.so");
    EXPECT_TRUE(m.isLibShared());
    EXPECT_TRUE(m.fromJson(json2));
    EXPECT_EQ(m.managementId(), PRIORITY2);
}

// Tests loading fastjson library
TEST(jsonLoadTest, MethodLoadFastJson)
{
    Json2msg m;
    EXPECT_TRUE(m.selectLib("fastjson"));
    const char *lib = m.loadLibrary();
    EXPECT_STREQ(std::string(lib, 22).c_str(), "libptpmgmt_fastjson.so");
    EXPECT_TRUE(m.isLibShared());
    EXPECT_TRUE(m.fromJson(json2));
    EXPECT_EQ(m.managementId(), PRIORITY2);
}
