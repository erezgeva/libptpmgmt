/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client connect message class. Implements client specific functionality.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLIENT_CONNECT_MSG_HPP
#define CLIENT_CONNECT_MSG_HPP

#include "common/connect_msg.hpp"

__CLKMGR_NAMESPACE_BEGIN

class ClientConnectMessage : public ConnectMessage
{
  private:
    bool parseBufferTail() override final;
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_CONNECT_MSG_HPP */
