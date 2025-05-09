/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy null message class. Used for debug only. Should not be used.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef PROXY_NULL_MSG_HPP
#define PROXY_NULL_MSG_HPP

#include "proxy/message.hpp"
#include "common/null_msg.hpp"

__CLKMGR_NAMESPACE_BEGIN

class ProxyNullMessage : virtual public ProxyMessage,
    virtual public CommonNullMessage
{
  protected:
    ProxyNullMessage() : MESSAGE_NULL() {};
  public:
    static bool initMessage() { return true; }
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_NULL_MSG_HPP */
