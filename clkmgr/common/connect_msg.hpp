/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Common connect message class.
 * Implements common functions and (de-)serialization
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef COMMON_CONNECT_MSG_HPP
#define COMMON_CONNECT_MSG_HPP

#include "common/message.hpp"

__CLKMGR_NAMESPACE_BEGIN

class ConnectMessage : public Message
{
  private:
    ClientId clientId;
  protected:
    ConnectMessage() = default;
  public:
    static msgId_t get_ClassMsgId() { return CONNECT_MSG; }
    msgId_t get_msgId() const override final { return CONNECT_MSG; }
    ClientId &getClientId() { return clientId; }
    bool parseBuffer(Listener &rxContext) override;
    bool transmitMessage(Transmitter &txContext) override;
    bool makeBuffer(Transmitter &txContext) const override;
    std::string toString() override;
};

__CLKMGR_NAMESPACE_END

#endif /* COMMON_CONNECT_MSG_HPP */
