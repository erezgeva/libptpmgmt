/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief internal ClientState class unit tests
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "client/client_state.hpp"

using namespace clkmgr;

// Tests subscribed status flag
// ClientState &getSingleInstance()
// bool get_connected() const
TEST(ClientStateTest, connected)
{
    const ClientState &cstate = ClientState::getSingleInstance();
    EXPECT_FALSE(cstate.get_connected());
}

// Tests client ID
// ClientId get_clientID() const
// void set_clientID(const std::string  &cID)
TEST(ClientStateTest, clientID)
{
    ClientState &cstate = ClientState::getSingleInstance();
    EXPECT_TRUE(cstate.get_clientID().empty());
    cstate.set_clientID("my new ID");
    EXPECT_STREQ(cstate.get_clientID().c_str(), "my new ID");
}

// Tests session ID
// sessionId_t get_sessionId() const
TEST(ClientStateTest, sessionId)
{
    const ClientState &cstate = ClientState::getSingleInstance();
    EXPECT_EQ(cstate.get_sessionId(), InvalidSessionId);
}

// TODO
// bool connect(uint32_t timeOut, timespec *lastConnectTime)
// bool connectReply(sessionId_t sessionId)
