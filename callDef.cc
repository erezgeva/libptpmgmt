%@* SPDX-License-Identifier: LGPL-3.0-or-later
 %- SPDX-FileCopyrightText: Copyright 2022 Erez Geva *%@
%_
%@** @file
 * @brief Base classes with callbacks for message dispacher classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2022 Erez Geva
 *
 * This header is generated automatically.
 *
 *%@
%_
%#ifndef __PTPMGMT_CALL_DEFS_H
%#define __PTPMGMT_CALL_DEFS_H
%_
%#include "msg.h"
%_
namespace ptpmgmt
{
%_
%@**
 * @brief dispacher for management TLV
 * @details
 *%- Call-backs used by the MessageDispatcher class.
 * @note this class is defined in @"msgCall.h@" header.
 *%- Use this include in your application
 *%- @code{.cpp} #include "msgCall.h" @endcode
 * @note User need to inherie MessageDispatcher and implement the call-backs
 * @attention Do @b NOT @p use this header in your application.
 *%@
class BaseMngDispatchCallback
{
  protected:
    virtual ~BaseMngDispatchCallback() = default;
#define _ptpmCaseUF(n)\
 %- %- %@**%^\
 %- %- * Handle n management TLV%^\
 %- %- * @param[in] msg referance to the Message object%^\
 %- %- * @param[in] tlv referance to the management tlv%^\
 %- %- * @param[in] idStr ID string of the management tlv%^\
 %- %- *%@%^\
 %-%- virtual void n##_h(const Message &msg,%^\
 %- %- %- %-const n##_t &tlv, const char *idStr) const;
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
#include "ids.h"
};
%_
%@**
 * @brief builder for a management TLV
 * @details
 *%- Call-backs used by the MessageBulder class.
 * @note this class is defined in @"msgCall.h@" header.
 *%- Use this include in your application
 *%- @code{.cpp} #include "msgCall.h" @endcode
 * @note User need to inherie MessageBulder and implement the call-backs
 * @attention Do @b NOT @p use this header in your application.
 *%@
class BaseMngBuildCallback
{
  protected:
    virtual ~BaseMngBuildCallback() = default;
#define _ptpmCaseUFB(n)\
 %- %- %@**%^\
 %- %- * Handle n management TLV%^\
 %- %- * Set values in the new TLV%^\
 %- %- * @param[in] msg referance to the Message object%^\
 %- %- * @param[in, out] tlv referance to the new management tlv%^\
 %- %- * @return true if set success%^\
 %- %- * @note MessageBulder::buildTlv call setAction with new TLV%^\
 %- %- *%- if this handler return true!%^\
 %- %- *%@%^\
 %-%- virtual bool n##_b(const Message &msg,%^\
 %- %- %- %-n##_t &tlv);
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
#include "ids.h"
};
%_
}; %@* namespace ptpmgmt *%@
%_
%#endif %@* __PTPMGMT_CALL_DEFS_H *%@
