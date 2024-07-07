/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Get Last Error happend in library, C API
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 * @note library do not use C++ exceptions,
 *       users can use this class to fetch error.
 */

#ifndef __PTPMGMT_C_ERR_H
#define __PTPMGMT_C_ERR_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Query if we have error
 * @return last error string
 */
int ptpmgmt_err_isError();
/**
 * Get last error full string
 * @return last error string
 */
const char *ptpmgmt_err_getError();
/**
 * Get last error source file name
 * @return last error source file name
 */
const char *ptpmgmt_err_getFile();
/**
 * Get last error source file line number
 * @return last error source file line number
 */
int ptpmgmt_err_getFileLine();
/**
 * Get last error function name
 * @return last error function name
 */
const char *ptpmgmt_err_getFunc();
/**
 * Get last error errno value
 * @return last errno value or zero if last error was not using errno
 */
int ptpmgmt_err_getErrno();
/**
 * Get last error message
 * @return last error message
 * @note The error do not include the errno message!
 */
const char *ptpmgmt_err_getMsg();
/**
 * Get last errno message
 * @return last error message
 * @note The error do not include the errno message!
 */
const char *ptpmgmt_err_getErrnoMsg();

#ifdef __cplusplus
}
#endif

#endif /* __PTPMGMT_C_ERR_H */
