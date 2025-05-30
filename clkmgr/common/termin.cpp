/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief A class to call stop and finalize
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include "common/termin.hpp"

#include <thread>

__CLKMGR_NAMESPACE_USE;

DECLARE_STATIC(End::all);

End::End()
{
    all.push_back(this);
}
bool End::stopAll(uint32_t wait)
{
    bool ret = true;
    for(End *e : all) {
        e->stopPass = e->stop();
        ret = ret && e->stopPass;
    }
    // So we give time to other thread to ends
    std::this_thread::sleep_for(std::chrono::milliseconds(wait));
    for(End *e : all) {
        if(e->stopPass)
            ret = ret && e->finalize();
    }
    return ret;
}
