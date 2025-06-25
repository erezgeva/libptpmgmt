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
#include "client/connect_msg.hpp"
#include "common/print.hpp"

using namespace clkmgr;

static Transmitter txContext;
bool utest_open_proxy = false;
bool thread_stopped = false;

static void signal_handler(int)
{
    thread_stopped = true;
    End::stopAll();
}

Transmitter *ClientState::getTransmitter()
{
    return &txContext;
}

class ClientStateTest : public ::testing::Test
{
  protected:
    Queue proxyListenerMq;
    std::string mqListenerName;
    void SetUp() override {
        mqListenerName = mqProxyName + "." + std::to_string(getpid());
        // Mock a proxy rx message queue
        proxyListenerMq.RxOpen(mqProxyName, 8);
    }

    void TearDown() override {
        // Cleanup the mock message queue
        proxyListenerMq.close();
        proxyListenerMq.remove();
    }

  public:
    static void SetUpTestSuite() {
        setLogLevel(0);
        signal(SIGINT, signal_handler);
        signal(SIGSEGV, signal_handler);
        signal(SIGTERM, signal_handler);
    }
    static void TearDownTestSuite() {
        if(!thread_stopped)
            // Ensure all threads are stopped
            End::stopAll();
    }
};

// Tests subscribed status flag
// bool get_connected()
TEST_F(ClientStateTest, connected)
{
    EXPECT_FALSE(ClientState::get_connected());
}

// Tests client ID
// ClientId get_clientID()
TEST_F(ClientStateTest, clientID)
{
    EXPECT_TRUE(ClientState::get_clientID().empty());
}

// Tests session ID
// sessionId_t get_sessionId()
TEST_F(ClientStateTest, sessionId)
{
    EXPECT_EQ(ClientState::get_sessionId(), InvalidSessionId);
}

// Tests init function
// bool init()
// ClientId get_clientID()
TEST_F(ClientStateTest, init)
{
    ASSERT_TRUE(ClientState::init());
    EXPECT_EQ(ClientState::get_clientID(), mqListenerName);
}

// Tests sendMessage function
// bool sendMessage(Message &msg)
TEST_F(ClientStateTest, sendMessage)
{
    ClientConnectMessage *cmsg = new ClientConnectMessage();
    EXPECT_TRUE(ClientState::sendMessage(*cmsg));
}

// Tests getTransmitter function
// Transmitter *getTransmitter()
TEST_F(ClientStateTest, getTransmitter)
{
    // getTransmitter should return a non-null pointer
    Transmitter *tx1 = ClientState::getTransmitter();
    EXPECT_NE(tx1, nullptr);
    // getTransmitter should return the same pointer(singleton)
    Transmitter *tx2 = ClientState::getTransmitter();
    EXPECT_EQ(tx1, tx2);
}

// Tests connect and connectReply function
// bool connect(uint32_t timeOut, timespec *lastConnectTime = nullptr)
// bool connectReply(sessionId_t sessionId)
// bool get_connected()
// sessionId_t get_sessionId()
// const std::string &get_clientID()
TEST_F(ClientStateTest, connectAndReply)
{
    timespec lastConnectTime = {};
    bool result = false;
    // Simulate client not able to connect to proxy within timeout
    EXPECT_FALSE(ClientState::get_connected());
    ASSERT_FALSE(ClientState::connect(1000, &lastConnectTime));
    EXPECT_FALSE(ClientState::get_connected());
    EXPECT_EQ(ClientState::get_sessionId(), InvalidSessionId);
    EXPECT_EQ(lastConnectTime.tv_sec, 0);
    // Simulate client able to connect to proxy
    // Create a thread to run so that in parallel can simulate the proxy respond
    std::thread connectThread([&]() {
        result = ClientState::connect(1000, &lastConnectTime);
    });
    usleep(1000);
    // Set the session ID directly for testing purposes
    // Assume client received reply from proxy and call connectReply()
    ClientState::connectReply(12345);
    // Wait for the thread to finish
    connectThread.join();
    ASSERT_TRUE(result);
    EXPECT_TRUE(ClientState::get_connected());
    EXPECT_EQ(ClientState::get_clientID(), mqListenerName);
    EXPECT_EQ(ClientState::get_sessionId(), 12345);
    EXPECT_GT(lastConnectTime.tv_sec, 0);
}
