/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy queue class.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef PROXY_MSGQ_TPORT_HPP
#define PROXY_MSGQ_TPORT_HPP

#include "common/msgq_tport.hpp"

__CLKMGR_NAMESPACE_BEGIN

static const size_t MAX_CLIENT_COUNT = 8;

class ProxyQueue
{
  public:
    static bool init();
    static bool stop();
    static bool finalize();
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_MSGQ_TPORT_HPP */
