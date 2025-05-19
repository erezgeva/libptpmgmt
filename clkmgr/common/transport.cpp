/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Common transport base class implementation for Clkmgr.
 * It provides common functionality for derived transport classes.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "common/transport.hpp"
#include "common/msgq_tport.hpp"
#include "common/null_tport.hpp"

using namespace std;

__CLKMGR_NAMESPACE_USE;

bool Transport::init()
{
    return _initTransport<NullTransport, MessageQueue>();
}

bool Transport::stop()
{
    /* Do any transport specific stop */
    return _stopTransport<NullTransport, MessageQueue>();
}

bool Transport::finalize()
{
    return _finalizeTransport<NullTransport, MessageQueue>();
}
