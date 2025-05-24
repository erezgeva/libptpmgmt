/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client queue class.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLIENT_MSGQ_TPORT_HPP
#define CLIENT_MSGQ_TPORT_HPP

#include "common/msgq_tport.hpp"

#include <string>

__CLKMGR_NAMESPACE_BEGIN

class Message;

class ClientQueue
{
  public:
    static bool init();
    static bool sendMessage(Message *msg);
    static Transmitter *getTransmitter();
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_MSGQ_TPORT_HPP */
