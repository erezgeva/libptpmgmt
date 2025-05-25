/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief A class to call stop and finalize
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#ifndef TERMIN_HPP
#define TERMIN_HPP

#include "common/util.hpp"

#include <vector>

__CLKMGR_NAMESPACE_BEGIN

/**
 * Registered classes will be called with stop and finalize
 * When calling the class stopAll
 *
 * Note: this class will NOT delete any registered object!
 */

class End
{
  private:
    static std::vector<End *> all;
    bool stopPass = true;

  protected:
    End();
    virtual bool stop() = 0;
    virtual bool finalize() = 0;

  public:
    /**
     * Call stop method than wait and then call finalize
     * @param[in] wait time in microsecond
     * @return true if @b all @b stop and finalize methods return true
     * @note we will call all stop and finalize regarding the result!
     */
    static bool stopAll(uint32_t wait = 100);
};

__CLKMGR_NAMESPACE_END

#endif /* TERMIN_HPP */
