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
#include "pub/clkmgr/types.h"

__CLKMGR_NAMESPACE_BEGIN

static const size_t MAX_CLIENT_COUNT = 8;

class Transmitter;
class ClientRemoveAll;

class ptpEvent
{
  private:
    size_t timeBaseIndex;
  public:
    struct ptp_event event;
    ptpEvent(size_t index);
    void portClear() {
        event.clockOffset = 0;
        event.gmClockUUID = 0;
        event.syncInterval = 0;
    }
    void copy();
};

class chronyEvent
{
  private:
    size_t timeBaseIndex;
  public:
    struct chrony_event event;
    chronyEvent(size_t index);
    void clear() {
        event = { 0 };
        copy();
    }
    void copy();
};

class Client
{
  private:
    sessionId_t m_sessionId = InvalidSessionId;
    std::unique_ptr<Transmitter> m_transmitter;
    static sessionId_t CreateClientSession(const std::string &id);
    static void RemoveClient(sessionId_t sessionId);
    static Client *getClient(sessionId_t sessionId);
    static bool connect_ptp4l();
    #ifdef HAVE_LIBCHRONY
    static bool connect_chrony();
    #endif

  protected:
    static Transmitter *getTransmitter(sessionId_t sessionId);
    Transmitter *getTransmitter() { return m_transmitter.get(); }
    friend class Transmitter;
    friend class ClientRemoveAll;

  public:
    static bool init(bool useMsgQAllAccess);
    static sessionId_t connect(sessionId_t sessionId, const std::string &id);
    static bool subscribe(size_t timeBaseIndex, sessionId_t sessionId);
    static void NotifyClients(size_t timeBaseIndex, ClockType type);
    static void getPTPEvent(size_t timeBaseIndex, ptp_event &event);
    static void getChronyEvent(size_t timeBaseIndex, chrony_event &event);
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_CLIENT_HPP */
