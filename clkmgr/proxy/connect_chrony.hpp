/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy connect chrony message class.
 *
 * @author Lai Peter Jun Ann <peter.jun.ann.lai@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "common/connect_msg.hpp"
#include "proxy/message.hpp"

__CLKMGR_NAMESPACE_BEGIN

class ConnectChrony
{
  public:
    static void connect_chrony();
    static int subscribe_chrony(int timeBaseIndex, sessionId_t sessionId);
    static int remove_chrony_subscriber(sessionId_t sessionId);
};

__CLKMGR_NAMESPACE_END
