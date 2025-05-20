/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client POSIX message queue transport class.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLIENT_MSGQ_TPORT_HPP
#define CLIENT_MSGQ_TPORT_HPP

#include "common/msgq_tport.hpp"
#include "client/transport.hpp"

#include <string>

__CLKMGR_NAMESPACE_BEGIN

class ClientMessageQueue : public MessageQueue, public ClientTransport
{
  public:
    static bool initTransport();
    static bool stopTransport();
    static bool finalizeTransport();
    static bool stop();
    static bool writeTransportClientId(Message *msg);
    static bool sendMessage(Message *msg);
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_MSGQ_TPORT_HPP */
