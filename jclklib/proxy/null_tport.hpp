/*! \file null_tport.hpp
    \brief Null transport class. Non-functional for testing only.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#ifndef PROXY_NULL_TPORT_HPP
#define PROXY_NULL_TPORT_HPP

#include <proxy/transport.hpp>
#include <common/null_tport.hpp>

#include <cstdint>

namespace JClkLibProxy
{
	class NullProxyTransport : public JClkLibCommon::NullTransport
	{
	public:
		static SEND_PROXY_MESSAGE(sendMessage);
	};
}

#endif/*PROXY_NULL_TPORT_HPP*/
