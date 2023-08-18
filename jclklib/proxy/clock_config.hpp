/*! \file clock_config.hpp
    \brief Proxy configuration class. One configuration object per session is instatiated

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <cstdint>
#include <mutex>

#ifndef CLOCK_CONFIG_HPP
#define CLOCK_CONFIG_HPP

#include <common/jclklib_import.hpp>
#include <proxy/message.hpp>

namespace JClkLibProxy
{

	class ClockConfiguration
	{
	private:
		enum { event = 0, value };
		bool update, writeUpdate;
		std::mutex update_lock;
		struct Config {
			JClkLibCommon::jcl_event event;
			JClkLibCommon::jcl_value value;
		};
		Config writeShadow, readShadow;
		Config config;
	public:
		ClockConfiguration();
		void speculateWrite();
		void setEvent( const JClkLibCommon::jcl_event &event );
		void setValue( const JClkLibCommon::jcl_value &value );
		void commitWrite();

		bool readConsume();
		const JClkLibCommon::jcl_event &getEvent() { return readShadow.event; }
		const JClkLibCommon::jcl_value &getValue() { return readShadow.value; }

		static JClkLibCommon::sessionId_t getSessionId();
	};
}

#endif/*CLOCK_CONFIG_HPP*/
