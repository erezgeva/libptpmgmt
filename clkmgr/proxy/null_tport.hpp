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

#ifndef PROXY_NULL_TPORT_HPP
#define PROXY_NULL_TPORT_HPP

#include <cstdint>

#include <common/null_tport.hpp>
#include <proxy/transport.hpp>

__CLKMGR_NAMESPACE_BEGIN

class NullProxyTransport : public NullTransport
{
  public:
    static SEND_PROXY_MESSAGE(sendMessage);
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_NULL_TPORT_HPP */
