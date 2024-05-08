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

#include <proxy/null_tport.hpp>
#include <common/print.hpp>

#include <cstdint>

using namespace JClkLibProxy;
using namespace std;

SEND_PROXY_MESSAGE(NullProxyTransport::sendMessage)
{
	PrintError("NullTransport should not be used");
	return false;
}

