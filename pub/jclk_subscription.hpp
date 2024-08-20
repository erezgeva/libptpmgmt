/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file jclk_subscription.hpp
 * @brief structure needed for PTP event subsciption
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef JCLK_SUBSCRIPTION_HPP
#define JCLK_SUBSCRIPTION_HPP

#include <cstdint>
#include <string>
#include <common/util.hpp>

namespace JClkLibCommon
{
/**
 * Types for class jcl_value
 */
typedef enum : std::uint8_t {
    gmOffsetValue, /**< GM offset value */
    valueLast /**< Last value type */
} valueType;

#define MAX_VALUE_COUNT 12

/**
 * Class to hold upper and lower limits
 */
class jcl_value
{
  private:
    /**
     * Structure to hold upper and lower limits
     */
    struct value_t {
        std::int32_t upper; /**< Upper limit */
        std::int32_t lower; /**< Lower limit */
        value_t();
        value_t(uint32_t limit);
        bool equal(const value_t &v);
        bool operator== (const value_t &value) { return this->equal(value); }
        bool operator!= (const value_t &value) { return !this->equal(value); }
        void zero() { upper = 0; lower = 0; }
    };
    /**
     * Array of values
     */
    value_t value[valueLast];
    /**
     * Reserved values
     */
    value_t reserved[MAX_VALUE_COUNT - sizeof(value) / sizeof(value[0])];
  public:
    std::uint8_t *parse(std::uint8_t *buf, std::size_t &length);
    std::uint8_t *write(std::uint8_t *buf, std::size_t &length);
    bool equal(const jcl_value &c);
    bool operator== (const jcl_value &value) { return this->equal(value); }
    bool operator!= (const jcl_value &value) { return !this->equal(value); }
    /**
     * @brief Set the upper and lower limits of a specific index in value array
     * @param index Index of the value_t
     * @param upper Upper limit
     * @param lower Lower limit
     */
    void setValue(int index, std::int32_t upper, std::int32_t lower) {
        if(index >= 0 && index < valueLast) {
            value[index].upper = upper;
            value[index].lower = lower;
        }
    }
    /**
     * @brief Get the upper limit of a specific index in value array
     * @param index Index of the value_t
     * @return Upper limit
     */
    std::int32_t getUpper(int index) {
        if(index >= 0 && index < valueLast)
            return value[index].upper;
        return 0;
    }
    /**
     * @brief Get the lower limit of a specific index in value array
     * @param index Index of the value_t
     * @return Lower limit
     */
    std::int32_t getLower(int index) {
        if(index >= 0 && index < valueLast)
            return value[index].lower;
        return 0;
    }
    std::string toString();
};

#define BITS_PER_BYTE (8)
#define MAX_EVENT_COUNT (128)

/**
 * Types of PTP events subscription
 */
typedef enum : std::uint8_t {
    gmOffsetEvent, /**< Primary-secondary clock offset event */
    syncedToPrimaryClockEvent, /**< Synced to primary clock event */
    asCapableEvent, /**< IEEE 802.1AS capable event */
    gmChangedEvent, /**< Primary clock UUID changed event */
    eventLast /**< Last event type */
} eventType;

/**
 * Class to hold event mask
 */
class jcl_event
{
  private:
    /**
     * Event mask
     */
    std::uint32_t event_mask[eventLast / (sizeof(std::uint32_t)*BITS_PER_BYTE) + 1];
    /**
     * Reserved events
     */
    std::uint32_t reserved[MAX_EVENT_COUNT /
                        sizeof(event_mask[0]*BITS_PER_BYTE)
                        - sizeof(event_mask) / sizeof(std::uint32_t)];
  public:
    std::uint8_t *parse(std::uint8_t *buf, std::size_t &length);
    std::uint8_t *write(std::uint8_t *buf, std::size_t &length);
    void zero();
    bool equal(const jcl_event &c);
    bool operator== (const jcl_event &event) { return this->equal(event); }
    bool operator!= (const jcl_event &event) { return !this->equal(event); }
    std::string toString();
    int8_t writeEvent(uint32_t *newEvent, std::size_t length);
    int8_t readEvent(uint32_t *readEvnt, std::size_t length);
    int8_t copyEventMask(jcl_event &newEvent);
    /**
     * Get the event mask
     * @return Pointer to the event mask
     */
    std::uint32_t *getEventMask() { return event_mask; }
};

/**
 * Class to hold event counts
 */
class jcl_eventcount
{
    std::uint32_t count[eventLast]; /**< Event counts */
    /**
     * Reserved event counts
     */
    std::uint32_t reserved[MAX_EVENT_COUNT - eventLast];
  public:
    std::uint8_t *parse(std::uint8_t *buf, std::size_t &length);
    std::uint8_t *write(std::uint8_t *buf, std::size_t &length);
    void zero();
    bool equal(const jcl_eventcount &ec);
    bool operator== (const jcl_eventcount &ec) { return this->equal(ec); }
    bool operator!= (const jcl_eventcount &ec) { return !this->equal(ec); }
};

/**
 * Class to hold event subscriptions
 */
class jcl_subscription
{
  private:
    jcl_event        event; /**< Event subscription */
    jcl_value        value; /**< value of upper and lower limits */
    jcl_event        composite_event; /**< Composite event subscription */
  public:
    DECLARE_ACCESSOR(event); /**< Declare accessor for event */
    DECLARE_ACCESSOR(value); /**< Declare accessor for value */
    DECLARE_ACCESSOR(composite_event); /**< Declare accessor for composite event */
};
}

#endif /* JCLK_SUBSCRIPTION_HPP */
