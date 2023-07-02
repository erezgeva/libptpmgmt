/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Get Last Error happend in library
 * @note library do not use C++ exceptions,
 *       users can use this class to fetch error.
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#ifndef __PTPMGMT_ERR_H
#define __PTPMGMT_ERR_H

#include "name.h"

__PTPMGMT_NAMESPACE_BEGIN

/**
 * @brief Keep last error
 * @details
 *  The class keep the last error created in the libptpmgmt library
 *  The class is thread-safe, the error is kept per thread seperatly!
 */
class Error
{
  private:
    int m_errno;
    int m_line;
    std::string m_file;
    std::string m_msg;
    std::string m_func;
    void doClear();
    Error() : m_line(0) {}
    void doError(bool use_errno, const char *file, int line, const char *func,
        const std::string &msg);
    void fetch(std::string &ret);
    static Error &getCur();
  public:

    #ifndef SWIG
    /** @cond internal used function only ! */
    static void perror(const char *file, int line, const char *func,
        const std::string &msg)
    { getCur().doError(true, file, line, func, msg); }
    static void error(const char *file, int line, const char *func,
        const std::string &msg)
    { getCur().doError(false, file, line, func, msg); }
    static std::string doFormat(const char *format, ...);
    static void clear() { getCur().m_line = 0; }
    /** @endcond */
    #endif /* SWIG */

    /**
     * Query if we have error
     * @return last error string
     */
    static bool isError() { return getCur().m_line > 0; }
    /**
     * Get last error full string
     * @return last error string
     */
    static std::string getError()
    { std::string ret; getCur().fetch(ret); return ret; }
    /**
     * Get last error source file name
     * @return last error source file name
     */
    static const std::string &getFile() { return getCur().m_file; }
    /**
     * Get last error source file line number
     * @return last error source file line number
     */
    static int getFileLine() { return getCur().m_line; }
    /**
     * Get last error function name
     * @return last error function name
     */
    static const std::string &getFunc() { return getCur().m_func; }
    /**
     * Get last error errno value
     * @return last errno value or zero if last error was not using errno
     */
    static int getErrno() { return getCur().m_errno; }
    /**
     * Get last error message
     * @return last error message
     * @note The error do not include the errno message!
     */
    static const std::string &getMsg() { return getCur().m_msg; }
    /**
     * Get last errno message
     * @return last error message
     * @note The error do not include the errno message!
     */
    static std::string getErrnoMsg();
};

__PTPMGMT_NAMESPACE_END

#endif /* __PTPMGMT_ERR_H */
