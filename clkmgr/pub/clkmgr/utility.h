/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Helper functions and macros
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLKMGR_UTILITY_H
#define CLKMGR_UTILITY_H

#include <array>
#include <cstdint>

#ifndef __CLKMGR_NAMESPACE_USE
/** Using clkmgr namespace */
#define __CLKMGR_NAMESPACE_USE using namespace clkmgr
/** Define start of clkmgr namespace block */
#define __CLKMGR_NAMESPACE_BEGIN namespace clkmgr {
/** Define end of clkmgr namespace block */
#define __CLKMGR_NAMESPACE_END }
#endif

__CLKMGR_NAMESPACE_BEGIN

/** Maximum number of character for UDS address. */
const int UDS_ADDRESS_LENGTH = 64;

/** Array to store UDS address. */
typedef std::array<uint8_t, UDS_ADDRESS_LENGTH> UDSAddress;

__CLKMGR_NAMESPACE_END

#endif /* CLKMGR_UTILITY_H */
