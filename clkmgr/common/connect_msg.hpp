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

class CommonConnectMessage : virtual public Message
{
  private:
    ClientId clientId;
  protected:
    CommonConnectMessage() : Message(CONNECT_MSG) {}
  public:
    ClientId &getClientId() { return clientId; }
    virtual bool parseBuffer(Listener &LxContext);
    static bool registerBuild();
    virtual bool transmitMessage(Transmitter &TxContext);
    virtual bool makeBuffer(Transmitter &TxContext) const;
    virtual std::string toString();
};

__CLKMGR_NAMESPACE_END

#endif /* COMMON_CONNECT_MSG_HPP */
