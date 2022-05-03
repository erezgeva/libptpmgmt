/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/* Library Swig header file
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

/* Module name */
#ifdef SWIGPERL
%module PtpMgmtLib  /* Perl only */
#else
%module ptpmgmt
#endif /* SWIGPERL */

/* Headers and namespace for moudle source code */
%{
    #include "sock.h"
    #include "json.h"
    #include "ver.h"
    #include "init.h"
    using namespace ptpmgmt;
%}

/* Handle multithreads support */
#ifdef SWIG_USE_MULTITHREADS
%nothread;
#define SWIG_THREAD_START %thread
#define SWIG_THREAD_END %nothread
#endif

/* Include standatd types and SWIG macroes */
%include "stdint.i"
%include "std_string.i"
%include "std_vector.i"
%include "cpointer.i"
/* The type is POSIX only, not standard! */
%apply long { ssize_t };

/*************************************************************************
 * Handle ignores and renames per script language.
 * Users of a script language should look here
 *  for the specific ignores and renames.
 * Each ignored operator overlaod have an alternative function
 ************************************************************************/
/*****
 * Ruby
 *********/
#ifdef SWIGRUBY
/* Ignore Wrong constant name.
 * Ruby capitalize first letter! */
%warnfilter(801) clockType_e;
%warnfilter(801) implementSpecific_e;
/* Operator overload ignored.
 * Scripts can use Binary::append() */
%warnfilter(365) Binary::operator+=;
#endif /* SWIGRUBY */
/*****
 * PHP
 *********/
#ifdef SWIGPHP
/* PHP rename to c_empty */
%warnfilter(314) Binary::empty;
/* PHP rename to c_list */
#define list(n) %warnfilter(314) n::list;
list(ACCEPTABLE_MASTER_TABLE_t)
list(SLAVE_RX_SYNC_TIMING_DATA_t)
list(SLAVE_RX_SYNC_COMPUTED_DATA_t)
list(SLAVE_TX_EVENT_TIMESTAMPS_t)
list(SLAVE_DELAY_TIMING_DATA_NP_t)
list(SLAVE_RX_SYNC_TIMING_DATA_t)
/* PHP rename c_interface */
%warnfilter(314) PORT_PROPERTIES_NP_t::interface;
/* Operator overload ignored.
 * Scripts can use Binary::append() */
%warnfilter(503) Binary::operator+=;
#define SWIG_OPERS_503
#endif /* SWIGPHP */
/*****
 * Tcl
 *********/
#ifdef SWIGTCL
/* Operator overload ignored.
 * Scripts can use Binary::append() */
%warnfilter(365) Binary::operator+=;
#define SWIG_OPERS_503
#endif /* SWIGTCL */
/*****
 * PHP and Tcl ignore operators overload
 *********/
#ifdef SWIG_OPERS_503
/* Operator overload ignored.
 * Scripts can use Buf::get() */
%warnfilter(503) Buf::operator();
/* Operator overload ignored.
 * Scripts can use class eq() method */
%warnfilter(503) Binary::operator==;
%warnfilter(503) ClockIdentity_t::operator==;
%warnfilter(503) PortIdentity_t::operator==;
%warnfilter(503) PortAddress_t::operator==;
/* Operator overload ignored.
 * Scripts can use class less() method */
%warnfilter(503) Binary::operator<;
%warnfilter(503) ClockIdentity_t::operator<;
%warnfilter(503) PortIdentity_t::operator<;
%warnfilter(503) PortAddress_t::operator<;
#endif /* SWIG_OPERS_503 */

/* Mark base sockets as non-abstract classes */
%feature("notabstract") SockBase;
%feature("notabstract") SockBaseIf;
/* library code */
%include "cfg.h"
%include "ptp.h"
%include "sock.h"
%include "bin.h"
%include "buf.h"
%include "types.h"
%include "mngIds.h" /* Add Management TLVs enumerator */
%include "proc.h"
%include "sig.h"
%include "msg.h"
%include "json.h"
%include "ver.h"
%include "opt.h"
%include "init.h"
/* Handle management vectors inside structures
 * See documenting of XXXX_v classes in mngIds.h and
 *  Doxygen generated documents
 */
#define mkVec(n) %template(n##_v) std::vector<n##_t>
mkVec(FaultRecord);
mkVec(ClockIdentity);
mkVec(PortAddress);
mkVec(AcceptableMaster);
/* Handle signalig vectors inside structures
 * See documenting of SigXXXX classes in mngIds.h and
 *  Doxygen generated documents
 */
#define mkRecVec(n, m) %template(n) std::vector<m##_rec_t>
mkRecVec(SigTime, SLAVE_RX_SYNC_TIMING_DATA);
mkRecVec(SigComp, SLAVE_RX_SYNC_COMPUTED_DATA);
mkRecVec(SigEvent, SLAVE_TX_EVENT_TIMESTAMPS);
mkRecVec(SigDelay, SLAVE_DELAY_TIMING_DATA_NP);
/* convert base management tlv to a specific management tlv structure
 * See documenting of conv_XXX functions in mngIds.h and
 *  Doxygen generated documents
 */
#define caseUF(n) %pointer_cast(BaseMngTlv*, n##_t*, conv_##n);
#define A(n, v, sc, a, sz, f) case##f(n)
%include "ids.h"
/* convert base signaling tlv to a specific signaling tlv structure
 * See documenting of conv_XXX functions in mngIds.h and
 *  Doxygen generated documents
 */
#define sigCnv(n) %pointer_cast(BaseSigTlv*, n##_t*, conv_##n);
sigCnv(ORGANIZATION_EXTENSION)
sigCnv(PATH_TRACE)
sigCnv(ALTERNATE_TIME_OFFSET_INDICATOR)
sigCnv(ENHANCED_ACCURACY_METRICS)
sigCnv(L1_SYNC)
sigCnv(PORT_COMMUNICATION_AVAILABILITY)
sigCnv(PROTOCOL_ADDRESS)
sigCnv(SLAVE_RX_SYNC_TIMING_DATA)
sigCnv(SLAVE_RX_SYNC_COMPUTED_DATA)
sigCnv(SLAVE_TX_EVENT_TIMESTAMPS)
sigCnv(CUMULATIVE_RATE_RATIO)
sigCnv(SLAVE_DELAY_TIMING_DATA_NP)
