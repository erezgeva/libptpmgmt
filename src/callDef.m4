dnl SPDX-License-Identifier: LGPL-3.0-or-later
dnl SPDX-FileCopyrightText: Copyright 2022 Erez Geva */
dnl
dnl @file
dnl @brief List of management IDs
dnl
dnl @author Erez Geva <ErezGeva2@@gmail.com>
dnl @copyright 2022 Erez Geva
dnl
dnl Create callDef.h used by msgCall.h
dnl Contain virtual function per TLV
dnl
/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2022 Erez Geva */

/** @file
 * @brief Base classes with callbacks for message dispacher classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2022 Erez Geva
 *
 * This header is generated automatically.
 *
 */

#ifndef __PTPMGMT_CALL_DEFS_H
#define __PTPMGMT_CALL_DEFS_H

#include "msg.h"

__PTPMGMT_NAMESPACE_BEGIN

/**
 * @brief Dispacher for management TLV
 * @details
 *  Call-backs used by the MessageDispatcher and the MessageBulder classes.
 * @note these classes are defined in @"msgCall.h@" header.
 *  Use it in your application
 *  @code{.cpp} #include "msgCall.h" @endcode
 */
class BaseMngDispatchCallback
{
  protected:
    virtual ~BaseMngDispatchCallback() = default;
define(D,`    /**
     * Handle $1 management TLV
     * @param[in] msg referance to the Message object
     * @param[in] tlv referance to the management tlv
     * @param[in] idStr ID string of the management tlv
     */
    virtual void $1_h(const Message &msg,
        const $1_t &tlv, const char *idStr)
    {}')dnl
define(A, `ifelse(regexp($6, `^UF', `0'),`0',`D($1)',`dnl')')dnl
include(ids_base.m4)dnl
undefine(`A')dnl
};

/**
 * @brief Builder for a management TLV
 * @details
 *  Call-backs used by the MessageBulder class.
 * @note this class is defined in @"msgCall.h@" header.
 *  Use this include in your application
 *  @code{.cpp} #include "msgCall.h" @endcode
 * @note User need to inherie MessageBulder and implement the call-backs
 * @attention Do @b NOT @p use this header in your application.
 */
class BaseMngBuildCallback
{
  protected:
    virtual ~BaseMngBuildCallback() = default;
define(B,`    /**
     * Handle $1 management TLV
     * Set values in the new TLV
     * @param[in] msg referance to the Message object
     * @param[in, out] tlv referance to the new management tlv
     * @return true if set success
     * @note MessageBulder::buildTlv call setAction with new TLV
     *  if this handler return true!
     */
    virtual bool $1_b(const Message &msg,
        $1_t &tlv)
    {return false;}')dnl
define(A, `ifelse(regexp($6, `^UFB', `0'),`0',`B($1)',`dnl')')dnl
include(ids_base.m4)dnl
};

__PTPMGMT_NAMESPACE_END

#endif /* __PTPMGMT_CALL_DEFS_H */
