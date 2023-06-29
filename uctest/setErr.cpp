/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Set Error in Error class using internal methods
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2023 Erez Geva
 *
 */

#include "comp.h"

__PTPMGMT_NAMESPACE_USE;

extern "C" {
    void uctest_error(const char *file, int line, const char *func)
    {
        std::string m = Error::doFormat("test %s", "macro");
        Error::error(file, line, func, m);
    }
    void uctest_perror(const char *file, int line, const char *func)
    {
        std::string m = Error::doFormat("test %s", "macro");
        Error::perror(file, line, func, m);
    }
}
