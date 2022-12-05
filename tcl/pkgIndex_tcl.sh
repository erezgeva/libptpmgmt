#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright 2022 Erez Geva
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2022 Erez Geva
#
# Create pkgIndex.tcl file for testing.
###############################################################################
main()
{
if [[ -n "$1" ]]; then
  local -r file=$1/pkgIndex.tcl
else
  local -r file=pkgIndex.tcl
fi
if [[ -n "$2" ]]; then
  local -r ver_maj=$2
else
  local -r ver_maj=1
fi
cat << EOF > $file
package ifneeded ptpmgmt $ver_maj [list load [file join $PWD $1 ptpmgmt.so]]
EOF
}
main "$@"
