/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Utility functions for printing
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef PRINT_HPP
#define PRINT_HPP

#include "common/util.hpp"

#include <cstdint>
#include <errno.h>
#include <string>

__CLKMGR_NAMESPACE_BEGIN

typedef std::remove_reference<decltype(errno)>::type errno_type;
#define PrintErrorCode(msg) PrintError(msg, errno)
#define PrintError(msg,...) ::clkmgr::_PrintError(msg, \
    __LINE__, __FILE__, __func__ __VA_OPT__(,) \
    __VA_ARGS__)
void _PrintError(std::string msg, uint16_t line, const char *file,
    const char *func, errno_type errnum = (errno_type) - 1);

#define PrintDebug(msg) ::clkmgr::_PrintDebug(msg, \
    __LINE__, __FILE__, __func__)
#define PrintInfo(msg) ::clkmgr::_PrintInfo(msg, \
    __LINE__, __FILE__, __func__)

void _PrintDebug(std::string msg, uint16_t line, const char *file,
    const char *func);
void _PrintInfo(std::string msg, uint16_t line, const char *file,
    const char *func);

#define DumpOctetArray(msg,arr,size) clkmgr::_DumpOctetArray(msg, arr, \
    size, __LINE__, __FILE__, __func__)

void _DumpOctetArray(std::string msg, const uint8_t *arr, size_t length,
    uint16_t line, const char *file, const char *func);

__CLKMGR_NAMESPACE_END

#endif /* PRINT_HPP */
