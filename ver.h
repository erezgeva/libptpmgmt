/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief version functions
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#ifndef __PTPMGMT_VER_H
#define __PTPMGMT_VER_H

#ifndef SWIG
namespace ptpmgmt
{
#endif

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

#ifndef SWIG
}; /* namespace ptpmgmt */
#endif

#endif /* __PTPMGMT_VER_H */
