/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client transport base class for clkmgr.
 * It is extended for specific transports such as POSIX message queue.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLIENT_TRANSPORT_HPP
#define CLIENT_TRANSPORT_HPP

#include "client/message.hpp"
#include "common/transport.hpp"
#include "common/util.hpp"

#include <cstdint>

__CLKMGR_NAMESPACE_BEGIN

class ClientTransportContext : virtual public TransportContext
{
  public:
    virtual ~ClientTransportContext() = default;
};

#define SEND_CLIENT_MESSAGE(name) bool name (Message *msg)

class ClientTransport : public Transport
{
  protected:
    static bool processMessage(Message *msg) { return false; }
  public:
    static bool init();
    static bool stop();
    static bool finalize();
    static void writeTransportClientId(Message0 &msg) {}
    static SEND_CLIENT_MESSAGE(sendMessage) { return false; }
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_TRANSPORT_HPP */
