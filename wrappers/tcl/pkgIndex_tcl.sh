#!/bin/bash
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
else
  local -r file=pkgIndex.tcl
fi
local -r ver="$ver_maj.$ver_min"
cat << EOF > $file
package ifneeded ptpmgmt $ver [list load [file join $PWD $1 ptpmgmt.so]]
package ifneeded clkmgr $ver [list load [file join $PWD $1 clkmgr.so]]
EOF
}
main "$@"
