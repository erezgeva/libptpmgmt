#!/bin/bash -e
# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com>
#
# @author Erez Geva <ErezGeva2@@gmail.com>
# @copyright © 2023 Erez Geva
#
# script to build the libptpmgmt package on a gentoo system.
###############################################################################
cmd()
{
  echo "$@"
  "$@"
}
run_root()
{
  if [[ ! -f /var/cache/distfiles/libptpmgmt-${version}.txz ]]; then
    cp $base/libptpmgmt-${version}.txz /var/cache/distfiles/
  fi
  local -r d=/var/db/repos/gentoo/net-libs/libptpmgmt
  mkdir -p "$d"
  cd "$d"
  local -r s="libptpmgmt-${version}.ebuild"
  cp "$base/gentoo/libptpmgmt.ebuild" "$s"
  cmd ebuild "./$s" manifest clean unpack
  cmd ebuild "$s" clean test install
}
run_user()
{
  cp $base/libptpmgmt-${version}.txz /var/cache/distfiles/
  sudo "$me"
}
main()
{
  local -r me="$(realpath $0)"
  local -r base="$(realpath $(dirname $0)/..)"
  local -r ver="$base/tools/version"
  if [[ -f "$ver" ]]; then
    local ver_maj ver_min
    . "$ver"
    local -r version="$ver_maj.$ver_min"
    if [[ $(id -u) -eq 0 ]]; then
      run_root
    else
      run_user
    fi
  fi
}
if [[ -n "$(which ebuild)" ]]; then
  main
fi
