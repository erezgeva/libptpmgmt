/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/* Swig warning suppress for PHP
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

/* PHP already use Error */
%rename(c_error) Error;
/* PHP rename to 'c_empty' */
%warnfilter(SWIGWARN_PARSE_KEYWORD) Binary::empty;
/* PHP rename to 'c_list' */
#define _ptpmList(n) %warnfilter(SWIGWARN_PARSE_KEYWORD) n::list;
_ptpmList(ACCEPTABLE_MASTER_TABLE_t)
_ptpmList(SLAVE_RX_SYNC_TIMING_DATA_t)
_ptpmList(SLAVE_RX_SYNC_COMPUTED_DATA_t)
_ptpmList(SLAVE_TX_EVENT_TIMESTAMPS_t)
_ptpmList(SLAVE_DELAY_TIMING_DATA_NP_t)
_ptpmList(SLAVE_RX_SYNC_TIMING_DATA_t)
/* PHP rename to 'c_interface' */
%warnfilter(SWIGWARN_PARSE_KEYWORD) PORT_PROPERTIES_NP_t::interface;

/* See more warnings in libptpmgmt.i for SWIG_OPERS_xxxx */
#define SWIG_OPERS_LANG_IDENTIFIER
#define SWIG_OPERS_LANG_IDENTIFIER2
