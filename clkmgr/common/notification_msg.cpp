/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Common notification message implementation.
 * Implements common functions and (de-)serialization
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "common/notification_msg.hpp"
#include "common/serialize.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

bool NotificationMessage::parseBufferComm()
{
    return PARSE_RX(FIELD, timeBaseIndex, rxContext);
}

bool NotificationMessage::makeBufferComm(Buffer &buff) const
{
    return WRITE_TX(FIELD, timeBaseIndex, buff);
}
