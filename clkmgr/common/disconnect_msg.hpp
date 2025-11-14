/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Common disconnect message class.
 * Implements common functions and (de-)serialization
 *
 * @author Goh Wei Sheng <wei.sheng.goh@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#ifndef COMMON_DISCONNECT_MSG_HPP
#define COMMON_DISCONNECT_MSG_HPP

#include "common/message.hpp"

__CLKMGR_NAMESPACE_BEGIN

class DisconnectMessage : public Message
{
  protected:
    DisconnectMessage() = default;
    bool parseBufferComm() override final;
    bool makeBufferComm(Buffer &buff) const override final;

  public:
    msgId_t get_msgId() const override final { return DISCONNECT_MSG; }
};

__CLKMGR_NAMESPACE_END

#endif /* COMMON_DISCONNECT_MSG_HPP */
