/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Version functions unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include <stdio.h>
#include "ver.h"

// Tests Version major macro
// #define LIBPTPMGMT_VER_MAJ
Test(VerTest, MacroMajor)
{
    cr_expect(gt(int, LIBPTPMGMT_VER_MAJ, 0));
}

// Tests Version minor macro
// #define LIBPTPMGMT_VER_MIN
Test(VerTest, MacroMinor)
{
    cr_expect(ge(int, LIBPTPMGMT_VER_MIN, 0));
    cr_expect(le(int, LIBPTPMGMT_VER_MIN, 0xFF));
}

// Tests Version macro
// #define LIBPTPMGMT_VER_VAL
Test(VerTest, MacroVal)
{
    cr_expect(eq(int, LIBPTPMGMT_VER_VAL,
            LIBPTPMGMT_VER_MIN + LIBPTPMGMT_VER_MAJ * 0x100));
}

// Tests Version string
// #define LIBPTPMGMT_VER
Test(VerTest, MacroStr)
{
    char ver[100];
    snprintf(ver, sizeof(ver), "%d.%d", LIBPTPMGMT_VER_MAJ, LIBPTPMGMT_VER_MIN);
    cr_expect(eq(str, ver, LIBPTPMGMT_VER));
}

// NOTE: macros and function return the same value,
//       since the unit test always run on the same library it test!

// Tests get Version string
// const char *ptpmgmt_getVersion()
Test(VerTest, FuncStr)
{
    char *s = (char *)ptpmgmt_getVersion();
    cr_expect(eq(str, s, LIBPTPMGMT_VER));
}

// Tests get Version major
// int ptpmgmt_getVersionMajor()
Test(VerTest, FuncMajor)
{
    cr_expect(eq(int, ptpmgmt_getVersionMajor(), LIBPTPMGMT_VER_MAJ),
        "function and macro must be identical in test");
}

// Tests get Version minor
// int ptpmgmt_getVersionMinor()
Test(VerTest, FuncMinor)
{
    cr_expect(eq(int, ptpmgmt_getVersionMinor(), LIBPTPMGMT_VER_MIN),
        "function and macro must be identical in test");
}

// Tests get Version value
// unsigned ptpmgmt_getVersionValue()
Test(VerTest, FuncVal)
{
    cr_expect(eq(int, ptpmgmt_getVersionValue(), LIBPTPMGMT_VER_VAL),
        "function and macro must be identical in test");
}
