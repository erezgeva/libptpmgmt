/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy client infomation implementation
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef PROXY_CLIENT_HPP
#define PROXY_CLIENT_HPP

#include "common/msgq_tport.hpp"
#include "common/message.hpp"
#include "common/ptp_event.hpp"

#include <memory>
#include <map>
#include <rtpi/mutex.hpp>

__CLKMGR_NAMESPACE_BEGIN

static const size_t MAX_CLIENT_COUNT = 8;

class ClientRemoveAll;

class Client
{
  private:
    sessionId_t m_sessionId = InvalidSessionId;
    std::unique_ptr<Transmitter> m_transmitContext;
    static sessionId_t CreateClientSession(const std::string &id);
    static void RemoveClient(sessionId_t sessionId);
    static Client *getClient(sessionId_t sessionId);
    static bool connect_ptp4l();
    #ifdef HAVE_LIBCHRONY
    static bool connect_chrony();
    #endif

  protected:
    Transmitter *getTxContext() { return m_transmitContext.get(); }
    friend class ClientRemoveAll;

  public:
    static bool init();
    static sessionId_t connect(sessionId_t sessionId, const std::string &id);
    static bool subscribe(size_t timeBaseIndex, sessionId_t sessionId);
    static void NotifyClients(size_t timeBaseIndex);
    static Transmitter *getTxContext(sessionId_t sessionId);
    static rtpi::mutex &getTimeBaseLock(size_t timeBaseIndex);
    static ptp_event &getPTPEvent(size_t timeBaseIndex);
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_CLIENT_HPP */
