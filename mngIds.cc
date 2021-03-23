%@* SPDX-License-Identifier: LGPL-3.0-or-later *%@
%_
%@** @file
 * @brief List of managment IDs and conversions functions
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 * This header is generated automaticlly, do @b NOT chagne
 *  or use it in your application!
 *%@
%_
%#ifndef __MNG_IDS_H
%#define __MNG_IDS_H
%_
%#include "msg.h"
%_
%@**
 * @brief Management TLVs ID
 * @details
 *%- The Managment TLVs the application may use on message.
 * @note this enumerator is defines in @"msg.h@" header.
 *%- Use this include in your application
 *%- @code{.cpp} #include "msg.h" @endcode
 * @attention Do @b NOT @p use this header in your application.
 *%@
enum mng_vals_e {
#define A(n, v, sc, a, sz, f) %- %-  n,
#include "ids.h"
};
%_
%#ifndef SWIG
%@* For Doxygen only *%@
%@**
 * Convert string to a buffer for build, parse, send and recieve
 * @param[in] string
 * @return pointer to buffer
 * @note This function is availible in scripts only!
 *%- C++ code can simply cast.%^\
 * @note The script need to ensure the string is in the proper length.
 *%@
void *conv_buf(char *string);
%_
#define caseUF(n) \
%@**%^\
 * Convert data to n##_t structure%^\
 * @param[in] data pointer to the message dataFiled%^\
 * @return pointer to n##_t%^\
 * @note This function is availible in scripts only!%^\
 *%- C++ code can simply cast.%^\
 * @note Use message.getData() to reteive the data from the message%^\
 *%@%^\
n##_t%^*conv_##n(baseData *data);
#define A(n, v, sc, a, sz, f) case##f(n)
#include "ids.h"
%#endif %@*SWIG*%@
%_
%#endif %@*__MNG_IDS_H*%@
