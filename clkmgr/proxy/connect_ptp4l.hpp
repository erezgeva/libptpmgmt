/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy connect ptp4l message class.
 *
 * @author Lai Peter Jun Ann <peter.jun.ann.lai@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "common/connect_msg.hpp"
#include "proxy/message.hpp"

__CLKMGR_NAMESPACE_BEGIN

class ConnectPtp4l
{
  private:
  public:
    static int connect_ptp4l(std::string ptp4lUdsAddress, uint8_t domain);
    static void disconnect_ptp4l();
};

__CLKMGR_NAMESPACE_END
