/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2022 Erez Geva */

/** @file
 * @brief unit test for loading Json libraries
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2022 Erez Geva
 *
 */

#include <dlfcn.h>
#include "json.h"

using namespace ptpmgmt;
const char *json1 = "{\"actionField\":\"GET\",\"managementId\":\"PRIORITY1\"}";
const char *json2 = "{\"actionField\":\"GET\",\"managementId\":\"PRIORITY2\"}";
// We use dlsym(), since we can not include headers from both json libraries,
//  as they use the same names. User can simply include the json library header!
void *(*json_tokener_parse)(const char *);
int (*json_object_put)(void *);

// Tests loading json-c library
// static bool selectLib(const std::string &libName);
// static const char *loadLibrary();
// static bool isLibShared();
// bool fromJsonObj(const void *jobj);
TEST(jsonLoadTest, MethodLoadJsonC)
{
    Json2msg m;
    EXPECT_TRUE(m.selectLib("jsonc"));
    const char *lib = m.loadLibrary();
    EXPECT_STREQ(std::string(lib, 19).c_str(), "libptpmgmt_jsonc.so");
    EXPECT_TRUE(m.isLibShared());
    EXPECT_TRUE(m.fromJson(json1)); // To ensure libraries are load in memory
    void *handle = dlopen(lib, RTLD_NOW);
    ASSERT_NE(handle, nullptr);
    json_tokener_parse = (void *(*)(const char *))
        dlsym(handle, "json_tokener_parse");
    ASSERT_NE(json_tokener_parse, nullptr);
    json_object_put = (int (*)(void *))dlsym(handle, "json_object_put");
    ASSERT_NE(json_object_put, nullptr);
    void *obj = json_tokener_parse(json2);
    ASSERT_NE(obj, nullptr);
    EXPECT_TRUE(m.fromJsonObj(obj));
    EXPECT_EQ(m.managementId(), PRIORITY2);
    json_object_put(obj);
    dlclose(handle);
}

// Tests loading fastjson library
TEST(jsonLoadTest, MethodLoadFastJson)
{
    Json2msg m;
    EXPECT_TRUE(m.selectLib("fastjson"));
    const char *lib = m.loadLibrary();
    EXPECT_STREQ(std::string(lib, 22).c_str(), "libptpmgmt_fastjson.so");
    EXPECT_TRUE(m.isLibShared());
    EXPECT_TRUE(m.fromJson(json1)); // To ensure libraries are load in memory
    void *handle = dlopen(lib, RTLD_NOW);
    ASSERT_NE(handle, nullptr);
    json_tokener_parse = (void *(*)(const char *))
        dlsym(handle, "fjson_tokener_parse");
    ASSERT_NE(json_tokener_parse, nullptr);
    json_object_put = (int (*)(void *))dlsym(handle, "fjson_object_put");
    ASSERT_NE(json_object_put, nullptr);
    void *obj = json_tokener_parse(json2);
    ASSERT_NE(obj, nullptr);
    EXPECT_TRUE(m.fromJsonObj(obj));
    EXPECT_EQ(m.managementId(), PRIORITY2);
    json_object_put(obj);
    dlclose(handle);
}
