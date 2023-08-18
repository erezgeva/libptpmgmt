/*! \file null_tport.hpp
    \brief Null transport class. Non-functional for testing only.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#ifndef CLIENT_NULL_TPORT_HPP
#define CLIENT_NULL_TPORT_HPP

#include <client/transport.hpp>
#include <common/null_tport.hpp>

#include <cstdint>

namespace JClkLibClient
{
	class NullClientTransport : public JClkLibCommon::NullTransport
	{
	public:
		static bool sendMessage(std::uint8_t *buffer, std::size_t length);
	};
}

#endif/*CLIENT_NULL_TPORT_HPP*/
