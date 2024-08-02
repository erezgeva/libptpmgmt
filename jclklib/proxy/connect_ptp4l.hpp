/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file connect.hpp
 * @brief Proxy connect ptp4l message class.
 *
 * @author Lai Peter Jun Ann <peter.jun.ann.lai@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#include <common/connect_msg.hpp>
#include <proxy/message.hpp>

namespace JClkLibProxy
{
    class Connect
    {
    private:
    public:
        static int connect();
        static void disconnect();
    };
}