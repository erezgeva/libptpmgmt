/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief C wrapper to set clock event subscription
 *
 * @author Goh Wei Sheng <wei.sheng.goh@@intel.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#ifndef CLKMGR_SUBSCRIPTION_C_H
#define CLKMGR_SUBSCRIPTION_C_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Opaque alias for clock subscription
 */
typedef struct Clkmgr_Subscription Clkmgr_Subscription;

/**
 * Construct a clock subscription object
 * @return Pointer to a new Clkmgr_Subscription object
 */
Clkmgr_Subscription *clkmgr_constructSubscriptionInstance(void);

/**
 * Destroy a clock subscription object
 */
void clkmgr_destroySubscriptionInstance(Clkmgr_Subscription *sub_c);

/**
 * Set the event mask
 * @param[in] sub_c Pointer of the Clkmgr_Subscription
 * @param[in] clock_type The type of clock to set
 * @param[in] mask The new event mask to set
 * @return True if the event mask is set successfully, false otherwise
 * @note The clock type is a bit that represents a type of clock,
 * as defined by enum Clkmgr_ClockType
 * @note The event mask is a bitmask where each bit represents an event,
 * as defined by enum Clkmgr_EventIndex
 */
bool clkmgr_setEventMask(Clkmgr_Subscription *sub_c,
    enum Clkmgr_ClockType clock_type, uint32_t mask);

/**
 * Get the value of the event mask
 * @param[in] sub_c Pointer of the Clkmgr_Subscription
 * @param[in] clock_type The type of clock to get
 * @return The value of the event mask
 * @note The clock type is a bit that represents a type of clock,
 * as defined by enum Clkmgr_ClockType
 */
uint32_t clkmgr_getEventMask(const Clkmgr_Subscription *sub_c,
    enum Clkmgr_ClockType clock_type);

/**
 * Set the PTP composite event mask.
 * @param[in] sub_c Pointer of the Clkmgr_Subscription
 * @param[in] mask The new composite event mask to set.
 * @return True if the composite event mask is set successfully, false
 * otherwise.
 * @note The compositie event mask is a bitmask where each bit represents an
 * event, as defined by enum Clkmgr_EventIndex and COMPOSITE_EVENT_ALL.
 */
bool clkmgr_setPtpCompositeEventMask(Clkmgr_Subscription *sub_c,
    uint32_t mask);

/**
 * Get the value of the PTP composite event mask.
 * @param[in] sub_c Pointer of the Clkmgr_Subscription
 * @return The composite event mask.
 */
uint32_t clkmgr_getPtpCompositeEventMask(const Clkmgr_Subscription *sub_c);

/**
 * Set the threshold of specify clock offset
 * @param[in] sub_c Pointer of the Clkmgr_Subscription
 * @param[in] clock_type The type of clock to set
 * @param[in] threshold Threshold of clock offset
 * @return True if the offset threshold is set successfully, false otherwise
 * @note The clock type is a bit that represents a type of clock,
 * as defined by enum Clkmgr_ClockType
 * @note The threshold sets a symmetric range of clock offset
 */
bool clkmgr_setClockOffsetThreshold(Clkmgr_Subscription *sub_c,
    enum Clkmgr_ClockType clock_type, uint32_t threshold);

/**
 * Get the threshold of specify clock offset
 * @param[in] sub_c Pointer of the Clkmgr_Subscription
 * @param[in] clock_type The type of clock to get
 * @return Threshold of clock offset
 * @note The clock type is a bit that represents a type of clock,
 * as defined by enum Clkmgr_ClockType
 */
uint32_t clkmgr_getClockOffsetThreshold(const Clkmgr_Subscription *sub_c,
    enum Clkmgr_ClockType clock_type);

/**
 * Enable the subscription of the specify clock.
 * @param[in] sub_c Pointer of the Clkmgr_Subscription
 * @param[in] clock_type Type of clock to enable
 * @return True if the subscription of clock is enable successfully, false
 * otherwise.
 * @note The clock type is a bit that represents a type of clock,
 * as defined by enum Clkmgr_ClockType
 */
bool clkmgr_enableSubscription(Clkmgr_Subscription *sub_c,
    enum Clkmgr_ClockType clock_type);

/**
 * Disable the subscription of the specify clock.
 * @param[in] sub_c Pointer of the Clkmgr_Subscription
 * @param[in] clock_type Type of clock to disable
 * @return True if the subscription of clock is disable successfully, false
 * otherwise.
 * @note The clock type is a bit that represents a type of clock,
 * as defined by enum Clkmgr_ClockType
 */
bool clkmgr_disableSubscription(Clkmgr_Subscription *sub_c,
    enum Clkmgr_ClockType clock_type);

/**
 * Check if the specify clock subscription is enabled
 * @param[in] sub_c Pointer of the Clkmgr_Subscription
 * @param[in] clock_type Type of clock to check
 * @return True if the specify clock subscription is enabled, false otherwise
 * @note The clock type is a bit that represents a type of clock,
 * as defined by enum Clkmgr_ClockType
 */
bool clkmgr_isSubscriptionEnabled(const Clkmgr_Subscription *sub_c,
    enum Clkmgr_ClockType clock_type);

#ifdef __cplusplus
}
#endif

#endif /* CLKMGR_SUBSCRIPTION_C_H */
