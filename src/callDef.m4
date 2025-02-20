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

ics(CALL_DEFS)

cpp_st()dnl
incb(msg)

ns_s()

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
cpp_cod(`class BaseMngDispatchCallback')dnl
cpp_cod(`{')dnl
cpp_cod(`    /** @cond internal */')dnl
cpp_cod(`  private:')dnl
cpp_cod(`    bool _nc; /**< flag indicating no inherite call back */')dnl
cpp_cod(`  protected:')dnl
cpp_cod(`    void _noTlvClear() { _nc = false; }')dnl
cpp_cod(`    bool _noTlv() {bool r = _nc; _nc = false; return r;}')dnl
cpp_cod(`    /**< @endcond */')dnl
cpp_cod(`  public:')dnl
cpp_cod(`    BaseMngDispatchCallback() = default;')dnl
cpp_cod(`    virtual ~BaseMngDispatchCallback() = default;')dnl
define(D,`    /**
     * Handle $1 management TLV
cpp_cod(`     * @param[in] msg ref_s() to the Message object')dnl
     * @param[in] tlv ref_s() to the management tlv
     * @param[in] idStr ID string of the management tlv
     */
cpp_cod(`    virtual void $1_h(const Message &msg,')dnl
cpp_cod(`        const $1_t &tlv, const char *idStr)')dnl
cpp_cod(`    {_nc = true;}')dnl')dnl
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
cpp_cod(`class BaseMngBuildCallback')dnl
cpp_cod(`{')dnl
cpp_cod(`  public:')dnl
cpp_cod(`    BaseMngBuildCallback() = default;')dnl
cpp_cod(`    virtual ~BaseMngBuildCallback() = default;')dnl
define(B,`    /**
     * Handle $1 management TLV
     * Set values in the new TLV
cpp_cod(`     * @param[in] msg ref_s() to the Message object')dnl
     * @param[in, out] tlv ref_s() to the new management tlv
     * @return true if set success
     * @note MessageBuilder::buildTlv call setAction with new TLV
     *  if an inherit method return true!
     */
cpp_cod(`    virtual bool $1_b(const Message &msg,')dnl
cpp_cod(`        $1_t &tlv)')dnl
cpp_cod(`    {return false;}')dnl')dnl
define(A, `ifelse(regexp($6, `^UFB', `0'),`0',`B($1)',`dnl')')dnl
include(ids_base.m4)dnl
};

ns_e()
cpp_en(callDef)dnl

ice(CALL_DEFS)
