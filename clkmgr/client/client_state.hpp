/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Class to get and set the state of client-runtime
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLIENT_CLIENT_STATE_H
#define CLIENT_CLIENT_STATE_H

#include "common/util.hpp"
#include "pub/clkmgr/subscription.h"

#include <string>
#include <atomic>

__CLKMGR_NAMESPACE_BEGIN

/**
 * Class to keep the current state of client-runtime
 */
class ClientState
{
  private:
    std::atomic_bool connected{false}; /**< Connection status */
    sessionId_t m_sessionId = InvalidSessionId; /**< Session ID */
    TransportClientId clientID = {}; /**< Client ID */

  public:
    /**
     * Default constructor
     */
    ClientState();

    /**
     * Copy constructor
     * @param[in] newState Reference to the new state
     */
    ClientState(const ClientState &newState);

    /**
     * Set the client state
     * @param[in] newState Reference to the new state
     */
    void set_clientState(const ClientState &newState);

    /**
     * Get the connection status
     * @return true if connected, false otherwise
     */
    bool get_connected() const;

    /**
     * Set the connection status
     * @param[in] state Connection status
     */
    void set_connected(bool state);

    /**
     * Get the client ID
     * @return Client ID
     */
    TransportClientId get_clientID() const;

    /**
     * Set the client ID
     * @param[in] cID Reference to the client ID
     */
    void set_clientID(const TransportClientId &cID);

    /**
    * Get the constant reference to the session ID.
    * @return session ID.
    */
    sessionId_t get_sessionId() const;

    /**
    * Set the session ID.
    * @param[in] sessionId The new session ID to set.
    */
    void set_sessionId(sessionId_t sessionId);
};

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_CLIENT_STATE_H */
