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
 cd $(dirname "$(realpath "$0")")
 [[ -f rpm/libptpmgmt.spec ]] || [[ -f archlinux/PKGBUILD.org ]] ||\
   return
 local -r email=$(git config --get user.email)
 local -r cur_ver=$(dpkg-parsechangelog --show-field Version)
 local -r maj_ver=${cur_ver%.*} min_ver=${cur_ver##*.}
 case $1 in
   -u)
     local -ri next_min_ver=min_ver
     local -ri next_maj_ver=maj_ver
     local -r update_only=true
     ;;
   -m)
     local -ri next_min_ver=0
     local -ri next_maj_ver=maj_ver+1
     ;;
   *)
     local -ri next_min_ver=min_ver+1
     local -ri next_maj_ver=maj_ver
     ;;
 esac
 local -r next_ver=$next_maj_ver.$next_min_ver
 sed -i -e "s/^\s*ver_min=.*/ver_min=$next_min_ver/"\
        -e "s/^\s*ver_maj=.*/ver_maj=$next_maj_ver/" version
 sed -i -e "s/^\(Version:\s*\).*/\1$next_ver/"\
        -e "s/\.so\.[0-9]*{/.so.$next_maj_ver{/" rpm/libptpmgmt.spec
 sed -i "s/^pkgver=.*/pkgver=$next_ver/" archlinux/PKGBUILD.org
 sed -i "s/ver_maj=[0-9][0-9]*/ver_maj=$next_maj_ver/" tcl/pkgIndex_tcl.sh
 if [[ "$update_only" != "true" ]]; then
   local -r hash=$(git blame debian/changelog  | head -n1 | sed 's/ .*//')
   local -a log
   mapfile -t log < <(git log $hash..HEAD | grep -v '^Author:.*' |\
     grep -v '^\s*$')
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
   gvim debian/changelog
   gitk &
 fi
}
main "$@"
