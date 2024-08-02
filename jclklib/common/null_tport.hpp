/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file null_tport.hpp
 * @brief Null transport class. Non-functional for testing only.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#ifndef NULL_TPORT_HPP
#define NULL_TPORT_HPP

#include <common/transport.hpp>

#include <cstdint>

namespace JClkLibCommon
{
class NullTransport : public Transport
{
  public:
    static bool initTransport() { return true; }
    static bool stopTransport() { return true; }
    static bool finalizeTransport() { return true; }
};
}

#endif /* NULL_TPORT_HPP */
