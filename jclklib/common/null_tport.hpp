/*! \file null_tport.hpp
    \brief Null transport class. Non-functional for testing only.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#ifndef NULL_TPORT_HPP
#define NULL_TPORT_HPP

#include <common/transport.hpp>

#include <cstdint>

namespace JClkLibCommon
{
	class NullTransport : public Transport
	{
	public:
		static bool initTransport() { return true; }
		static bool stopTransport()	{ return true; }
		static bool finalizeTransport()	{ return true; }
	};
}

#endif/*NULL_TPORT_HPP*/
