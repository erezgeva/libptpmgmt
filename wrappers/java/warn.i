/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com> */

/* Swig warning suppress for Java
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2026 Erez Geva
 *
 */

/* See more warnings in libptpmgmt.i for these SWIG_OPERS_xxxx */
#define SWIG_OPERS_LANG_IDENTIFIER

/* Java already have Error class */
%rename(PtpmError) Error;

/* Java rename to _interface */
%warnfilter(SWIGWARN_PARSE_KEYWORD) PORT_PROPERTIES_NP_t::interface;

/* Copy is ignored.
 * Scripts can use Binary::setBin() */
%warnfilter(SWIGWARN_LANG_IDENTIFIER) Binary::operator=;
/* Append is ignored.
 * Scripts can use Binary::append() */
%warnfilter(SWIGWARN_LANG_IDENTIFIER) Binary::operator+=;
/* Adding is ignored.
 * Scripts can use Timestamp_t::add() */
%warnfilter(SWIGWARN_LANG_IDENTIFIER) Timestamp_t::operator+=;
/* Subtract is ignored.
 * Scripts can use Timestamp_t::subt() */
%warnfilter(SWIGWARN_LANG_IDENTIFIER) Timestamp_t::operator-=;
