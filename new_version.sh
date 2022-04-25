#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright 2021 Erez Geva
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright 2021 Erez Geva
#
# Update to a new version
# Increase the minor version.
# Updating major version is done manually.
###############################################################################
main()
{
 local -r email=$(git config --get user.email)
 local -r cur_ver=$(dpkg-parsechangelog --show-field Version)
 local -r maj_ver=${cur_ver%.*} min_ver=${cur_ver##*.}
 local -ri next_min_ver=min_ver+1
 local -r next_ver=$maj_ver.$next_min_ver
 local -r hash=$(git blame debian/changelog  | head -n1 | sed 's/ .*//')
 local -a log
 mapfile -t log < <(git log $hash..HEAD | grep -v '^Author:.*' |\
  grep -v '^\s*$')
 sed -i "s/^\s*ver_min=.*/ver_min=$next_min_ver/" version
 local l add_line=false
 for l in "${log[@]}"; do
  if [[ $l =~ ^Date|^commit ]]; then
   if $add_line; then
    DEBEMAIL="$email" debchange -v $next_ver "==============="
   fi
   DEBEMAIL="$email" debchange -v $next_ver "** $l"
   add_line=false
  else
   DEBEMAIL="$email" debchange -v $next_ver "$l"
   add_line=true
  fi
 done
 DEBEMAIL="$email" debchange -r bullseye
 sed -i "s/^\(Version:\s*\).*/\1$next_ver/" rpm/libptpmgmt.spec
 sed -i "s/^pkgver=.*/pkgver=$next_ver/" archlinux/PKGBUILD.org
 gvim debian/changelog
 gitk &
}
main
