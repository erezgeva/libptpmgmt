/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file
 * @brief Null transport implementation. Non-functional for testing only.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLIENT_NULL_TPORT_HPP
#define CLIENT_NULL_TPORT_HPP

#include <cstdint>

#include <client/transport.hpp>
#include <common/null_tport.hpp>

__CLKMGR_NAMESPACE_BEGIN

class NullClientTransport : public NullTransport
{
  public:
    static bool sendMessage(std::uint8_t *buffer, std::size_t length);
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_NULL_TPORT_HPP */
