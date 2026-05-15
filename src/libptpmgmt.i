/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/* Library Swig header file
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

%module(directors="1") ptpmgmt

/* Headers and namespace for moudle source code */
%{
#include "sock.h"
#include "json.h"
#include "ver.h"
#include "init.h"
#include "timeCvrt.h"
#include "err.h"
using namespace ptpmgmt;
SWIGINTERNINLINE const BaseMngTlv* get_BaseMngTlv(const BaseSigTlv* x) {
  const MANAGEMENT_t *mng = dynamic_cast<const MANAGEMENT_t *>(x);
  return mng != nullptr ? mng->tlvData.get() : nullptr;
}
SWIGINTERNINLINE mng_vals_e get_MngTlvId(const BaseSigTlv* x) {
  const MANAGEMENT_t *mng = dynamic_cast<const MANAGEMENT_t *>(x);
  return mng != nullptr ? mng->managementId : NULL_PTP_MANAGEMENT;
}
%}

/* Unrecognized C++ keywords */
#define final
/* prevent C++ namespace in swig */
#define __PTPMGMT_NAMESPACE_BEGIN
#define __PTPMGMT_NAMESPACE_END
/* Handle obsolete functions */
#ifdef SWIGGO
/* go is used for development only, no need for obsolete functions */
#define __PTPMGMT_DEPRECATED(_func, _body)
#define __PTPMGMT_DEPRECATED_DEC(_func)
#else
#define __PTPMGMT_DEPRECATED(_func, _body) _func { _body; }
#define __PTPMGMT_DEPRECATED_DEC(_func) _func
#endif
/* Handle multithreads support. */
#ifdef SWIG_USE_MULTITHREADS
%nothread;
/* Mark functions,
 * that are allowed to block when using multithreads.
 * The other threads would not be blocked when using these functions.
 */
#define __PTPMGMT_SWIG_THREAD_START %thread
#define __PTPMGMT_SWIG_THREAD_END %nothread
#endif

/* Include standatd types and SWIG macros */
%include <stdint.i>
%include <std_string.i>
%include <std_vector.i>
%include <argcargv.i>
/* The type is POSIX only, not standard! */
%apply long { ssize_t };
/* SWIG does not support */
%apply double { long double };
%apply int { clockid_t };
/* Project float definitions */
%apply double { float_seconds };
%apply double { float_nanoseconds };
%apply double { float_freq };
/* Support Options::parse_options in scripts */
%apply (int ARGC, char **ARGV) {(int argc, char *const argv[])}

/*************************************************************************
 * Handle ignores and renames per script language.
 * Users of a script language should look here
 *  for the specific ignores and renames.
 * Each ignored operator overlaod have an alternative function
 * Warn codes defined in: swig/Source/Include/swigwarn.h
 *   SWIGWARN_PARSE_KEYWORD           314
 *   SWIGWARN_TYPE_UNDEFINED_CLASS    401
 *   SWIGWARN_PARSE_BUILTIN_NAME      321
 *   SWIGWARN_IGNORE_OPERATOR_EQ      362
 *   SWIGWARN_IGNORE_OPERATOR_PLUSEQ  365
 *   SWIGWARN_IGNORE_OPERATOR_MINUSEQ 366
 *   SWIGWARN_IGNORE_OPERATOR_INDEX   389
 *   SWIGWARN_LANG_IDENTIFIER         503
 *   SWIGWARN_LANG_OVERLOAD_SHADOW    509
 *   SWIGWARN_RUBY_WRONG_NAME         801
 ************************************************************************/
/*********
 * Generic
 *********/
/* Casting to string ignored.
 * Scripts can use Timestamp_t::toFloat() */
%warnfilter(SWIGWARN_LANG_IDENTIFIER) Timestamp_t::operator std::string;
/* Casting to double ignored.
 * Scripts can use Timestamp_t::string() */
%warnfilter(SWIGWARN_LANG_IDENTIFIER) Timestamp_t::operator float_seconds;

/* warning definitions per language */
%include "warn.i"

/*****
 * PHP, Tcl and Go ignore operators overload
 *********/
#ifdef SWIG_OPERS_LANG_IDENTIFIER
/* use Binary::getBin() and Binary::setBin() */
%warnfilter(SWIGWARN_LANG_IDENTIFIER) Binary::operator [];
/* Operator overload ignored.
 * Scripts can use Buf::get() */
%warnfilter(SWIGWARN_LANG_IDENTIFIER) Buf::operator();
/* Operator overload ignored.
 * Scripts can use class eq() method */
%warnfilter(SWIGWARN_LANG_IDENTIFIER) Binary::operator==;
%warnfilter(SWIGWARN_LANG_IDENTIFIER) ClockIdentity_t::operator==;
%warnfilter(SWIGWARN_LANG_IDENTIFIER) PortIdentity_t::operator==;
%warnfilter(SWIGWARN_LANG_IDENTIFIER) PortAddress_t::operator==;
%warnfilter(SWIGWARN_LANG_IDENTIFIER) Timestamp_t::operator==;
/* Operator overload ignored.
 * Scripts can use class less() method */
%warnfilter(SWIGWARN_LANG_IDENTIFIER) Binary::operator<;
%warnfilter(SWIGWARN_LANG_IDENTIFIER) ClockIdentity_t::operator<;
%warnfilter(SWIGWARN_LANG_IDENTIFIER) PortIdentity_t::operator<;
%warnfilter(SWIGWARN_LANG_IDENTIFIER) PortAddress_t::operator<;
%warnfilter(SWIGWARN_LANG_IDENTIFIER) Timestamp_t::operator<;
/* Operator overload ignored.
 * Scripts can use class add() method */
%warnfilter(SWIGWARN_LANG_IDENTIFIER) Timestamp_t::operator+;
/* Operator overload ignored.
 * Scripts can use class subt() method */
%warnfilter(SWIGWARN_LANG_IDENTIFIER) Timestamp_t::operator-;
#endif /* SWIG_OPERS_LANG_IDENTIFIER */

/*****
 * PHP and Go ignore operators overload
 *********/
#ifdef SWIG_OPERS_LANG_IDENTIFIER2
/* Operator overload ignored.
 * Scripts can use Binary::append() */
%warnfilter(SWIGWARN_LANG_IDENTIFIER) Binary::operator+=;
/* Operator overload ignored
   Scripts can use the copy constructor :-) */
%warnfilter(SWIGWARN_LANG_IDENTIFIER) Binary::operator=;
/* Operator overload ignored.
 * Scripts can use Timestamp_t::add() */
%warnfilter(SWIGWARN_LANG_IDENTIFIER) Timestamp_t::operator+=;
/* Operator overload ignored.
 * Scripts can use Timestamp_t::subt() */
%warnfilter(SWIGWARN_LANG_IDENTIFIER) Timestamp_t::operator-=;
#endif /* SWIG_OPERS_LANG_IDENTIFIER2 */

/*********
 * Tcl and Ruby ignore operator plus equal
 *********/
#ifdef SWIG_OPERS_OPERATOR_PLUSEQ
/* Operator overload ignored.
 * Scripts can use Binary::append() */
%warnfilter(SWIGWARN_IGNORE_OPERATOR_PLUSEQ) Binary::operator+=;
/* Operator overload ignored.
 * Scripts can use Timestamp_t::add() */
%warnfilter(SWIGWARN_IGNORE_OPERATOR_PLUSEQ) Timestamp_t::operator+=;
/* Operator overload ignored.
 * Scripts can use Timestamp_t::subt() */
%warnfilter(SWIGWARN_IGNORE_OPERATOR_MINUSEQ) Timestamp_t::operator-=;
#endif /* SWIG_OPERS_OPERATOR_PLUSEQ */

/*********
 * Perl, python and Ruby ignore operators overload
 *********/
#ifdef SWIG_OPERS_IGNORE_OPERATOR
/* use Binary::getBin() and Binary::setBin() */
%warnfilter(SWIGWARN_IGNORE_OPERATOR_INDEX) Binary::operator [];
/* Use the copy constructor :-) */
%warnfilter(SWIGWARN_IGNORE_OPERATOR_EQ) Binary::operator=;
#endif /* SWIG_OPERS_IGNORE_OPERATOR */

/* Support Message::traversSigTlvsCl() */
%feature("director") MessageSigTlvCallback;

/* library code */
%include "cfg.h"
%include "ptp.h"
%include "sock.h"
%include "bin.h"
%include "buf.h"
%include "types.h"
%include "mngIds.h"
%include "mngTlvs.h"
%include "sigTlvs.h"
%include "msg.h"
%include "json.h"
%include "ver.h"
%include "opt.h"
%include "init.h"
%include "err.h"
%include "timeCvrt.h"

/* Handle management vectors inside structures */
#define _ptpmMkVec(n) %template(n##_v) std::vector<n##_t>
_ptpmMkVec(FaultRecord);
_ptpmMkVec(ClockIdentity);
_ptpmMkVec(PortAddress);
_ptpmMkVec(AcceptableMaster);
_ptpmMkVec(LinuxptpUnicastMaster);
_ptpmMkVec(PtpEvent);
_ptpmMkVec(PtpSample);
_ptpmMkVec(PtpSampleExt);
/* Handle signalig vectors inside structures */
#define _ptpmMkRecVec(n, m) %template(n) std::vector<m##_rec_t>
_ptpmMkRecVec(SigTime, SLAVE_RX_SYNC_TIMING_DATA);
_ptpmMkRecVec(SigComp, SLAVE_RX_SYNC_COMPUTED_DATA);
_ptpmMkRecVec(SigEvent, SLAVE_TX_EVENT_TIMESTAMPS);
_ptpmMkRecVec(SigDelay, SLAVE_DELAY_TIMING_DATA_NP);

%define %pointer_dynamic_cast(TYPE1,TYPE2,NAME)
%{SWIGINTERNINLINE TYPE2* NAME(TYPE1* x){return dynamic_cast<TYPE2*>(x);}%}
TYPE2* NAME(TYPE1* x);
%enddef

/* convert base management tlv to a specific management tlv structure
 * See documenting of conv_XXX functions in cnvFunc.h and
 *  Doxygen generated documents
 */
#define _ptpmCaseUF(n) %pointer_dynamic_cast(BaseMngTlv, n##_t, conv_##n);
#define A(n, v, sc, a, sz, f) _ptpmCase##f(n)
%include "ids.h"
/* convert base signalling tlv to a specific signalling tlv structure
 * See documenting of conv_XXX functions in cnvFunc.h and
 *  Doxygen generated documents
 */
#define _ptpmSigCnv(n) %pointer_dynamic_cast(BaseSigTlv, n##_t, conv_##n);
_ptpmSigCnv(ORGANIZATION_EXTENSION)
_ptpmSigCnv(PATH_TRACE)
_ptpmSigCnv(ALTERNATE_TIME_OFFSET_INDICATOR)
_ptpmSigCnv(ENHANCED_ACCURACY_METRICS)
_ptpmSigCnv(L1_SYNC)
_ptpmSigCnv(PORT_COMMUNICATION_AVAILABILITY)
_ptpmSigCnv(PROTOCOL_ADDRESS)
_ptpmSigCnv(SLAVE_RX_SYNC_TIMING_DATA)
_ptpmSigCnv(SLAVE_RX_SYNC_COMPUTED_DATA)
_ptpmSigCnv(SLAVE_TX_EVENT_TIMESTAMPS)
_ptpmSigCnv(CUMULATIVE_RATE_RATIO)
_ptpmSigCnv(SLAVE_DELAY_TIMING_DATA_NP)
/* Convert signaling TLV to management TLV and ID */
const BaseMngTlv* get_BaseMngTlv(const BaseSigTlv* x);
mng_vals_e get_MngTlvId(const BaseSigTlv* x);

#if defined SWIGTCL
/* Used in wrappers/tcl/msgCall.i on TCL 9 we fail to use GET directly.
   Scripts does use GET properly! */
%{SWIGINTERNINLINE actionField_e GET_VAL(){return GET;}%}
actionField_e GET_VAL();
#endif

#if defined SWIGLUA || defined SWIGTCL
/* MessageDispatcher and MessageBuilder classes per language */
%include "msgCall.i"
#else
/* Use SWIG director */
%feature("director") BaseMngDispatchCallback;
%feature("director") BaseMngBuildCallback;
%feature("director") MessageDispatcher;
%feature("director") MessageBuilder;
%{
#include "msgCall.h"
%}
%include "callDef.h"
%include "msgCall.h"
#endif
