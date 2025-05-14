/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Non-functional message implementation for debug
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "common/null_msg.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

bool CommonNullMessage::buildMessage(Message *&msg,
    TransportListenerContext &LxContext)
{
    return true;
}

bool CommonNullMessage::initMessage()
{
    return true;
}
