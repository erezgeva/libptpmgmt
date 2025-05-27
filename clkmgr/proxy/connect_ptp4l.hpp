/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy connect ptp4l message class.
 *
 * @author Lai Peter Jun Ann <peter.jun.ann.lai@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */
#ifndef PROXY_CONNECT_PTP4L_HPP
#define PROXY_CONNECT_PTP4L_HPP

#include "common/util.hpp"

#include <cstddef>

__CLKMGR_NAMESPACE_BEGIN

class ConnectPtp4l
{
  public:
    static int connect_ptp4l();
    static int subscribe_ptp4l(size_t timeBaseIndex, sessionId_t sessionId);
    static int remove_ptp4l_subscriber(sessionId_t sessionId);
};

__CLKMGR_NAMESPACE_END

#endif /* PROXY_CONNECT_PTP4L_HPP */
