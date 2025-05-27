/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Set and get the client subscribe event state
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "client/client_state.hpp"

#include <cstring>
#include <string>

__CLKMGR_NAMESPACE_USE;

using namespace std;

ClientState::ClientState()
{
    connected = false;
    m_sessionId = InvalidSessionId;
}

ClientState::ClientState(const ClientState &newState)
{
    connected = newState.get_connected();
    m_sessionId = newState.get_sessionId();
    clientID = newState.get_clientID();
}

void ClientState::set_clientState(const ClientState &newState)
{
    connected = newState.get_connected();
    m_sessionId = newState.get_sessionId();
    clientID = newState.get_clientID();
}

bool ClientState::get_connected() const
{
    return connected;
}

void ClientState::set_connected(bool new_state)
{
    connected = new_state;
}

const string &ClientState::get_clientID() const
{
    return clientID;
}

void ClientState::set_clientID(const string &new_cID)
{
    clientID = new_cID.data();
}

sessionId_t ClientState::get_sessionId() const
{
    return m_sessionId;
}

void ClientState::set_sessionId(sessionId_t sessionId)
{
    m_sessionId = sessionId;
}
