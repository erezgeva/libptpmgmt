/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Error class unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

#include <locale.h>
#include "comp.h"

__PTPMGMT_NAMESPACE_USE;

// Tests error macro
// #define PTPMGMT_ERROR(format, ...)
TEST(ErrorTest, MacroError)
{
    PTPMGMT_ERROR("test %s", "macro");
    EXPECT_TRUE(Error::isError());
    EXPECT_STREQ(Error::getError().c_str(),
        "[utest/err.cpp:21:TestBody] test macro");
}

// Tests system error macro
// #define PTPMGMT_ERROR_P(format, ...)
TEST(ErrorTest, MacroPerror)
{
    errno = EINTR;
    PTPMGMT_ERROR_P("test %s", "macro");
    EXPECT_TRUE(Error::isError());
    setlocale(LC_MESSAGES, "C");
    EXPECT_STREQ(Error::getError().c_str(),
        "[utest/err.cpp:32:TestBody] test macro: Interrupted system call");
}

// Tests error clear macro
// #define PTPMGMT_ERROR_CLR
TEST(ErrorTest, MacroErrorClr)
{
    PTPMGMT_ERROR_CLR;
    EXPECT_FALSE(Error::isError());
}

// Tests is error method
// static bool isError()
TEST(ErrorTest, MethodIsError)
{
    errno = EAGAIN;
    PTPMGMT_ERROR_P("test %s", "macro");
    EXPECT_TRUE(Error::isError());
    setlocale(LC_MESSAGES, "C");
    EXPECT_STREQ(Error::getError().c_str(),
        "[utest/err.cpp:52:TestBody] test macro: Resource temporarily unavailable");
}

// Tests get error full message
// static std::string getError()
TEST(ErrorTest, MethodGetError)
{
    PTPMGMT_ERROR("test %s", "macro");
    EXPECT_TRUE(Error::isError());
    EXPECT_STREQ(Error::getError().c_str(),
        "[utest/err.cpp:63:TestBody] test macro");
}

// Tests get error file name
// static std::string getFile()
TEST(ErrorTest, MethodGetFile)
{
    PTPMGMT_ERROR("test %s", "macro");
    EXPECT_TRUE(Error::isError());
    EXPECT_STREQ(Error::getFile().c_str(), "utest/err.cpp");
}

// Tests get error file line
// static int getFileLine()
TEST(ErrorTest, MethodGetFileLine)
{
    PTPMGMT_ERROR("test %s", "macro");
    EXPECT_TRUE(Error::isError());
    EXPECT_EQ(Error::getFileLine(), 82);
}

// Tests get error in function name
// static std::string getFunc()
TEST(ErrorTest, MethodGetFunc)
{
    PTPMGMT_ERROR("test %s", "macro");
    EXPECT_TRUE(Error::isError());
    EXPECT_STREQ(Error::getFunc().c_str(), "TestBody");
}

// Tests get error errno
// static int getErrno()
TEST(ErrorTest, MethodGetErrno)
{
    PTPMGMT_ERROR("test %s", "macro");
    EXPECT_TRUE(Error::isError());
    EXPECT_EQ(Error::getErrno(), 0);
    errno = EAGAIN;
    PTPMGMT_ERROR_P("test %s", "macro");
    EXPECT_TRUE(Error::isError());
    EXPECT_EQ(Error::getErrno(), EAGAIN);
}

// Tests get error mesage part
// static std::string getMsg()
TEST(ErrorTest, MethodGetMsg)
{
    PTPMGMT_ERROR("test %s", "macro");
    EXPECT_TRUE(Error::isError());
    EXPECT_STREQ(Error::getMsg().c_str(), "test macro");
}

// Tests get error errno message
// static std::string getErrnoMsg()
TEST(ErrorTest, MethodGetErrnoMsg)
{
    errno = ENOENT;
    PTPMGMT_ERROR_P("test %s", "macro");
    EXPECT_TRUE(Error::isError());
    setlocale(LC_MESSAGES, "C");
    EXPECT_STREQ(Error::getErrnoMsg().c_str(), "No such file or directory");
}
