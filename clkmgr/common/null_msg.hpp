/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file
 * @brief Non-functional message class for debug
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef COMMON_NULL_MSG_HPP
#define COMMON_NULL_MSG_HPP

#include <common/message.hpp>

__CLKMGR_NAMESPACE_BEGIN

class CommonNullMessage : virtual public Message
{
  protected:
  public:
    static msgId_t getMsgId() { return NULL_MSG; }
    static MAKE_RXBUFFER_TYPE(buildMessage);
#define MESSAGE_NULL() Message(NULL_MSG)
    CommonNullMessage() : MESSAGE_NULL() {};
    static bool initMessage();
};

__CLKMGR_NAMESPACE_END

#endif /* COMMON_NULL_MSG_HPP */
