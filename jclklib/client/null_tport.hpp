/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file null_tport.cpp
 * @brief Null transport implementation. Non-functional for testing only.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
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
