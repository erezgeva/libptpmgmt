/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Allocate buffer
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#include "comp.h"
#include <cstdarg>

__PTPMGMT_NAMESPACE_BEGIN

void Error::doError(bool use_errno, const char *file, int line,
    const char *func, const string &msg)
{
    m_file = file;
    m_line = line;
    m_func = func;
    m_msg = msg;
    m_errno = use_errno ? errno : 0;
}
const string &Error::fetch()
{
    if(m_line == 0)
        m_fmsg.clear();
    m_fmsg = "[" + m_file +
        ":" + to_string(m_line) +
        ":" + m_func + "] " + m_msg;
    if(m_errno != 0) {
        m_fmsg += ": ";
        m_fmsg += strerror(m_errno);
    }
    return m_fmsg;
}
Error &Error::getCur()
{
    static thread_local Error obj;
    return obj;
}
string Error::doFormat(const char *format, ...)
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
const string &Error::getErrnoMsg()
{
    getCur().m_emsg = getCur().m_errno == 0 ? "" : strerror(getCur().m_errno);
    return getCur().m_emsg;
}

__PTPMGMT_NAMESPACE_END

__PTPMGMT_NAMESPACE_USE;
extern "C" {
    int ptpmgmt_err_isError() { return Error::isError(); }
    const char *ptpmgmt_err_getError() { return Error::getError().c_str(); }
    const char *ptpmgmt_err_getFile() { return Error::getFile().c_str(); }
    int ptpmgmt_err_getFileLine() { return Error::getFileLine(); }
    const char *ptpmgmt_err_getFunc() { return Error::getFunc().c_str(); }
    int ptpmgmt_err_getErrno() { return Error::getErrno(); }
    const char *ptpmgmt_err_getMsg() { return Error::getMsg().c_str(); }
    const char *ptpmgmt_err_getErrnoMsg() { return Error::getErrnoMsg().c_str(); }
}
