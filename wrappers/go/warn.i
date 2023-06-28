/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com> */

/* Swig warning suppress for go
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2023 Erez Geva
 *
 */

/* Go rename to 'Xinterface' */
%warnfilter(SWIGWARN_PARSE_KEYWORD) PORT_PROPERTIES_NP_t::interface;

/* See more warnings in libptpmgmt.i for SWIG_OPERS_xxxx */
#define SWIG_OPERS_LANG_IDENTIFIER
#define SWIG_OPERS_LANG_IDENTIFIER2
