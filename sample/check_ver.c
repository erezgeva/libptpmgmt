/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Sample using the C wrapper, just print version
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 * @note compile : gcc -Wall check_ver.c -o check_ver -lptpmgmt
 *
 */

#include <stdio.h>
#include <ptpmgmt/ver.h>

int main(int argc, char *argv[])
{
    printf("compile version: %d.%d = %s (0x%d)\n", LIBPTPMGMT_VER_MAJ,
        LIBPTPMGMT_VER_MIN, LIBPTPMGMT_VER, LIBPTPMGMT_VER_VAL);
    printf("running version: %d.%d = %s (0x%d)\n", ptpmgmt_getVersionMajor(),
        ptpmgmt_getVersionMinor(), ptpmgmt_getVersion(), ptpmgmt_getVersionValue());
    return 0;
}
