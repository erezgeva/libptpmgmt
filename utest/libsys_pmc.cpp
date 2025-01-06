/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Inplement library C calls for PMC dump unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2023 Erez Geva
 *
 */

#include <string>
#include <cstdarg>
#include <dlfcn.h>
/*****************************************************************************/
static bool testMode = false;
static std::string pmc_out;
void useTestMode(bool n)
{
    testMode = n;
    if(n)
        pmc_out.clear();
}
const char *getPmcOut() {testMode = false; return pmc_out.c_str();}
void useRoot(bool n) {}
/*****************************************************************************/
#define sysFuncDec(ret, name, ...)\
    ret (*_##name)(__VA_ARGS__);\
    extern "C" ret name(__VA_ARGS__)
#define sysFuncAgn(ret, name, ...)\
    _##name = (ret(*)(__VA_ARGS__))dlsym(RTLD_NEXT, #name);\
    if(_##name == nullptr) {\
        fprintf(stderr, "Fail allocating " #name "\n");\
        fail = true;}
sysFuncDec(int, printf, const char *, ...);
sysFuncDec(int, __printf_chk, int, const char *, ...);
sysFuncDec(int, puts, const char *);
sysFuncDec(int, vprintf, const char *, va_list);
sysFuncDec(int, __vprintf_chk, int, const char *, va_list);
void initLibSys(void)
{
    bool fail = false;
    sysFuncAgn(int, printf, const char *, ...);
    sysFuncAgn(int, __printf_chk, int, const char *, ...);
    sysFuncAgn(int, puts, const char *);
    sysFuncAgn(int, vprintf, const char *, va_list);
    sysFuncAgn(int, __vprintf_chk, int, const char *, va_list);
    if(fail)
        fprintf(stderr, "Fail obtain address of functions\n");
    useTestMode(false);
}
/*****************************************************************************/
#define VA_PRINT(ap, format) _vprintf(ap, format)
#define retTest(name, ...)\
    if(!testMode)\
        return _##name(__VA_ARGS__)
#define L_PRINTF(format)\
    int ret = 0;\
    va_list ap;\
    va_start(ap, format);\
    if(!testMode)\
        ret = VA_PRINT(format, ap);\
    else {\
        char str[1024];\
        ret = vsnprintf(str, sizeof(str), format, ap);\
        pmc_out += str;\
    }\
    va_end(ap);\
    return ret
#define V_PRINTF(format, ap)\
    int ret = 0;\
    va_list ap1;\
    va_copy(ap1, ap);\
    if(!testMode)\
        ret = VA_PRINT(format, ap1);\
    else {\
        char str[1024];\
        ret = vsnprintf(str, sizeof(str), format, ap1);\
        pmc_out += str;\
    }\
    va_end(ap1);\
    return ret
/*****************************************************************************/
int printf(const char *format, ...)
{
    L_PRINTF(format);
}
int __printf_chk(int __flag, const char *format, ...)
{
    L_PRINTF(format);
}
int puts(const char *s)
{
    retTest(puts, s);
    return 0;
}
int vprintf(const char *format, va_list ap)
{
    V_PRINTF(format, ap);
}
int __vprintf_chk(int __flag, const char *format, va_list ap)
{
    V_PRINTF(format, ap);
}
