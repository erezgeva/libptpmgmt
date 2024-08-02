/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file transport.hpp
 * @brief Client transport base class for JClkLib.
 * It is extended for specific transports such as POSIX message queue.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#ifndef CLIENT_TRANSPORT_HPP
#define CLIENT_TRANSPORT_HPP

#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <thread>
#include <vector>

#include <client/message.hpp>
#include <common/transport.hpp>
#include <common/util.hpp>

namespace JClkLibClient
{
class ClientTransportContext : virtual public JClkLibCommon::TransportContext
{
  public:
    virtual ~ClientTransportContext() = default;
};

#define SEND_CLIENT_MESSAGE(name) bool name (::JClkLibCommon::Message *msg)

class ClientTransport : public JClkLibCommon::Transport
{
  protected:
    static bool processMessage(JClkLibCommon::Message *msg) { return false; }
  public:
    static bool init();
    static bool stop();
    static bool finalize();
    static void writeTransportClientId(Message0 &msg) {}
    static SEND_CLIENT_MESSAGE(sendMessage) { return false; }
};
}

#endif /* CLIENT_TRANSPORT_HPP */
