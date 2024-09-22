/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/* Swig warning suppress for Ruby
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

/* Ignore Wrong constant name.
 * Ruby capitalize first letter! */
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) clockType_e;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) implementSpecific_e;

/* clkmgr warnings */
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) clkmgr_event_state;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) clkmgr_event_count;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) EventIndex;
%warnfilter(SWIGWARN_RUBY_WRONG_NAME) ThresholdIndex;

/* See more warnings in libptpmgmt.i for SWIG_OPERS_xxxx */
#define SWIG_OPERS_OPERATOR_PLUSEQ
#define SWIG_OPERS_IGNORE_OPERATOR
