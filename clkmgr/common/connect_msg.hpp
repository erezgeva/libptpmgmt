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
    bool parseBufferComm() override final;
    bool makeBufferComm(Transmitter &txContext) const override final;

  protected:
    ConnectMessage() = default;
  public:
    msgId_t get_msgId() const override final { return CONNECT_MSG; }
    const ClientId &getClientId() const { return clientId; }
    std::string toString() const override;
};

__CLKMGR_NAMESPACE_END

#endif /* COMMON_CONNECT_MSG_HPP */
