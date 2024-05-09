/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file jclklib_import.hpp
 * @brief C API import.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#include <cstdint>

#ifndef JCLKLIB_IMPORT_HPP
#define JCLKLIB_IMPORT_HPP

#include <common/util.hpp>

namespace JClkLibCommon
{
	typedef enum : std::uint8_t
		{ gmOffsetValue,
		  valueLast } valueType;
#define MAX_VALUE_COUNT 12
	
	/** Servo State */
enum servoState_e  {
    /**
     * The servo is not yet ready to track the master clock.
     */
    SERVO_UNLOCKED = 0,
    /**
     * The servo is ready to track and requests a clock jump to
     * immediately correct the estimated offset.
     */
    SERVO_JUMP     = 1,
    /**
     * The servo is tracking the master clock.
     */
    SERVO_LOCKED   = 2,
    /**
     * The Servo has stabilized. The last 'servo_num_offset_values' values
     * of the estimated threshold are less than servo_offset_threshold.
     */
    SERVO_LOCKED_STABLE  = 3,
};

	class jcl_value
	{
	private:
		struct value_t {
			std::int32_t upper;
			std::int32_t lower;
			value_t();
			value_t(uint32_t limit);
			bool equal(const value_t &v);
			bool operator== (const value_t &value) { return this->equal(value); }
			bool operator!= (const value_t &value) { return !this->equal(value); }
			void zero() { upper = 0; lower = 0; }
		};
		value_t value[valueLast];
		value_t reserved[MAX_VALUE_COUNT-sizeof(value)/sizeof(value[0])];
	public:
		std::uint8_t *parse(std::uint8_t *buf, std::size_t &length);
		std::uint8_t *write(std::uint8_t *buf, std::size_t &length);
		bool equal( const jcl_value &c);
		bool operator== (const jcl_value &value) { return this->equal(value); }
		bool operator!= (const jcl_value &value) { return !this->equal(value); }
		// Add a method to set the upper and lower values of a specific value_t in the value array
		void setValue(int index, std::int32_t upper, std::int32_t lower) {
			if (index >= 0 && index < valueLast) {
 				value[index].upper = upper;
				value[index].lower = lower;
			}
		}
		std::int32_t getUpper(int index) {
			if (index >= 0 && index < valueLast) {
				return value[index].upper;
			}
			return 0;
		}
 
		std::int32_t getLower(int index) {
			if (index >= 0 && index < valueLast) {
				return value[index].lower;
			}
			return 0;
		}
		std::string toString();
	};

	/* Events clients can subscribe to */
	typedef enum : std::uint8_t
		{ gmOffsetEvent, servoLockedEvent, asCapableEvent, gmPresentEvent, gmChangedEvent,
		  eventLast } eventType;

#define BITS_PER_BYTE (8)
#define MAX_EVENT_COUNT (128)

	class jcl_event
	{
	private:
		std::uint32_t event_mask[eventLast/(sizeof(std::uint32_t)*BITS_PER_BYTE)+1];
		std::uint32_t reserved[MAX_EVENT_COUNT/
				       sizeof(event_mask[0]*BITS_PER_BYTE)
				       -sizeof(event_mask)/sizeof(std::uint32_t)];
	public:
		std::uint8_t *parse(std::uint8_t *buf, std::size_t &length);
		std::uint8_t *write(std::uint8_t *buf, std::size_t &length);
		void zero();
		bool equal( const jcl_event &c);
		bool operator== (const jcl_event &event) { return this->equal(event); }
		bool operator!= (const jcl_event &event) { return !this->equal(event); }
		std::string toString();
		int8_t writeEvent(uint32_t *newEvent, std::size_t length);
		int8_t readEvent(uint32_t *readEvnt, std::size_t length);
		int8_t copyEventMask(jcl_event &newEvent);
		std::uint32_t *getEventMask() { return event_mask; }
	};


	class jcl_eventcount
	{
		std::uint32_t count[eventLast];
		std::uint32_t reserved[MAX_EVENT_COUNT-eventLast];
	public:
		std::uint8_t *parse(std::uint8_t *buf, std::size_t &length);
		std::uint8_t *write(std::uint8_t *buf, std::size_t &length);
		void zero();
		bool equal(const jcl_eventcount &ec);
		bool operator== (const jcl_eventcount &ec) { return this->equal(ec); }
		bool operator!= (const jcl_eventcount &ec) { return !this->equal(ec); }
	};

	class jcl_subscription
	{
	private:
		jcl_event        event;
		jcl_value        value;
		jcl_event        composite_event;
	public:
		std::uint8_t *parse(std::uint8_t *buf, std::size_t &length);
		std::uint8_t *write(std::uint8_t *buf, std::size_t &length);

		DECLARE_ACCESSOR(event);
		DECLARE_ACCESSOR(value);
		DECLARE_ACCESSOR(composite_event);
	};

	struct ptp_event {
		int64_t master_offset;
		uint8_t gmIdentity[8]; /* Grandmaster clock ID */
		int32_t asCapable; /* 802@.1AS Capable */
		uint8_t servo_state;
		int32_t gmPresent;
		uint8_t ptp4l_id;
	};

	struct client_ptp_event {
		int64_t master_offset;
		int64_t master_offset_low;
		int64_t master_offset_high;
		bool master_offset_within_boundary;
		uint8_t gmIdentity[8]; /* Grandmaster clock ID */
		int32_t asCapable; /* 802@.1AS Capable */
		int32_t gmPresent;
		uint8_t servo_state;
		uint8_t ptp4l_id;
		bool composite_event;
		std::atomic<int> offset_event_count{};
		std::atomic<int> asCapable_event_count{};
		std::atomic<int> servo_state_event_count{};
		std::atomic<int> gmPresent_event_count{};
		std::atomic<int> gmChanged_event_count{};
		std::atomic<int> composite_event_count{};
	};

	struct jcl_state
	{
		bool     as_Capable;
		bool     gm_present;
		bool     offset_in_range;
		bool     servo_locked;
		bool     gm_changed;
		bool     composite_event;
		uint8_t  gmIdentity[8];
	};

	struct jcl_state_event_count
	{
		uint64_t gmPresent_event_count;
		uint64_t offset_in_range_event_count;
		uint64_t gm_changed_event_count;
		uint64_t asCapable_event_count;
		uint64_t servo_locked_event_count;
		uint64_t composite_event_count;
	};



}

#endif/*JCLKLIB_IMPORT_HPP*/
