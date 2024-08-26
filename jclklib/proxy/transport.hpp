/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file transport.hpp
 * @brief Proxy transport base class for JClkLib.
 * It provides common functionality for derived transport classes.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#ifndef PROXY_TRANSPORT_HPP
#define PROXY_TRANSPORT_HPP

#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <thread>
#include <vector>

#include <common/transport.hpp>
#include <common/util.hpp>
#include <proxy/connect_msg.hpp>
#include <proxy/clock_config.hpp>
#include <proxy/message.hpp>

__CLKMGR_NAMESPACE_BEGIN

class ProxyTransportTransmitterContext : virtual public
    TransportTransmitterContext
{
  protected:
    ProxyTransportTransmitterContext() {}
  public:
    virtual ~ProxyTransportTransmitterContext() = default;
};

class ProxyTransportListenerContext : virtual public
    TransportListenerContext
{
  public:
    virtual ~ProxyTransportListenerContext() = default;
};

class ProxyTransport : public Transport
{
  public:
    static bool init();
    static bool stop();
    static bool finalize();
#define SEND_PROXY_MESSAGE(name)                    \
    bool name (const ProxyMessage *msg)
    static SEND_PROXY_MESSAGE(sendMessage) { return false; }
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_TRANSPORT_HPP */
