/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy message base class.
 * Extended for specific messages such as connect and subscribe.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef PROXY_MESSAGE_HPP
#define PROXY_MESSAGE_HPP

#include "common/message.hpp"

#include <cstdint>

__CLKMGR_NAMESPACE_BEGIN

class ProxyMessage : virtual public Message
{
  protected:
    ProxyMessage() : Message(NULL_MSG) {}
  public:
    static bool init();
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_MESSAGE_HPP */
