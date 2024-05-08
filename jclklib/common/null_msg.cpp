/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file null_msg.cpp
 * @brief Non-functional message implementation for debug
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#include <common/null_msg.hpp>

using namespace JClkLibCommon;
using namespace std;

MAKE_RXBUFFER_TYPE(CommonNullMessage::buildMessage)
{
	return true;
}

bool CommonNullMessage::initMessage()
{
	return true;
}
