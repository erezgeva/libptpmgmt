/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Common message type
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef COMMON_JCLTYPES_HPP
#define COMMON_JCLTYPES_HPP

#include "common/util.hpp"

__CLKMGR_NAMESPACE_BEGIN

typedef uint8_t msgAck_t;
enum  : msgAck_t {ACK_FAIL = (msgAck_t) -1, ACK_NONE = 0, ACK_SUCCESS = 1, };

typedef uint8_t msgId_t;
enum msgId : msgId_t {INVALID_MSG = (msgId_t) -1, NULL_MSG = 1, CONNECT_MSG,
    SUBSCRIBE_MSG, NOTIFY_MESSAGE, DISCONNECT_MSG
};

__CLKMGR_NAMESPACE_END

#endif /* COMMON_JCLTYPES_HPP */
