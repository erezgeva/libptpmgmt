/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file print.hpp
 * @brief Utility functions for printing
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#ifndef PRINT_HPP
#define PRINT_HPP

#include <cstdint>
#include <errno.h>
#include <string>
#include <common/util.hpp>

__CLKMGR_NAMESPACE_BEGIN

typedef std::remove_reference<decltype(errno)>::type errno_type;
#define PrintErrorCode(msg) PrintError(msg, errno)
#define PrintError(msg,...) ::clkmgr::_PrintError(msg, \
    __LINE__, __FILE__, __func__ __VA_OPT__(,) \
    __VA_ARGS__)
void _PrintError(std::string msg, uint16_t line, std::string file,
    std::string func, errno_type errnum = (errno_type) - 1);

#define PrintDebug(msg) ::clkmgr::_PrintDebug(msg, \
    __LINE__, __FILE__, __func__)
#define PrintInfo(msg) ::clkmgr::_PrintInfo(msg, \
    __LINE__, __FILE__, __func__)

void _PrintDebug(std::string msg, uint16_t line, std::string file,
    std::string func);
void _PrintInfo(std::string msg, uint16_t line, std::string file,
    std::string func);

#define DumpOctetArray(msg,arr,size) clkmgr::_DumpOctetArray(msg, arr, \
    size, __LINE__, __FILE__, __func__)

void _DumpOctetArray(std::string msg, const std::uint8_t *arr,
    std::size_t length,
    std::uint16_t line, std::string file, std::string func);

__CLKMGR_NAMESPACE_END

#endif /* PRINT_HPP */
