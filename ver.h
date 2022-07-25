/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief Version functions
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#ifndef __PTPMGMT_VER_H
#define __PTPMGMT_VER_H

#include "name.h"

__PTPMGMT_NAMESPACE_BEGIN

/**
 * Get this library version
 * @return this library version
 */
const char *getVersion();
/**
 * Get this library major version
 * @return this library version major
 */
int getVersionMajor();
/**
 * Get this library minor version
 * @return this library version minor
 */
int getVersionMinor();
/**
 * Get this library version in 16 bits value
 * @return this library version value
 * @note version value use high byte for major and low byte for minor
 */
unsigned getVersionValue();

__PTPMGMT_NAMESPACE_END

#endif /* __PTPMGMT_VER_H */
