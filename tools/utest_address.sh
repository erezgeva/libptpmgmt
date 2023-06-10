#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# Run unit tests with Address Sanitizer
###############################################################################
main()
{
 cd "$(dirname "$(realpath "$0")")/.."
 [[ -f defs.mk ]] || make config
 make clean
 ASAN_OPTIONS='verbosity=1:strict_string_checks=1'
 ASAN_OPTIONS+=':detect_stack_use_after_return=1'
 ASAN_OPTIONS+=':check_initialization_order=1:strict_init_order=1'
 ASAN_OPTIONS+=':detect_invalid_pointer_pairs=2'
 export ASAN_OPTIONS
 make utest_no_sys utest_lua utest_tcl USE_ASAN=1 V=1
 # TODO add utest_json
}
main "$@"
