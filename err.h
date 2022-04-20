/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief error log
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#ifndef __PTPMGMT_ERR_H
#define __PTPMGMT_ERR_H

#include <string>
#include <cstdint>
#include <cstring>
#include <cstdio>

#ifndef PTPMGMT_ERROR
/**
 * Print error without parameters
 * @param[in] format message to print
 */
#define PTPMGMT_ERROR(format) do\
    { fprintf(stderr, "[%s:%d] " format "\n", __FILE__, __LINE__); }\
    while(0)
#endif /* PTPMGMT_ERROR */
#ifndef PTPMGMT_ERRORA
/**
 * Print error with parameters
 * @param[in] format message to print
 * @param[in] ... parameters match for format
 */
#define PTPMGMT_ERRORA(format, ...) do\
    { fprintf(stderr, "[%s:%d] " format "\n", __FILE__, __LINE__, __VA_ARGS__); }\
    while(0)
#endif /* PTPMGMT_ERRORA */
#ifndef PTPMGMT_PERROR
/**
 * Print system error
 * @param[in] msg
 */
#define PTPMGMT_PERROR(msg) do\
    { fprintf(stderr, "[%s:%d] " msg ": %s\n", __FILE__, __LINE__,\
            strerror(errno)); }\
    while(0)
#endif /* PTPMGMT_PERROR */

#endif /* __PTPMGMT_ERR_H */
