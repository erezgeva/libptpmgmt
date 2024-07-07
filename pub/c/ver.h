/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Version functions for C
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 * This header is generated automatically.
 * @note The compilation macroes are provided for compilation only
 *       scripts should use version functions.
 */

#ifndef __PTPMGMT_C_VER_H
#define __PTPMGMT_C_VER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get this library version
 * @return this library version
 */
const char *ptpmgmt_getVersion();
/**
 * Get this library major version
 * @return this library version major
 */
int ptpmgmt_getVersionMajor();
/**
 * Get this library minor version
 * @return this library version minor
 */
int ptpmgmt_getVersionMinor();
/**
 * Get this library version in 16 bits value
 * @return this library version value
 * @note version value use high byte for major and low byte for minor
 */
unsigned ptpmgmt_getVersionValue();

#ifdef __cplusplus
}
#endif

#endif /* __PTPMGMT_C_VER_H */
