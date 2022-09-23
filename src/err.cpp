/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief Allocate buffer
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#include "err.h"
#include <stdarg.h>

__PTPMGMT_NAMESPACE_BEGIN

void Error::doError(bool use_errno, const char *file, int line,
    const char *func, const std::string &msg)
{
    m_file = file;
    m_line = line;
    m_func = func;
    m_msg = msg;
    m_errno = use_errno ? errno : 0;
}
void Error::fetch(std::string &ret)
{
    if(m_line == 0)
        ret.clear();
    ret = "[" + m_file +
        ":" + std::to_string(m_line) +
        ":" + m_func + "] " + m_msg;
    if(m_errno != 0) {
        ret += ": ";
        ret += strerror(m_errno);
    }
}
Error &Error::getCur()
{
    static thread_local Error obj;
    return obj;
}
std::string Error::doFormat(const char *format, ...)
{
    va_list va;
    va_start(va, format);
    va_list va2;
    va_copy(va2, va);
    int len = vsnprintf(nullptr, 0, format, va2) + 1;
    va_end(va2);
    char buf[len];
    vsnprintf(buf, len, format, va);
    va_end(va);
    return buf;
}
std::string Error::getErrnoMsg()
{
    return getCur().m_errno == 0 ? "" : strerror(getCur().m_errno);
}

__PTPMGMT_NAMESPACE_END
