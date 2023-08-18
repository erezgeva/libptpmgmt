/*! \file clock_status.hpp
    \brief Proxy status class. One status object per session.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <mutex>

#ifndef CLOCK_STATUS_HPP
#define CLOCK_STATUS_HPP

#include <common/jclklib_import.hpp>

namespace JClkLibProxy
{
	class ClockStatus
	{
	private:
		class Status {
		public:
			JClkLibCommon::jcl_event	event;
			JClkLibCommon::jcl_eventcount	count;
		};
		bool update, writeUpdate;
		std::mutex update_lock;
		Status status;
		Status writeShadow, readShadow;
	public:
		ClockStatus();
		void speculateWrite();
		void setEvent( const JClkLibCommon::jcl_event &event );
		void setCount( const JClkLibCommon::jcl_eventcount &count );
		void commitWrite();

		bool readConsume();
		const JClkLibCommon::jcl_event &getEvent() { return readShadow.event; }
		const JClkLibCommon::jcl_eventcount &getCount() { return readShadow.count; }
	};
}

#endif/*CLOCK_STATUS_HPP*/
