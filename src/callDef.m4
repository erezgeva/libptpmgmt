dnl SPDX-License-Identifier: LGPL-3.0-or-later
dnl SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */
dnl
dnl @file
dnl @brief List of management IDs
dnl
dnl @author Erez Geva <ErezGeva2@@gmail.com>
dnl @copyright © 2022 Erez Geva
dnl
dnl Create callDef.h used by msgCall.h
dnl Contain virtual function per TLV
dnl
include(lang().m4)dnl
/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Base classes with callbacks for message dispacher classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 * This header is generated automatically.
 *
 */

#ifndef __PTPMGMT_CALL_DEFS_H
#define __PTPMGMT_CALL_DEFS_H

#ifdef __cplusplus
#include "msg.h"

__PTPMGMT_NAMESPACE_BEGIN

/**
 * @brief Dispacher for management TLV
 * @details
 *  Call-backs used by the MessageDispatcher class.
 * @note this class is defined in @"msgCall.h@" header.
 *  Use this include in your application
 *  @code{.cpp} #include "msgCall.h" @endcode
 * @note User need to inherit MessageDispatcher and may override these methods
    to handle the dispached TLVs.
 * @attention Do @b NOT @p use this header in your application.
 */
class BaseMngDispatchCallback
{
  private:
    /** @cond internal */
    bool _nc; /**< flag indicating no inherite call back */
  protected:
    /* To improve backward compatibility
       Call virtual functions here in this inline function,
       so we compile it in the application
       and avoid using virtual function table in the library code */
    const char *iCallHandler(const Message &msg, mng_vals_e tlv_id,
        const BaseMngTlv *tlv) {
        const char *idStr = nullptr;
        _nc = false;
        switch(tlv_id) {
define(E,`            case $1: {
                idStr = "$1";
                auto d = dynamic_cast
                    <const $1_t *>(tlv);
                if(d != nullptr)
                    $1_h(msg, *d, idStr);
                break;
            }')dnl
define(A, `ifelse(regexp($6, `^UF', `0'),`0',`E($1)',`dnl')')dnl
include(ids_base.m4)dnl
undefine(`A')dnl
            default:
                break;
        }
        return idStr;
    }
    bool isNoTlv() { return _nc; }
    /**< @endcond */
  public:
    BaseMngDispatchCallback() = default;
    virtual ~BaseMngDispatchCallback() = default;
define(D,`    /**
     * Handle $1 management TLV
     * @param[in] msg referance to the Message object
     * @param[in] tlv referance to the management tlv
     * @param[in] idStr ID string of the management tlv
     */
    virtual void $1_h(const Message &msg,
        const $1_t &tlv, const char *idStr)
    {_nc = true;}')dnl
define(A, `ifelse(regexp($6, `^UF', `0'),`0',`D($1)',`dnl')')dnl
include(ids_base.m4)dnl
undefine(`A')dnl
};

/**
 * @brief Builder for a management TLV
 * @details
 *  Call-backs used by the MessageBuilder class.
 * @note this class is defined in @"msgCall.h@" header.
 *  Use this include in your application
 *  @code{.cpp} #include "msgCall.h" @endcode
 * @note Developers need to inherit MessageBuilder and
 *  may override these methods.
 *  Only override methods that return true are used.
 *  Developers may set the TLV values.
 * @attention Do @b NOT @p use this header in your application.
 */
class BaseMngBuildCallback
{
  protected:
    /** @cond internal */
    /* To improve backward compatibility
       Call virtual functions here in this inline function,
       so we compile it in the application
       and avoid using virtual function table in the library code */
    BaseMngTlv *iBuild(Message &msg, mng_vals_e tlv_id) {
        switch(tlv_id) {
define(F,`            case $1: {
                auto d = new $1_t;
                if(d != nullptr) {
                    if($1_b(msg, *d))
                        return d;
                    delete d;
                }
                break;
            }')dnl
define(A, `ifelse(regexp($6, `^UFB', `0'),`0',`F($1)',`dnl')')dnl
include(ids_base.m4)dnl
undefine(`A')dnl
            default:
                break;
        }
        return nullptr;
    }
    /**< @endcond */
  public:
    BaseMngBuildCallback() = default;
    virtual ~BaseMngBuildCallback() = default;
define(B,`    /**
     * Handle $1 management TLV
     * Set values in the new TLV
     * @param[in] msg referance to the Message object
     * @param[in, out] tlv referance to the new management tlv
     * @return true if set success
     * @note MessageBuilder::buildTlv call setAction with new TLV
     *  if an inherit method return true!
     */
    virtual bool $1_b(const Message &msg,
        $1_t &tlv)
    {return false;}')dnl
define(A, `ifelse(regexp($6, `^UFB', `0'),`0',`B($1)',`dnl')')dnl
include(ids_base.m4)dnl
};

__PTPMGMT_NAMESPACE_END
#else /* __cplusplus */
#error "this header is for C++ and scripts use only, NOT for C"
#endif /* __cplusplus */

#endif /* __PTPMGMT_CALL_DEFS_H */
