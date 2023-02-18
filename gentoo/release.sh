#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# Create release Ebuild for current version and official release from github.
###############################################################################
main()
{
  local ver_maj ver_min
  . ../version
  local -r version="$ver_maj.$ver_min"
  head -n -3 libptpmgmt.ebuild | sed 's/^# #!#//' > libptpmgmt-${version}.ebuild
}
if [[ -f libptpmgmt.ebuild ]] && [[ -f ../version ]]; then
  main
fi
