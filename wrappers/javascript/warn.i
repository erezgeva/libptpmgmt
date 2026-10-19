/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com> */

/* Swig warning suppress for javascript
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2026 Erez Geva
 *
 */

/* See more warnings in libptpmgmt.i for SWIG_OPERS_xxxx */
#define SWIG_OPERS_LANG_IDENTIFIER
#define SWIG_OPERS_LANG_IDENTIFIER2

/*
 * Javascroipt do not support SWIG director.
 * We include an explicit Message call clases interface in ptpmgmt.js
 */
#define SWIG_USE_MSG_CALL

/* vim: set filetype=swig: */
