/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief Clock Manager class unit tests
 *
 * @author Song Yoong Siang <yoong.siang.song@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include "pub/clockmanager.h"
#include "client/client_state.hpp"
#include "client/timebase_state.hpp"
#include "common/timebase.hpp"

using namespace clkmgr;

// Used to define static members in ClientState class
DECLARE_STATIC(ClientState::m_clientID);
DECLARE_STATIC(ClientState::m_sessionId, InvalidSessionId);
DECLARE_STATIC(ClientState::m_connected, false);

// Used in ClockManager::connect() to setup listener and transmitter queues
bool ClientState::init()
{
    m_clientID = "/clkmgr.112233";
    return true;
}

// Used in ClockManager::connect() and check_proxy_liveness() to send a connect
// message to the proxy and wait for a reply
bool utest_connected_with_proxy = false;
bool ClientState::connect(uint32_t timeOut, timespec *lastConnectTime)
{
    // Simulate failure for testing
    if(!utest_connected_with_proxy) {
        m_connected = false;
        return false; // Simulate failure for testing
    }
    // Simulate a successful connection
    m_connected = true;
    if(lastConnectTime != nullptr)
        clock_gettime(CLOCK_REALTIME, lastConnectTime);
    return true;
}

// Used to define functions in TimeBaseState class
void TimeBaseState::set_subscribed(bool subscriptionState)
{
    subscribed = subscriptionState;
}
const timespec &TimeBaseState::get_last_notification_time() const
{
    return last_notification_time;
}
bool TimeBaseState::get_subscribed() const
{
    return subscribed;
}
bool TimeBaseState::is_event_changed() const
{
    return event_changed;
}
const PTPClockEvent &TimeBaseState::get_ptpEventState() const
{
    return ptpEventState;
}
const SysClockEvent &TimeBaseState::get_sysEventState() const
{
    return sysEventState;
}
void TimeBaseState::set_event_changed(bool state)
{
    event_changed = state;
}
void TimeBaseState::set_ptpEventState(const PTPClockEvent &ptpState)
{
    ptpEventState = ptpState;
}
void TimeBaseState::set_sysEventState(const SysClockEvent &chronyState)
{
    sysEventState = chronyState;
}
void TimeBaseState::set_last_notification_time(const timespec &newTime)
{
    last_notification_time = newTime;
}
bool TimeBaseState::havePtp() const
{
    return true;
}
bool TimeBaseState::haveSys() const
{
    return true;
}

// Used in _subscribe() to send a subscribe message to the proxy and wait for
// a confirmation reply
bool utest_subscribed_with_proxy = true;
bool TimeBaseStates::subscribe(size_t timeBaseIndex,
    const ClockSyncSubscription &newSub)
{
    // Check whether connection between Proxy and Client is established or not
    if(!ClientState::get_connected())
        return false;
    // Check whether requested timeBaseIndex is valid or not
    if(!TimeBaseConfigurations::isTimeBaseIndexPresent(timeBaseIndex))
        return false;
    // Simulate failure for testing
    if(!utest_subscribed_with_proxy) {
        setSubscribed(timeBaseIndex, false);
        return false;
    }
    // Simulate a successful subscription
    setSubscribed(timeBaseIndex, true);
    return true;
}

// Used in _subscribe() and _statusWait() to get the current clock sync data
// and reset the event counts
static ClockEventHandler ptpClockEventHandler(PTPClock);
static ClockEventHandler sysClockEventHandler(SysClock);
bool TimeBaseStates::getTimeBaseState(size_t timeBaseIndex,
    TimeBaseState &state)
{
    auto it = timeBaseStateMap.find(timeBaseIndex);
    if(it != timeBaseStateMap.end()) {
        state = it->second; // Copy the TimeBaseState object
        // Get the current state of the timebase
        PTPClockEvent ptpEventState = it->second.get_ptpEventState();
        SysClockEvent sysEventState = it->second.get_sysEventState();
        // Reset the Event Count
        ptpClockEventHandler.setOffsetInRangeEventCount(ptpEventState, 0);
        ptpClockEventHandler.setSyncedWithGmEventCount(ptpEventState, 0);
        ptpClockEventHandler.setGmChangedEventCount(ptpEventState, 0);
        ptpClockEventHandler.setAsCapableEventCount(ptpEventState, 0);
        ptpClockEventHandler.setCompositeEventCount(ptpEventState, 0);
        ptpClockEventHandler.setGmChanged(ptpEventState, false);
        it->second.set_ptpEventState(ptpEventState);
        sysClockEventHandler.setOffsetInRangeEventCount(sysEventState, 0);
        it->second.set_sysEventState(sysEventState);
        it->second.set_event_changed(false);
        return true;
    }
    // If timeBaseIndex is not found, return false
    return false;
}

// Create dummy timebase state for testing
void TimeBaseStates::setTimeBaseStatePtp(size_t timeBaseIndex,
    const ptp_event &newEvent)
{
    auto &state = timeBaseStateMap[1];
    // Update the notification timestamp
    timespec last_notification_time = {};
    clock_gettime(CLOCK_REALTIME, &last_notification_time);
    state.set_last_notification_time(last_notification_time);
    state.set_event_changed(true);
    // Get the current state of the timebase
    PTPClockEvent ptpEventState = state.get_ptpEventState();
    // Update EventOffsetInRange
    ptpClockEventHandler.setClockOffset(ptpEventState, 23);
    ptpClockEventHandler.setOffsetInRange(ptpEventState, true);
    ptpClockEventHandler.setOffsetInRangeEventCount(ptpEventState, 8);
    // Update EventSyncedWithGm
    ptpClockEventHandler.setSyncedWithGm(ptpEventState, true);
    ptpClockEventHandler.setSyncedWithGmEventCount(ptpEventState, 9);
    // Update EventGmChanged
    ptpClockEventHandler.setGmIdentity(ptpEventState, 0x1234ABCD);
    ptpClockEventHandler.setGmChanged(ptpEventState, true);
    ptpClockEventHandler.setGmChangedEventCount(ptpEventState, 10);
    // Update EventAsCapable
    ptpClockEventHandler.setAsCapable(ptpEventState, true);
    ptpClockEventHandler.setAsCapableEventCount(ptpEventState, 11);
    // Update composite event
    ptpClockEventHandler.setCompositeEvent(ptpEventState, true);
    ptpClockEventHandler.setCompositeEventCount(ptpEventState, 12);
    // Update notification timestamp
    uint64_t notification_timestamp = last_notification_time.tv_sec;
    notification_timestamp *= NSEC_PER_SEC;
    notification_timestamp += last_notification_time.tv_nsec;
    ptpClockEventHandler.setNotificationTimestamp(ptpEventState,
        notification_timestamp);
    // Update GM logSyncInterval
    ptpClockEventHandler.setSyncInterval(ptpEventState, 125000);
    state.set_ptpEventState(ptpEventState);
}

// Create dummy timebase state for testing
void TimeBaseStates::setTimeBaseStateSys(size_t timeBaseIndex,
    const chrony_event &newEvent)
{
    auto &state = timeBaseStateMap[1];
    // Update the notification timestamp
    timespec last_notification_time = {};
    clock_gettime(CLOCK_REALTIME, &last_notification_time);
    state.set_last_notification_time(last_notification_time);
    state.set_event_changed(true);
    // Get the current state of the timebase
    SysClockEvent sysEventState = state.get_sysEventState();
    // Update Chrony clock offset
    sysClockEventHandler.setClockOffset(sysEventState, 50000);
    sysClockEventHandler.setOffsetInRange(sysEventState, true);
    sysClockEventHandler.setOffsetInRangeEventCount(sysEventState, 13);
    sysClockEventHandler.setGmIdentity(sysEventState, 0x5678EF01);
    sysClockEventHandler.setSyncInterval(sysEventState, 10000);
    state.set_sysEventState(sysEventState);
}

// Create dummy TimeBaseConfigurations for testing
class clkmgr::ClientConnectMessage
{
  public:
    static void set() {
        const TimeBaseConfigurations &cfg = TimeBaseConfigurations::getInstance();
        cfg.addTimeBaseCfg({
            .timeBaseIndex = 1,
            .timeBaseName = { 'm', 'e', 0 },
            .interfaceName = { 'e', 't', 'h', '0', 0 },
            .transportSpecific = 4,
            .domainNumber = 1,
            .haveSys = true,
            .havePtp = true
        });
        cfg.addTimeBaseCfg({
            .timeBaseIndex = 2,
            .timeBaseName = { 't', 'o', 'o', 0 },
            .interfaceName = { 'e', 't', 'h', '1', 0 },
            .transportSpecific = 5,
            .domainNumber = 5,
            .haveSys = false,
            .havePtp = true
        });
    }
};

class ClockManagerTest : public ::testing::Test
{
  protected:
    void SetUp() override {
        ClientConnectMessage::set();
        ptp_event ptpData = {};
        chrony_event chronyData = {};
        TimeBaseStates::getInstance().setTimeBaseStatePtp(1, ptpData);
        TimeBaseStates::getInstance().setTimeBaseStateSys(1, chronyData);
    }
};

// static ClockManager &fetchSingleInstance()
TEST_F(ClockManagerTest, singleInstance)
{
    ClockManager &cm1 = ClockManager::fetchSingleInstance();
    ClockManager &cm2 = ClockManager::fetchSingleInstance();
    EXPECT_EQ(&cm1, &cm2);
}

// static bool connect()
// static bool disconnect()
TEST_F(ClockManagerTest, connectAndDisconnect)
{
    EXPECT_FALSE(ClockManager::connect());
    utest_connected_with_proxy = true;
    EXPECT_TRUE(ClockManager::connect());
    EXPECT_TRUE(ClockManager::connect());
    EXPECT_TRUE(ClockManager::disconnect());
    EXPECT_TRUE(ClockManager::disconnect());
}

// static const TimeBaseConfigurations &getTimebaseCfgs()
TEST_F(ClockManagerTest, getTimeBaseCfgs)
{
    TimeBaseConfigurations cfgs = ClockManager::getTimebaseCfgs();
    const auto record1 = cfgs.getRecord(1);
    EXPECT_EQ(record1.index(), 1);
    EXPECT_STREQ(record1.name_c(), "me");
    EXPECT_STREQ(record1.ptp().ifName_c(), "eth0");
    EXPECT_EQ(record1.ptp().transportSpecific(), 4);
    EXPECT_EQ(record1.ptp().domainNumber(), 1);
    EXPECT_TRUE(record1.haveSysClock());
    EXPECT_TRUE(record1.havePtp());
    const auto record2 = cfgs.getRecord(2);
    EXPECT_EQ(record2.index(), 2);
    EXPECT_STREQ(record2.name().c_str(), "too");
    EXPECT_STREQ(record2.ptp().ifName().c_str(), "eth1");
    EXPECT_EQ(record2.ptp().transportSpecific(), 5);
    EXPECT_EQ(record2.ptp().domainNumber(), 5);
    EXPECT_FALSE(record2.haveSysClock());
    EXPECT_TRUE(record2.havePtp());
    EXPECT_EQ(cfgs.size(), 2);
    EXPECT_TRUE(cfgs.isTimeBaseIndexPresent(1));
    EXPECT_TRUE(cfgs.isTimeBaseIndexPresent(2));
    EXPECT_FALSE(cfgs.isTimeBaseIndexPresent(3));
    size_t timeBaseIndex = 0;
    EXPECT_TRUE(cfgs.BaseNameToBaseIndex("me", timeBaseIndex));
    EXPECT_EQ(timeBaseIndex, 1);
    EXPECT_TRUE(cfgs.BaseNameToBaseIndex("too", timeBaseIndex));
    EXPECT_EQ(timeBaseIndex, 2);
    EXPECT_FALSE(cfgs.BaseNameToBaseIndex("unknown", timeBaseIndex));
    EXPECT_EQ(cfgs.getRecord(3).index(), 0);
    EXPECT_STREQ(cfgs.getRecord(3).name_c(), "");
}

// static bool subscribeByName(const ClockSyncSubscription &newSub,
//     const std::string &timeBaseName, ClockSyncData &clockSyncData)
// static bool subscribe(const ClockSyncSubscription &newSub,
//    size_t timeBaseIndex, ClockSyncData &clockSyncData)
TEST_F(ClockManagerTest, subscribe)
{
    // Ensure connection is established before testing subscribe functionality
    utest_connected_with_proxy = true;
    ClockManager::connect();
    ClockSyncSubscription sub;
    ClockSyncData clData;
    EXPECT_FALSE(ClockManager::subscribeByName(sub, "xyz", clData));
    EXPECT_FALSE(ClockManager::subscribe(sub, 0, clData));
    utest_subscribed_with_proxy = false;
    EXPECT_FALSE(ClockManager::subscribe(sub, 1, clData));
    utest_subscribed_with_proxy = true;
    EXPECT_TRUE(ClockManager::subscribe(sub, 1, clData));
    // check data
    EXPECT_TRUE(clData.havePTP());
    EXPECT_EQ(clData.getPtp().getClockOffset(), 23);
    EXPECT_TRUE(clData.getPtp().isOffsetInRange());
    EXPECT_EQ(clData.getPtp().getOffsetInRangeEventCount(), 8);
    EXPECT_TRUE(clData.getPtp().isSyncedWithGm());
    EXPECT_EQ(clData.getPtp().getSyncedWithGmEventCount(), 9);
    EXPECT_EQ(clData.getPtp().getGmIdentity(), 0x1234ABCD);
    EXPECT_TRUE(clData.getPtp().isGmChanged());
    EXPECT_EQ(clData.getPtp().getGmChangedEventCount(), 10);
    EXPECT_TRUE(clData.getPtp().isAsCapable());
    EXPECT_EQ(clData.getPtp().getAsCapableEventCount(), 11);
    EXPECT_TRUE(clData.getPtp().isCompositeEventMet());
    EXPECT_EQ(clData.getPtp().getCompositeEventCount(), 12);
    EXPECT_EQ(clData.getPtp().getSyncInterval(), 125000);
    EXPECT_TRUE(clData.haveSys());
    EXPECT_EQ(clData.getSysClock().getClockOffset(), 50000);
    EXPECT_TRUE(clData.getSysClock().isOffsetInRange());
    EXPECT_EQ(clData.getSysClock().getOffsetInRangeEventCount(), 13);
    EXPECT_EQ(clData.getSysClock().getGmIdentity(), 0x5678EF01);
    EXPECT_EQ(clData.getSysClock().getSyncInterval(), 10000);
}

// static int statusWaitByName(int timeout, const std::string &timeBaseName,
//     ClockSyncData &clockSyncData)
// static int statusWait(int timeout, size_t timeBaseIndex,
//     ClockSyncData &clockSyncData)
TEST_F(ClockManagerTest, statusWait)
{
    // Ensure connection and subscription are established before testing
    utest_connected_with_proxy = true;
    ClockManager::connect();
    // Subscribe to timebase 1 to ensure the timebase state is active
    utest_subscribed_with_proxy = true;
    ClockSyncSubscription sub;
    ClockSyncData dummyData;
    ClockManager::subscribe(sub, 1, dummyData);
    // Ensure timebase state is properly set up with events
    ptp_event ptpData = {};
    chrony_event chronyData = {};
    TimeBaseStates::getInstance().setTimeBaseStatePtp(1, ptpData);
    TimeBaseStates::getInstance().setTimeBaseStateSys(1, chronyData);
    ClockSyncData clData;
    EXPECT_EQ(ClockManager::statusWaitByName(0, "xyz", clData), -1);
    EXPECT_EQ(ClockManager::statusWait(0, 1, clData), 1);
    EXPECT_EQ(clData.getPtp().getClockOffset(), 23);
    EXPECT_EQ(clData.getPtp().getOffsetInRangeEventCount(), 8);
    EXPECT_EQ(clData.getPtp().getSyncedWithGmEventCount(), 9);
    EXPECT_EQ(clData.getPtp().getGmChangedEventCount(), 10);
    EXPECT_EQ(clData.getPtp().getAsCapableEventCount(), 11);
    EXPECT_EQ(clData.getPtp().getCompositeEventCount(), 12);
    EXPECT_EQ(clData.getSysClock().getClockOffset(), 50000);
    EXPECT_EQ(clData.getSysClock().getOffsetInRangeEventCount(), 13);
    EXPECT_EQ(ClockManager::statusWait(0, 1, clData), 0);
    EXPECT_EQ(clData.getPtp().getClockOffset(), 23);
    EXPECT_EQ(clData.getPtp().getOffsetInRangeEventCount(), 0);
    EXPECT_EQ(clData.getPtp().getSyncedWithGmEventCount(), 0);
    EXPECT_EQ(clData.getPtp().getGmChangedEventCount(), 0);
    EXPECT_EQ(clData.getPtp().getAsCapableEventCount(), 0);
    EXPECT_EQ(clData.getPtp().getCompositeEventCount(), 0);
    EXPECT_EQ(clData.getSysClock().getClockOffset(), 50000);
    EXPECT_EQ(clData.getSysClock().getOffsetInRangeEventCount(), 0);
    utest_connected_with_proxy = false;
    EXPECT_EQ(ClockManager::statusWait(1, 1, clData), -2);
}
