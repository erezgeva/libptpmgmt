/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Error class unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include <locale.h>
#include <errno.h>
#include "err.h"

extern void uctest_error(const char *file, int line, const char *func);
extern void uctest_perror(const char *file, int line, const char *func);

// Tests is error method
// int ptpmgmt_err_isError()
Test(ErrorTest, MethodIsError)
{
    errno = EAGAIN;
    uctest_perror(__FILE__, __LINE__, __func__);
    cr_expect(ptpmgmt_err_isError());
    setlocale(LC_MESSAGES, "C");
    cr_expect(eq(str, (char *)ptpmgmt_err_getError(),
            "[uctest/err.c:24:ErrorTest_MethodIsError_impl] test macro: "
            "Resource temporarily unavailable"));
}

// Tests get error full message
// const char *ptpmgmt_err_getError()
Test(ErrorTest, MethodGetError)
{
    uctest_error(__FILE__, __LINE__, __func__);
    cr_expect(ptpmgmt_err_isError());
    cr_expect(eq(str, (char *)ptpmgmt_err_getError(),
            "[uctest/err.c:36:ErrorTest_MethodGetError_impl] test macro"));
}

// Tests get error file name
// const char *ptpmgmt_err_getFile()
Test(ErrorTest, MethodGetFile)
{
    uctest_error(__FILE__, __LINE__, __func__);
    cr_expect(ptpmgmt_err_isError());
    cr_expect(eq(str, (char *)ptpmgmt_err_getFile(), "uctest/err.c"));
}

// Tests get error file line
// int ptpmgmt_err_getFileLine()
Test(ErrorTest, MethodGetFileLine)
{
    uctest_error(__FILE__, __LINE__, __func__);
    cr_expect(ptpmgmt_err_isError());
    cr_expect(eq(i32, ptpmgmt_err_getFileLine(), 55));
}

// Tests get error in function name
// const char *ptpmgmt_err_getFunc()
Test(ErrorTest, MethodGetFunc)
{
    uctest_error(__FILE__, __LINE__, __func__);
    cr_expect(ptpmgmt_err_isError());
    cr_expect(eq(str, (char *)ptpmgmt_err_getFunc(),
            "ErrorTest_MethodGetFunc_impl"));
}

// Tests get error errno
// int ptpmgmt_err_getErrno()
Test(ErrorTest, MethodGetErrno)
{
    uctest_error(__FILE__, __LINE__, __func__);
    cr_expect(ptpmgmt_err_isError());
    cr_expect(eq(i32, ptpmgmt_err_getErrno(), 0));
    errno = EAGAIN;
    uctest_perror(__FILE__, __LINE__, __func__);
    cr_expect(ptpmgmt_err_isError());
    cr_expect(eq(i32, ptpmgmt_err_getErrno(), EAGAIN));
}

// Tests get error mesage part
// const char *ptpmgmt_err_getMsg()
Test(ErrorTest, MethodGetMsg)
{
    uctest_error(__FILE__, __LINE__, __func__);
    cr_expect(ptpmgmt_err_isError());
    cr_expect(eq(str, (char *)ptpmgmt_err_getMsg(), "test macro"));
}

// Tests get error errno message
// const char *ptpmgmt_err_getErrnoMsg()
Test(ErrorTest, MethodGetErrnoMsg)
{
    errno = ENOENT;
    uctest_perror(__FILE__, __LINE__, __func__);
    cr_expect(ptpmgmt_err_isError());
    setlocale(LC_MESSAGES, "C");
    cr_expect(eq(str, (char *)ptpmgmt_err_getErrnoMsg(),
            "No such file or directory"));
}
