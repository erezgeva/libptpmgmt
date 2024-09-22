/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Get Last Error happend in library
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 * @note library do not use C++ exceptions,
 *       users can use this class to fetch error.
 */

#ifndef __PTPMGMT_ERR_H
#define __PTPMGMT_ERR_H

#include "name.h"

#ifdef __cplusplus
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
    int m_errno = 0;
    int m_line = 0;
    std::string m_file;
    std::string m_msg;
    std::string m_func;
    std::string m_fmsg;
    std::string m_emsg;
    void doClear();
    void doError(bool use_errno, const char *file, int line, const char *func,
        const std::string &msg);
    const std::string &fetch();
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
    static bool isError();
    /**
     * Get last error full string
     * @return last error string
     */
    static const std::string &getError();
    /**
     * Get last error source file name
     * @return last error source file name
     */
    static const std::string &getFile();
    /**
     * Get last error source file line number
     * @return last error source file line number
     */
    static int getFileLine();
    /**
     * Get last error function name
     * @return last error function name
     */
    static const std::string &getFunc();
    /**
     * Get last error errno value
     * @return last errno value or zero if last error was not using errno
     */
    static int getErrno();
    /**
     * Get last error message
     * @return last error message
     * @note The error do not include the errno message!
     */
    static const std::string &getMsg();
    /**
     * Get last errno message
     * @return last error message
     * @note The error do not include the errno message!
     */
    static const std::string &getErrnoMsg();
};

__PTPMGMT_NAMESPACE_END
#else /* __cplusplus */
#include "c/err.h"
#endif /* __cplusplus */

#endif /* __PTPMGMT_ERR_H */
