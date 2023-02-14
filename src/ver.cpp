/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Version functions
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#include "ver.h"
#include "comp.h"

__PTPMGMT_NAMESPACE_BEGIN

// Version comes from 'version' file during build.
#define VER_STR(a, b) stringify(a) "." stringify(b)
// Must be here, must be in library Binary and not header source code!
const char *getVersion()
{
    return VER_STR(VER_MAJ, VER_MIN);
}
int getVersionMajor()
{
    return VER_MAJ;
}
int getVersionMinor()
{
    return VER_MIN;
}
unsigned getVersionValue()
{
    return VER_VAL;
}

__PTPMGMT_NAMESPACE_END
