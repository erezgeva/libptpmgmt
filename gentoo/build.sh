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
  mkdir -p "/var/db/repos/gentoo/net-libs/libptpmgmt"
  cd "/var/db/repos/gentoo/net-libs/libptpmgmt"
  local -r s="libptpmgmt-${version}.ebuild"
  cp "$base/gentoo/libptpmgmt.ebuild" "$s"
  cp "$base/gentoo/metadata.xml" .
  # manifest    : create a manifest file for the package
  # clean       : clean up all source and temporary files
  # unpack      : unpack sources
  cmd ebuild "./$s" manifest clean unpack
  # configure   : configure sources
  # compile     : compile sources
  # test        : test package (unpack/configure/compile)
  # install     : install the package to the temporary install directory
  cmd ebuild "$s" clean test install
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
    if ! [[ -f /var/cache/distfiles/libptpmgmt-${version}.txz ]]; then
      cp $base/libptpmgmt-${version}.txz /var/cache/distfiles/
    fi
    if [[ $(id -u) -eq 0 ]]; then
      run_root
    else
      sudo "$me"
    fi
  fi
}
if [[ -n "$(which ebuild 2> /dev/null)" ]]; then
  main
fi
