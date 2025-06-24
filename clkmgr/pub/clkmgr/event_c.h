/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief C wrapper to get clock synchronization events data
 *
 * @author Goh Wei Sheng <wei.sheng.goh@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#ifndef EVENT_C_H
#define EVENT_C_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque alias for clock synchronization data
 */
typedef struct Clkmgr_ClockSyncData Clkmgr_ClockSyncData;

/**
 * Construct a clock sync data object
 * @return Pointer to a new Clkmgr_ClockSyncData object
 */
Clkmgr_ClockSyncData *clkmgr_constructClockSyncDataInstance(void);

/**
 * Destroy a ClockSyncData_C object
 * @param[in] data_c Pointer to the Clkmgr_ClockSyncData object to destroy
 */
void clkmgr_destroyClockSyncDataInstance(Clkmgr_ClockSyncData *data_c);

/**
 * Check if the PTP clock is available
 * @param[in] data_c Pointer of the Clkmgr_ClockSyncData
 * @return True if the PTP clock is available, false otherwise
 */
bool clkmgr_havePtpData(const Clkmgr_ClockSyncData *data_c);

/**
 * Check if the system clock is available
 * @param[in] data_c Pointer of the Clkmgr_ClockSyncData
 * @return True if the system clock is available, false otherwise
 */
bool clkmgr_haveSysData(const Clkmgr_ClockSyncData *data_c);

/**
 * Get the clock offset in nanosecond
 * @param[in] data_c Pointer of the Clkmgr_ClockSyncData
 * @param[in] clock_type The type of clock to set
 * @return Clock offset in nanosecond
 * @note The clock type is a bit that represents a type of clock,
 *  as defined by enum Clkmgr_ClockType
 */
int64_t clkmgr_getClockOffset(const Clkmgr_ClockSyncData *data_c,
    enum Clkmgr_ClockType clock_type);

/**
 * Check if the clock offset is in-range
 * @param[in] data_c Pointer of the Clkmgr_ClockSyncData
 * @param[in] clock_type The type of clock to set
 * @return True if the clock offset is in-range, false otherwise
 * @note The range of clock offset is defined by user during subscription
 * @note The clock type is a bit that represents a type of clock,
 *  as defined by enum Clkmgr_ClockType
 */
bool clkmgr_isOffsetInRange(const Clkmgr_ClockSyncData *data_c,
    enum Clkmgr_ClockType clock_type);

/**
 * Get the count of clock offset in-range event
 * @param[in] data_c Pointer of the Clkmgr_ClockSyncData
 * @param[in] clock_type The type of clock to set
 * @return The count of clock offset in-range event
 * @note This count indicates the number of times the clock offset has
 *  transitioned from in-range to out-of-range or vice versa since the last
 *  call to statusWait() or statusWaitByName()
 * @note The clock type is a bit that represents a type of clock,
 *  as defined by enum Clkmgr_ClockType
 */
uint32_t clkmgr_getOffsetInRangeEventCount(const Clkmgr_ClockSyncData *data_c,
    enum Clkmgr_ClockType clock_type);

/**
 * Get the synchronization interval in microsecond
 * @param[in] data_c Pointer of the Clkmgr_ClockSyncData
 * @param[in] clock_type The type of clock to set
 * @return Synchronization interval in microsecond
 * @note This interval indicates the time between clock update messages send
 *  to the local service that synchronizing clock. This interval specify the
 *  time the synchronization protocol may need to react for a change in the
 *  grandmaster.
 * @note The clock type is a bit that represents a type of clock,
 *  as defined by enum Clkmgr_ClockType
 */
uint64_t clkmgr_getSyncInterval(const Clkmgr_ClockSyncData *data_c,
    enum Clkmgr_ClockType clock_type);

/**
 * Get the grandmaster clock identity
 * @param[in] data_c Pointer of the Clkmgr_ClockSyncData
 * @param[in] clock_type The type of clock to set
 * @return Grandmaster clock identity
 * @note The grandmaster is the source clock for the PTP domain (network),
 *  whom all the clocks will synchronize to.
 * @note The clock type is a bit that represents a type of clock,
 *  as defined by enum Clkmgr_ClockType
 */
uint64_t clkmgr_getGmIdentity(const Clkmgr_ClockSyncData *data_c,
    enum Clkmgr_ClockType clock_type);

/**
 * Check if the grandmaster has changed
 * @param[in] data_c Pointer of the Clkmgr_ClockSyncData
 * @param[in] clock_type The type of clock to set
 * @return True if the grandmaster has changed, false otherwise
 * @note This boolean variable indicates if the grandmaster has changed
 *  since the last call of statusWait() or statusWaitByName()
 * @note The clock type is a bit that represents a type of clock,
 *  as defined by enum Clkmgr_ClockType
 */
bool clkmgr_isGmChanged(const Clkmgr_ClockSyncData *data_c,
    enum Clkmgr_ClockType clock_type);

/**
 * Get the count of grandmaster changed event
 * @param[in] data_c Pointer of the Clkmgr_ClockSyncData
 * @param[in] clock_type The type of clock to set
 * @return The count of grandmaster changed event
 * @note This count indicates how many times the grandmaster has changed
 *  since the last call to statusWait() or statusWaitByName()
 * @note The clock type is a bit that represents a type of clock,
 *  as defined by enum Clkmgr_ClockType
 */
uint32_t clkmgr_getGmChangedEventCount(const Clkmgr_ClockSyncData *data_c,
    enum Clkmgr_ClockType clock_type);

/**
 * Get the notification timestamp in nanosecond
 * @param[in] data_c Pointer of the Clkmgr_ClockSyncData
 * @param[in] clock_type The type of clock to set
 * @return Notification timestamp in nanosecond
 * @note This timestamp indicates to the most recent time in CLOCK_REALTIME
 *  when the Client received a clock sync data notification from the Proxy
 * @note The clock type is a bit that represents a type of clock,
 * as defined by enum Clkmgr_ClockType
 */
uint64_t clkmgr_getNotificationTimestamp(const Clkmgr_ClockSyncData *data_c,
    enum Clkmgr_ClockType clock_type);

/**
 * Check if the PTP clock is synchronized with a grandmaster
 * @param[in] data_c Pointer of the Clkmgr_ClockSyncData
 * @return True if the clock is synchronized with a grandmaster, false if
 *  otherwise
 */
bool clkmgr_isPtpSyncedWithGm(const Clkmgr_ClockSyncData *data_c);

/**
 * Get the count of clock synced with grandmaster event
 * @param[in] data_c Pointer of the Clkmgr_ClockSyncData
 * @return The count of clock synced with grandmaster event
 * @note This count indicates the number of times the clock has transitioned
 *  from synced with grandmaster to out-of-sync or vice versa since the last
 *  call to statusWait() or statusWaitByName()
 */
uint32_t clkmgr_getPtpSyncedWithGmEventCount(const Clkmgr_ClockSyncData
    *data_c);

/**
 * Check if the clock is an IEEE 802.1AS capable
 * @param[in] data_c Pointer of the Clkmgr_ClockSyncData
 * @return True if the clock is an IEEE 802.1AS capable, false otherwise
 */
bool clkmgr_isPtpAsCapable(const Clkmgr_ClockSyncData *data_c);

/**
 * Get the count of IEEE 802.1AS capable event
 * @param[in] data_c Pointer of the Clkmgr_ClockSyncData
 * @return The count of IEEE 802.1AS capable event
 * @note This count indicates the number of times the clock has transitioned
 *  from non capable to capable or vice versa since the last call to
 *  statusWait() or statusWaitByName()
 */
uint32_t clkmgr_getPtpAsCapableEventCount(const Clkmgr_ClockSyncData *data_c);

/**
 * Check if all the conditions in the subscribed composite event are met
 * @param[in] data_c Pointer of the Clkmgr_ClockSyncData
 * @return True if all the conditions are met, false otherwise
 * @note Condition of composite event is defined by the user during
 *  subsciption
 */
bool clkmgr_isPtpCompositeEventMet(const Clkmgr_ClockSyncData *data_c);

/**
 * Get the count of composite event
 * @param[in] data_c Pointer of the Clkmgr_ClockSyncData
 * @return The count of composite event
 * @note This count indicates the number of times the condition of composite
 *  event has transitioned from not met to met or vice versa since the last
 *  call to statusWait() or statusWaitByName()
 */
uint32_t clkmgr_getPtpCompositeEventCount(const Clkmgr_ClockSyncData *data_c);

#ifdef __cplusplus
}
#endif

#endif /* EVENT_C_H */
