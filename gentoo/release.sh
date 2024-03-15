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
  . ../tools/version
  local -r version="$ver_maj.$ver_min"
  sed '/^RESTRICT/ d;/^SRC_URI/ d;s/^#!#!#//' libptpmgmt.ebuild\
    > libptpmgmt-${version}.ebuild
}
cd "$(dirname "$(realpath "$0")")"
if [[ -f libptpmgmt.ebuild ]] && [[ -f ../tools/version ]]; then
  main
fi
