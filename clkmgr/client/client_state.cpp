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
#include "client/msgq_tport.hpp"
#include "common/clkmgrtypes.hpp"
#include "common/message.hpp"
#include "common/transport.hpp"

#include <cstring>
#include <string>

__CLKMGR_NAMESPACE_USE;

using namespace std;

ClientState::ClientState()
{
    connected = false;
    m_sessionId = InvalidSessionId;
    fill(begin(clientID), end(clientID), 0);
}

ClientState::ClientState(const ClientState &newState)
{
    connected = newState.get_connected();
    m_sessionId = newState.get_sessionId();
    strcpy((char *)clientID.data(), (char *)newState.get_clientID().data());
}

void ClientState::set_clientState(const ClientState &newState)
{
    connected = newState.get_connected();
    m_sessionId = newState.get_sessionId();
    strcpy((char *)clientID.data(), (char *)newState.get_clientID().data());
}

bool ClientState::get_connected() const
{
    return connected;
}

void ClientState::set_connected(bool new_state)
{
    connected = new_state;
}

TransportClientId ClientState::get_clientID() const
{
    return clientID;
}

void ClientState::set_clientID(const TransportClientId &new_cID)
{
    strcpy((char *)clientID.data(), (char *)new_cID.data());
}

sessionId_t ClientState::get_sessionId() const
{
    return m_sessionId;
}

void ClientState::set_sessionId(sessionId_t sessionId)
{
    m_sessionId = sessionId;
}
