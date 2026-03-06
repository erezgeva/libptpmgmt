#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2026 Intel Corporation.
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2026 Intel Corporation.
#
# Preperation builds for the simulation of the clock manager with clknetsim.
#
# build_linuxptp:  Build linuxptp
# build_clkmgr:    Build clock manager and libptpmgmt
###############################################################################
ecmd()
{
 echo "+$@"
 $@
}
###############################################################################
build_linuxptp()
{
 [[ -d "../linuxptp" ]] || return
 cd "../linuxptp"
 ecmd make
 ecmd ./ptp4l -v
}
###############################################################################
build_clkmgr()
{
 set -x
 make config
 make CXXFLAGS="$CXXFLAGS -U_FORTIFY_SOURCE"
 make -C clkmgr/sample
}
###############################################################################
main()
{
 local -r me1="$(realpath -s "$0")"
 local -r base="$(realpath "$(dirname "$0")/../..")"
 local -r dr1="$(dirname "$me1")"
 local -r me="${me1//$dr1\//}"
 local -r script="${me%\.sh}"
 cd "$base"
 local n m
 "$script" "$@"
}
main "$@"
