/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file sighandler.hpp
 * @brief Signal handling utilities
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#ifndef SIGHANDLER_HPP
#define SIGHANDLER_HPP

#include <thread>

namespace JClkLibCommon
{
    bool BlockStopSignal();
    bool WaitForStopSignal();
    bool EnableSyscallInterruptSignal();
    bool SendSyscallInterruptSignal(std::thread &t);
}
#endif /* SIGHANDLER_HPP */

