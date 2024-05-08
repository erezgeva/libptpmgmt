/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file null_tport.hpp
 * @brief Null transport class. Non-functional for testing only.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
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
