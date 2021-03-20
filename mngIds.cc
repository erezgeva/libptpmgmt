%@* SPDX-License-Identifier: LGPL-3.0-or-later *%@
%_
%@** %!file
 * %!brief List of managment IDs and conversions functions
 *
 * %!author Erez Geva <ErezGeva2%!gmail.com>
 * %!copyright 2021 Erez Geva
 *
 * This header is generated automaticlly, do %!b NOT chagne
 *  or use it in your application!
 *%@
%_
?@ifndef __MNG_IDS_H
?@define __MNG_IDS_H
%_
?@include "msg.h"
%_
%@**
 * %!brief Management TLVs ID
 * %!details
 *%? The Managment TLVs the application may use on message
 * %!note this enumerator is defines in "msg.h" header.
 *%? use in your application %!code{.cpp} #include "msg.h" %!endcode
 * %!attention Do %!b NOT %!p use this header in your application
 *%@
enum mng_vals_e {
#define A(n, v, sc, a, sz, f) %?%?%?%?n,
#include "ids.h"
};
%_
?@ifndef SWIG
%@* For Doxygen only *%@
#define caseUF(n) \
%@**%^\
 * Convert data to n##_t structure%^\
 * %!param[in] data dataFiled from message%^\
 * %!return pointer to n##_t%^\
 * %!note These function are availible in scripts only!%^\
 *%? C++ code can simply use casting.%^\
 * %!note Use message.getData() to reteive the data from the message%^\
 *%@%^\
n##_t%^*conv_##n(const baseData *data);
#define A(n, v, sc, a, sz, f) case##f(n)
#include "ids.h"
?@endif %@*SWIG*%@
%_
?@endif %@*__MNG_IDS_H*%@
