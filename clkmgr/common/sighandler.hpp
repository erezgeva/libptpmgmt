/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Signal handling utilities
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef SIGHANDLER_HPP
#define SIGHANDLER_HPP

#include "common/util.hpp"

#include <thread>

__CLKMGR_NAMESPACE_BEGIN

bool BlockStopSignal();
bool WaitForStopSignal();
bool EnableSyscallInterruptSignal();
bool SendSyscallInterruptSignal(std::thread &t);

__CLKMGR_NAMESPACE_END
#endif /* SIGHANDLER_HPP */
