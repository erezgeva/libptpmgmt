#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2022 Erez Geva
#
# Create pkgIndex.tcl file for testing.
###############################################################################
main()
{
local base="$(dirname "$(realpath "$0")")"
local ver_maj ver_min
. $base/../../tools/version
if [[ -n "$1" ]]; then
  local -r file=$1/pkgIndex.tcl
  local -r pwds="$PWD $1"
else
  local -r file=pkgIndex.tcl
  local -r pwds="$PWD"
fi
local -r ver="$ver_maj.$ver_min"
local l txt
for l in ptpmgmt clkmgr utest_help
do txt+="package ifneeded $l $ver [list load [file join $pwds $l.so]]\n"
done
printf "$txt" > $file
}
main "$@"
