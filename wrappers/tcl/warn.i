/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/* Swig warning suppress for TCL
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

/* See more warnings in libptpmgmt.i for SWIG_OPERS_xxxx */
#define SWIG_OPERS_OPERATOR_PLUSEQ
#define SWIG_OPERS_LANG_IDENTIFIER

/*
 * TCL do not support SWIG director.
 * We include an explicit Message call clases interface
 */
#define SWIG_USE_MSG_CALL

/* vim: set filetype=swig: */
