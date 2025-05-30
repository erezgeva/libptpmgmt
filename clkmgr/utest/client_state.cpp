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
// bool get_connected()
TEST(ClientStateTest, connected)
{
    EXPECT_FALSE(ClientState::get_connected());
}

// Tests client ID
// ClientId get_clientID()
TEST(ClientStateTest, clientID)
{
    EXPECT_TRUE(ClientState::get_clientID().empty());
}

// Tests session ID
// sessionId_t get_sessionId()
TEST(ClientStateTest, sessionId)
{
    EXPECT_EQ(ClientState::get_sessionId(), InvalidSessionId);
}

// TODO
// bool init()
// bool sendMessage(Message *msg)
// Transmitter *getTransmitter()
// bool connect(uint32_t timeOut, timespec *lastConnectTime)
// bool connectReply(sessionId_t sessionId)
