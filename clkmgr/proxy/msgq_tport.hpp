/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy POSIX message queue transport class.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef PROXY_MSGQ_TPORT_HPP
#define PROXY_MSGQ_TPORT_HPP

#include "common/msgq_tport.hpp"
#include "proxy/transport.hpp"

__CLKMGR_NAMESPACE_BEGIN

static const size_t MAX_CLIENT_COUNT = 8;

class ProxyMessageQueue : public MessageQueue, public ProxyTransport
{
  public:
    static bool initTransport();
    static bool stopTransport();
    static bool finalizeTransport();
    static bool stop();
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_MSGQ_TPORT_HPP */
