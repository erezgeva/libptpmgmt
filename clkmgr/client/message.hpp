/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client message base class.
 * Extended for specific messages such as connect and subscribe.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef CLIENT_MESSAGE_HPP
#define CLIENT_MESSAGE_HPP

#include "client/client_state.hpp"
#include "common/message.hpp"

#include <memory>

__CLKMGR_NAMESPACE_BEGIN

bool clientMessageRegister();

__CLKMGR_NAMESPACE_END

#endif /* CLIENT_MESSAGE_HPP */
