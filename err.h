/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief error log
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 * @copyright GNU Lesser General Public License 3.0 or later
 *
 */

#ifndef __PMC_ERR_H
#define __PMC_ERR_H

#ifndef PMC_ERROR
/**
 * Print error without parameters
 * @param[in] format message to print
 */
#define PMC_ERROR(format) do\
    { fprintf(stderr, "[%s:%d] " format "\n", __FILE__, __LINE__); }\
    while(0)
#endif /*PMC_ERROR*/
#ifndef PMC_ERRORA
/**
 * Print error with parameters
 * @param[in] format message to print
 * @param[in] ... parameters match for format
 */
#define PMC_ERRORA(format, ...) do\
    { fprintf(stderr, "[%s:%d] " format "\n", __FILE__, __LINE__, __VA_ARGS__); }\
    while(0)
#endif /*PMC_ERRORA*/
#ifndef PMC_PERROR
/**
 * Print system error
 * @param[in] msg
 */
#define PMC_PERROR(msg) do\
    { fprintf(stderr, "[%s:%d] " msg ": %s\n", __FILE__, __LINE__, strerror(errno)); }\
    while(0)
#endif /*PMC_PERROR*/

#endif /*__PMC_ERR_H*/
